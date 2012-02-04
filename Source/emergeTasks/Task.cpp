//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------
//
// Note: The task menu code was derived from BBSystemBar which is part of the
// BB4Win source code, copyright 2001-2004 The Blackbox for Windows Development
// Team.
//
//----  --------------------------------------------------------------------------------------------------------

#include "Task.h"
#include <stdio.h>

//----  --------------------------------------------------------------------------------------------------------
// Function:	Constructor
// Requires:	HWND wnd - window handle of the task
// Returns:	Nothing
// Purpose:	Creates TrayIcon Class Object
//----  --------------------------------------------------------------------------------------------------------
Task::Task(HWND task, HINSTANCE mainInstance)
{
  (*this).mainInstance = mainInstance;
  newIcon = NULL;
  wnd = task;
  rect.left = 0;
  rect.right = 0;
  rect.top = 0;
  rect.bottom = 0;
  flash = false;
  visible = true;
  flashCount = 0;
  origIcon = NULL;

  convertIcon = true;
}

LRESULT CALLBACK Task::dwmWindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
}

void Task::CreateNewIcon(BYTE foregroundAlpha, BYTE backgroundAlpha)
{
  /**< Don't bother converting NULL icons, just set newIcon and return */
  if (origIcon == NULL)
    {
      newIcon = NULL;
      return;
    }

  if (convertIcon)
    {
      convertIcon = false;

      // Destroy any existing newIcon
      if (newIcon != NULL)
        DestroyIcon(newIcon);

      // If the background if fully opaque, don't bother converting the icon, simply copy it
      if (backgroundAlpha == 0xff)
        newIcon = CopyIcon(origIcon);
      else
        /**< Don't bail if EGConvertIcon returns a NULL icon, since in this case it may be valid (icon flashing) */
        newIcon = EGConvertIcon(origIcon, foregroundAlpha);
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	Deconstructor
// Requires:	Nothing
// Returns:	Nothing
// Purpose:	Deletes the task icon
//----  --------------------------------------------------------------------------------------------------------
Task::~Task()
{
  DestroyIcon(origIcon);
  DestroyIcon(newIcon);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetWnd
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Retrieves the task window
//----  --------------------------------------------------------------------------------------------------------
HWND Task::GetWnd()
{
  return wnd;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetFlash
// Requires:	Nothing
// Returns:		bool
// Purpose:		Retrieves the flash state of the task
//----  --------------------------------------------------------------------------------------------------------
bool Task::GetFlash()
{
  return flash;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetVisible
// Requires:	Nothing
// Returns:	bool
// Purpose:	Retrieves the visibility state of the task icon
//----  --------------------------------------------------------------------------------------------------------
bool Task::GetVisible()
{
  return visible;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetIcon
// Requires:	Nothing
// Returns:	HICON
// Purpose:	Retrieves the task icon
//----  --------------------------------------------------------------------------------------------------------
HICON Task::GetIcon()
{
  return newIcon;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetRect
// Requires:	Nothing
// Returns:	RECT*
// Purpose:	Retrieves the bounding rectangle of the task
//----  --------------------------------------------------------------------------------------------------------
RECT *Task::GetRect()
{
  return &rect;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetIcon
// Requires:	HICON icon - new icon to use
// Returns:	Nothing
// Purpose:	Replaces existing task icon with new icon
//----  --------------------------------------------------------------------------------------------------------
void Task::SetIcon(HICON icon, int iconSize)
{
  std::wstring applicationName;

  if (icon)
    {
      if (origIcon)
        DestroyIcon(origIcon);

      origIcon = CopyIcon(icon);

      convertIcon = true;
    }
  /* if the passed icon is NULL and origIcon is also NULL, generate a default
   * icon using the application's icon.
   */
  else
    {
      if (origIcon == NULL)
        {
          applicationName = ELGetWindowApp(wnd, true);
          origIcon = EGGetFileIcon(applicationName.c_str(), iconSize);
        }
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetRect
// Requires:	RECT rect - new bounding rectangle
// Returns:	Nothing
// Purpose:	Replaces the bounding rectangle of the task
//----  --------------------------------------------------------------------------------------------------------
void Task::SetRect(RECT rect)
{
  if (!EqualRect(&(*this).rect, &rect))
    {
      (*this).rect = rect;

      convertIcon = true;
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetFlash
// Requires:	bool flash - flash the icon?
// Returns:		Nothing
// Purpose:		Toggles the flash state of the task
//----  --------------------------------------------------------------------------------------------------------
void Task::SetFlash(bool flash)
{
  (*this).flash = flash;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetFlashCount
// Requires:	Nothing
// Returns:	UINT
// Purpose:	Returns the current task flashcount
//----  --------------------------------------------------------------------------------------------------------
UINT Task::GetFlashCount()
{
  return flashCount;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetFlashCount
// Requires:	UINT flashCount - Value for flashcount
// Returns:	Nothing
// Purpose:	Sets the flash count value
//----  --------------------------------------------------------------------------------------------------------
void Task::SetFlashCount(UINT flashCount)
{
  (*this).flashCount = flashCount;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetDwmThumbnail
// Requires:	Nothing
// Returns:	HTHUMBNAIL
// Purpose:	Returns the current task's DWM thumbnail
//----  --------------------------------------------------------------------------------------------------------
HTHUMBNAIL Task::GetDwmThumbnail()
{
  return dwmThumbnailId;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetDwmThumbnail
// Requires:	HTHUMBNAIL dwmThumbnailId - Value for DWM thumbnail
// Returns:	Nothing
// Purpose:	Sets the current task's DWM thumbnail
//----  --------------------------------------------------------------------------------------------------------
void Task::SetDwmThumbnail(HTHUMBNAIL dwmThumbnailId)
{
  (*this).dwmThumbnailId = dwmThumbnailId;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetDwmThumbnailWnd
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Returns the window that holds the current task's DWM thumbnail
//----  --------------------------------------------------------------------------------------------------------
HWND Task::GetDwmThumbnailWnd()
{
  return dwmThumbnailWnd;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetDwmThumbnailWnd
// Requires:	HTHUMBNAIL dwmThumbnailWnd - Value for DWM thumbnail
// Returns:	Nothing
// Purpose:	Sets the window that holds the current task's DWM thumbnail
//----  --------------------------------------------------------------------------------------------------------
void Task::SetDwmThumbnailWnd(HWND dwmThumbnailWnd)
{
  (*this).dwmThumbnailWnd = dwmThumbnailWnd;
}

void Task::CreateDwmThumbnail(HWND ownerWnd)
{
  if ((dwmThumbnailId != NULL) || (dwmThumbnailWnd != NULL))
    DestroyDwmThumbnail(); //remove any existing thumbnail before creating a new one

  HWND hwndSource = GetWnd();
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
  wincl.lpfnWndProc = dwmWindowProcedure;
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

  centerPoint.x = ELMid(GetRect()->left, GetRect()->right);
  centerPoint.y = GetRect()->top;
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
  SetWindowPos(dwmThumbnailWnd, NULL, thumbnailWndSnapMoveInfo.AppletRect->left, thumbnailWndSnapMoveInfo.AppletRect->top - (thumbnailWndSnapMoveInfo.AppletRect->bottom - thumbnailWndSnapMoveInfo.AppletRect->top), (thumbnailWndSnapMoveInfo.AppletRect->right - thumbnailWndSnapMoveInfo.AppletRect->left), (thumbnailWndSnapMoveInfo.AppletRect->bottom - thumbnailWndSnapMoveInfo.AppletRect->top), SWP_NOZORDER|SWP_SHOWWINDOW);

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

void Task::DestroyDwmThumbnail()
{
  if ((dwmThumbnailId != NULL) && (EGDwmUnregisterThumbnail(dwmThumbnailId) == S_OK))
    dwmThumbnailId = NULL;

  if ((dwmThumbnailWnd != NULL) && (DestroyWindow(dwmThumbnailWnd)))
    dwmThumbnailWnd = NULL;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	ToggleVisible
// Requires:	Nothing
// Returns:	Nothing
// Purpose:	Toggles the visibility state of the task icon
//----  --------------------------------------------------------------------------------------------------------
void Task::ToggleVisible()
{
  if (visible)
    visible = false;
  else
    visible = true;
}

void Task::UpdateIcon()
{
  convertIcon = true;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	UpdateTip
// Requires:	HINSTANCE mainInst - instance of calling window
// 		HWND mainWnd - handle of calling window
// 		HWND toolWnd - handle to the tooltip window
// 		WCHAR *tip - tooltip text
// Returns:	Nothing
// Purpose:	Updates the window handler with the task tooltip.  If
// 		the tip already exists, its updated.  If not, it is
// 		created.
//----  --------------------------------------------------------------------------------------------------------
void Task::UpdateTip(HWND mainWnd, HWND toolWnd, WCHAR *tip)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  bool exists;

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = mainWnd;
  ti.uId = (ULONG_PTR)wnd;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  //  complete the rest of the TOOLINFO structure
  ti.hinst =  (*this).mainInstance;
  ti.lpszText = tip;
  ti.rect = rect;

  // If it exists, modify the tooltip, if not add it
  if (exists)
    SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
  else
    SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	DeleteTip
// Requires:	HWND mainWnd - handle of calling window
// 		HWND toolWnd - handle to the tooltip window
// Returns:	Nothing
// Purpose:	Deletes the task tooltip from the tooltip window
//----  --------------------------------------------------------------------------------------------------------
void Task::DeleteTip(HWND mainWnd, HWND toolWnd)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  bool exists;

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.hwnd = mainWnd;
  ti.uId = (ULONG_PTR)wnd;

  // Check to see if the tooltip exists
  exists = SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti) ? true : false;

  if (exists)
    SendMessage(toolWnd, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	DisplayMenu
// Requires:	HWND callingWnd - Handle of calling Window
// Returns:	Nothing
// Purpose:	Builds task menu
//----  --------------------------------------------------------------------------------------------------------
void Task::DisplayMenu(HWND callingWnd)
{
  int res;
  POINT pt;
  GetCursorPos(&pt);

  /**< Bring the task window to the foreground like Explorer does. */
  ELSwitchToThisWindow(wnd);

  res = EAEDisplayMenu(callingWnd, wnd);
  switch (res)
    {
    case SC_SIZE:
    case SC_MOVE:
    case SC_MAXIMIZE:
    case SC_RESTORE:
      ELSwitchToThisWindow(wnd);
      break;
    }
  if (res)
    PostMessage(wnd, WM_SYSCOMMAND, (WPARAM)res, MAKELPARAM(pt.x, pt.y));
}

