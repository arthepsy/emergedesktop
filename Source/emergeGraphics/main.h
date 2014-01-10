/*!
  @file main.h
  @brief header for emergeGraphics
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

#ifndef __GUARD_aeffdc86_e182_446c_be2d_5a6ac51f2205
#define __GUARD_aeffdc86_e182_446c_be2d_5a6ac51f2205

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

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

#define ICON_LOOKUP_TIMEOUT 1000

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <objbase.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "BImage.h"
#include "emergeGraphics.h"
#include "resource.h"

//GetVersionEx is deprecated in Visual Studio, to be replaced by IsWindows* functions. It's supposed to be deprecated
//in other compilers too, but so far only VS has the replacements implemented. So we'll have to roll our own for other
//compilers until GCC gets around to adding proper support.
#ifdef _MSC_VER
#include <versionhelpers.h>
#else
inline bool IsWindows7OrGreater()
{
  OSVERSIONINFO osv;

  ZeroMemory(&osv, sizeof(osv));

  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);

  return ((osv.dwMajorVersion + (osv.dwMinorVersion / 10.0)) >= 6.1);
}
#endif

#ifndef GIL_DEFAULTICON
#define GIL_DEFAULTICON 64
#endif

typedef UINT (WINAPI* fnPrivateExtractIcons)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT);
static fnPrivateExtractIcons MSPrivateExtractIcons = NULL;

typedef UINT (WINAPI* fnPickIcon)(HWND, WCHAR*, UINT, int*);
static fnPickIcon MSPickIcon = NULL;

typedef HRESULT (WINAPI* fnDwmIsCompositionEnabled)(BOOL*);
static fnDwmIsCompositionEnabled MSDwmIsCompositionEnabled = NULL;

typedef HRESULT (WINAPI* fnDwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);
static fnDwmEnableBlurBehindWindow MSDwmEnableBlurBehindWindow = NULL;

typedef HRESULT (WINAPI* fnDwmRegisterThumbnail)(HWND, HWND, PHTHUMBNAIL);
static fnDwmRegisterThumbnail MSDwmRegisterThumbnail = NULL;

typedef HRESULT (WINAPI* fnDwmUpdateThumbnailProperties)(HTHUMBNAIL, const DWM_THUMBNAIL_PROPERTIES*);
static fnDwmUpdateThumbnailProperties MSDwmUpdateThumbnailProperties = NULL;

typedef HRESULT (WINAPI* fnDwmUnregisterThumbnail)(HTHUMBNAIL);
static fnDwmUnregisterThumbnail MSDwmUnregisterThumbnail = NULL;

typedef HRESULT (WINAPI* fnDwmQueryThumbnailSourceSize)(HTHUMBNAIL, PSIZE);
static fnDwmQueryThumbnailSourceSize MSDwmQueryThumbnailSourceSize = NULL;

// Globals
static HMODULE dwmapiDLL = NULL;
static HMODULE shell32DLL = NULL;
static HMODULE user32DLL = NULL;

#endif
