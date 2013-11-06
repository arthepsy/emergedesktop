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

#ifndef __EV_APPLET_H
#define __EV_APPLET_H

// Define required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

// Define required for window transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#define TASK_WAIT_TIME 250

#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include <vector>
#include <string>
#include <process.h>
#include "Task.h"
#include "Config.h"
#include "Settings.h"
#include "../emergeBaseClasses/BaseApplet.h"

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  std::vector< std::tr1::shared_ptr<Task> > taskList;
  int currentRow, currentColumn;
  bool lButtonDown;
  HWND oldActiveWindow, selectedWindow, oldTipWindow;
  TOOLINFO ti;
  POINT referencePt;
  void MakeCurrent(HWND taskWnd);
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  static BOOL CALLBACK EnumTasksList(HWND hwnd, LPARAM lParam);
  static BOOL CALLBACK BuildTasksList(HWND hwnd, LPARAM lParam);
  static BOOL CALLBACK EnumFullScreenWindows(HWND hwnd, LPARAM lParam);
  bool PaintTask(Task* task);
  LRESULT PaintContent(HDC hdc, RECT clientrt);
  void ShowConfig();
  LRESULT MySize();
  HANDLE taskThread;
  static DWORD WINAPI TaskThreadProc(LPVOID lpParameter);
  void RemoveInvalidTasks();

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoNotify(HWND hwnd, LPARAM lParam);
  LRESULT DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT DesktopMouseEvent(HWND hwnd, UINT message, LPARAM lParam);
  void AppletUpdate();
  bool IsWindowValidTask(HWND hwnd);
  void SwitchDesktop(int row, int column, bool gather);
  LRESULT DoCopyData(COPYDATASTRUCT* cds);
  bool AddTasks(HWND hwnd);
  bool GetTaskRowColumn(HWND hwnd, int* row, int* column);
  GUIINFO GetGUIInfo();
};
#endif

