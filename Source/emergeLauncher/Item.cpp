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
  if (type == IT_SEPARATOR)
    wcscpy(this->tip, TEXT(""));
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
  WCHAR source[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH], *lwrApp = _wcslwr(_wcsdup(app));

  if (origIcon)
    DestroyIcon(origIcon);

  convertIcon = true;

  switch (type)
    {
    case IT_SEPARATOR:
      if (_wcsicmp(app, TEXT("single")) == 0)
        {
          if (_wcsicmp(orientation, TEXT("vertical")) == 0)
            origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 15, iconSize);
          else
            origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 14, iconSize);
        }
      else if (_wcsicmp(app, TEXT("double")) == 0)
        {
          if (_wcsicmp(orientation, TEXT("vertical")) == 0)
            origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 17, iconSize);
          else
            origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 16, iconSize);
        }
      else if ((_wcsicmp(app, TEXT("custom")) == 0) && (wcslen(iconPath) > 0))
        origIcon = EGGetFileIcon(iconPath, iconSize);
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
                  if ((wcsstr(lwrApp, TEXT("%documents%")) != NULL) ||
                      (wcsstr(lwrApp, TEXT("%commondocuments%")) != NULL))
                    origIcon = EGGetSpecialFolderIcon(CSIDL_PERSONAL, 16);
                  else if ((wcsstr(lwrApp, TEXT("%desktop%")) != NULL) ||
                           (wcsstr(lwrApp, TEXT("%commondesktop%")) != NULL))
                    origIcon = EGGetSpecialFolderIcon(CSIDL_DESKTOP, 16);
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
            origIcon = EGGetSpecialFolderIcon(specialFolder, iconSize);
        }
      break;
    }

    free(lwrApp);
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

