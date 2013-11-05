//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2012  The Emerge Desktop Development Team
//
//  Emerge Desktop is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  Emerge Desktop is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//----  --------------------------------------------------------------------------------------------------------

#ifndef __EC_SHELL_H
#define __EC_SHELL_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x501

#include <vector>
#include <string>
#include <windows.h>
#include <regstr.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <process.h>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../emergeLib/emergeWindowLib.h"

#define MAX_HANDLES 500

struct StartKey
{
    HKEY key;
    std::wstring subkey;
    bool clear;
    StartKey(HKEY K, const WCHAR *S, bool C)
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
  IOleCommandTarget *traySSO;
  std::vector<HWND> taskList;
  UINT ShellMessage;
  bool UpdateSessionInformation(bool add, HWND task);
  static BOOL CALLBACK GetTaskCount(HWND hwnd, LPARAM lParam);
};

#endif
