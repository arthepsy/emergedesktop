// vim: tags+=../emergeLib/tags
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

#include "Item.h"
#include "resource.h"
#include <stdio.h>

//----  --------------------------------------------------------------------------------------------------------
// Function:	Constructor
// Requires:	HWND wnd - window handle of the task
// Returns:	Nothing
// Purpose:	Creates TrayIcon Class Object
//----  --------------------------------------------------------------------------------------------------------
Item::Item(ITEMTYPE type, std::wstring app, std::wstring icon, std::wstring tip, std::wstring workingDir)
{
  this->type = type;
  this->app = app;
  if (type == IT_SEPARATOR)
  {
    this->tip = TEXT("");
  }
  else
  {
    this->tip = tip;
  }
  iconPath = icon;
  this->workingDir = workingDir;
  convertIcon = false;
  active = false;

  rect.left = 0;
  rect.right = 0;
  rect.top = 0;
  rect.bottom = 0;
}

void Item::CreateNewIcon(BYTE foregroundAlpha, BYTE backgroundAlpha)
{
  HICON tmpIcon = NULL;

  /**< Don't bother converting NULL icons, just set newIcon and return */
  if (origIcon == NULL)
  {
    newIcon = NULL;
    return;
  }

  if (convertIcon)
  {
    convertIcon = false;

    // If the background if fully opaque, don't bother converting the icon, simply copy it
    if (backgroundAlpha == 0xff)
    {
      if (newIcon != NULL)
      {
        DestroyIcon(newIcon);
      }
      newIcon = CopyIcon(origIcon);
    }
    else
    {
      /**< Don't bail if EGConvertIcon returns a NULL icon, since in this case it may be valid (icon flashing) */
      tmpIcon = EGConvertIcon(origIcon, foregroundAlpha);
      if (newIcon != NULL)
      {
        DestroyIcon(newIcon);
      }
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
  if (origIcon)
  {
    DestroyIcon(origIcon);
  }
  if (newIcon)
  {
    DestroyIcon(newIcon);
  }
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

bool Item::GetActive()
{
  return active;
}

void Item::SetActive(bool active)
{
  if (type == IT_SEPARATOR)
  {
    this->active = false;
  }
  else
  {
    this->active = active;
  }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	GetRect
// Requires:	Nothing
// Returns:	RECT*
// Purpose:	Retrieves the bounding rectangle of the task
//----  --------------------------------------------------------------------------------------------------------
RECT* Item::GetRect()
{
  return &rect;
}

ITEMTYPE Item::GetType()
{
  return type;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	SetIcon
// Requires:	HICON icon - new icon to use
// Returns:	Nothing
// Purpose:	Replaces existing task icon with new icon
//----  --------------------------------------------------------------------------------------------------------
void Item::SetIcon(int iconSize, std::wstring orientation)
{
  WCHAR source[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH];
  std::wstring workingApp, lwrApp;

  workingApp = app;
  lwrApp = ELToLower(app);

  if (origIcon)
  {
    DestroyIcon(origIcon);
  }

  convertIcon = true;

  switch (type)
  {
  case IT_SEPARATOR:
    if (ELToLower(app) == TEXT("single"))
    {
      if (ELToLower(orientation) == TEXT("vertical"))
      {
        origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 15, iconSize);
      }
      else
      {
        origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 14, iconSize);
      }
    }
    else if (ELToLower(app) == TEXT("double"))
    {
      if (ELToLower(orientation) == TEXT("vertical"))
      {
        origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 17, iconSize);
      }
      else
      {
        origIcon = EGExtractIcon(TEXT("emergeIcons.dll"), 16, iconSize);
      }
    }
    else if ((ELToLower(app) == TEXT("custom")) && (!iconPath.empty()))
    {
      origIcon = EGGetFileIcon(iconPath, iconSize);
    }
    break;
  default:
    if (!iconPath.empty())
    {
      origIcon = EGGetFileIcon(iconPath, iconSize);
    }
    else
    {
      int specialFolder = ELGetSpecialFolderIDFromPath(app);
      if (specialFolder == 0)
      {
        workingApp = workingApp.substr(0, workingApp.find_first_of(TEXT(" \t")));
        int internalCommand = ELGetInternalCommandValue(workingApp);
        if (internalCommand == 0)
        {
          if ((ELToLower(lwrApp) == TEXT("%documents%")) ||
              (ELToLower(lwrApp) == TEXT("%commondocuments%")))
          {
            origIcon = EGGetSpecialFolderIcon(CSIDL_PERSONAL, iconSize);
          }
          else if ((ELToLower(lwrApp) == TEXT("%desktop%")) ||
                   (ELToLower(lwrApp) == TEXT("%commondesktop%")))
          {
            origIcon = EGGetSpecialFolderIcon(CSIDL_DESKTOP, iconSize);
          }
          else
          {
            std::wstring workingApp = ELGetAbsolutePath(lwrApp);
            ELParseCommand(workingApp.c_str(), source, tmp);
            origIcon = EGGetFileIcon(source, iconSize);
          }
        }
        else
        {
          switch (internalCommand)
          {
          case CORE_LOGOFF:
            origIcon = EGGetSystemIcon(ICON_LOGOFF, iconSize);
            break;

          case CORE_QUIT:
            origIcon = EGGetSystemIcon(ICON_QUIT, iconSize);
            break;

          case CORE_RUN:
            origIcon = EGGetSystemIcon(ICON_RUN, iconSize);
            break;

          case CORE_SHUTDOWN:
            origIcon = EGGetSystemIcon(ICON_SHUTDOWN, iconSize);
            break;

          case CORE_LOCK:
            origIcon = EGGetSystemIcon(ICON_LOCK, iconSize);
            break;
          }
        }
      }
      else
      {
        origIcon = EGGetSpecialFolderIcon(specialFolder, iconSize);
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

std::wstring Item::GetApp()
{
  return app;
}

std::wstring Item::GetTip()
{
  return tip;
}

std::wstring Item::GetIconPath()
{
  return iconPath;
}

std::wstring Item::GetWorkingDir()
{
  return workingDir;
}

