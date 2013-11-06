/*!
  @file Applet.h
  @brief header for emergeLang
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

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
  */

#ifndef __GUARD_cff61a43_2dcd_437e_ab0b_9a2d9405dfb1
#define __GUARD_cff61a43_2dcd_437e_ab0b_9a2d9405dfb1

#define UNICODE 1

// Defines required for tooltip
#undef _WIN32_IE
#define _WIN32_IE 0x0600

// Define required for the Window Transparency
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

// Define required for menu animation
#undef WINVER
#define WINVER 0x0501

#define MAX_LABEL 50

#include <locale.h>
#include <vector>
#include "../emergeAppletEngine/emergeAppletEngine.h"
#include "../emergeBaseClasses/BaseApplet.h"
#include "../emergeStyleEngine/emergeStyleEngine.h"
#include "Config.h"
#include "Settings.h"

class Applet: public BaseApplet
{
private:
  HFONT mainFont;
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  DWORD GetCurrentActiveThread();
  LCID GetLocaleId(DWORD threadId);
  std::tr1::shared_ptr<Settings> pSettings;

  void UpdateLanguage();

  LCID displayLang;
  WCHAR displayLabel[MAX_LABEL];
  void UpdateLabel(LCID lang);
  void UpdateTooltip(LCID lang);

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoTimer(UINT_PTR timerID);
  void AppletUpdate();
  void ShowConfig();
  LRESULT PaintContent(HDC hdc, RECT clientrt);
};

#endif
