// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#include "Item.h"
#include "resource.h"
#include <stdio.h>

//----  --------------------------------------------------------------------------------------------------------
// Function:	Constructor
// Requires:	HWND wnd - window handle of the task
// Returns:	Nothing
// Purpose:	Creates TrayIcon Class Object
//----  --------------------------------------------------------------------------------------------------------
Item::Item(int type, LPCTSTR app, LPCTSTR icon, LPCTSTR tip, LPCTSTR workingDir)
{
  this->type = type;
  wcscpy(this->app, app);
  if ((type == 0) || (type == 5))
    ZeroMemory((void*)tip, TIP_SIZE);
  else
    wcscpy(this->tip, tip);
  wcscpy(iconPath, icon);
  wcscpy(this->workingDir, workingDir);
  convertIcon = false;

  rect.left = 0;
  rect.right = 0;
  rect.top = 0;
  rect.bottom = 0;
}

void Item::CreateNewIcon(HDC backgroundDC, BYTE foregroundAlpha)
{
  HICON tmpIcon = NULL;
  if (convertIcon)
    {
      convertIcon = false;

      tmpIcon = EGConvertIcon(origIcon, rect, backgroundDC, foregroundAlpha);
      if (tmpIcon != NULL)
        {
          if (newIcon != NULL)
            DestroyIcon(newIcon);
          newIcon = CopyIcon(tmpIcon);
          DestroyIcon(tmpIcon);
        }
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	Deconstructor
// Requires:	Nothing
// Returns:	Nothing
// Purpose:	Deletes the task icon
//----  --------------------------------------------------------------------------------------------------------
Item::~Item()
{
  DestroyIcon(origIcon);
  DestroyIcon(newIcon);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetIcon
// Requires:	Nothing
// Returns:	HICON
// Purpose:	Retrieves the task icon
//----  --------------------------------------------------------------------------------------------------------
HICON Item::GetIcon()
{
  return newIcon;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetRect
// Requires:	Nothing
// Returns:	RECT*
// Purpose:	Retrieves the bounding rectangle of the task
//----  --------------------------------------------------------------------------------------------------------
RECT *Item::GetRect()
{
  return &rect;
}

int Item::GetType()
{
  return type;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetIcon
// Requires:	HICON icon - new icon to use
// Returns:	Nothing
// Purpose:	Replaces existing task icon with new icon
//----  --------------------------------------------------------------------------------------------------------
void Item::SetIcon(int iconSize, WCHAR *orientation)
{
  WCHAR source[MAX_LINE_LENGTH];
  WCHAR tmp[MAX_LINE_LENGTH];

  if (origIcon)
    DestroyIcon(origIcon);

  convertIcon = true;

  switch (type)
    {
    case IT_SEPARATOR:
      if (wcsicmp(orientation, TEXT("vertical")) == 0)
        origIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_HSEPARATOR), IMAGE_ICON, iconSize, iconSize, 0);
      else
        origIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_VSEPARATOR), IMAGE_ICON, iconSize, iconSize, 0);
      break;
    case IT_DOUBLESEPARATOR:
      if (wcsicmp(orientation, TEXT("vertical")) == 0)
        origIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DOUBLEHSEPARATOR), IMAGE_ICON, iconSize, iconSize, 0);
      else
        origIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DOUBLEVSEPARATOR), IMAGE_ICON, iconSize, iconSize, 0);
      break;
    default:
      if (wcslen(iconPath) > 0)
        origIcon = EGGetFileIcon(iconPath, iconSize);
      else
        {
          UINT specialFolder = ELIsSpecialFolder(app);
          if (specialFolder == 0)
            {
              UINT internalCommand = ELIsInternalCommand(app);
              if (internalCommand == 0)
                {
                  if ((_wcsicmp(app, TEXT("%documents%")) == 0) ||
                      (_wcsicmp(app, TEXT("%commondocuments%")) == 0))
                    origIcon = EGGetSystemIcon(ICON_MYDOCUMENTS, iconSize);
                  else if ((_wcsicmp(app, TEXT("%desktop%")) == 0) ||
                           (_wcsicmp(app, TEXT("%commondesktop%")) == 0))
                    origIcon = EGGetSystemIcon(ICON_DESKTOP, iconSize);
                  else
                    {
                      ELParseCommand(app, source, tmp);
                      origIcon = EGGetFileIcon(source, iconSize);
                    }
                }
              else
                {
                  switch (internalCommand)
                    {
                    case COMMAND_LOGOFF:
                      origIcon = EGGetSystemIcon(ICON_LOGOFF, iconSize);
                      break;

                    case COMMAND_QUIT:
                      origIcon = EGGetSystemIcon(ICON_QUIT, iconSize);
                      break;

                    case COMMAND_RUN:
                      origIcon = EGGetSystemIcon(ICON_RUN, iconSize);
                      break;

                    case COMMAND_SHUTDOWN:
                      origIcon = EGGetSystemIcon(ICON_SHUTDOWN, iconSize);
                      break;

                    case COMMAND_LOCK:
                      origIcon = EGGetSystemIcon(ICON_LOCK, iconSize);
                      break;
                    }
                }
            }
          else
            {
              switch (specialFolder)
                {
                case CSIDL_DRIVES:
                  origIcon = EGGetSystemIcon(ICON_MYCOMPUTER, iconSize);
                  break;

                case CSIDL_CONTROLS:
                  origIcon = EGGetSystemIcon(ICON_CONTROLPANEL, iconSize);
                  break;

                case CSIDL_NETWORK:
                  origIcon = EGGetSystemIcon(ICON_NETWORKPLACES, iconSize);
                  break;

                case CSIDL_BITBUCKET:
                  origIcon = EGGetSystemIcon(ICON_RECYCLEBIN, iconSize);
                  break;
                }
            }
        }
      break;
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetRect
// Requires:	RECT rect - new bounding rectangle
// Returns:	Nothing
// Purpose:	Replaces the bounding rectangle of the task
//----  --------------------------------------------------------------------------------------------------------
void Item::SetRect(RECT rect)
{
  if (!EqualRect(&(*this).rect, &rect))
    {
      (*this).rect = rect;

      convertIcon = true;
    }
}

WCHAR *Item::GetApp()
{
  return app;
}

WCHAR *Item::GetTip()
{
  return tip;
}

WCHAR *Item::GetIconPath()
{
  return iconPath;
}

WCHAR *Item::GetWorkingDir()
{
  return workingDir;
}

