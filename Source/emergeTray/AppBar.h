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

#ifndef __ETR_APPBAR_H
#define __ETR_APPBAR_H

#define UNICODE 1

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#define MAKEHWND(a) ((HWND)((DWORD_PTR)(a)))
#define MAKEHICON(a) ((HICON)((DWORD_PTR)(a)))
#define MAKEHANDLE(a) ((HANDLE)((DWORD_PTR)(a)))

#include <windows.h>

typedef DWORD HWND32;
typedef DWORD HICON32;
typedef DWORD HANDLE32;

typedef struct
{
  DWORD cbSize;
  HWND32 hWnd;
  UINT uCallbackMessage;
  UINT uEdge;
  RECT rc;
  LPARAM lParam;
}
APPBARDATA_WOW32;

//====================
// The AppBar Class
class AppBar
{
public:
  AppBar(APPBARDATA *pAppBarData);
  ~AppBar();
  bool IsEqual(APPBARDATA *pAppBarData);
  UINT GetEdge();
  RECT GetRect();
  bool SetEdge(UINT edge);
  bool SetRect(RECT &rect);
  bool IsAutoHide();
  HWND GetWnd();

private:
  APPBARDATA localABD;
  HWND localWnd;
  bool localAutoHide;
};

//====================

#endif
