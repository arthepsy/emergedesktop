// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009-2011  The Emerge Desktop Development Team
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

#include "Importer.h"
#include "CoreSettings.h"
#include "CommandSettings.h"
#include "LauncherSettings.h"
#include "VWMSettings.h"
#include "TasksSettings.h"
#include "TraySettings.h"
#include "HotkeysSettings.h"
#include "DesktopSettings.h"

BOOL CALLBACK Importer::ImporterDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Importer *pImporter = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pImporter = reinterpret_cast<Importer*>(lParam);
      if (!pImporter)
        break;
      return pImporter->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pImporter->DoCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

Importer::Importer(HINSTANCE hInstance)
{
  (*this).hInstance = hInstance;

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

  ELOpenRegKey(TEXT("emergeCommand"), &commandKey, false);
  ELOpenRegKey(TEXT("emergeCore"), &coreKey, false);
  ELOpenRegKey(TEXT("emergeDesktop"), &desktopKey, false);
  ELOpenRegKey(TEXT("emergeHotkeys"), &hotkeysKey, false);
  ELOpenRegKey(TEXT("emergeLauncher"), &launcherKey, false);
  ELOpenRegKey(TEXT("emergeTasks"), &tasksKey, false);
  ELOpenRegKey(TEXT("emergeTray"), &trayKey, false);
  ELOpenRegKey(TEXT("emergeVWM"), &vwmKey, false);
}

Importer::~Importer()
{
  ELCloseRegKey(commandKey);
  ELCloseRegKey(coreKey);
  ELCloseRegKey(desktopKey);
  ELCloseRegKey(hotkeysKey);
  ELCloseRegKey(launcherKey);
  ELCloseRegKey(tasksKey);
  ELCloseRegKey(trayKey);
  ELCloseRegKey(vwmKey);

  DestroyWindow(toolWnd);
}

int Importer::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_IMPORT), NULL, (DLGPROC)ImporterDlgProc, (LPARAM)this);
}

BOOL Importer::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  HWND okWnd = GetDlgItem(hwndDlg, IDOK);
  HWND cancelWnd = GetDlgItem(hwndDlg, IDCANCEL);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND coreWnd = GetDlgItem(hwndDlg, IDC_CORE);
  HWND desktopWnd = GetDlgItem(hwndDlg, IDC_DESKTOP);
  HWND hotkeysWnd = GetDlgItem(hwndDlg, IDC_HOTKEYS);
  HWND launcherWnd = GetDlgItem(hwndDlg, IDC_LAUNCHER);
  HWND tasksWnd = GetDlgItem(hwndDlg, IDC_TASKS);
  HWND trayWnd = GetDlgItem(hwndDlg, IDC_TRAY);
  HWND vwmWnd = GetDlgItem(hwndDlg, IDC_VWM);

  if (commandKey)
    {
      EnableWindow(commandWnd, true);
      SendMessage(commandWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(commandWnd, false);
  if (coreKey)
    {
      EnableWindow(coreWnd, true);
      SendMessage(coreWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(coreWnd, false);
  if (desktopKey)
    {
      EnableWindow(desktopWnd, true);
      SendMessage(desktopWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(desktopWnd, false);
  if (hotkeysKey)
    {
      EnableWindow(hotkeysWnd, true);
      SendMessage(hotkeysWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(hotkeysWnd, false);
  if (launcherKey)
    {
      EnableWindow(launcherWnd, true);
      SendMessage(launcherWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(launcherWnd, false);
  if (tasksKey)
    {
      EnableWindow(tasksWnd, true);
      SendMessage(tasksWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(tasksWnd, false);
  if (trayKey)
    {
      EnableWindow(trayWnd, true);
      SendMessage(trayWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(trayWnd, false);
  if (vwmKey)
    {
      EnableWindow(vwmWnd, true);
      SendMessage(vwmWnd, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    EnableWindow(vwmWnd, false);

  SetDlgItemText(hwndDlg, IDC_THEME, TEXT("Imported"));

  ELGetWindowRect(hwndDlg, &rect);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = okWnd;
  ti.uId = (ULONG_PTR)okWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("OK");
  GetClientRect(okWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = cancelWnd;
  ti.uId = (ULONG_PTR)cancelWnd;
  ti.lpszText = (WCHAR*)TEXT("Cancel");
  GetClientRect(cancelWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  return TRUE;
}

BOOL Importer::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      if (!DoImport(hwndDlg))
        return FALSE;
    case IDCANCEL:
      EndDialog(hwndDlg, wParam);
      return TRUE;
    }

  return FALSE;
}

bool Importer::DoImport(HWND hwndDlg)
{
  if (!ELSetEmergeVars())
    {
      ELMessageBox(hwndDlg, TEXT("Failed to initialize Environment variables."),
                   TEXT("Registry Importer"), ELMB_ICONERROR|ELMB_MODAL|ELMB_OK);
      return false;
    }

  WCHAR themeName[MAX_LINE_LENGTH], errorText[MAX_LINE_LENGTH];
  if (GetDlgItemText(hwndDlg, IDC_THEME, themeName, MAX_LINE_LENGTH) == 0)
    {
      swprintf(errorText, TEXT("The Theme Name cannot be empty."));
      ELMessageBox(hwndDlg, errorText, TEXT("Registry Importer"), ELMB_ICONERROR|ELMB_MODAL|ELMB_OK);
      return false;
    }
  if (_wcsicmp(themeName, TEXT("Default")) == 0)
    {
      swprintf(errorText, TEXT("The Theme Name '%s' is reserved."), themeName);
      ELMessageBox(hwndDlg, errorText, TEXT("Registry Importer"), ELMB_ICONERROR|ELMB_MODAL|ELMB_OK);
      return false;
    }
  std::wstring appletCmd = TEXT("%AppletDir%\\files\\cmd.txt"), emergeCmd = TEXT("%EmergeDir%\\files\\");
  std::wstring themeDir = TEXT("%EmergeDir%\\themes\\");
  themeDir += themeName;
  themeDir = ELExpandVars(themeDir);
  if (!ELPathIsDirectory(themeDir.c_str()))
    ELCreateDirectory(themeDir.c_str());
  SetEnvironmentVariable(TEXT("ThemeDir"), themeDir.c_str());
  ELSetTheme(themeDir);

  if ((SendDlgItemMessage(hwndDlg, IDC_CORE, BM_GETCHECK, 0, 0) == BST_CHECKED) && coreKey)
    {
      CoreSettings coreSettings(coreKey);
      coreSettings.ConvertShells();
      coreSettings.ConvertLaunch();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_COMMAND, BM_GETCHECK, 0, 0) == BST_CHECKED) && commandKey)
    {
      CommandSettings commandSettings(commandKey);
      commandSettings.ConvertReg();
      commandSettings.ConvertHistory();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_LAUNCHER, BM_GETCHECK, 0, 0) == BST_CHECKED) && launcherKey)
    {
      LauncherSettings launcherSettings(launcherKey);
      launcherSettings.ConvertReg();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_VWM, BM_GETCHECK, 0, 0) == BST_CHECKED) && vwmKey)
    {
      VWMSettings vwmSettings(vwmKey);
      vwmSettings.ConvertReg();
      vwmSettings.ConvertSticky();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_TASKS, BM_GETCHECK, 0, 0) == BST_CHECKED) && tasksKey)
    {
      TasksSettings tasksSettings(tasksKey);
      tasksSettings.ConvertReg();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_TRAY, BM_GETCHECK, 0, 0) == BST_CHECKED) && trayKey)
    {
      TraySettings traySettings(trayKey);
      traySettings.ConvertReg();
      traySettings.ConvertHide();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_HOTKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED) && hotkeysKey)
    {
      HotkeysSettings hotkeysSettings(hotkeysKey);
      hotkeysSettings.ConvertActions();
    }

  if ((SendDlgItemMessage(hwndDlg, IDC_DESKTOP, BM_GETCHECK, 0, 0) == BST_CHECKED) && desktopKey)
    {
      DesktopSettings desktopSettings(desktopKey);
      desktopSettings.ConvertReg();
      desktopSettings.ConvertMenus();
    }

  appletCmd = ELExpandVars(appletCmd);
  emergeCmd = ELExpandVars(emergeCmd);
  if (!ELPathIsDirectory(emergeCmd.c_str()))
    ELCreateDirectory(emergeCmd);
  emergeCmd += TEXT("cmd.txt");
  CopyFile(appletCmd.c_str(), emergeCmd.c_str(), TRUE);

  return true;
}
