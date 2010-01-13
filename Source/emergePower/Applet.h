//---
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
//---

#ifndef __APPLET_H
#define __APPLET_H

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

// Defines required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

#include <vector>
#include <process.h>
#include "../emergeSchemeEngine/emergeSchemeEngine.h"
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"
#include "Settings.h"
#include "Config.h"
#include <commctrl.h>

#define TIP_SIZE 256

#define MODIFY_POLL_TIME  100

class Applet: public BaseApplet
{
private:
  HFONT mainFont;
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  SYSTEM_POWER_STATUS status;
  void UpdateStatus();
  void DrawStatusChar(HDC& hdc, WCHAR *text, RECT &clientrt);
  std::tr1::shared_ptr<Settings> pSettings;

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoTimer(UINT_PTR timerID);
  LRESULT DoEmergeNotify(UINT messageClass, UINT message);
  void AppletUpdate();
  void ShowConfig();
  LRESULT PaintContent(HDC hdc, RECT clientrt);
};

#endif
