/*!
  @file emergeTrayExplorerHook.h
  @brief export header for the emergeTray Explorer tray hook
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

#ifndef __GUARD_9af480de_1b6e_455b_8b99_2691f8d399a3
#define __GUARD_9af480de_1b6e_455b_8b99_2691f8d399a3

/*!
  @def UNICODE
  @brief Define to use UNICODE versions of functions
  */
#define UNICODE 1

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

#include <windows.h>

HWND trayMsgHandler;
HINSTANCE hInst;

extern "C" LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam);
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL UNUSED, DWORD fdwReason, LPVOID lpvReserved UNUSED);

static const UINT TRAYHOOK_MSGPROC_ATTACH = RegisterWindowMessage(TEXT("TrayHook_MsgProc_Attach"));

#endif
