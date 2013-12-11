/*!
  @file FileSystemOps.h
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

#ifndef __GUARD_8f6e4c1f_de2d_4ffe_9213_207e5915699a
#define __GUARD_8f6e4c1f_de2d_4ffe_9213_207e5915699a

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <map>
#include <string>
#include <vector>
#include "../emergeCoreLib.h"
#include "../emergeFileRegistryLib.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "../emergeLibGlobals.h"

#ifndef SEE_MASK_NOASYNC
#define SEE_MASK_NOASYNC 0x00000100
#endif

typedef DWORD (WINAPI* fnWNetGetConnection)(LPCTSTR, LPTSTR, LPDWORD);
static fnWNetGetConnection MSWNetGetConnection = NULL;

//Helper functions
bool IsAlias(std::wstring filePath);
bool IsCLSID(std::wstring filePath);
bool IsShortcut(std::wstring filePath);
bool IsURL(std::wstring filePath);
std::wstring FindFileOnPATH(std::wstring path);
std::wstring FindFilePathFromRegistry(std::wstring filePath);
std::wstring CleanPath(std::wstring filePath);
std::wstring GetSpecialFolderGUID(int folderID);
std::map<int, std::wstring> GetSpecialFolderMap();
bool Execute(std::wstring application, std::wstring workingDir = TEXT(""), int nShow = SW_SHOW, std::wstring verb = TEXT(""));
bool ExecuteAlias(std::wstring alias);
bool ExecuteSpecialFolder(std::wstring folder);
std::wstring stripSpecialChars(std::wstring filePath);

#endif
