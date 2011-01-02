// vim:tags+=../emergeLib/tags
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

#ifndef __EC_THEMESAVER_H
#define __EC_THEMESAVER_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#define UNICODE 1

#include "../emergeLib/emergeLib.h"
#include <commctrl.h>
#include <shlwapi.h>
#include "resource.h"

class ThemeSaver
{
public:
  ThemeSaver(HINSTANCE hInstance, HWND mainWnd);
  ~ThemeSaver();
  int Show(WCHAR *theme);
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoThemeCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);

private:
  WCHAR title[MAX_LINE_LENGTH];
  WCHAR theme[MAX_PATH];
  HINSTANCE hInstance;
  HWND mainWnd;
  void PopulateThemes(HWND themeWnd);
  bool SaveTheme(HWND hwndDlg);
  static BOOL CALLBACK ThemeSaverDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
