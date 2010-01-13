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

#ifndef __DESKTOP_H
#define __DESKTOP_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#include "MessageControl.h"

#define DESKTOP_TIMER         1
#define DESKTOP_POLL_INTERVAL 100

class Desktop
{
private:
  std::tr1::shared_ptr<MessageControl> pMessageControl;
  HINSTANCE mainInst;
  HWND mainWnd;
  bool registered;
  UINT ShellMessage;
  bool SetBackgroundImage();
  static LRESULT CALLBACK DesktopProcedure (HWND, UINT, WPARAM, LPARAM);
  static VOID CALLBACK DesktopTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  static BOOL CALLBACK SetMonitorArea(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

public:
  Desktop(HINSTANCE hInstance, std::tr1::shared_ptr<MessageControl> pMessageControl);
  ~Desktop();
  bool Initialize();
  LRESULT DoDefault(UINT message, UINT shellMessage, HWND task);
  void ShowMenu(UINT menu);
  void SetWorkArea();
};

#endif

