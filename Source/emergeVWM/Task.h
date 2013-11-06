/*!
  @file Task.h
  @brief header for emergeVWM
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

  @note The task menu code was derived from BBSystemBar which is part of the
  BB4Win source code, copyright 2001-2004 The Blackbox for Windows Development
  Team.
  */

#ifndef __GUARD_7723ae32_0d0f_42e9_9388_674712f7b3fe
#define __GUARD_7723ae32_0d0f_42e9_9388_674712f7b3fe

#define UNICODE 1

// Define icon and tip settings
#define TIP_SIZE 256

#define REFRESH_TIMER 1
#define REFRESH_POLL_TIME 100

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "Settings.h"

//====================
// The Task Class
class Task
{
public:
  Task(HWND taskWnd, HWND mainWnd, HINSTANCE mainInst, UINT currentRow, UINT currentColumn,
       UINT maxRows, UINT maxColumns, GUIINFO guiInfo);
  ~Task();

  RECT* GetRect();
  HWND GetTaskWnd();
  void DisplayMenu(HWND callingWnd);
  RECT* GetReferenceRect();
  void SetReferenceRect(RECT referenceRect);
  void UpdateDimensions(UINT currentColumn, UINT currentRow, UINT maxColumns, UINT maxRows, GUIINFO guiInfo);
  std::wstring GetAppName();
  void SetMinimized(bool minimized);
  bool GetMinimized();

private:
  HWND taskWnd, mainWnd;
  HINSTANCE mainInst;
  RECT taskRect, referenceRect;
  std::wstring appName;
  bool minimized;
};

//====================

#endif
