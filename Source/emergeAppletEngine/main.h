/*!
  @file main.h
  @brief header for emergeAppletEngine
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

#ifndef __GUARD_11df1d02_602c_475d_b36e_e57e0f717f59
#define __GUARD_11df1d02_602c_475d_b36e_e57e0f717f59

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

#undef _WIN32_IE
#define _WIN32_IE 0x0501

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#undef WINVER
#define WINVER 0x0501

#define SYSMENUMIN 1
#define SYSMENUMAX 0x7ff

// fix for mingw
#include <basetyps.h>
#include <windef.h>
#include <wtypesbase.h>

#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <map>
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "emergeAppletEngine.h"

// Global definitions
typedef struct tagCONTEXTINFO
{
  IContextMenu2* ic2;
  WCHAR value[MAX_PATH];
} CONTEXTINFO;
WCHAR appletClass[ ] = TEXT("EmergeDesktopApplet");
WNDPROC oldWndProc = NULL;
HMENU selectedMenu = NULL;
typedef std::map<HMENU, CONTEXTINFO> ContextMap;
ContextMap contextMap;

// Helper functions
LRESULT CALLBACK MenuProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT AddContextMenu(WCHAR* file);

#endif
