//---
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
//---

#include "AppBar.h"

AppBar::AppBar(APPBARDATA* pAppBarData)
{
  CopyMemory(&localABD, &pAppBarData, sizeof(APPBARDATA));
  localAutoHide = false;
  localWnd = NULL;
}

AppBar::~AppBar()
{
}

bool AppBar::IsEqual(APPBARDATA* pAppBarData)
{
  return ((pAppBarData->cbSize == localABD.cbSize) && (pAppBarData->hWnd == localABD.hWnd));
}

bool AppBar::IsAutoHide()
{
  return localAutoHide;
}

RECT AppBar::GetRect()
{
  return localABD.rc;
}

bool AppBar::SetRect(RECT& rect)
{
  return (CopyRect(&localABD.rc, &rect) == TRUE);
}

UINT AppBar::GetEdge()
{
  return localABD.uEdge;
}

bool AppBar::SetEdge(UINT edge)
{
  localABD.uEdge = edge;

  return true;
}

HWND AppBar::GetWnd()
{
  return localABD.hWnd;
}
