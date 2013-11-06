/*!
  @file ThemeSaver.h
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

#ifndef __GUARD_44b1b6d7_4a76_43ff_9be7_52cd93f039f8
#define __GUARD_44b1b6d7_4a76_43ff_9be7_52cd93f039f8

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "resource.h"

class ThemeSaver
{
public:
  ThemeSaver(HINSTANCE hInstance, HWND mainWnd);
  ~ThemeSaver();
  int Show(WCHAR* theme);
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
