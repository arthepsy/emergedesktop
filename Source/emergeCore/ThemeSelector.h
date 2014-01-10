/*!
  @file ThemeSelector.h
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

#ifndef __GUARD_9f36cb48_edd8_46f3_8b4d_5b41e5cb8546
#define __GUARD_9f36cb48_edd8_46f3_8b4d_5b41e5cb8546

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
#include <shlobj.h>
#include "../emergeLib/emergeWindowLib.h"
#include "resource.h"
#include "ThemeSaver.h"

class ThemeSelector
{
public:
  ThemeSelector(HINSTANCE hInstance, HWND mainWnd);
  ~ThemeSelector();
  int Show();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoThemeCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);

private:
  std::tr1::shared_ptr<ThemeSaver> pThemeSaver;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd;
  HICON saveasIcon, saveIcon, delIcon, exportIcon, importIcon;
  void PopulateThemes(HWND themeWnd, WCHAR* currentTheme);
  bool SaveTheme(HWND hwndDlg);
  void DoSaveAs(HWND hwndDlg);
  void DoDelTheme(HWND hwndDlg);
  void DoSave(HWND hwndDlg);
  void DoExport(HWND hwndDlg);
  void DoImport(HWND hwndDlg);
  static BOOL CALLBACK ThemeSelectorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  BOOL DoThemeCheck(HWND hwndDlg);
};

#endif
