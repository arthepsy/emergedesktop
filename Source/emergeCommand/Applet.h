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

#ifndef __ECM_APPLET_H
#define __ECM_APPLET_H

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

#include <locale.h>
#include <vector>
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"
#include "Config.h"
#include "Command.h"

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  std::tr1::shared_ptr<Command> pCommand;
  std::wstring commandText;
  HFONT mainFont;
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  void ShowConfig();
  GUIINFO GetGUIInfo();
  UINT Initialize();
  LRESULT DoMove();
  LRESULT DoTimer(UINT idEvent);
  void AppletUpdate();
  void WriteAppletSettings();
  LRESULT DoButtonDown(UINT message, WPARAM wParam, LPARAM lParam);
  LRESULT ShowCommand();
  void SetCommandText(std::wstring commandText);
  void UpdateTip(std::wstring tip);
  LRESULT PaintContent(HDC hdc, RECT clientrt);
  void Activate();
  void Show();
};

#endif
