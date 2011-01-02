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

#ifndef __APPLET_H
#define __APPLET_H

// Defines required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#define MOUSE_TIMER 0
#define MOUSE_POLL_TIME 250
#define ID_CLOCKTIMER 101

#define FULLSCREEN_TIMER 1
#define FULLSCREEN_POLL_TIME 100

#include <vector>
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"
#include "Config.h"

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  WCHAR commandText[MAX_LINE_LENGTH];
  HFONT mainFont;
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  void ShowConfig();
  UINT Initialize();
  LRESULT DoMove();
  void AppletUpdate();
  void WriteAppletSettings();
  LRESULT DoButtonDown(UINT message, WPARAM wParam, LPARAM lParam);
  void UpdateTip(WCHAR *tip);
  LRESULT PaintContent(HDC hdc, RECT clientrt);
};

#endif
