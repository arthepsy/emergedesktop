// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
//
//  Emerge Desktop is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  Emerge Desktop is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//----  --------------------------------------------------------------------------------------------------------

#include "Core.h"
#include <wtsapi32.h>

typedef void (__stdcall *lpfnMSSwitchToThisWindow)(HWND, BOOL);
typedef BOOL (__stdcall *lpfnWTSRegisterSessionNotification)(HWND, DWORD);
typedef BOOL (__stdcall *lpfnWTSUnRegisterSessionNotification)(HWND);

WCHAR emergeCoreClass[ ] = TEXT("EmergeDesktopCore");

Core::Core(HINSTANCE hInstance)
{
  mainInst = hInstance;
  xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += TEXT("emergeCore.xml");
  registered = false;
}

bool Core::Initialize(WCHAR *commandLine)
{
  WNDCLASSEX wincl;

  _wcslwr(commandLine);

  if (!ELSetEmergeVars())
    {
      ELMessageBox(GetDesktopWindow(), TEXT("Failed to initialize Environment variables."),
                   TEXT("emergeCore"), ELMB_ICONERROR|ELMB_MODAL|ELMB_OK);
      return false;
    }

  // Initialize Settings
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  pSettings->ReadSettings();

  if (wcsstr(commandLine, TEXT("/shellchanger")) != 0)
    {
      pShellChanger = std::tr1::shared_ptr<ShellChanger>(new ShellChanger(mainInst, NULL, pSettings));
      pShellChanger->Show();

      return false;
    }

  //OleInitialize(NULL);
  CoInitialize(NULL);

  // Start the shell functions
  pShell = std::tr1::shared_ptr<Shell>(new Shell());

  // Start the DDE Service
  //pDDEService = std::tr1::shared_ptr<DDEService>(new DDEService());
  //pDDEService->Start();

  // Register the window class
  wincl.hInstance = mainInst;
  wincl.lpszClassName = emergeCoreClass;
  wincl.lpfnWndProc = CoreProcedure;
  wincl.style = 0;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.lpszMenuName = NULL;
  wincl.cbClsExtra = 0;
  wincl.cbWndExtra = 0;
  wincl.hbrBackground = NULL;

  // Register the window class, and if it fails quit the program
  if (!RegisterClassEx (&wincl))
    return false;

  // The class is registered, let's create the window
  mainWnd = CreateWindowEx (
              WS_EX_TOOLWINDOW,
              emergeCoreClass,
              NULL,
              WS_POPUP,
              0, 0,
              0, 0,
              NULL,
              NULL,
              mainInst,
              reinterpret_cast<LPVOID>(this)
            );

  // If the window failed to get created, unregister the class and quit the program
  if (!mainWnd)
    {
      ELMessageBox(GetDesktopWindow(),
                   (WCHAR*)TEXT("Failed to create core window"),
                   (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }

  registered = true;

  pMessageControl = std::tr1::shared_ptr<MessageControl>(new MessageControl());

  // Create desktop window
  pDesktop = std::tr1::shared_ptr<Desktop>(new Desktop(mainInst, pMessageControl));
  pDesktop->Initialize();

  // Launch additional Emerge Desktop applets
  RunLaunchItems();

  pShell->RegisterShell(mainWnd, true);
  pShell->BuildTaskList();

  // Load the start up entries in the registry and the startup
  // folders only if the startup items have not already been started
  if (pShell->FirstRunCheck())
    {
      if (!ELIsKeyDown(VK_SHIFT))
        pShell->RunFolderStartup(pSettings->GetShowStartupErrors());

      if (!ELIsKeyDown(VK_CONTROL))
        pShell->RunRegStartup(pSettings->GetShowStartupErrors());
    }

  pMessageControl->AddType(mainWnd, EMERGE_CORE);

  pLaunchEditor = std::tr1::shared_ptr<LaunchEditor>(new LaunchEditor(mainInst, mainWnd));
  pShellChanger = std::tr1::shared_ptr<ShellChanger>(new ShellChanger(mainInst, mainWnd, pSettings));
  pThemeSelector = std::tr1::shared_ptr<ThemeSelector>(new ThemeSelector(mainInst, mainWnd));

  HMODULE wtslib = NULL;
  wtslib = ELLoadSystemLibrary(TEXT("wtsapi32.dll"));
  if (wtslib)
    {
      lpfnWTSRegisterSessionNotification wtsrsn = (lpfnWTSRegisterSessionNotification)
          GetProcAddress(wtslib, "WTSRegisterSessionNotification");
      if (wtsrsn)
        wtsrsn(mainWnd, NOTIFY_FOR_THIS_SESSION);
      FreeLibrary(wtslib);
    }

  return true;
}

Core::~Core()
{
  if (registered)
    {
      HMODULE wtslib = NULL;
      wtslib = ELLoadSystemLibrary(TEXT("wtsapi32.dll"));
      if (wtslib)
        {
          lpfnWTSUnRegisterSessionNotification wtsursn = (lpfnWTSUnRegisterSessionNotification)
              GetProcAddress(wtslib, "WTSUnRegisterSessionNotification");
          if (wtsursn)
            wtsursn(mainWnd);
          FreeLibrary(wtslib);
        }
      //pDDEService->Stop();
      pShell->RegisterShell(mainWnd, false);
      pShell->ClearSessionInformation();

      OleUninitialize();

      ELClearEmergeVars();

      // Unregister the window class
      UnregisterClass(emergeCoreClass, mainInst);
    }
}

bool Core::RunLaunchItems()
{
  bool found = false, writeXML = false;
  WCHAR path[MAX_PATH], data[MAX_LINE_LENGTH], installDir[MAX_PATH];
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *first, *sibling, *section;
  std::wstring theme = ELToLower(ELGetThemeName()), defaultTheme = TEXT("default");

  if (theme != defaultTheme)
    {
      configXML = ELOpenXMLConfig(xmlFile, false);
      if (configXML)
        {
          section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Launch"), false);

          if (section)
            {
              first = ELGetFirstXMLElement(section);
              if (first)
                {
                  if (ELReadXMLStringValue(first, TEXT("Command"), data, TEXT("")))
                    {
                      found = true;
                      if (ELStringReplace(data, TEXT("emergeDesktop"), TEXT("emergeWorkspace"), true) != 0)
                        {
                          writeXML = true;
                          ELWriteXMLStringValue(first, TEXT("Command"), data);
                        }
                      ELExecute(data);
                    }

                  sibling = ELGetSiblingXMLElement(first);
                  while (sibling)
                    {
                      first = sibling;

                      if (ELReadXMLStringValue(first, TEXT("Command"), data, TEXT("")))
                        {
                          if (ELStringReplace(data, TEXT("emergeDesktop"), TEXT("emergeWorkspace"), true) != 0)
                            {
                              writeXML = true;
                              ELWriteXMLStringValue(first, TEXT("Command"), data);
                            }
                          ELExecute(data);
                        }

                      sibling = ELGetSiblingXMLElement(first);
                    }
                }
            }

          if (writeXML)
            ELWriteXMLConfig(configXML.get());
        }
    }

  if (!found)
    {
      ELGetCurrentPath(installDir);
      wcscat(installDir, TEXT("\\"));

      wcscpy(path, installDir);
      wcscat(path, TEXT("emergeTasks.exe"));
      if (ELPathFileExists(path))
        ELExecute(path);

      wcscpy(path, installDir);
      wcscat(path, TEXT("emergeTray.exe"));
      if (ELPathFileExists(path))
        ELExecute(path);

      wcscpy(path, installDir);
      wcscat(path, TEXT("emergeWorkspace.exe"));
      if (ELPathFileExists(path))
        ELExecute(path);

      wcscpy(path, installDir);
      wcscat(path, TEXT("emergeCommand.exe"));
      if (ELPathFileExists(path))
        ELExecute(path);

      wcscpy(path, installDir);
      wcscat(path, TEXT("emergeLauncher.exe"));
      if (ELPathFileExists(path))
        ELExecute(path);

      wcscpy(path, installDir);
      wcscat(path, TEXT("emergeHotkeys.exe"));
      if (ELPathFileExists(path))
        ELExecute(path);
    }

  return found;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	CoreProcedure
// Required:	HWND hwnd - window handle that message was sent to
// 		UINT message - action to handle
// 		WPARAM wParam - dependant on message
// 		LPARAM lParam - dependant on message
// Returns:	LRESULT
// Purpose:	Handles messages sent from DispatchMessage
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Core::CoreProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  COPYDATASTRUCT *cpData;
  CREATESTRUCT *cs;
  static Core *pCore = NULL;

  if (message == WM_CREATE)
    {
      cs = (CREATESTRUCT*)lParam;
      pCore = reinterpret_cast<Core*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pCore == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
    case WM_COPYDATA:
      cpData = (COPYDATASTRUCT *)lParam;
      if (cpData->dwData == EMERGE_MESSAGE)
        return pCore->DoCopyData(cpData);
      break;

    case WM_SYSCOMMAND:
      switch (wParam)
        {
        case SC_CLOSE:
        case SC_MAXIMIZE:
        case SC_MINIMIZE:
          break;
        default:
          return DefWindowProc(hwnd, message, wParam, lParam);
        }
      break;

    case WM_WTSSESSION_CHANGE:
      return pCore->DoWTSSessionChange((UINT)wParam);

    case WM_DESTROY:
    case WM_NCDESTROY:
      // PostQuitMessage(1); - use with SetShellWindow
      PostQuitMessage(0);
      break;

      // If not handled just forward the message on
    default:
      return pCore->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

LRESULT Core::DoCopyData(COPYDATASTRUCT *cds)
{
  std::wstring theme = reinterpret_cast<WCHAR*>(cds->lpData);

  if (cds->dwData == EMERGE_MESSAGE)
    {
      SetEnvironmentVariable(TEXT("ThemeDir"), theme.c_str());
      return 1;
    }

  return 0;
}

LRESULT Core::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == EMERGE_REGISTER)
    {
      pMessageControl->AddType((HWND)wParam, (UINT)lParam);
      return 0;
    }

  if (message ==  EMERGE_UNREGISTER)
    {
      pMessageControl->RemoveType((HWND)wParam, (UINT)lParam);
      return 0;
    }

  if (message ==  EMERGE_DISPATCH)
    {
      pMessageControl->DispatchMessage((UINT)wParam, (UINT)lParam);
      return 0;
    }

  if ((message ==  EMERGE_NOTIFY) && (wParam == EMERGE_CORE))
    {
      switch (lParam)
        {
        case CORE_LAUNCH:
          pLaunchEditor->Show();
          break;

        case CORE_SHELL:
          if (pShellChanger->Show() == IDOK)
            pSettings->ReadSettings();
          break;

        case CORE_THEMESELECT:
          if (pThemeSelector->Show() == IDOK)
            {
              CheckLaunchList();
              pMessageControl->DispatchMessage(EMERGE_CORE, CORE_RECONFIGURE);
            }
          break;

        case CORE_WRITESETTINGS:
          BuildLaunchList();
          break;

        case CORE_RUN:
          ELRun();
          break;

        case CORE_SHUTDOWN:
          ELShutdown(hwnd);
          break;

        case CORE_EMPTYBIN:
          SHEmptyRecycleBin(NULL, NULL, 0);
          break;

        case CORE_LOGOFF:
          ELExit(EWX_LOGOFF, true);
          break;

        case CORE_REBOOT:
          ELExit(EWX_REBOOT, true);
          break;

        case CORE_HALT:
          ELExit(EWX_POWEROFF, true);
          break;

        case CORE_SUSPEND:
          ELExit(EMERGE_SUSPEND, true);
          break;

        case CORE_HIBERNATE:
          ELExit(EMERGE_HIBERNATE, true);
          break;

        case CORE_DISCONNECT:
          ELExit(EMERGE_DISCONNECT, true);
          break;

        case CORE_DESKTOP:
          pDesktop->ToggleDesktop();
          pMessageControl->DispatchMessage(EMERGE_CORE, CORE_REPOSITION);
          break;

        case CORE_ABOUT:
          About();
        }

      return 0;
    }

  // If not handled just forward the message on
  if (!pShell->UpdateTaskCount(message, (UINT)wParam, (HWND)lParam))
    return DefWindowProc(hwnd, message, wParam, lParam);

  return 0;
}

LRESULT Core::DoWTSSessionChange(UINT message)
{
  switch (message)
    {
    case WTS_SESSION_LOGON:
    case WTS_SESSION_UNLOCK:
      ELPlaySound(TEXT("WindowsLogon"));
      break;
    case WTS_SESSION_LOGOFF:
    case WTS_SESSION_LOCK:
      ELPlaySound(TEXT("WindowsLogoff"));
      break;
    }

  return 0;
}

void Core::About()
{
  WCHAR tmp[MAX_LINE_LENGTH];
  VERSIONINFO coreInfo, libInfo, graphicsInfo, styleInfo, baseInfo, engineInfo;

  ELAppletFileVersion((WCHAR*)TEXT("emergeLib.dll"), &libInfo);
  ELAppletFileVersion((WCHAR*)TEXT("emergeGraphics.dll"), &graphicsInfo);
  ELAppletFileVersion((WCHAR*)TEXT("emergeStyleEngine.dll"), &styleInfo);
  ELAppletFileVersion((WCHAR*)TEXT("emergeBaseClasses.dll"), &baseInfo);
  ELAppletFileVersion((WCHAR*)TEXT("emergeAppletEngine.dll"), &engineInfo);

  if (ELAppletVersionInfo(mainWnd, &coreInfo))
    {
      swprintf(tmp, TEXT("%s\n\nVersion:\t\t\t%s\n\nemergeLib:\t\t%s\nemergeGraphics:\t%s\nemergeStyleEngine:\t%s\n")
               TEXT("emergeBaseClasses:\t%s\nemergeAppletEngine:\t%s\n\nAuthor: %s"),
               coreInfo.Description,
               coreInfo.Version,
               libInfo.Version,
               graphicsInfo.Version,
               styleInfo.Version,
               baseInfo.Version,
               engineInfo.Version,
               coreInfo.Author);

      ELMessageBox(GetDesktopWindow(), tmp, (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONQUESTION|ELMB_MODAL);
    }
}

bool Core::BuildLaunchList()
{
  LaunchMap launchMap;
  LaunchMap::iterator iter;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section, *item;
  WCHAR program[MAX_PATH], arguments[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH];
  bool found;
  std::wstring theme = ELToLower(ELGetThemeName()), defaultTheme = TEXT("default");

  if (theme == defaultTheme)
    return false;

  EnumWindows(LaunchMapEnum, (LPARAM)&launchMap);

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Launch"), true);

      if (section)
        {
          while (!launchMap.empty())
            {
              found = true;
              iter = launchMap.begin();
              ELParseCommand((WCHAR*)iter->first.c_str(), program, arguments);
              swprintf(command, TEXT("%s %s"), PathFindFileName(program), arguments);
              item = ELSetFirstXMLElement(section, TEXT("item"));
              if (item)
                ELWriteXMLStringValue(item, TEXT("Command"), command);
              launchMap.erase(iter);
            }

          if (found)
            ELWriteXMLConfig(configXML.get());
        }
    }

  return found;
}

bool Core::CheckLaunchList()
{
  LaunchMap launchMap;
  LaunchMap::iterator iter;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *first, *tmp, *section;
  WCHAR data[MAX_LINE_LENGTH];
  bool found = false, writeXML = false;

  EnumWindows(LaunchMapEnum, (LPARAM)&launchMap);

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Launch"), false);

      if (section)
        {
          first = ELGetFirstXMLElement(section);

          while (first)
            {
              if (ELReadXMLStringValue(first, TEXT("Command"), data, TEXT("")))
                {
                  found = true;
                  if (ELStringReplace(data, TEXT("emergeDesktop"), TEXT("emergeWorkspace"), true) != 0)
                    {
                      writeXML = true;
                      ELWriteXMLStringValue(first, TEXT("Command"), data);
                    }
                  CheckLaunchItem(&launchMap, data);
                }

              tmp = first;
              first = ELGetSiblingXMLElement(tmp);
            }
        }

      if (writeXML)
        ELWriteXMLConfig(configXML.get());
    }

  if (!found)
    {
      CheckLaunchItem(&launchMap, TEXT("emergeTasks.exe"));
      CheckLaunchItem(&launchMap, TEXT("emergeTray.exe"));
      CheckLaunchItem(&launchMap, TEXT("emergeWorkspace.exe"));
      CheckLaunchItem(&launchMap, TEXT("emergeCommand.exe"));
      CheckLaunchItem(&launchMap, TEXT("emergeLauncher.exe"));
      CheckLaunchItem(&launchMap, TEXT("emergeHotkeys.exe"));
    }

  while (!launchMap.empty())
    {
      iter = launchMap.begin();
      PostMessage(iter->second, WM_NCDESTROY, 0, 0);
      launchMap.erase(iter);
    }

  pMessageControl->DispatchMessage(EMERGE_CORE, CORE_RECONFIGURE);

  return true;
}

void Core::CheckLaunchItem(LaunchMap *launchMap, const WCHAR *item)
{
  LaunchMap::iterator iter;
  std::wstring program = TEXT("%AppletDir%\\");
  WCHAR *workingItem = _wcsdup(item);

  if (ELPathIsRelative(workingItem))
    program += workingItem;
  else
    program = workingItem;

  program = ELExpandVars(program);
  program = ELToLower(program);

  iter = launchMap->find(program);
  if (iter == launchMap->end())
    ELExecute((WCHAR*)program.c_str());
  else
    launchMap->erase(program.c_str());

  free(workingItem);
}

BOOL Core::LaunchMapEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR windowClass[MAX_LINE_LENGTH], windowName[MAX_LINE_LENGTH];
  std::wstring workingName;

  if (RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH) != 0)
    {
      if ((_wcsicmp(windowClass, TEXT("EmergeDesktopApplet")) == 0) ||
          (_wcsicmp(windowClass, TEXT("EmergeDesktopMenuBuilder")) == 0))
        {
          ELGetWindowApp(hwnd, windowName, true);
          _wcslwr(windowName);
          workingName = windowName;
          ((LaunchMap*)lParam)->insert(std::pair<std::wstring, HWND>(workingName, hwnd));
        }
    }

  return TRUE;
}
