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

#include "Applet.h"

//----  --------------------------------------------------------------------------------------------------------
// Function:	Constructor
// Requires:	HWND wnd - window handle of the task
// Returns:	Nothing
// Purpose:	Creates TrayIcon Class Object
//----  --------------------------------------------------------------------------------------------------------
Task::Task(HWND taskWnd, HWND mainWnd, HINSTANCE mainInst, UINT currentRow, UINT currentColumn,
           UINT maxRows, UINT maxColumns, GUIINFO guiInfo)
{
  (*this).taskWnd = taskWnd;
  (*this).mainWnd = mainWnd;
  (*this).mainInst = mainInst;

  minimized = (IsIconic(taskWnd) == TRUE);

  appName = ELToLower(ELGetWindowApp(taskWnd, false));

  UpdateDimensions(currentColumn, currentRow, maxRows, maxColumns, guiInfo);

  referenceRect = ELGetWindowRect(taskWnd);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	Deconstructor
// Requires:	Nothing
// Returns:	Nothing
// Purpose:	Deletes the task icon
//----  --------------------------------------------------------------------------------------------------------
Task::~Task()
{}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetAppName
// Requires:	Nothing
// Returns:	WCHAR *
// Purpose:	Retrieves the application the window belongs to
//----  --------------------------------------------------------------------------------------------------------
std::wstring Task::GetAppName()
{
  return appName;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetReferenceRect
// Requires:	Nothing
// Returns:	RECT*
// Purpose:	Retrieves the task reference rectangle
//----  --------------------------------------------------------------------------------------------------------
RECT* Task::GetReferenceRect()
{
  return &referenceRect;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetReferenceRect
// Requires:	RECT
// Returns:	Nothing
// Purpose:	Sets the task reference rectangle
//----  --------------------------------------------------------------------------------------------------------
void Task::SetReferenceRect(RECT referenceRect)
{
  (*this).referenceRect = referenceRect;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetWnd
// Requires:	Nothing
// Returns:	HWND
// Purpose:	Retrieves the task window
//----  --------------------------------------------------------------------------------------------------------
HWND Task::GetTaskWnd()
{
  return taskWnd;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetRect
// Requires:	Nothing
// Returns:	RECT*
// Purpose:	Retrieves the task rect
//----  --------------------------------------------------------------------------------------------------------
RECT* Task::GetRect()
{
  return &taskRect;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	UpdateDimensions
// Requires:	UINT currentcolumn - currenct column
//              UINT currentRow - current row
// Returns:	Nothing
// Purpose:	Builds task menu
//----  --------------------------------------------------------------------------------------------------------
void Task::UpdateDimensions(UINT currentColumn, UINT currentRow, UINT maxColumns, UINT maxRows, GUIINFO guiInfo)
{
  RECT sourceRect, clientRect;
  int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN) + 10;
  int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN) + 10;
  int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
  int windowWidth, windowHeight;
  float rowScalar, columnScalar;

  sourceRect = ELGetWindowRect(taskWnd);
  GetClientRect(mainWnd, &clientRect);

  rowScalar = (float)(clientRect.bottom - (2 * guiInfo.dragBorder)) /
              (float)maxRows;
  rowScalar /= (float)screenHeight;

  columnScalar = (float)(clientRect.right - (2 * guiInfo.dragBorder)) /
                 (float)maxColumns;
  columnScalar /= (float)screenWidth;

  windowWidth = sourceRect.right - sourceRect.left;
  windowWidth = (int)(windowWidth * columnScalar);
  windowHeight = sourceRect.bottom - sourceRect.top;
  windowHeight = (int)(windowHeight * rowScalar);

  taskRect.left = (sourceRect.left - screenLeft) + (currentColumn * screenWidth);
  taskRect.left = (int)(taskRect.left * columnScalar);
  taskRect.left += guiInfo.dragBorder;
  taskRect.top = (sourceRect.top - screenTop) + (currentRow * screenHeight);
  taskRect.top = (int)(taskRect.top * rowScalar);
  taskRect.top += guiInfo.dragBorder;

  taskRect.bottom = taskRect.top + windowHeight;
  taskRect.right = taskRect.left + windowWidth;
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

  res = EAEDisplayMenu(callingWnd, taskWnd);
  if (res)
  {
    if (res != SC_CLOSE)
    {
      ELSwitchToThisWindow(taskWnd);
    }
    PostMessage(taskWnd, WM_SYSCOMMAND, (WPARAM)res, MAKELPARAM(pt.x, pt.y));
  }
}

void Task::SetMinimized(bool minimized)
{
  (*this).minimized = minimized;
}

bool Task::GetMinimized()
{
  return minimized;
}

