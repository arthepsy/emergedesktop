//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

#ifndef __GUARD_103666e8_fc94_48ec_ae0d_e0348628b3ed
#define __GUARD_103666e8_fc94_48ec_ae0d_e0348628b3ed

#define UNICODE 1

#ifdef EMERGEAPPLETENGINE_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#ifndef TIP_SIZE
#define TIP_SIZE 256
#endif

#define ASI_VERTICAL    1
#define ASI_HORIZONTAL  2
#define ASI_LEFT        10
#define ASI_RIGHT       11
#define ASI_UP          12
#define ASI_DOWN        13
#define ASI_MIDDLE      14
#define ASI_CENTER      15

#include <windows.h>
#include <string>

typedef struct _COMMONSETTINGS
{
  int x;
  int y;
  int height;
  int width;
}
COMMONSETTINGS, *LPCOMMONSETTINGS;

typedef struct _AUTOSIZEINFO
{
  HWND hwnd;
  RECT* rect;
  RECT titleBarRect;
  int dragBorder;
  UINT visibleIconCount;
  UINT iconSize;
  UINT iconSpacing;
  UINT orientation;
  UINT verticalDirection;
  UINT horizontalDirection;
  UINT limit;
}
AUTOSIZEINFO, *LPAUTOSIZEINFO;

// Declaration of functions to import
DLL_EXPORT LRESULT EAEHitTest(HWND hwnd, int guiBorder, bool autoSize, int x, int y);
DLL_EXPORT HWND EAEUpdateGUI(HWND hwnd, bool shadow, std::wstring zposition);
DLL_EXPORT HWND EAEInitializeAppletWindow(HINSTANCE inst, WNDPROC windowProc, LPVOID lpParam);
DLL_EXPORT HWND EAEInitializeTooltipWindow(HINSTANCE appletInstance);
DLL_EXPORT bool EAEAutoSize(AUTOSIZEINFO autoSizeInfo);
DLL_EXPORT LRESULT EAEDisplayChange(HMONITOR appletMonitor, RECT* wndRect, RECT* oldDeskRect);
DLL_EXPORT int EAEDisplayMenu(HWND callingWnd, HWND taskWnd);
DLL_EXPORT int EAEDisplayFileMenu(WCHAR* file, HWND callingWnd);

#endif
