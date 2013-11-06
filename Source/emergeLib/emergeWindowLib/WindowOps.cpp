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

#include "WindowOps.h"

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELGetCoreWindow
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Returns emergeCore's control window
//----  --------------------------------------------------------------------------------------------------------
HWND ELGetCoreWindow()
{
  return FindWindow(TEXT("EmergeDesktopCore"), NULL);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELGetDesktopWindow
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Returns emergeCore's desktop window
//----  --------------------------------------------------------------------------------------------------------
HWND ELGetDesktopWindow()
{
  // Check for emergeCore's Desktop before progman to handle running on top of
  // Explorer.
  HWND deskWindow = FindWindow(TEXT("EmergeDesktopProgman"), NULL);
  if (deskWindow == NULL)
  {
    deskWindow = FindWindow(TEXT("progman"), NULL);
  }

  if (deskWindow == NULL)
  {
    deskWindow = HWND_BOTTOM;
  }
  else
  {
    deskWindow = GetNextWindow(deskWindow, GW_HWNDPREV);
  }

  return deskWindow;
}

RECT ELGetMonitorRect(int monitor)
{
  APPLETMONITORINFO appMonInfo;
  ZeroMemory(&appMonInfo, sizeof(appMonInfo));

  appMonInfo.appletMonitorNum = monitor;

  enumCount = 0;
  if (EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)MonitorRectEnum, (LPARAM)&appMonInfo))
  {
    appMonInfo.appletMonitorRect.top = 0;
    appMonInfo.appletMonitorRect.left = 0;
    appMonInfo.appletMonitorRect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    appMonInfo.appletMonitorRect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
  }

  return appMonInfo.appletMonitorRect;
}

HMONITOR ELGetDesktopRect(RECT* appletRect, RECT* rect)
{
  MONITORINFO appletMonitorInfo;
  HMONITOR appletMonitor = MonitorFromRect(appletRect, MONITOR_DEFAULTTONEAREST);

  appletMonitorInfo.cbSize = sizeof(MONITORINFO);
  if (!GetMonitorInfo(appletMonitor, &appletMonitorInfo))
  {
    return NULL;
  }

  if (!CopyRect(rect, &appletMonitorInfo.rcMonitor))
  {
    return NULL;
  }

  return appletMonitor;
}

RECT ELGetWindowRect(HWND hwnd)
{
  BOOL dwmCompositionEnabled = FALSE;
  RECT returnValue;

  if (emergeLibGlobals::getDwmapiDLL())
  {
    if (MSDwmIsCompositionEnabled == NULL)
    {
      MSDwmIsCompositionEnabled = (fnDwmIsCompositionEnabled)GetProcAddress(emergeLibGlobals::getDwmapiDLL(), "DwmIsCompositionEnabled");
    }
    if (MSDwmGetWindowAttribute == NULL)
    {
      MSDwmGetWindowAttribute = (fnDwmGetWindowAttribute)GetProcAddress(emergeLibGlobals::getDwmapiDLL(), "DwmGetWindowAttribute");
    }
  }

  if (MSDwmIsCompositionEnabled && MSDwmGetWindowAttribute)
  {
    if (SUCCEEDED(MSDwmIsCompositionEnabled(&dwmCompositionEnabled)))
    {
      if (dwmCompositionEnabled)
      {
        if (SUCCEEDED(MSDwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &returnValue, sizeof(RECT))))
        {
          return returnValue;
        }
      }
    }
  }

  if ((!dwmCompositionEnabled) || (IsRectEmpty(&returnValue)))
  {
    GetWindowRect(hwnd, &returnValue);
  }

  return returnValue;
}

bool ELStealFocus(HWND hwnd)
{
  DWORD threadID1, threadID2;
  bool result = false;

  threadID1 = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
  threadID2 = GetWindowThreadProcessId(hwnd, NULL);

  if (threadID1 != threadID2)
  {
    AttachThreadInput(threadID1, threadID2, TRUE);
    result = ((SetFocus(hwnd) != NULL) && SetForegroundWindow(hwnd));
    AttachThreadInput(threadID1, threadID2, FALSE);
  }
  else
  {
    result = ((SetFocus(hwnd) != NULL) && SetForegroundWindow(hwnd));
  }

  return result;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ELSwitchTorhisWindow
// Required:	HWND wnd - window that will receive focus
// Returns:	bool
// Purpose:	Forces the window supplied to receive focus
//----  --------------------------------------------------------------------------------------------------------
bool ELSwitchToThisWindow(HWND wnd)
{
  if ((emergeLibGlobals::getUser32DLL()) && (MSSwitchToThisWindow == NULL))
  {
    MSSwitchToThisWindow = (lpfnMSSwitchToThisWindow)GetProcAddress(emergeLibGlobals::getUser32DLL(), "SwitchToThisWindow");
  }
  if (MSSwitchToThisWindow)
  {
    MSSwitchToThisWindow(wnd, TRUE);
    return true;
  }

  return false;
}

bool ELSetForeground(HWND wnd)
{
  WINDOWPLACEMENT winPlacement;
  winPlacement.length = sizeof(WINDOWPLACEMENT);

  if (!GetWindowPlacement(wnd, &winPlacement))
  {
    return false;
  }

  if ((winPlacement.showCmd == SW_SHOWMINIMIZED) &&
      (winPlacement.flags == WPF_RESTORETOMAXIMIZED))
  {
    PostMessage(wnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
  }
  else if (winPlacement.showCmd == SW_SHOWMINIMIZED)
  {
    PostMessage(wnd, WM_SYSCOMMAND, SC_RESTORE, 0);
  }

  return (SetForegroundWindow(wnd) == TRUE);
}

bool ELIsFullScreen(HWND appletWnd)
{
  return (EnumWindows(FullscreenEnum, (LPARAM)appletWnd) == FALSE);
}

bool ELIsModal(HWND window)
{
  HWND lastWnd = GetLastActivePopup(window);
  WCHAR windowClass[MAX_LINE_LENGTH];

  if (IsWindowVisible(lastWnd))
  {
    RealGetWindowClass(lastWnd, windowClass, MAX_LINE_LENGTH);
    if (_wcsicmp(windowClass, TEXT("#32770")) == 0)
    {
      return true;
    }
  }

  return false;
}

/*!
  @fn ELIsValidTaskWindow(HWND hwnd)
  @brief Check to seee if the window is a valid 'task'
  @param hwnd window handle
  */

bool ELIsValidTaskWindow(HWND hwnd)
{
  RECT hwndRT;
  GetWindowRect(hwnd, &hwndRT);

  // To be a valid task it should be visible...
  if (IsWindowVisible(hwnd) &&
      // ... be enabled ...
      IsWindowEnabled(hwnd) &&
      // ... not be a toolwindow ...
      !(GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) &&
      // ... have no parent ...
      !GetParent(hwnd) &&
      // ... have no owner ...
      !GetWindow(hwnd, GW_OWNER) &&
      // ... and have a window rect of some size when not iconic.
      !(IsRectEmpty(&hwndRT) && !IsIconic(hwnd)))
  {
    return true;
  }

  return false;
}

bool ELIsApplet(HWND hwnd)
{
  WCHAR windowClass[MAX_LINE_LENGTH];
  RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH);

  // emergeApplet Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopApplet")) == 0)
  {
    return true;
  }

  // emergeCore Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopCore")) == 0)
  {
    return true;
  }

  // emergeDesktop Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopMenuBuilder")) == 0)
  {
    return true;
  }

  // Desktop Class
  if (_wcsicmp(windowClass, TEXT("EmergeDesktopProgman")) == 0)
  {
    return true;
  }

  return false;
}

bool ELIsExplorer(HWND hwnd)
{
  WCHAR windowClass[MAX_LINE_LENGTH];
  RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH);

  // Explorer Class
  if (_wcsicmp(windowClass, TEXT("progman")) == 0)
  {
    return true;
  }

  // Explorer Desktop Class
  if (_wcsicmp(windowClass, TEXT("WorkerW")) == 0)
  {
    return true;
  }

  return false;
}

POINT ELGetAnchorPoint(HWND hwnd)
{
  POINT pt;
  RECT wndRect;

  wndRect = ELGetWindowRect(hwnd);
  GetCursorPos(&pt);

  pt.x -= wndRect.left;
  pt.y -= wndRect.top;

  return pt;
}

int ELGetAppletMonitor(HWND hwnd)
{
  APPLETMONITORINFO appMonInfo;
  ZeroMemory(&appMonInfo, sizeof(appMonInfo));

  appMonInfo.appletWnd = hwnd;
  appMonInfo.appletMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
  appMonInfo.appletMonitorNum = 0;

  EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)AppletMonitorEnum, (LPARAM)&appMonInfo);

  return appMonInfo.appletMonitorNum;
}

bool ELSnapMove(LPSNAPMOVEINFO snapMove)
{
  bool moved = false;
  POINT pt;

  if ((snapMove->AppletRect->left == snapMove->AppletRect->right) ||
      (snapMove->AppletRect->top == snapMove->AppletRect->bottom))
  {
    return false;
  }

  GetCursorPos(&pt);
  OffsetRect(snapMove->AppletRect, pt.x - (snapMove->AppletRect->left + snapMove->origin.x),
             pt.y - (snapMove->AppletRect->top + snapMove->origin.y));

  if (EnumWindows(SnapMoveEnum, (LPARAM)snapMove) == ERROR_SUCCESS)
  {
    moved = true;
  }

  moved = ELSnapMoveToDesk(snapMove->AppletRect);

  return moved;
}

bool ELSnapMoveToDesk(RECT* AppletRect)
{
  bool verticalMove = false;
  bool horizontalMove = false;
  int verticalOffset = 0;
  int horizontalOffset = 0;
  bool moved = false;
  MONITORINFO appletMonitorInfo;
  appletMonitorInfo.cbSize = sizeof(MONITORINFO);
  HMONITOR appletMonitor = MonitorFromRect(AppletRect, MONITOR_DEFAULTTONULL);

  if (appletMonitor == NULL)
  {
    return false;
  }

  if (!GetMonitorInfo(appletMonitor, &appletMonitorInfo))
  {
    return false;
  }

  if (IsClose(AppletRect->top, appletMonitorInfo.rcMonitor.top))
  {
    verticalMove = true;
    verticalOffset = appletMonitorInfo.rcMonitor.top - AppletRect->top;
  }
  else if (IsClose(AppletRect->bottom, appletMonitorInfo.rcMonitor.bottom))
  {
    verticalMove = true;
    verticalOffset = appletMonitorInfo.rcMonitor.bottom - AppletRect->bottom;
  }

  if (IsClose(AppletRect->left, appletMonitorInfo.rcMonitor.left))
  {
    horizontalMove = true;
    horizontalOffset = appletMonitorInfo.rcMonitor.left - AppletRect->left;
  }
  else if (IsClose(AppletRect->right, appletMonitorInfo.rcMonitor.right))
  {
    horizontalMove = true;
    horizontalOffset = appletMonitorInfo.rcMonitor.right - AppletRect->right;
  }

  if (horizontalMove)
  {
    OffsetRect(AppletRect, horizontalOffset, 0);
    moved = true;
  }

  if (verticalMove)
  {
    OffsetRect(AppletRect, 0, verticalOffset);
    moved = true;
  }

  return moved;
}

bool ELSnapSize(LPSNAPSIZEINFO snapSize)
{
  RECT rt;
  bool moved = false;

  if ((snapSize->AppletRect->left == snapSize->AppletRect->right) ||
      (snapSize->AppletRect->top == snapSize->AppletRect->bottom))
  {
    return false;
  }

  if (EnumWindows(SnapSizeEnum, (LPARAM)snapSize) == ERROR_SUCCESS)
  {
    moved = true;
  }

  ELGetDesktopRect(snapSize->AppletRect, &rt);

  switch (snapSize->AppletEdge)
  {
  case WMSZ_TOP:
    if ((IsClose(snapSize->AppletRect->top, rt.top)) || (snapSize->AppletRect->top < rt.top))
    {
      moved = true;
      snapSize->AppletRect->top = rt.top;
    }
    break;
  case WMSZ_TOPLEFT:
    if ((IsClose(snapSize->AppletRect->top, rt.top)) || (snapSize->AppletRect->top < rt.top))
    {
      moved = true;
      snapSize->AppletRect->top = rt.top;
    }
    if ((IsClose(snapSize->AppletRect->left, rt.left)) || (snapSize->AppletRect->left < rt.left))
    {
      moved = true;
      snapSize->AppletRect->left = rt.left;
    }
    break;
  case WMSZ_TOPRIGHT:
    if ((IsClose(snapSize->AppletRect->top, rt.top)) || (snapSize->AppletRect->top < rt.top))
    {
      moved = true;
      snapSize->AppletRect->top = rt.top;
    }
    if ((IsClose(snapSize->AppletRect->right, rt.right)) || (snapSize->AppletRect->right > rt.right))
    {
      moved = true;
      snapSize->AppletRect->right = rt.right;
    }
    break;
  case WMSZ_LEFT:
    if ((IsClose(snapSize->AppletRect->left, rt.left)) || (snapSize->AppletRect->left < rt.left))
    {
      moved = true;
      snapSize->AppletRect->left = rt.left;
    }
    break;
  case WMSZ_RIGHT:
    if ((IsClose(snapSize->AppletRect->right, rt.right)) || (snapSize->AppletRect->right > rt.right))
    {
      moved = true;
      snapSize->AppletRect->right = rt.right;
    }
    break;
  case WMSZ_BOTTOM:
    if ((IsClose(snapSize->AppletRect->bottom, rt.bottom)) || (snapSize->AppletRect->bottom > rt.bottom))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.bottom;
    }
    break;
  case WMSZ_BOTTOMLEFT:
    if ((IsClose(snapSize->AppletRect->bottom, rt.bottom)) || (snapSize->AppletRect->bottom > rt.bottom))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.bottom;
    }
    if ((IsClose(snapSize->AppletRect->left, rt.left)) || (snapSize->AppletRect->left < rt.left))
    {
      moved = true;
      snapSize->AppletRect->left = rt.left;
    }
    break;
  case WMSZ_BOTTOMRIGHT:
    if ((IsClose(snapSize->AppletRect->bottom, rt.bottom)) || (snapSize->AppletRect->bottom > rt.bottom))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.bottom;
    }
    if ((IsClose(snapSize->AppletRect->right, rt.right)) || (snapSize->AppletRect->right > rt.right))
    {
      moved = true;
      snapSize->AppletRect->right = rt.right;
    }
    break;
  }

  return moved;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	IsClose
// Required:	int side - side of window
//		int edge - desktop edge
// Returns:	bool
// Purpose:	Determines if a window size is near the desktop edge
//----  --------------------------------------------------------------------------------------------------------
bool IsClose(int side, int edge)
{
  return abs(edge - side) < 10;
}

bool SnapMoveToEdge(LPSNAPMOVEINFO snapMove, RECT rt)
{
  bool verticalSnap = false;
  bool horizontalSnap = false;
  int verticalOffset = 0;
  int horizontalOffset = 0;

  if (IsClose(snapMove->AppletRect->top, rt.top))
  {
    verticalSnap = true;
    verticalOffset = rt.top - snapMove->AppletRect->top;
  }
  else if (IsClose(snapMove->AppletRect->bottom, rt.top))
  {
    verticalSnap = true;
    verticalOffset = rt.top - snapMove->AppletRect->bottom;
  }
  else if (IsClose(snapMove->AppletRect->bottom, rt.bottom))
  {
    verticalSnap = true;
    verticalOffset = rt.bottom - snapMove->AppletRect->bottom;
  }
  else if (IsClose(snapMove->AppletRect->top, rt.bottom))
  {
    verticalSnap = true;
    verticalOffset = rt.bottom - snapMove->AppletRect->top;
  }
  else if (((snapMove->AppletRect->top < rt.bottom) && (snapMove->AppletRect->top > rt.top)) ||
           ((snapMove->AppletRect->bottom > rt.top) && (snapMove->AppletRect->bottom < rt.bottom)))
  {
    verticalSnap = true;
    verticalOffset = 0;
  }

  if (IsClose(snapMove->AppletRect->left, rt.left))
  {
    horizontalSnap = true;
    horizontalOffset = rt.left - snapMove->AppletRect->left;
  }
  else if (IsClose(snapMove->AppletRect->right, rt.left))
  {
    horizontalSnap = true;
    horizontalOffset = rt.left - snapMove->AppletRect->right;
  }
  else if (IsClose(snapMove->AppletRect->right, rt.right))
  {
    horizontalSnap = true;
    horizontalOffset = rt.right - snapMove->AppletRect->right;
  }
  else if (IsClose(snapMove->AppletRect->left, rt.right))
  {
    horizontalSnap = true;
    horizontalOffset = rt.right - snapMove->AppletRect->left;
  }
  else if (((snapMove->AppletRect->left < rt.right) && (snapMove->AppletRect->left > rt.left)) ||
           ((snapMove->AppletRect->right > rt.left) && (snapMove->AppletRect->right < rt.right)))
  {
    horizontalSnap = true;
    horizontalOffset = 0;
  }

  if (verticalSnap && horizontalSnap)
  {
    OffsetRect(snapMove->AppletRect, horizontalOffset, verticalOffset);
    return true;
  }

  return false;
}

bool SnapSizeToEdge(LPSNAPSIZEINFO snapSize, RECT rt)
{
  bool verticalSnap = false;
  bool horizontalSnap = false;
  bool moved = false;

  if (IsClose(snapSize->AppletRect->top, rt.bottom) || IsClose(snapSize->AppletRect->top, rt.top) ||
      IsClose(snapSize->AppletRect->bottom, rt.bottom) || IsClose(snapSize->AppletRect->bottom, rt.top))
  {
    verticalSnap = true;
  }
  if (IsClose(snapSize->AppletRect->left, rt.right) || IsClose(snapSize->AppletRect->left, rt.left) ||
      IsClose(snapSize->AppletRect->right, rt.right) || IsClose(snapSize->AppletRect->right, rt.left))
  {
    horizontalSnap = true;
  }

  if (!verticalSnap || !horizontalSnap)
  {
    return false;
  }

  switch (snapSize->AppletEdge)
  {
  case WMSZ_TOP:
    if (IsClose(snapSize->AppletRect->top, rt.top))
    {
      snapSize->AppletRect->top = rt.top;
      moved = true;
    }
    else if (IsClose(snapSize->AppletRect->top, rt.bottom))
    {
      snapSize->AppletRect->top = rt.bottom;
      moved = true;
    }
    break;
  case WMSZ_TOPLEFT:
    if (IsClose(snapSize->AppletRect->top, rt.top))
    {
      snapSize->AppletRect->top = rt.top;
      moved = true;
    }
    else if (IsClose(snapSize->AppletRect->top, rt.bottom))
    {
      snapSize->AppletRect->top = rt.bottom;
      moved = true;
    }

    if (IsClose(snapSize->AppletRect->left, rt.left))
    {
      moved = true;
      snapSize->AppletRect->left = rt.left;
    }
    else if (IsClose(snapSize->AppletRect->left, rt.right))
    {
      moved = true;
      snapSize->AppletRect->left = rt.right;
    }
    break;
  case WMSZ_TOPRIGHT:
    if (IsClose(snapSize->AppletRect->top, rt.top))
    {
      snapSize->AppletRect->top = rt.top;
      moved = true;
    }
    else if (IsClose(snapSize->AppletRect->top, rt.bottom))
    {
      snapSize->AppletRect->top = rt.bottom;
      moved = true;
    }

    if (IsClose(snapSize->AppletRect->right, rt.right))
    {
      moved = true;
      snapSize->AppletRect->right = rt.right;
    }
    else if (IsClose(snapSize->AppletRect->right, rt.left))
    {
      moved = true;
      snapSize->AppletRect->right = rt.left;
    }
    break;
  case WMSZ_LEFT:
    if (IsClose(snapSize->AppletRect->left, rt.left))
    {
      moved = true;
      snapSize->AppletRect->left = rt.left;
    }
    else if (IsClose(snapSize->AppletRect->left, rt.right))
    {
      moved = true;
      snapSize->AppletRect->left = rt.right;
    }
    break;
  case WMSZ_RIGHT:
    if (IsClose(snapSize->AppletRect->right, rt.right))
    {
      moved = true;
      snapSize->AppletRect->right = rt.right;
    }
    else if (IsClose(snapSize->AppletRect->right, rt.left))
    {
      moved = true;
      snapSize->AppletRect->right = rt.left;
    }
    break;
  case WMSZ_BOTTOM:
    if (IsClose(snapSize->AppletRect->bottom, rt.bottom))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.bottom;
    }
    else if (IsClose(snapSize->AppletRect->bottom, rt.top))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.top;
    }
    break;
  case WMSZ_BOTTOMLEFT:
    if (IsClose(snapSize->AppletRect->bottom, rt.bottom))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.bottom;
    }
    else if (IsClose(snapSize->AppletRect->bottom, rt.top))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.top;
    }

    if (IsClose(snapSize->AppletRect->left, rt.left))
    {
      moved = true;
      snapSize->AppletRect->left = rt.left;
    }
    else if (IsClose(snapSize->AppletRect->left, rt.right))
    {
      moved = true;
      snapSize->AppletRect->left = rt.right;
    }
    break;
  case WMSZ_BOTTOMRIGHT:
    if (IsClose(snapSize->AppletRect->bottom, rt.bottom))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.bottom;
    }
    else if (IsClose(snapSize->AppletRect->bottom, rt.top))
    {
      moved = true;
      snapSize->AppletRect->bottom = rt.top;
    }

    if (IsClose(snapSize->AppletRect->right, rt.right))
    {
      moved = true;
      snapSize->AppletRect->right = rt.right;
    }
    else if (IsClose(snapSize->AppletRect->right, rt.left))
    {
      moved = true;
      snapSize->AppletRect->right = rt.left;
    }
    break;
  }

  return moved;
}

BOOL CALLBACK AppletMonitorEnum(HMONITOR hMonitor, HDC hdcMonitor UNUSED, LPRECT lprcMonitor UNUSED, LPARAM dwData)
{
  if (((LPAPPLETMONITORINFO)dwData)->appletMonitor == hMonitor)
  {
    return FALSE;
  }

  ((LPAPPLETMONITORINFO)dwData)->appletMonitorNum++;

  return TRUE;
}

// Since this function is used via an enum, return FALSE in the case where a
// full screen window is detected to break out of the enum.
BOOL CALLBACK FullscreenEnum(HWND hwnd, LPARAM lParam)
{
  HMONITOR hwndMonitor, appletMonitor;
  WCHAR windowClass[MAX_LINE_LENGTH];
  MONITORINFO hwndMonitorInfo;
  hwndMonitorInfo.cbSize = sizeof(MONITORINFO);
  RECT wndRect;

  // Get the monitors that the hwnd and applet are on.
  appletMonitor = MonitorFromWindow((HWND)lParam, MONITOR_DEFAULTTONEAREST);
  hwndMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  GetMonitorInfo(hwndMonitor, &hwndMonitorInfo);

  // Get the class name for hwnd.
  RealGetWindowClass(hwnd, windowClass, MAX_LINE_LENGTH);

  GetWindowRect(hwnd, &wndRect);

  // A full screen window is determined if hwnd is visible...
  return !(IsWindowVisible(hwnd) &&
           // and the hwnd is not InstallShield...
           (wcsicmp(windowClass, TEXT("InstallShield_Win")) != 0) &&
           // and the hwnd is on the same monitor as the applet...
           (hwndMonitor == appletMonitor) &&
           // and hwnd is not an eD window...
           !ELIsApplet(hwnd) &&
           // and hwnd is not the Explorer desktop...
           !ELIsExplorer(hwnd) &&
           // and hwnd size is greater than the resolution of the monitor which the
           // applet is on, hwnd is full screen.
           (wndRect.left <= hwndMonitorInfo.rcMonitor.left) &&
           (wndRect.top <= hwndMonitorInfo.rcMonitor.top) &&
           (wndRect.right >= hwndMonitorInfo.rcMonitor.right) &&
           (wndRect.bottom >= hwndMonitorInfo.rcMonitor.bottom));
}

BOOL CALLBACK MonitorRectEnum(HMONITOR hMonitor UNUSED, HDC hdcMonitor UNUSED, LPRECT lprcMonitor, LPARAM dwData)
{
  if (enumCount == ((LPAPPLETMONITORINFO)dwData)->appletMonitorNum)
  {
    CopyRect(&((LPAPPLETMONITORINFO)dwData)->appletMonitorRect, lprcMonitor);
    return FALSE;
  }

  enumCount++;

  return TRUE;
}

BOOL CALLBACK SnapMoveEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR hwndClass[MAX_LINE_LENGTH];
  RECT hwndRect;

  if (hwnd == ((LPSNAPMOVEINFO)lParam)->AppletWindow)
  {
    return true;
  }

  if (GetClassName(hwnd, hwndClass, MAX_LINE_LENGTH) == 0)
  {
    return true;
  }

  if (_wcsicmp(hwndClass, TEXT("EmergeDesktopApplet")) != 0)
  {
    return true;
  }

  hwndRect = ELGetWindowRect(hwnd);

  if (SnapMoveToEdge((LPSNAPMOVEINFO)lParam, hwndRect))
  {
    SetLastError(ERROR_SUCCESS);
    return false;
  }

  return true;
}

BOOL CALLBACK SnapSizeEnum(HWND hwnd, LPARAM lParam)
{
  WCHAR hwndClass[MAX_LINE_LENGTH];
  RECT hwndRect;

  if (hwnd == ((LPSNAPSIZEINFO)lParam)->AppletWindow)
  {
    return true;
  }

  if (GetClassName(hwnd, hwndClass, MAX_LINE_LENGTH) == 0)
  {
    return true;
  }

  if (_wcsicmp(hwndClass, TEXT("EmergeDesktopApplet")) != 0)
  {
    return true;
  }

  hwndRect = ELGetWindowRect(hwnd);

  if (SnapSizeToEdge((LPSNAPSIZEINFO)lParam, hwndRect))
  {
    SetLastError(ERROR_SUCCESS);
    return false;
  }

  return true;
}
