/*!
  @file emergeTrayExplorerHook.h
  @brief export header for the emergeTray Explorer tray hook
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2011  The Emerge Desktop Development Team

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

#ifndef _EMERGETRAYEXPLORERHOOK_H
#define _EMERGETRAYEXPLORERHOOK_H

#ifdef EMERGETRAYEXPLORERHOOK_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

/*!
  @def UNICODE
  @brief Define to use UNICODE versions of functions
  */
#define UNICODE 1

#include <windows.h>

HWND trayMsgHandler;
HINSTANCE hInst;
LONG_PTR oldTaskbarWndProc, oldNotifyWndProc, oldTrayWndProc, oldClockWndProc, oldRebarWndProc, oldTaskWndProc;

static const UINT TRAYHOOK_MSGPROC_ATTACH = RegisterWindowMessage(TEXT("TrayHook_MsgProc_Attach"));

#endif
