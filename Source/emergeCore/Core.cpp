// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

Core::Core(HINSTANCE hInstance)
{
  mainInst = hInstance;
  mainWnd = NULL;
  xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += TEXT("emergeCore.xml");
  registered = false;
  explorerThread = NULL;
}

bool Core::Initialize(WCHAR* commandLine)
{
  WNDCLASSEX wincl;

  _wcslwr(commandLine);

  // Set the critical environment variables
  if (!ELSetEmergeVars())
  {
    ELMessageBox(GetDesktopWindow(), TEXT("Failed to initialize Environment variables."),
                 TEXT("emergeCore"), ELMB_ICONERROR | ELMB_MODAL | ELMB_OK);
    return false;
  }

  // Initialize Settings
  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  pSettings->Init(mainWnd, TEXT("emergeCore"), 0);
  pSettings->ReadSettings();
  pSettings->ReadUserSettings();

  if (wcsstr(commandLine, TEXT("/shellchanger")) != 0)
  {
    pShellChanger = std::tr1::shared_ptr<ShellChanger>(new ShellChanger(mainInst, NULL, pSettings));
    pShellChanger->Show();

    return false;
  }

  OleInitialize(NULL);

  // Set the non-critical environment variables
  ELSetEnvironmentVars(pSettings->GetShowStartupErrors());

  // Start the shell functions
  pShell = std::tr1::shared_ptr<Shell>(new Shell());

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
  {
    return false;
  }

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
                 TEXT("Failed to create core window"),
                 TEXT("emergeCore"),
                 ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    return false;
  }

  registered = true;

  pMessageControl = std::tr1::shared_ptr<MessageControl>(new MessageControl());
  pMessageControl->AddType(mainWnd, EMERGE_CORE);

  // Create desktop window
  pDesktop = std::tr1::shared_ptr<Desktop>(new Desktop(mainInst, pMessageControl));
  pDesktop->Initialize(pSettings->GetShowExplorerDesktop());

  StartExplorer();
  EnableExplorerDesktop();

  // Hide Explorer's taskbar
  pShell->HideExplorerBar();

  // Launch additional Emerge Desktop applets
  ConvertTheme();
  RunLaunchItems();

  pShell->RegisterShell(mainWnd, true);
  pShell->BuildTaskList();

  // Load the start up entries in the registry and the startup
  // folders only if the startup items have not already been started
  // and explorer.exe is not running as the shell
  if ((wcsstr(commandLine, TEXT("/nostartup")) == 0) &&
      pShell->FirstRunCheck() && !ELIsExplorerShell())
  {
    if (!ELIsKeyDown(VK_SHIFT))
    {
      pShell->RunFolderStartup(pSettings->GetShowStartupErrors());
    }

    if (!ELIsKeyDown(VK_CONTROL))
    {
      pShell->RunRegStartup(pSettings->GetShowStartupErrors());
    }
  }

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
    {
      wtsrsn(mainWnd, NOTIFY_FOR_THIS_SESSION);
    }
    FreeLibrary(wtslib);
  }

  if (pSettings->GetShowWelcome())
  {
    ShowWelcome();
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
      {
        wtsursn(mainWnd);
      }
      FreeLibrary(wtslib);
    }

    /**< Only unload SSO objects if not running on top of Explorer */
    if (!ELIsExplorerShell() && (ELOSVersionInfo() > 6.0))
    {
      pShell->UnloadSSO();
    }

    pShell->RegisterShell(mainWnd, false);
    pShell->ClearSessionInformation();

    if (!ELIsEmergeShell()) // Running on top of Explorer; show the Taskbar before exiting.
      //Do this after pShell unloads so any HSHELL_RUDEAPPACTIVATED messages won't re-hide the Taskbar.
    {
      //get the Taskbar window
      HWND taskBarWnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);

      //get the start button window
      HWND startWnd = FindWindow(TEXT("Button"), NULL);

      if (taskBarWnd)
      {
        ShowWindow(taskBarWnd, SW_SHOW);
      }
      if (startWnd)
      {
        ShowWindow(startWnd, SW_SHOW);
      }
    }

    OleUninitialize();

    ELClearEmergeVars();

    // Unregister the window class
    UnregisterClass(emergeCoreClass, mainInst);
  }
}

bool Core::RunLaunchItems()
{
  bool found = false;
  std::wstring data, path, installDir;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* first, *sibling, *section = NULL, *settings;
  std::wstring theme = ELToLower(ELGetThemeName()), defaultTheme = TEXT("default");

  if (theme != defaultTheme)
  {
    configXML = ELOpenXMLConfig(xmlFile, false);
    if (configXML)
    {
      settings = ELGetXMLSection(configXML.get(), TEXT("Settings"), false);
      if (settings)
      {
        section = ELGetFirstXMLElementByName(settings, TEXT("Launch"), false);
      }
      if (section)
      {
        first = ELGetFirstXMLElement(section);
        if (first)
        {
          data = ELReadXMLStringValue(first, TEXT("Command"), TEXT(""));
          if (!data.empty())
          {
            found = true;
            ELExecuteFileOrCommand(data);
          }

          sibling = ELGetSiblingXMLElement(first);
          while (sibling)
          {
            first = sibling;

            data = ELReadXMLStringValue(first, TEXT("Command"), TEXT(""));
            if (!data.empty())
            {
              ELExecuteFileOrCommand(data);
            }

            sibling = ELGetSiblingXMLElement(first);
          }
        }
      }
    }
  }

  if (!found)
  {
    installDir = ELGetCurrentPath();
    installDir = installDir + TEXT("\\");

    path = installDir;
    path = path + TEXT("emergeTasks.exe");
    if (ELFileExists(path))
    {
      ELExecuteFileOrCommand(path);
    }

    path = installDir;
    path = path + TEXT("emergeTray.exe");
    if (ELFileExists(path))
    {
      ELExecuteFileOrCommand(path);
    }

    path = installDir;
    path = path + TEXT("emergeWorkspace.exe");
    if (ELFileExists(path))
    {
      ELExecuteFileOrCommand(path);
    }

    path = installDir;
    path = path + TEXT("emergeCommand.exe");
    if (ELFileExists(path))
    {
      ELExecuteFileOrCommand(path);
    }

    path = installDir;
    path = path + TEXT("emergeLauncher.exe");
    if (ELFileExists(path))
    {
      ELExecuteFileOrCommand(path);
    }

    path = installDir;
    path = path + TEXT("emergeHotkeys.exe");
    if (ELFileExists(path))
    {
      ELExecuteFileOrCommand(path);
    }
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
  CREATESTRUCT* cs;
  static Core* pCore = NULL;

  if (message == WM_CREATE)
  {
    cs = (CREATESTRUCT*)lParam;
    pCore = reinterpret_cast<Core*>(cs->lpCreateParams);
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  if (pCore == NULL)
  {
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  switch (message)
  {
  case WM_COPYDATA:
    return pCore->DoCopyData((COPYDATASTRUCT*)lParam);

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

LRESULT Core::DoCopyData(COPYDATASTRUCT* cds)
{
  if (cds->dwData == EMERGE_MESSAGE)
  {
    std::wstring theme = reinterpret_cast<WCHAR*>(cds->lpData);
    SetEnvironmentVariable(TEXT("ThemeDir"), theme.c_str());
    return 1;
  }

  if ((cds->dwData == EMERGE_DISPATCH) && (cds->cbData == sizeof(NOTIFYINFO)))
  {
    LPNOTIFYINFO notifyInfo = reinterpret_cast<LPNOTIFYINFO>(cds->lpData);
    pMessageControl->Dispatch_Message(notifyInfo->Type, notifyInfo->Message, notifyInfo->InstanceName);

    return 1;
  }

  if ((cds->dwData == EMERGE_NOTIFY) && (cds->cbData == sizeof(NOTIFYINFO)))
  {
    LPNOTIFYINFO notifyInfo = reinterpret_cast<LPNOTIFYINFO>(cds->lpData);

    if ((notifyInfo->Type & EMERGE_CORE) == EMERGE_CORE)
    {
      switch (notifyInfo->Message)
      {
      case CORE_SHELL:
        if (pShellChanger->Show() == IDOK)
        {
          pSettings->ReadUserSettings();
        }
        break;

      case CORE_THEMESELECT:
        if (pThemeSelector->Show() == IDOK)
        {
          ConvertTheme();
          pSettings->ReadSettings();
          // Check existing applets and run any additional applets
          CheckLaunchList();
          // Tell the existing applets to reconfigure
          pMessageControl->Dispatch_Message(EMERGE_CORE, CORE_RECONFIGURE, NULL);

          EnableExplorerDesktop();
        }
        break;

      case CORE_WRITESETTINGS:
        BuildLaunchList();
        break;

      case CORE_RUN:
        ELDisplayRunDialog();
        break;

      case CORE_SHUTDOWN:
        ELDisplayShutdownDialog(mainWnd);
        break;

      case CORE_EMPTYBIN:
        SHEmptyRecycleBin(NULL, NULL, 0);
        break;

      case CORE_LOGOFF:
        ELExit(EMERGE_LOGOFF, true);
        break;

      case CORE_REBOOT:
        ELExit(EMERGE_REBOOT, true);
        break;

      case CORE_HALT:
        ELExit(EMERGE_HALT, true);
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
        pMessageControl->Dispatch_Message(EMERGE_CORE, CORE_REPOSITION, NULL);
        break;

      case CORE_ABOUT:
        About();
        break;

      case CORE_CONFIGURE:
        ShowConfig(2);
        break;

      case CORE_ALIAS:
        ShowConfig(1);
        break;

      case CORE_LAUNCH:
        ShowConfig(0);
        break;

      case CORE_WELCOME:
        ShowWelcome();
        break;
      }

      return 1;
    }
  }

  return 0;
}

void Core::ShowWelcome()
{
  Welcome welcome(mainInst, mainWnd, pSettings);

  welcome.Show();
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

  if (message == TASKBAR_CREATED && !ELIsExplorerShell())
  {
    pShell->LoadSSO();
    return 0;
  }

  // If not handled just forward the message on
  if (!pShell->UpdateTaskCount(message, (UINT)wParam, (HWND)lParam))
  {
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  return 0;
}

void Core::ShowConfig(UINT startPage)
{
  Config config(mainInst, mainWnd, pSettings);

  if (config.Show(startPage) == IDOK)
  {
    EnableExplorerDesktop();
  }
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

  ELAppletFileVersion(TEXT("emergeLib.dll"), &libInfo);
  ELAppletFileVersion(TEXT("emergeGraphics.dll"), &graphicsInfo);
  ELAppletFileVersion(TEXT("emergeStyleEngine.dll"), &styleInfo);
  ELAppletFileVersion(TEXT("emergeBaseClasses.dll"), &baseInfo);
  ELAppletFileVersion(TEXT("emergeAppletEngine.dll"), &engineInfo);

  if (ELAppletVersionInfo(mainWnd, &coreInfo))
  {
    swprintf(tmp, TEXT("%ls\n\nVersion:\t\t\t%ls\n\nemergeLib:\t\t%ls\nemergeGraphics:\t%ls\nemergeStyleEngine:\t%ls\n")
             TEXT("emergeBaseClasses:\t%ls\nemergeAppletEngine:\t%ls\n\nAuthor: %ls"),
             coreInfo.Description,
             coreInfo.Version,
             libInfo.Version,
             graphicsInfo.Version,
             styleInfo.Version,
             baseInfo.Version,
             engineInfo.Version,
             coreInfo.Author);

    ELMessageBox(GetDesktopWindow(), tmp, TEXT("emergeCore"),
                 ELMB_OK | ELMB_ICONQUESTION | ELMB_MODAL);
  }
}

bool Core::BuildLaunchList()
{
  LaunchMap launchMap;
  LaunchMap::iterator iter;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* section = NULL, *item, *settings;
  WCHAR program[MAX_PATH], arguments[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH];
  bool found;
  std::wstring theme = ELToLower(ELGetThemeName()), defaultTheme = TEXT("default");

  if (theme == defaultTheme)
  {
    return false;
  }

  EnumWindows(LaunchMapEnum, (LPARAM)&launchMap);

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
  {
    settings = ELGetXMLSection(configXML.get(), TEXT("Settings"), true);
    if (settings)
    {
      section = ELSetFirstXMLElementByName(settings, TEXT("Launch"));
    }
    if (section)
    {
      while (!launchMap.empty())
      {
        found = true;
        iter = launchMap.begin();
        ELParseCommand(iter->first, program, arguments);
        swprintf(command, TEXT("%ls %ls"), PathFindFileName(program), arguments);
        item = ELSetFirstXMLElementByName(section, TEXT("item"));
        if (item)
        {
          ELWriteXMLStringValue(item, TEXT("Command"), command);
        }
        launchMap.erase(iter);
      }

      if (found)
      {
        ELWriteXMLConfig(configXML.get());
      }
    }
  }

  return found;
}

void Core::StartExplorer()
{
  std::wstring explorerCmd;

  explorerCmd = ELGetCurrentPath();
  explorerCmd = explorerCmd + TEXT("\\Explorer.exe");
  ELExecuteFileOrCommand(explorerCmd);
}

void Core::EnableExplorerDesktop()
{
  DWORD enableID, threadState;

  // Create the thread to handle enabling Explorer Desktop
  GetExitCodeThread(explorerThread, &threadState);
  if (threadState != STILL_ACTIVE)
    explorerThread = CreateThread(NULL, 0, EnableExplorerThreadProc,
                                  pSettings.get(), 0, &enableID);

  if (pSettings->GetEnableExplorerDesktop())
  {
    pDesktop->ShowDesktop(!pSettings->GetShowExplorerDesktop());
  }
  else
  {
    pDesktop->ShowDesktop(true);
  }
}

DWORD WINAPI Core::EnableExplorerThreadProc(LPVOID lpParameter)
{
  Settings* pSettings = reinterpret_cast< Settings* >(lpParameter);

  HWND explorerWnd = FindWindow(TEXT("EmergeDesktopExplorer"), NULL);
  while (!explorerWnd)
  {
    // Pause the current thread for 100 ms
    WaitForSingleObject(GetCurrentThread(), 100);
    explorerWnd = FindWindow(TEXT("EmergeDesktopExplorer"), NULL);
  }
  SendMessage(explorerWnd, EMERGE_MESSAGE, EXPLORER_ENABLE,
              pSettings->GetEnableExplorerDesktop());

  if (pSettings->GetEnableExplorerDesktop())
  {
    //Sleep(500);
    SendMessage(explorerWnd, EMERGE_MESSAGE, EXPLORER_SHOW,
                pSettings->GetShowExplorerDesktop());
  }

  return 0;
}

void Core::ConvertTheme()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* first, *tmp, *section, *settings, *oldSection = NULL;
  WCHAR data[MAX_LINE_LENGTH];

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
  {
    oldSection = ELGetXMLSection(configXML.get(), TEXT("Launch"), false);
    if (oldSection)
    {
      settings = ELGetXMLSection(configXML.get(), TEXT("Settings"), true);
      if (settings)
      {
        section = ELGetFirstXMLElementByName(settings, TEXT("Launch"), true);
        if (section)
        {
          first = ELGetFirstXMLElement(oldSection);

          while (first)
          {
            wcscpy(data, ELReadXMLStringValue(first, TEXT("Command"), TEXT("")).c_str());
            if (wcslen(data) > 0)
            {
              ELStringReplace(data, TEXT("emergeDesktop"), TEXT("emergeWorkspace"), true);
              tmp = ELSetFirstXMLElementByName(section, TEXT("item"));
              if (tmp)
              {
                ELWriteXMLStringValue(tmp, TEXT("Command"), data);
              }
            }

            tmp = first;
            first = ELGetSiblingXMLElement(tmp);
          }

          ELRemoveXMLElement(oldSection);
          ELWriteXMLConfig(configXML.get());
        }
      }
    }
  }
}

bool Core::CheckLaunchList()
{
  LaunchMap launchMap;
  LaunchMap::iterator mapIter;
  WindowSet::iterator setIter;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* first, *tmp, *section = NULL, *settings;
  std::wstring data;
  bool found = false;

  EnumWindows(LaunchMapEnum, (LPARAM)&launchMap);

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
  {
    settings = ELGetXMLSection(configXML.get(), TEXT("Settings"), false);
    if (settings)
    {
      section = ELGetFirstXMLElementByName(settings, TEXT("Launch"), false);
    }
    if (section)
    {
      first = ELGetFirstXMLElement(section);

      while (first)
      {
        data = ELReadXMLStringValue(first, TEXT("Command"), TEXT(""));
        if (!data.empty())
        {
          found = true;
          CheckLaunchItem(&launchMap, data);
        }

        tmp = first;
        first = ELGetSiblingXMLElement(tmp);
      }
    }
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
    mapIter = launchMap.begin();
    setIter = mapIter->second.begin();
    while (setIter != mapIter->second.end())
    {
      SendMessage((HWND)*setIter, WM_NCDESTROY, 0, 0);
      setIter++;
    }
    launchMap.erase(mapIter);
  }

  return true;
}

void Core::CheckLaunchItem(LaunchMap* launchMap, std::wstring item)
{
  LaunchMap::iterator iter;
  std::wstring program = TEXT("%AppletDir%\\");
  std::wstring workingItem = item;

  if (ELPathIsRelative(workingItem))
  {
    program = program + workingItem;
  }
  else
  {
    program = workingItem;
  }

  program = ELExpandVars(program);
  program = ELToLower(program);

  iter = launchMap->find(program);
  if (iter == launchMap->end())
  {
    ELExecuteFileOrCommand(program);
  }
  else
  {
    launchMap->erase(program);
  }
}

BOOL Core::LaunchMapEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR windowClass[MAX_LINE_LENGTH];
  std::wstring windowName;
  LaunchMap* launchMap = (LaunchMap*)lParam;
  LaunchMap::iterator mapIter;

  if (RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH) != 0)
  {
    if ((_wcsicmp(windowClass, TEXT("EmergeDesktopApplet")) == 0) ||
        (_wcsicmp(windowClass, TEXT("EmergeDesktopMenuBuilder")) == 0))
    {
      windowName = ELToLower(ELGetWindowApp(hwnd, true));
      mapIter = launchMap->find(windowName);
      if (mapIter == launchMap->end())
      {
        WindowSet windowSet;
        windowSet.insert(windowSet.begin(), hwnd);
        launchMap->insert(std::pair<std::wstring, WindowSet>(windowName, windowSet));
      }
      else
      {
        mapIter->second.insert(mapIter->second.begin(), hwnd);
      }
    }
  }

  return TRUE;
}
