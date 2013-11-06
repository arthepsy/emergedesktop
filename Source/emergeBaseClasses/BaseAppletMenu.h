//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2012  The Emerge Desktop Development Team
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
//---

#ifndef __EB_BASEAPPLETMENU_H
#define __EB_BASEAPPLETMENU_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

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
