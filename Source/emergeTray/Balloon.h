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

#ifndef __ETR_BALLOON_H
#define __ETR_BALLOON_H

#include "../emergeLib/emergeLib.h"

class Balloon
{
public:
  Balloon(HINSTANCE hInstance);
  ~Balloon();
  bool Initialize();
  bool Show(POINT showPt);

private:
  static LRESULT CALLBACK BalloonProcedure (HWND, UINT, WPARAM, LPARAM);
  HINSTANCE mainInst;
  HWND balloonWnd;
};

#endif
