/*!
  @file WindowOps.h
  @brief internal header for emergeLib
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

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

#ifndef __GUARD_d276c2ac_2003_436b_8893_d2f356478aa7
#define __GUARD_d276c2ac_2003_436b_8893_d2f356478aa7

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>
#include "../emergeLibGlobals.h"
#include "../emergeWindowLib.h"

typedef struct _APPLETMONITORINFO
{
  HWND appletWnd;
  HMONITOR appletMonitor;
  int appletMonitorNum;
  RECT appletMonitorRect;
} APPLETMONITORINFO, *LPAPPLETMONITORINFO;

typedef HRESULT (WINAPI* fnDwmIsCompositionEnabled)(BOOL*);
static fnDwmIsCompositionEnabled MSDwmIsCompositionEnabled = NULL;

typedef HRESULT (WINAPI* fnDwmGetWindowAttribute)(HWND, DWORD, PVOID, DWORD);
static fnDwmGetWindowAttribute MSDwmGetWindowAttribute = NULL;

// MS SwitchToThisWindow
typedef void (__stdcall* lpfnMSSwitchToThisWindow)(HWND, BOOL);
static lpfnMSSwitchToThisWindow MSSwitchToThisWindow = NULL;

typedef enum _DWMWINDOWATTRIBUTE
{
  DWMWA_NCRENDERING_ENABLED           = 1,
  DWMWA_NCRENDERING_POLICY,
  DWMWA_TRANSITIONS_FORCEDISABLED,
  DWMWA_ALLOW_NCPAINT,
  DWMWA_CAPTION_BUTTON_BOUNDS,
  DWMWA_NONCLIENT_RTL_LAYOUT,
  DWMWA_FORCE_ICONIC_REPRESENTATION,
  DWMWA_FLIP3D_POLICY,
  DWMWA_EXTENDED_FRAME_BOUNDS,
  DWMWA_HAS_ICONIC_BITMAP,
  DWMWA_DISALLOW_PEEK,
  DWMWA_EXCLUDED_FROM_PEEK,
  DWMWA_LAST
} DWMWINDOWATTRIBUTE;

static int enumCount = 0;

//Helper functions
bool IsClose(int side, int edge);
bool SnapMoveToEdge(LPSNAPMOVEINFO snapMove, RECT rt);
bool SnapSizeToEdge(LPSNAPSIZEINFO snapSize, RECT rt);
BOOL CALLBACK AppletMonitorEnum(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK FullscreenEnum(HWND hwnd, LPARAM lParam);
BOOL CALLBACK MonitorRectEnum(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK SnapMoveEnum(HWND hwnd, LPARAM lParam);
BOOL CALLBACK SnapSizeEnum(HWND hwnd, LPARAM lParam);
#endif
