//---
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
//---

#ifndef __BUTTON_H
#define __BUTTON_H

#include <string>
#include "Settings.h"

class Applet;

class Button
{
private:
  RECT bounds;
  std::wstring title;
  Settings *pSettings;
  Applet &applet;
  HFONT &mainFont;
  GUIINFO &guiInfo;
  HWND hwnd;
  bool clicked;
  bool iterated;
  int iteration;

public:
  Button(Applet& app, Settings* sett, HFONT& font, GUIINFO& ginfo, HWND hwnd, bool iterated = false);
  void SetTitle(const std::wstring& text);
  void SetBounds(const RECT& rect);
  void Paint(HDC dc);
  bool LButtonDown(int x, int y);
  bool LButtonUp(int x, int y);
  void NextIteration();
};

#endif
