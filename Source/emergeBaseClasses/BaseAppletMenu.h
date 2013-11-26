/*!
  @file BaseAppletMenu.h
  @brief header for emergeBaseClasses
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

#ifndef __GUARD_3da397e6_a78e_418a_81ee_90af15e66f41
#define __GUARD_3da397e6_a78e_418a_81ee_90af15e66f41

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#define EBC_ABOUT         100
#define EBC_LOADSTYLE     200
#define EBC_RELOADSTYLE   202
#define EBC_CONFIGURE     400
#define EBC_EXIT          500
#define EBC_NEWINSTANCE   600
#define EBC_DELETEINSTANCE 602

#include <windows.h>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeUtilityLib.h"

class DLL_EXPORT BaseAppletMenu
{
public:
  BaseAppletMenu(HWND mainWnd, HINSTANCE hInstance, WCHAR* appletName, bool allowMultipleInstances);
  ~BaseAppletMenu();
  void Initialize();
  void BuildMenu();
  DWORD ActivateMenu(int x, int y, WCHAR* styleFile);
  void UpdateHook(DWORD menuAlpha);

private:
  HWND mainWnd;
  HMENU appletMenu;
  HHOOK menuHook;
  HINSTANCE hInstance;
  WCHAR appletName[MAX_LINE_LENGTH];
  bool allowMultipleInstances;
  static LRESULT CALLBACK HookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif
