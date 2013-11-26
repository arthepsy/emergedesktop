/*!
  @file Environment.h
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

#ifndef __GUARD_33f25978_f6e8_468d_a643_b07a89b87b61
#define __GUARD_33f25978_f6e8_468d_a643_b07a89b87b61

#define UNICODE 1

#undef WINVER
#define WINVER 0x0600

#define MAX_LINE_LENGTH 4096

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#endif

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <array>
#include <string>
#include "../emergeCoreLib.h"
#include "../emergeFileRegistryLib.h"
#include "../emergeLibGlobals.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "../emergeWindowLib.h"
#include "../../tinyxml/tinyxml.h"

typedef BOOL (WINAPI* lpfnIsWow64Process)(HANDLE, PBOOL);
static lpfnIsWow64Process MSIsWow64Process = NULL;

//Helper functions
std::wstring GetCustomDataPath();

#endif
