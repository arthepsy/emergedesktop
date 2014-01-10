/*!
  @file ShellChanger.h
  @brief header for emergeCore
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef __GUARD_2a2a38bf_054e_4261_8ba5_df3b0c4278d5
#define __GUARD_2a2a38bf_054e_4261_8ba5_df3b0c4278d5

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <map>
#include <string>
#include "resource.h"
#include "Settings.h"

#ifndef RRF_RT_ANY
#define RRF_RT_ANY 0x0000ffff
#endif

#ifndef RRF_RT_DWORD
#define RRF_RT_DWORD 0x00000018
#endif

#ifndef RRF_RT_QWORD
#define RRF_RT_QWORD 0x00000048
#endif

#ifndef RRF_RT_REG_BINARY
#define RRF_RT_REG_BINARY 0x00000008
#endif

#ifndef RRF_RT_REG_DWORD
#define RRF_RT_REG_DWORD 0x00000010
#endif

#ifndef RRF_RT_REG_EXPAND_SZ
#define RRF_RT_REG_EXPAND_SZ 0x00000004
#endif

#ifndef RRF_RT_REG_MULTI_SZ
#define RRF_RT_REG_MULTI_SZ 0x00000020
#endif

#ifndef RRF_RT_REG_NONE
#define RRF_RT_REG_NONE 0x00000001
#endif

#ifndef RRF_RT_REG_QWORD
#define RRF_RT_REG_QWORD 0x00000040
#endif

#ifndef RRF_RT_REG_SZ
#define RRF_RT_REG_SZ 0x00000002
#endif

typedef std::map<std::wstring, std::wstring> EmergeShellItemMap;
typedef std::pair<std::wstring, std::wstring> EmergeShellItem;

class ShellChanger
{
public:
  ShellChanger(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings);
  ~ShellChanger();
  int Show();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoShellCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
  EmergeShellItemMap shellMap;
  bool CheckFields(HWND hwndDlg);
  void PopulateShells(HWND shellWnd);
  void UpdateFields(HWND hwndDlg);
  bool EnableFields(HWND hwndDlg, bool enable);
  bool DoAddShell(HWND hwndDlg);
  bool DoSaveShell(HWND hwndDlg);
  bool DoBrowseShell(HWND hwndDlg);
  bool WriteShells(HWND hwndDlg);
  bool DoDelShell(HWND hwndDlg);
  bool DeleteShell(HWND hwndDlg, WCHAR* name);
  bool DoSetShell(HWND hwndDlg);
  bool DoEditShell(HWND hwndDlg);
  bool DoAbortShell(HWND hwndDlg);
  int GetShellCommand(HWND hwndDlg, WCHAR* name, WCHAR* command);
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd;
  bool edit;
  HICON addIcon, editIcon, delIcon, saveIcon, abortIcon, browseIcon, changeIcon;
  static BOOL CALLBACK ShellDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  std::wstring xmlFile;
};

#endif
