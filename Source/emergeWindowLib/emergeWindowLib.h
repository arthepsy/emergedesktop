/*!
  @file emergeWindowLib.h
  @brief export header for emergeWindowLib
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

#ifndef __EMERGEWINDOWLIB_H
#define __EMERGEWINDOWLIB_H

#define UNICODE 1

#ifdef EMERGEWINDOWLIB_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#include <windows.h>

/*!
  @struct _SNAPMOVEINFO
  @brief structure that contains information about a window to be 'snapped' when
  moving
  */
typedef struct _SNAPMOVEINFO
{
  HWND AppletWindow;
  POINT origin;
  RECT *AppletRect;
}
SNAPMOVEINFO, *LPSNAPMOVEINFO;

/*!
  @struct _SNAPSIZEINFO
  @brief structure that contains information about a window to be 'snapped' when
  being sized
  */
typedef struct _SNAPSIZEINFO
{
  HWND AppletWindow;
  UINT AppletEdge;
  RECT *AppletRect;
}
SNAPSIZEINFO, *LPSNAPSIZEINFO;

DLL_EXPORT HWND ELGetCoreWindow();
DLL_EXPORT HWND ELGetDesktopWindow();

DLL_EXPORT RECT ELGetMonitorRect(int monitor);
DLL_EXPORT HMONITOR ELGetDesktopRect(RECT *appletRect, RECT *rect);
DLL_EXPORT RECT ELGetWindowRect(HWND hwnd);

DLL_EXPORT bool ELStealFocus(HWND wnd);
DLL_EXPORT bool ELSwitchToThisWindow(HWND wnd);
DLL_EXPORT bool ELSetForeground(HWND wnd);

DLL_EXPORT bool ELIsFullScreen(HWND appletWnd);
DLL_EXPORT bool ELIsModal(HWND window);
DLL_EXPORT bool ELIsValidTaskWindow(HWND hwnd);
DLL_EXPORT bool ELIsApplet(HWND hwnd);
DLL_EXPORT bool ELIsExplorer(HWND hwnd);

DLL_EXPORT POINT ELGetAnchorPoint(HWND hwnd);
DLL_EXPORT int ELGetAppletMonitor(HWND hwnd);

DLL_EXPORT bool ELSnapMove(LPSNAPMOVEINFO snapMoveInfo);
DLL_EXPORT bool ELSnapMoveToDesk(RECT *AppletRect);
DLL_EXPORT bool ELSnapSize(LPSNAPSIZEINFO snapSizeInfo);

#endif // __EMERGEWINDOWLIB_H
