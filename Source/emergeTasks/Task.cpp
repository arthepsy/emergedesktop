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
  :Thumbnail(task, mainInstance)
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
  hidden = false;

  convertIcon = true;
}

bool Task::GetHidden()
{
  return hidden;
}

void Task::SetHidden(bool hidden)
{
  this->hidden = hidden;
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

