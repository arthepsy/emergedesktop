//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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
//---
//
// Note: The task menu code was derived from BBSystemBar which is part of the
// BB4Win source code, copyright 2001-2004 The Blackbox for Windows Development
// Team.
//
//-----

#ifndef __TASK_H
#define __TASK_H

//-----
//
// Note: for MSVC users, put the AggressiveOptimize.h header file (available from
// http://www.nopcode.com) in your \include directory.  It cuts down on executable
// filesize.
//
//-----
#include "Settings.h"
#include <commctrl.h>
#include <stdio.h>

// Define icon and tip settings
#define TIP_SIZE 256

#define REFRESH_TIMER 1
#define REFRESH_POLL_TIME 100

//====================
// The Task Class
class Task
{
public:
  Task(HWND taskWnd, HWND mainWnd, HINSTANCE mainInst, UINT currentRow, UINT currentColumn,
       UINT maxRows, UINT maxColumns, GUIINFO guiInfo);
  ~Task();

  RECT *GetRect();
  HWND GetTaskWnd();
  void DisplayMenu(HWND callingWnd);
  RECT *GetReferenceRect();
  void SetReferenceRect(RECT referenceRect);
  void UpdateDimensions(UINT currentColumn, UINT currentRow, UINT maxColumns, UINT maxRows, GUIINFO guiInfo);
  WCHAR *GetAppName();
  void SetMinimized(bool minimized);
  bool GetMinimized();

private:
  HWND taskWnd, mainWnd;
  HINSTANCE mainInst;
  RECT taskRect, referenceRect;
  WCHAR appName[MAX_PATH];
  bool minimized;
};

//====================

#endif

