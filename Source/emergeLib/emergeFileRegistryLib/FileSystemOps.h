/*!
  @file FileSystemOps.h
  @brief export header for emergeLib
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

#ifndef __FILESYSTEMOPS_H
#define __FILESYSTEMOPS_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <map>
#include <string>
#include <vector>
#include "../emergeCoreLib.h"
#include "../emergeFileRegistryLib.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "../emergeLibGlobals.h"

#define SI_PATH       0x01
#define SI_WORKINGDIR 0x02
#define SI_ARGUMENTS  0x04
#define SI_SHOW       0x08
#define SI_ICONPATH   0x10
#define SI_ICONINDEX  0x20
#define SI_RUNAS      0x40
#define SI_ALL        SI_PATH|SI_WORKINGDIR|SI_ARGUMENTS|SI_SHOW|SI_ICONPATH|SI_ICONINDEX|SI_RUNAS

#ifndef SEE_MASK_NOASYNC
#define SEE_MASK_NOASYNC 0x00000100
#endif

typedef DWORD (WINAPI *fnWNetGetConnection)(LPCTSTR, LPTSTR, LPDWORD);
static fnWNetGetConnection MSWNetGetConnection = NULL;

//Helper functions
bool IsCLSID(std::wstring filePath);
bool IsDirectory(std::wstring filePath);
bool IsShortcut(std::wstring filePath);
bool IsURL(std::wstring filePath);
std::wstring FindFileOnPATH(std::wstring path);
std::wstring FindFilePathFromRegistry(std::wstring filePath);
std::wstring CleanPath(std::wstring filePath);
std::wstring GetSpecialFolderGUID(int folderID);
std::map<int, std::wstring> GetSpecialFolderMap();
bool Execute(std::wstring application, std::wstring workingDir = TEXT(""), int nShow = SW_SHOW);
bool ExecuteSpecialFolder(std::wstring folder);

#endif
