//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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

#ifndef __EC_DESKTOP_H
#define __EC_DESKTOP_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#undef __MSVCRT_VERSION__
#define __MSVCRT_VERSION__ 0x0601

#include "MessageControl.h"
#include <deque>
#include <wchar.h>

#define BACKGROUND_TIMER         1
#define BACKGROUND_POLL_INTERVAL 1000

class Desktop
{
private:
  std::tr1::shared_ptr<MessageControl> pMessageControl;
  std::deque<HWND> minimizedWindowDeque;
  HINSTANCE mainInst;
  HWND mainWnd;
  bool registered, explorerDesktop;
  UINT ShellMessage;
  bool SetBackgroundImage();
  __time64_t modifyTime;
  WCHAR bgImage[MAX_PATH];
  static LRESULT CALLBACK DesktopProcedure (HWND, UINT, WPARAM, LPARAM);
  static VOID CALLBACK DesktopTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
  static BOOL CALLBACK SetMonitorArea(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
  static BOOL CALLBACK MinimizeWindowsEnum(HWND hwnd, LPARAM lParam);
  static DWORD WINAPI WallpaperThreadProc(LPVOID lpParameter);
  HANDLE wallpaperThread;

public:
  Desktop(HINSTANCE hInstance, std::tr1::shared_ptr<MessageControl> pMessageControl);
  ~Desktop();
  bool Initialize(bool explorerDesktop);
  void ShowMenu(UINT menu);
  void DoWindowPosChanging(LPWINDOWPOS winPos);
  void ToggleDesktop();
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DoTimer(UINT_PTR timerID);
  LRESULT DoDisplayChange(HWND hwnd);
  void ShowDesktop(bool show);
  BOOL InvalidateDesktop();
};

#endif

