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

#ifndef __EL_APPLET_H
#define __EL_APPLET_H

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

#define ACTIVE_DELAY_TIME 500

#include "Settings.h"
#include <vector>
#include <process.h>
#include "Item.h"
#include "Config.h"
#include "../emergeBaseClasses/BaseApplet.h"

class Applet: public BaseApplet
{
private:
  std::tr1::shared_ptr<Config> pConfig;
  std::tr1::shared_ptr<Settings> pSettings;
  HWND activeWnd;
  int iconSize;
  void UpdateIcons();
  static LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
  static BOOL CALLBACK EnumFullScreenWindows(HWND hwnd, LPARAM lParam);
  static VOID CALLBACK ActiveTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
  Applet(HINSTANCE hInstance);
  ~Applet();
  UINT Initialize();
  LRESULT DoSizing(HWND hwnd, UINT edge, LPRECT rect);
  LRESULT ItemMouseEvent(UINT message, LPARAM lParam);
  bool PaintItem(HDC hdc, UINT index, int x, int y, RECT rect);
  void ShowConfig();
  void CreateBrushes();
  void DestroyBrushes();
  void AppletUpdate();
  void UpdateTip(UINT index);
  void DeleteTip(UINT index);
  size_t GetIconCount();
  void ClearActive(UINT index);
};

#endif
