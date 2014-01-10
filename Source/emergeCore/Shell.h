/*!
  @file Shell.h
  @brief header for emergeCore
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

#ifndef __GUARD_ea9914d9_ec5c_486a_aa8b_394a67bacc0e
#define __GUARD_ea9914d9_ec5c_486a_aa8b_394a67bacc0e

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#define MAX_HANDLES 500

#include <windows.h>
#include <process.h>
#include <regstr.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"

struct StartKey
{
  HKEY key;
  std::wstring subkey;
  bool clear;
  StartKey(HKEY K, const WCHAR* S, bool C)
  {
    key = K;
    subkey = S;
    clear = C;
  }
};

//====================
// Methods are variables used by Emerge Desktop
class Shell
{
public:
  Shell();
  ~Shell();
  bool RunRegEntries(HKEY key, bool clearEntry, bool showStartupErrors);
  void RunRegStartup(bool showStartupErrors);
  void RunFolderEntries(LPTSTR path, bool showStartupErrors);
  void RunFolderStartup(bool showStartupErrors);
  bool FirstRunCheck();
  bool UpdateTaskCount(UINT message, UINT shellMessage, HWND task);
  void ClearSessionInformation();
  void RegisterShell(HWND hwnd, bool enable);
  void BuildTaskList();
  void CountTask(HWND hwnd);
  void UnloadSSO();
  void LoadSSO();
  LRESULT HideExplorerBar();

private:
  IOleCommandTarget* traySSO;
  std::vector<HWND> taskList;
  UINT ShellMessage;
  bool UpdateSessionInformation(bool add, HWND task);
  static BOOL CALLBACK GetTaskCount(HWND hwnd, LPARAM lParam);
};

#endif
