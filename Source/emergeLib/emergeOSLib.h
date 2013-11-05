/*!
  @file emergeOSLib.h
  @brief export header for emergeOSLib
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

#ifndef __EMERGEOSLIB_H
#define __EMERGEOSLIB_H

#define UNICODE 1

#ifdef EMERGELIB_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#include <windows.h>
#include <docobj.h>
#include <string>

typedef enum _EXITFLAGS
{
  EMERGE_SUSPEND = 100,
  EMERGE_HIBERNATE,
  EMERGE_DISCONNECT,
  EMERGE_HALT,
  EMERGE_REBOOT,
  EMERGE_LOGOFF
}
EXITFLAGS;

typedef enum _RSHFLAGS
{
  RSH_UNREGISTER = 0,
  RSH_REGISTER,
  RSH_PROGMAN,
  RSH_TASKMGR
}
RSHFLAGS;

//OSFunctions.h
DLL_EXPORT HMODULE ELGetSystemLibrary(std::wstring library);
DLL_EXPORT HMODULE ELGetEmergeLibrary(std::wstring library);
DLL_EXPORT HMODULE ELLoadSystemLibrary(std::wstring library);
DLL_EXPORT HMODULE ELLoadEmergeLibrary(std::wstring library);

DLL_EXPORT void *ELLockShared(HANDLE sharedMem, DWORD processID);
DLL_EXPORT bool ELUnlockShared(void *sharedPtr);

DLL_EXPORT bool ELDisplayRunDialog();
DLL_EXPORT bool ELQuit(bool prompt);
DLL_EXPORT bool ELDisplayShutdownDialog(HWND wnd);
DLL_EXPORT bool ELExit(EXITFLAGS uFlag, bool prompt);

DLL_EXPORT HANDLE ELActivateActCtxForDll(LPCTSTR pszDll, PULONG_PTR pulCookie);
DLL_EXPORT HANDLE ELActivateActCtxForClsid(REFCLSID rclsid, PULONG_PTR pulCookie);
DLL_EXPORT void ELDeactivateActCtx(HANDLE hActCtx, ULONG_PTR* pulCookie);

DLL_EXPORT IOleCommandTarget *ELStartSSO(CLSID clsid);

DLL_EXPORT bool ELRegisterShellHook(HWND hwnd, RSHFLAGS method);

DLL_EXPORT bool ELIsKeyDown(UINT virtualKey);

//SystemProcessInfo.h
DLL_EXPORT double ELOSVersionInfo();

DLL_EXPORT std::wstring ELGetProcessIDApp(DWORD processID, bool fullName);
DLL_EXPORT std::wstring ELGetWindowApp(HWND hWnd, bool fullName);

#endif // __EMERGEOSLIB_H
