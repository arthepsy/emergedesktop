//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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

#ifndef __SHELL_H
#define __SHELL_H

#include "../emergeLib/emergeLib.h"
#include <vector>
#include <string>
#include <regstr.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <process.h>

#define MAX_HANDLES 500

//====================
// Methods are variables used by Emerge Desktop
class Shell
{
public:
  Shell();
  ~Shell();
  bool Initialize();
  bool RunRegEntries(HKEY key, bool clearEntry, bool wait);
  void RunRegStartup();
  void RunFolderEntries(LPTSTR path);
  void RunFolderStartup();
  bool FirstRunCheck();
  bool UpdateTaskCount(UINT message, UINT shellMessage, HWND task);
  void ClearSessionInformation();
  void RegisterShell(HWND hwnd, bool enable);
  void BuildTaskList();
  void CountTask(HWND hwnd);

private:
  std::vector<HWND> taskList;
  UINT ShellMessage;
  bool UpdateSessionInformation(bool add, HWND task);
  static BOOL CALLBACK GetTaskCount(HWND hwnd, LPARAM lParam);
};

#endif
