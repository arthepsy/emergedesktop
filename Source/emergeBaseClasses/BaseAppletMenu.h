//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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

#ifndef __BASEAPPLETMENU_H
#define __BASEAPPLETMENU_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#define UNICODE 1

#include "../emergeLib/emergeLib.h"

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#define EBC_ABOUT         100
#define EBC_LOADSCHEME    200
#define EBC_RELOADSCHEME  202
#define EBC_CONFIGURE     400
#define EBC_EXIT          500

class DLL_EXPORT BaseAppletMenu
{
public:
  BaseAppletMenu(HWND mainWnd, HINSTANCE hInstance, WCHAR *appletName);
  ~BaseAppletMenu();
  void Initialize();
  void BuildMenu();
  DWORD ActivateMenu(int x, int y, WCHAR *schemeFile);
  void UpdateHook(DWORD menuAlpha);

private:
  HWND mainWnd;
  HMENU appletMenu;
  HHOOK menuHook;
  HINSTANCE hInstance;
  WCHAR appletName[MAX_LINE_LENGTH];
  static LRESULT CALLBACK HookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif
