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

WCHAR thumbnailName[] = TEXT("emergeTaskThumbnail");
WCHAR dwmWndClassName[] = TEXT("dwmThumbnailWnd");

Thumbnail::Thumbnail(HINSTANCE instance)
{
  mainInstance = instance;
}

Thumbnail::~Thumbnail()
{
}

/*bool Thumbnail::Show(POINT showPt)
{
  int x, y, xoffset, width, height;
  HMONITOR balloonMonitor = MonitorFromWindow(balloonWnd, MONITOR_DEFAULTTONULL);
  MONITORINFO balloonMonitorInfo;
  balloonMonitorInfo.cbSize = sizeof(MONITORINFO);

  if (!GetMonitorInfo(balloonMonitor, &balloonMonitorInfo))
    return false;

  // If the string length of info or infoTitle is 0, do not display a balloon.
  if ((wcslen(info) == 0) || (wcslen(infoTitle) == 0))
    return false;

  width = infoRect.right;
  if (titleRect.right > infoRect.right)
    width = titleRect.right;
  width += 5;
  height = infoRect.bottom + 5;

  y = showPt.y - height;
  if (y < balloonMonitorInfo.rcMonitor.top)
    y = showPt.y + ICON_SIZE;

  x = showPt.x - (width / 2);
  xoffset = balloonMonitorInfo.rcMonitor.right - (x + width);
  if (xoffset < 0)
    x += xoffset;
  if (x < balloonMonitorInfo.rcMonitor.left)
    x = balloonMonitorInfo.rcMonitor.left;

  SendMessage(NIN_BALLOONSHOW);
  SetTimer(balloonWnd, BALLOON_TIMER_ID, 10000, NULL);

  if (SetWindowPos(balloonWnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW))
    return DrawAlphaBlend();

  return false;
}*/

LRESULT CALLBACK Thumbnail::ThumbnailProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void Thumbnail::DestroyThumbnail()
{
  if ((dwmThumbnailId != NULL) && (EGDwmUnregisterThumbnail(dwmThumbnailId) == S_OK))
    dwmThumbnailId = NULL;

  if ((dwmThumbnailWnd != NULL) && (DestroyWindow(dwmThumbnailWnd)))
    dwmThumbnailWnd = NULL;
}

void Thumbnail::CreateThumbnail(HWND ownerWnd, HWND sourceWnd, RECT *taskRect)
{
  if ((dwmThumbnailId != NULL) || (dwmThumbnailWnd != NULL))
    DestroyThumbnail(); //remove any existing thumbnail before creating a new one

  HWND hwndSource = sourceWnd;
  SIZE thumbnailDimensions;
  POINT centerPoint;
  RECT appletRect;
  DWM_THUMBNAIL_PROPERTIES thumbnailProperties;
  SNAPMOVEINFO thumbnailWndSnapMoveInfo;
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  // Register the window class
  wincl.hInstance = mainInstance;
  wincl.lpszClassName = dwmWndClassName;
  wincl.lpfnWndProc = ThumbnailProcedure;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.style = CS_DROPSHADOW;
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.hbrBackground = NULL;

  RegisterClassEx (&wincl);

  dwmThumbnailWnd = CreateWindowEx(WS_EX_TOOLWINDOW, dwmWndClassName, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
  if (dwmThumbnailWnd == NULL)
    return;

  if (EGDwmRegisterThumbnail(dwmThumbnailWnd, hwndSource, &dwmThumbnailId) == E_FAIL)
    {
      DestroyWindow(dwmThumbnailWnd);
      return;
    }

  EGDwmQueryThumbnailSourceSize(dwmThumbnailId, &thumbnailDimensions);
  //4 is an arbitrary zoom factor; ideally, this would be configurable by the user
  thumbnailDimensions.cx = thumbnailDimensions.cx / 4;
  thumbnailDimensions.cy = thumbnailDimensions.cy / 4;

  centerPoint.x = ELMid(taskRect->left, taskRect->right);
  centerPoint.y = taskRect->top;
  ClientToScreen(ownerWnd, &centerPoint);
  centerPoint.x = centerPoint.x - (thumbnailDimensions.cx/2);
  centerPoint.y = centerPoint.y - thumbnailDimensions.cy;
  thumbnailWndSnapMoveInfo.AppletWindow = dwmThumbnailWnd;
  thumbnailWndSnapMoveInfo.origin = ELGetAnchorPoint(ownerWnd);
  appletRect.left = centerPoint.x;
  appletRect.top = centerPoint.y;
  appletRect.right = centerPoint.x + thumbnailDimensions.cx;
  appletRect.bottom = centerPoint.y + thumbnailDimensions.cy;
  thumbnailWndSnapMoveInfo.AppletRect = &appletRect;

  ELSnapMove(&thumbnailWndSnapMoveInfo); //snap the thumbnail window to the applet
  SetWindowPos(dwmThumbnailWnd, NULL, thumbnailWndSnapMoveInfo.AppletRect->left,
               thumbnailWndSnapMoveInfo.AppletRect->top - (thumbnailWndSnapMoveInfo.AppletRect->bottom - thumbnailWndSnapMoveInfo.AppletRect->top),
               (thumbnailWndSnapMoveInfo.AppletRect->right - thumbnailWndSnapMoveInfo.AppletRect->left),
               (thumbnailWndSnapMoveInfo.AppletRect->bottom - thumbnailWndSnapMoveInfo.AppletRect->top),
               SWP_NOZORDER|SWP_SHOWWINDOW);

  //set the thumbnail's properties; ideally, most/all of these would be configurable by the user
  thumbnailProperties.dwFlags = DWM_TNP_RECTDESTINATION|DWM_TNP_SOURCECLIENTAREAONLY|DWM_TNP_OPACITY|DWM_TNP_VISIBLE;
  thumbnailProperties.rcDestination.left = 0;
  thumbnailProperties.rcDestination.top = 0;
  thumbnailProperties.rcDestination.right = thumbnailDimensions.cx;
  thumbnailProperties.rcDestination.bottom = thumbnailDimensions.cy;
  thumbnailProperties.fSourceClientAreaOnly = true;
  thumbnailProperties.opacity = 255;
  thumbnailProperties.fVisible = true;
  EGDwmUpdateThumbnailProperties(dwmThumbnailId, &thumbnailProperties);
}
