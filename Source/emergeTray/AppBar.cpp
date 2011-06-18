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

#include "AppBar.h"

AppBar::AppBar(HWND wnd, APPBARDATA abd, bool autoHide)
{
  MONITORINFO appletMonitorInfo;
  appletMonitorInfo.cbSize = sizeof(MONITORINFO);
  HMONITOR appletMonitor = MonitorFromWindow(localWnd, MONITOR_DEFAULTTONEAREST);

  localWnd = wnd;
  CopyMemory(&localABD, &abd, sizeof(APPBARDATA));
  localAutoHide = autoHide;

  ELGetWindowRect(localWnd, &localABD.rc);
  localABD.uEdge = ABE_BOTTOM;

  if (GetMonitorInfo(appletMonitor, &appletMonitorInfo))
    {
      int topDelta = abs(localABD.rc.top - appletMonitorInfo.rcMonitor.top);
      int bottomDelta = abs(appletMonitorInfo.rcMonitor.bottom - localABD.rc.bottom);
      int leftDelta = abs(localABD.rc.left - appletMonitorInfo.rcMonitor.left);
      int rightDelta = abs(appletMonitorInfo.rcMonitor.right - localABD.rc.right);

      if ((leftDelta < topDelta) && (leftDelta < bottomDelta) && (leftDelta < rightDelta))
        localABD.uEdge = ABE_LEFT;
      else if ((rightDelta < topDelta) && (rightDelta < bottomDelta) && (rightDelta < leftDelta))
        localABD.uEdge = ABE_RIGHT;
      else if (topDelta < bottomDelta)
        localABD.uEdge = ABE_TOP;
    }
}

AppBar::~AppBar()
{
}

bool AppBar::IsEqual(APPBARDATA abd)
{
  return ((abd.cbSize == localABD.cbSize) && (abd.hWnd == localABD.hWnd));
}

bool AppBar::IsAutoHide()
{
  return localAutoHide;
}

RECT AppBar::GetRect()
{
  return localABD.rc;
}

bool AppBar::SetRect(RECT &rect)
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

