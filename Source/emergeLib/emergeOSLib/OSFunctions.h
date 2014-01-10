/*!
  @file OSFunctions.h
  @brief internal header for emergeLib
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

#ifndef __GUARD_62839a19_87ed_44aa_bf9b_512822d9fe99
#define __GUARD_62839a19_87ed_44aa_bf9b_512822d9fe99

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

#include <windows.h>
#include <docobj.h>
#include <powrprof.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include "../emergeCoreLib.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "../emergeLibGlobals.h"
#include "Shutdown.h"

// MS Run dialog
typedef void (__stdcall* lpfnMSRun)(HWND, HICON, LPCSTR, LPCSTR, LPCSTR, int);
static lpfnMSRun MSRun = NULL;

// MS RegisterShellHookWindow
typedef BOOL (WINAPI* lpfnMSRegisterShellHookWindow)(HWND hWnd, DWORD method);
static lpfnMSRegisterShellHookWindow MSRegisterShellHookWindow = NULL;

typedef void* (WINAPI* fnSHLockShared)(HANDLE, DWORD);
static fnSHLockShared MSSHLockShared = NULL;

typedef BOOL (WINAPI* fnSHUnlockShared)(void*);
static fnSHUnlockShared MSSHUnlockShared = NULL;

//Helper functions
HRESULT CLSIDToString(REFCLSID rclsid, LPTSTR ptzBuffer, size_t cchBuffer);

#endif
