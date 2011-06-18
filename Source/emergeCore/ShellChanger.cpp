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

#include "ShellChanger.h"

#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY 0x0100
#endif

BOOL CALLBACK ShellChanger::ShellDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ShellChanger *pShellChanger = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pShellChanger = reinterpret_cast<ShellChanger*>(lParam);
      if (!pShellChanger)
        break;
      return pShellChanger->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pShellChanger->DoShellCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

ShellChanger::ShellChanger(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;
  this->pSettings = pSettings;
  edit = false;

  InitCommonControls();

  toolWnd = CreateWindowEx(
                           0,
                           TOOLTIPS_CLASS,
                           NULL,
                           TTS_ALWAYSTIP|WS_POPUP|TTS_NOPREFIX,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           NULL,
                           NULL,
                           hInstance,
                           NULL);

  if (toolWnd)
    {
      SendMessage(toolWnd, TTM_SETMAXTIPWIDTH, 0, 300);
      SetWindowPos(toolWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                   SWP_NOACTIVATE);
    }

  ExtractIconEx(TEXT("emergeIcons.dll"), 2, NULL, &addIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 5, NULL, &editIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 3, NULL, &delIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 1, NULL, &abortIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &browseIcon, 1);

  xmlFile = TEXT("%EmergeDir%\\files\\emergeCore.xml");
}

ShellChanger::~ShellChanger()
{
  if (addIcon)
    DestroyIcon(addIcon);
  if (editIcon)
    DestroyIcon(editIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (abortIcon)
    DestroyIcon(abortIcon);
  if (browseIcon)
    DestroyIcon(browseIcon);

  DestroyWindow(toolWnd);
}

int ShellChanger::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SHELL), mainWnd, (DLGPROC)ShellDlgProc, (LPARAM)this);
}

BOOL ShellChanger::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  TOOLINFO ti;

  ZeroMemory(&ti, sizeof(TOOLINFO));
  ELGetWindowRect(hwndDlg, &rect);

  HWND nameWnd = GetDlgItem(hwndDlg, IDC_SHELLNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_SHELLCOMMAND);
  HWND commandTextWnd = GetDlgItem(hwndDlg, IDC_COMMANDTEXT);
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVESHELL);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTSHELL);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDSHELL);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITSHELL);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELSHELL);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSESHELL);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (editIcon)
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);
  if (browseIcon)
    SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add Shell Item");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit Shell Item");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Shell Item");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = saveWnd;
  ti.uId = (ULONG_PTR)saveWnd;
  ti.lpszText = (WCHAR*)TEXT("Save Changes");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = abortWnd;
  ti.uId = (ULONG_PTR)abortWnd;
  ti.lpszText = (WCHAR*)TEXT("Discard Changes");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse for File");
  GetClientRect(browseWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  EnableWindow(nameWnd, false);
  EnableWindow(commandWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(addWnd, true);
  EnableWindow(browseWnd, false);
  EnableWindow(nameTextWnd, false);
  EnableWindow(commandTextWnd, false);

  if (pSettings->GetShowStartupErrors())
    SendDlgItemMessage(hwndDlg, IDC_STARTERROR, BM_SETCHECK, BST_CHECKED, 0);

  PopulateShells(shellWnd);
  UpdateFields(hwndDlg);

  return TRUE;
}

void ShellChanger::PopulateShells(HWND shellWnd)
{
  DWORD size, result;
  WCHAR name[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH], ELPath[MAX_LINE_LENGTH], currentShell[MAX_LINE_LENGTH];
  HKEY key;
  bool doCheck = false;
  int shellIndex = -1;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *first, *sibling, *section = NULL, *settings = NULL;

  size = MAX_LINE_LENGTH * sizeof(currentShell[0]);
  if (RegCreateKeyEx(HKEY_CURRENT_USER,
                     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &key, &result) == ERROR_SUCCESS)
    {
      if (RegQueryValueEx(key, TEXT("Shell"), NULL, NULL, (BYTE*)currentShell, &size) == ERROR_SUCCESS)
        {
          _wcslwr(currentShell);
          doCheck = true;
          RegCloseKey(key);
        }
    }

  SendMessage(shellWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Emerge Desktop"));
  SendMessage(shellWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Windows Explorer"));

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      settings = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), false);
      if (settings)
        section = ELGetFirstXMLElementByName(settings, (WCHAR*)TEXT("Shells"), false);
      if (section == NULL) /**< Handle the broken file format where Shells is a top level XML element */
        section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Shells"), false);
      if (section)
        {
          first = ELGetFirstXMLElement(section);

          if (first)
            {
              ELReadXMLStringValue(first, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
              ELReadXMLStringValue(first, (WCHAR*)TEXT("Command"), command, (WCHAR*)TEXT("\0"));

              SendMessage(shellWnd, CB_ADDSTRING, 0, (LPARAM)name);
              shellMap.insert(EmergeShellItem(std::wstring(name), std::wstring(command)));

              sibling = ELGetSiblingXMLElement(first);
              while (sibling)
                {
                  first = sibling;
                  ELReadXMLStringValue(first, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
                  ELReadXMLStringValue(first, (WCHAR*)TEXT("Command"), command, (WCHAR*)TEXT("\0"));

                  SendMessage(shellWnd, CB_ADDSTRING, 0, (LPARAM)name);
                  shellMap.insert(EmergeShellItem(std::wstring(name), std::wstring(command)));

                  sibling = ELGetSiblingXMLElement(first);
                }
            }
        }
    }

  if (doCheck)
    {
      ELGetCurrentPath(ELPath);
      wcscat(ELPath, TEXT("\\emergeCore.exe"));
      _wcslwr(ELPath);
      if (wcsstr(currentShell, ELPath))
        shellIndex = (int)SendMessage(shellWnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)TEXT("Emerge Desktop"));
      else if (wcsstr(currentShell, TEXT("explorer.exe")))
        shellIndex = (int)SendMessage(shellWnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)TEXT("Windows Explorer"));
      else
        {
          std::wstring lowerIter;
          EmergeShellItemMap::iterator iter = shellMap.begin();
          while (iter != shellMap.end())
            {
              lowerIter = ELToLower(iter->second);
              if (lowerIter == currentShell)
                shellIndex = (int)SendMessage(shellWnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)iter->first.c_str());
              iter++;
            }
        }
    }

  SendMessage(shellWnd, CB_SETCURSEL, (WPARAM)shellIndex, 0);
}

bool ShellChanger::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_SHELLCOMMAND);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_SHELLNAME);

  if (((GetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, tmp, MAX_LINE_LENGTH) != 0) &&
       IsWindowEnabled(commandWnd)) ||
      ((GetDlgItemText(hwndDlg, IDC_SHELLNAME, tmp, MAX_LINE_LENGTH) != 0) &&
       IsWindowEnabled(nameWnd)))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("The current shell command will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeCore"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }
  else
    return true;
}

BOOL ShellChanger::DoShellCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      DoSetShell(hwndDlg);
    case IDCANCEL:
      if (!CheckFields(hwndDlg))
        break;
      shellMap.clear();
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDC_DELSHELL:
      DoDelShell(hwndDlg);
      WriteShells(hwndDlg);
    case IDC_EDITSHELL:
      return DoEditShell(hwndDlg);
    case IDC_ADDSHELL:
      return DoAddShell(hwndDlg);
    case IDC_SAVESHELL:
      DoSaveShell(hwndDlg);
      return WriteShells(hwndDlg);
    case IDC_ABORTSHELL:
      return DoAbortShell(hwndDlg);
    case IDC_BROWSESHELL:
      return DoBrowseShell(hwndDlg);
    }

  if (HIWORD(wParam) == CBN_SELENDOK)
    {
      UpdateFields(hwndDlg);
      return TRUE;
    }

  return FALSE;
}

bool ShellChanger::DoSetShell(HWND hwndDlg)
{
  HKEY key;
  DWORD result;
  WCHAR name[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH];
  DWORD process;
  REGSAM regMask = 0;
  bool success = false;
  HWND startErrorWnd = GetDlgItem(hwndDlg, IDC_STARTERROR);

  GetDlgItemText(hwndDlg, IDC_SHELLITEM, name, MAX_LINE_LENGTH);
  GetShellCommand(hwndDlg, name, command);
  PathQuoteSpaces(command);

  if (ELIsWow64())
    regMask = KEY_WOW64_64KEY;

  if (RegCreateKeyEx(HKEY_CURRENT_USER,
                     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | regMask, NULL, &key, &result) == ERROR_SUCCESS)
    {
      RegSetValueEx(key, TEXT("Shell"), 0, REG_SZ, (BYTE*)command,
                    (DWORD)wcslen(command) * sizeof(command[0]));

      RegCloseKey(key);
    }

  if (RegCreateKeyEx(HKEY_CURRENT_USER,
                     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | regMask, NULL, &key, &result) == ERROR_SUCCESS)
    {
      process = 1;
      wcscpy(command, TEXT("yes"));
      //RegSetValueEx(key, TEXT("DesktopProcess"), 0, REG_DWORD, (BYTE*)&process,
      //sizeof(DWORD));
      RegSetValueEx(key, TEXT("BrowseNewProcess"), 0, REG_SZ, (BYTE*)command,
                    (DWORD)wcslen(command) * sizeof(command[0]));

      RegCloseKey(key);
    }

  if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | regMask, NULL, &key, &result) == ERROR_SUCCESS)
    {
      wcscpy(command, TEXT("explorer.exe"));
      RegSetValueEx(key, TEXT("Shell"), 0, REG_SZ, (BYTE*)command,
                    (DWORD)wcslen(command) * sizeof(command[0]));

      RegCloseKey(key);
    }

  if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\system.ini\\boot"),
                     0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | regMask, NULL, &key, &result) == ERROR_SUCCESS)
    {
      wcscpy(command, TEXT("USR:Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"));
      RegSetValueEx(key, TEXT("Shell"), 0, REG_SZ, (BYTE*)command,
                    (DWORD)wcslen(command) * sizeof(command[0]));

      RegCloseKey(key);
    }

  // Create default directories and copy default files
  std::wstring appletCmd = TEXT("%AppletDir%\\files\\cmd.txt"), emergeCmd = TEXT("%EmergeDir%\\files\\");
  appletCmd = ELExpandVars(appletCmd);
  emergeCmd = ELExpandVars(emergeCmd);
  if (!PathIsDirectory(emergeCmd.c_str()))
    ELCreateDirectory(emergeCmd);
  emergeCmd += TEXT("cmd.txt");
  CopyFile(appletCmd.c_str(), emergeCmd.c_str(), TRUE);

  if (IsWindowEnabled(startErrorWnd))
    {
      if (SendDlgItemMessage(hwndDlg, IDC_STARTERROR, BM_GETCHECK, 0, 0) == BST_CHECKED)
        success = true;
      else if (SendDlgItemMessage(hwndDlg, IDC_STARTERROR, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
        success = false;
      pSettings->SetShowStartupErrors(success);
    }

  pSettings->WriteUserSettings();
  ELMessageBox(hwndDlg, (WCHAR*)TEXT("Changes will take affect after reboot."), (WCHAR*)TEXT("emergeCore"),
               ELMB_OK|ELMB_ICONQUESTION|ELMB_MODAL);

  return true;
}

int ShellChanger::GetShellCommand(HWND hwndDlg, WCHAR *name, WCHAR *command)
{
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  int index = (int)SendMessage(shellWnd, CB_GETCURSEL, 0, 0);
  EmergeShellItemMap::iterator iter;

  GetDlgItemText(hwndDlg, IDC_SHELLITEM, name, MAX_LINE_LENGTH);

  if (_wcsicmp(name, TEXT("Emerge Desktop")) == 0)
    {
      ELGetCurrentPath(command);
      wcscat(command, TEXT("\\emergeCore.exe"));
    }
  else if (_wcsicmp(name, TEXT("Windows Explorer")) == 0)
    wcscpy(command, TEXT("explorer.exe"));
  else
    {
      iter = shellMap.find(name);

      if (iter == shellMap.end())
        index = -1;
      else
        wcscpy(command, iter->second.c_str());
    }

  return index;
}

void ShellChanger::UpdateFields(HWND hwndDlg)
{
  WCHAR name[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH];
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITSHELL);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELSHELL);
  HWND changeWnd = GetDlgItem(hwndDlg, IDOK);
  HWND startErrorWnd = GetDlgItem(hwndDlg, IDC_STARTERROR);
  EmergeShellItemMap::iterator iter;
  int index;

  GetDlgItemText(hwndDlg, IDC_SHELLITEM, name, MAX_LINE_LENGTH);
  index = GetShellCommand(hwndDlg, name, command);

  if (index > -1)
    {
      EnableWindow(changeWnd, true);

      SetDlgItemText(hwndDlg, IDC_SHELLNAME, name);
      SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, command);

      if (_wcsicmp(name, TEXT("Emerge Desktop")) == 0)
        {
          if (pSettings->GetShowStartupErrors())
            SendDlgItemMessage(hwndDlg, IDC_STARTERROR, BM_SETCHECK, BST_CHECKED, 0);
          else
            SendDlgItemMessage(hwndDlg, IDC_STARTERROR, BM_SETCHECK, BST_UNCHECKED, 0);
          EnableWindow(startErrorWnd, true);
        }
      else
        {
          SendDlgItemMessage(hwndDlg, IDC_STARTERROR, BM_SETCHECK, BST_UNCHECKED, 0);
          EnableWindow(startErrorWnd, false);
        }

      if ((_wcsicmp(name, TEXT("Emerge Desktop")) == 0) ||
          (_wcsicmp(name, TEXT("Window Explorer")) == 0))
        {
          EnableWindow(editWnd, false);
          EnableWindow(delWnd, false);
        }
      else
        {
          EnableWindow(editWnd, true);
          EnableWindow(delWnd, true);
        }
    }
  else
    {
      EnableWindow(editWnd, false);
      EnableWindow(delWnd, false);
      EnableWindow(changeWnd, false);
    }
}

bool ShellChanger::WriteShells(HWND hwndDlg)
{
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  int count = (int)SendMessage(shellWnd, CB_GETCOUNT, 0, 0);
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *first, *section = NULL, *settings = NULL;
  WCHAR name[MAX_LINE_LENGTH];
  EmergeShellItemMap::iterator iter;

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Shells"), false);
      if (section) /**< Remove the 'Shells' top-level if it exists */
        ELRemoveXMLElement(section);
      settings = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);
      if (settings)
        section = ELGetFirstXMLElementByName(settings, (WCHAR*)TEXT("Shells"), true);
      if (section)
        {
          int index = 0;
          first = ELGetFirstXMLElement(section);
          while (first)
            {
              ELRemoveXMLElement(first);
              first = ELGetFirstXMLElement(section);
            }

          // Loop while there are entries in the key
          while (index < count)
            {
              SendMessage(shellWnd, CB_GETLBTEXT, (WPARAM)index, (LPARAM)name);
              if ((_wcsicmp(name, TEXT("Emerge Desktop")) != 0) &&
                  (_wcsicmp(name, TEXT("Windows Explorer")) != 0))
                {
                  iter = shellMap.find(name);

                  first = ELSetFirstXMLElement(section, TEXT("item"));
                  ELWriteXMLStringValue(first, (WCHAR*)TEXT("Name"), name);
                  ELWriteXMLStringValue(first, (WCHAR*)TEXT("Command"), (WCHAR*)iter->second.c_str());
                }

              index++;
            }
          ELWriteXMLConfig(configXML.get());
        }
    }

  return true;
}

bool ShellChanger::DoDelShell(HWND hwndDlg)
{
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITSHELL);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELSHELL);
  HWND changeWnd = GetDlgItem(hwndDlg, IDOK);
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  WCHAR name[MAX_LINE_LENGTH];

  GetDlgItemText(hwndDlg, IDC_SHELLITEM, name, MAX_LINE_LENGTH);

  SetDlgItemText(hwndDlg, IDC_SHELLNAME, TEXT("\0"));
  SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, TEXT("\0"));
  SendMessage(shellWnd, CB_SETCURSEL, (WPARAM)-1, 0);

  EnableWindow(editWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(changeWnd, false);

  return DeleteShell(hwndDlg, name);
}

bool ShellChanger::DoEditShell(HWND hwndDlg)
{
  EmergeShellItemMap::iterator iter;
  WCHAR name[MAX_LINE_LENGTH];

  GetDlgItemText(hwndDlg, IDC_SHELLNAME, name, MAX_LINE_LENGTH);

  iter = shellMap.find(name);

  if (iter == shellMap.end())
    return false;

  SetDlgItemText(hwndDlg, IDC_SHELLNAME, name);
  SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, iter->second.c_str());

  edit = true;

  return EnableFields(hwndDlg, true);
}

bool ShellChanger::DoAbortShell(HWND hwndDlg)
{
  EmergeShellItemMap::iterator iter;
  WCHAR name[MAX_LINE_LENGTH];

  if (edit)
    {
      GetDlgItemText(hwndDlg, IDC_SHELLITEM, name, MAX_LINE_LENGTH);
      iter = shellMap.find(name);

      if (iter == shellMap.end())
        return false;

      SetDlgItemText(hwndDlg, IDC_SHELLNAME, name);
      SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, iter->second.c_str());
    }
  else
    {
      SetDlgItemText(hwndDlg, IDC_SHELLNAME, TEXT("\0"));
      SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, TEXT("\0"));
    }

  return EnableFields(hwndDlg, false);
}

bool ShellChanger::DoAddShell(HWND hwndDlg)
{
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);

  SetDlgItemText(hwndDlg, IDC_SHELLNAME, TEXT("\0"));
  SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, TEXT("\0"));
  SendMessage(shellWnd, CB_SETCURSEL, (WPARAM)-1, 0);

  edit = false;

  return EnableFields(hwndDlg, true);
}

bool ShellChanger::EnableFields(HWND hwndDlg, bool enable)
{
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_SHELLNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_SHELLCOMMAND);
  HWND commandTextWnd = GetDlgItem(hwndDlg, IDC_COMMANDTEXT);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVESHELL);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTSHELL);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDSHELL);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITSHELL);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELSHELL);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSESHELL);
  HWND changeWnd = GetDlgItem(hwndDlg, IDOK);

  if (enable)
    {
      EnableWindow(nameWnd, true);
      EnableWindow(commandWnd, true);
      EnableWindow(saveWnd, true);
      EnableWindow(abortWnd, true);
      EnableWindow(browseWnd, true);
      EnableWindow(shellWnd, false);
      EnableWindow(addWnd, false);
      EnableWindow(editWnd, false);
      EnableWindow(delWnd, false);
      EnableWindow(changeWnd, false);
      EnableWindow(nameTextWnd, true);
      EnableWindow(commandTextWnd, true);
    }
  else
    {
      EnableWindow(nameWnd, false);
      EnableWindow(commandWnd, false);
      EnableWindow(saveWnd, false);
      EnableWindow(abortWnd, false);
      EnableWindow(browseWnd, false);
      EnableWindow(shellWnd, true);
      EnableWindow(addWnd, true);
      EnableWindow(editWnd, false);
      EnableWindow(delWnd, false);
      EnableWindow(changeWnd, true);
      EnableWindow(nameTextWnd, false);
      EnableWindow(commandTextWnd, false);
    }

  return true;
}

bool ShellChanger::DoSaveShell(HWND hwndDlg)
{
  WCHAR newName[MAX_LINE_LENGTH], oldName[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH];
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  int index;

  if (GetDlgItemText(hwndDlg, IDC_SHELLNAME, newName, MAX_LINE_LENGTH) == 0)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Shell name cannot be empty"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }
  if (GetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, command, MAX_LINE_LENGTH) == 0)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Shell command cannot be empty"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }

  if (edit)
    {
      GetDlgItemText(hwndDlg, IDC_SHELLITEM, oldName, MAX_LINE_LENGTH);
      DeleteShell(hwndDlg, oldName);
    }

  EmergeShellItemMap::iterator iter;
  iter = shellMap.find(newName);
  swprintf(tmp, TEXT("A Shell with the name '%s' already exists"), newName);
  if ((iter != shellMap.end()) ||
      (_wcsicmp(newName, TEXT("Windows Explorer")) == 0) ||
      (_wcsicmp(newName, TEXT("Emerge Desktop")) == 0))
    {
      ELMessageBox(hwndDlg, tmp, (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }


  index = (int)SendMessage(shellWnd, CB_ADDSTRING, 0, (LPARAM)newName);
  shellMap.insert(EmergeShellItem(std::wstring(newName), std::wstring(command)));
  SendMessage(shellWnd, CB_SETCURSEL, (WPARAM)index, 0);

  return EnableFields(hwndDlg, false);
}

bool ShellChanger::DeleteShell(HWND hwndDlg, WCHAR *name)
{
  HWND shellWnd = GetDlgItem(hwndDlg, IDC_SHELLITEM);
  EmergeShellItemMap::iterator iter;
  int item;

  if ((_wcsicmp(name, TEXT("Emerge Desktop")) == 0) ||
      (_wcsicmp(name, TEXT("Window Explorer")) == 0))
    return false;

  iter = shellMap.find(name);

  // Note: Order is important!
  if (iter == shellMap.end())
    return false;

  shellMap.erase(iter);

  item = (int)SendMessage(shellWnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)name);

  if (item != CB_ERR)
    {
      SendMessage(shellWnd, CB_DELETESTRING, (WPARAM)item, 0);
      return true;
    }

  return false;
}

bool ShellChanger::DoBrowseShell(HWND hwndDlg)
{
  OPENFILENAME ofn;
  WCHAR tmp[MAX_PATH];
  WCHAR initPath[MAX_PATH];

  ZeroMemory(tmp, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));

  ELGetCurrentPath(initPath);

  ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
  ofn.lpstrInitialDir = initPath;
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = TEXT("Browse For File");
  ofn.lpstrDefExt = NULL;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
    {
      SetDlgItemText(hwndDlg, IDC_SHELLCOMMAND, tmp);
      return true;
    }

  return false;
}

