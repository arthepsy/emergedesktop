// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
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

#include "Thumbnail.h"

WCHAR dwmWndClassName[] = TEXT("dwmThumbnailWnd");

Thumbnail::Thumbnail(HWND taskWnd, HINSTANCE instance)
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  // Register the window class
  wincl.hInstance = instance;
  wincl.lpszClassName = dwmWndClassName;
  wincl.lpfnWndProc = ThumbnailProcedure;
  wincl.cbSize = sizeof (WNDCLASSEX);
  //wincl.style = CS_DROPSHADOW;
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.hbrBackground = NULL;

  RegisterClassEx (&wincl);

  dwmThumbnailWnd = CreateWindowEx(WS_EX_TOOLWINDOW, dwmWndClassName, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
  if (dwmThumbnailWnd)
    {
      if (EGDwmRegisterThumbnail(dwmThumbnailWnd, taskWnd, &dwmThumbnailId) == E_FAIL)
        {
          DestroyWindow(dwmThumbnailWnd);
          dwmThumbnailWnd = NULL;
        }
    }
}

Thumbnail::~Thumbnail()
{
  EGDwmUnregisterThumbnail(dwmThumbnailId);
  DestroyWindow(dwmThumbnailWnd);
}

LRESULT CALLBACK Thumbnail::ThumbnailProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void Thumbnail::ShowThumbnail(HWND ownerWnd, RECT *taskRect)
{
  HMONITOR thumbnailMonitor = MonitorFromWindow(ownerWnd, MONITOR_DEFAULTTONULL);
  MONITORINFO thumbnailMonitorInfo;
  thumbnailMonitorInfo.cbSize = sizeof(MONITORINFO);
  if (!GetMonitorInfo(thumbnailMonitor, &thumbnailMonitorInfo))
    return;

  RECT ownerRect;
  if (!GetWindowRect(ownerWnd, &ownerRect))
    return;

  int x, y;
  SIZE thumbnailDimensions;
  DWM_THUMBNAIL_PROPERTIES thumbnailProperties;

  EGDwmQueryThumbnailSourceSize(dwmThumbnailId, &thumbnailDimensions);
  //4 is an arbitrary zoom factor; ideally, this would be configurable by the user
  thumbnailDimensions.cx = thumbnailDimensions.cx / 4;
  thumbnailDimensions.cy = thumbnailDimensions.cy / 4;

  y = (ownerRect.top + taskRect->top) - thumbnailDimensions.cy;
  if (y < thumbnailMonitorInfo.rcMonitor.top)
    y = ownerRect.top + taskRect->top + taskRect->bottom;

  x = (ownerRect.left + taskRect->left) - (thumbnailDimensions.cx / 2);
  int xoffset = thumbnailMonitorInfo.rcMonitor.right - (x + thumbnailDimensions.cx);
  if (xoffset < 0)
    x += xoffset;
  if (x < thumbnailMonitorInfo.rcMonitor.left)
    x = thumbnailMonitorInfo.rcMonitor.left;

  SetWindowPos(dwmThumbnailWnd, HWND_TOPMOST, x, y, thumbnailDimensions.cx,
               thumbnailDimensions.cy, SWP_SHOWWINDOW);

  //set the thumbnail's properties; ideally, most/all of these would be configurable by the user
  thumbnailProperties.dwFlags = DWM_TNP_RECTDESTINATION/*|DWM_TNP_SOURCECLIENTAREAONLY*/|DWM_TNP_OPACITY|DWM_TNP_VISIBLE;
  thumbnailProperties.rcDestination.left = 0;
  thumbnailProperties.rcDestination.top = 0;
  thumbnailProperties.rcDestination.right = thumbnailDimensions.cx;
  thumbnailProperties.rcDestination.bottom = thumbnailDimensions.cy;
  thumbnailProperties.fSourceClientAreaOnly = true;
  thumbnailProperties.opacity = 255;
  thumbnailProperties.fVisible = true;
  EGDwmUpdateThumbnailProperties(dwmThumbnailId, &thumbnailProperties);
}

void Thumbnail::HideThumbnail()
{
  ShowWindow(dwmThumbnailWnd, SW_HIDE);
}

HWND Thumbnail::GetThumbnailWnd()
{
  return dwmThumbnailWnd;
}
