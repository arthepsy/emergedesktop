// vim: tags+=../emergeLib/tags,../emergeGraphics/tags
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

#include "MenuItem.h"

MenuItem::MenuItem(MENUITEMDATA menuItemData, HMENU menu)
{
  CopyMemory(&this->menuItemData, &menuItemData, sizeof(MENUITEMDATA));
  this->menu = menu;
  icon = NULL;

  CreateDropTarget(&dropTarget, this->menuItemData, (UINT_PTR)this, menu);
}

MenuItem::~MenuItem()
{
  if (dropTarget)
    dropTarget->Release();

  DestroyIcon(icon);
}

HMENU MenuItem::GetMenu()
{
  return menu;
}

UINT_PTR MenuItem::GetID()
{
  return reinterpret_cast<UINT_PTR>(this);
}

UINT MenuItem::GetType()
{
  return menuItemData.type;
}

TiXmlElement *MenuItem::GetElement()
{
  return menuItemData.element;
}

WCHAR *MenuItem::GetValue()
{
  return menuItemData.value;
}

WCHAR *MenuItem::GetName()
{
  return menuItemData.name;
}

IDropTarget *MenuItem::GetDropTarget()
{
  return dropTarget;
}

WCHAR *MenuItem::GetWorkingDir()
{
  return menuItemData.workingDir;
}

HICON MenuItem::GetIcon()
{
  return icon;
}

MENUITEMDATA *MenuItem::GetMenuItemData()
{
  return &menuItemData;
}

void MenuItem::SetIcon()
{
  WCHAR command[MAX_PATH], args[MAX_PATH], lwrValue[MAX_LINE_LENGTH];
  HWND task;
  std::wstring app;

  wcscpy(lwrValue, menuItemData.value);
  _wcslwr(lwrValue);

  switch (menuItemData.type)
    {
    case IT_TASK:
#ifdef _W64
      task = (HWND)_wcstoi64(menuItemData.value, NULL, 10);
#else
      task = (HWND)wcstol(menuItemData.value, NULL, 10);
#endif
      icon = EGGetWindowIcon(NULL, task, true, true);
      /* If the task icon is NULL, generate a default icon using the
       * application's icon.
       */
      if (icon == NULL)
        {
          app = ELGetWindowApp(task, true);
          icon = EGGetFileIcon(app.c_str(), 16);
        }
      break;
    case IT_EXECUTABLE:
    case IT_FILE:
      if ((_wcsicmp(lwrValue, TEXT("%documents%")) == 0) ||
          (_wcsicmp(lwrValue, TEXT("%commondocuments%")) == 0))
        icon = EGGetSpecialFolderIcon(CSIDL_PERSONAL, 16);
      else if ((_wcsicmp(lwrValue, TEXT("%desktop%")) == 0) ||
               (_wcsicmp(lwrValue, TEXT("%commondesktop%")) == 0))
        icon = EGGetSpecialFolderIcon(CSIDL_DESKTOP, 16);
      else
        {
          ELParseCommand(lwrValue, command, args);
          icon = EGGetFileIcon(command, 16);
        }
      break;
    case IT_INTERNAL_COMMAND:
      app = menuItemData.value;
      app = ELToLower(app.substr(0, app.find_first_of(TEXT(" \t"))));
      if (app == TEXT("logoff"))
        icon = EGGetSystemIcon(ICON_LOGOFF, 16);
      else if (app == TEXT("shutdown"))
        icon = EGGetSystemIcon(ICON_SHUTDOWN, 16);
      else if (app == TEXT("run"))
        icon = EGGetSystemIcon(ICON_RUN, 16);
      else if (app == TEXT("quit"))
        icon = EGGetSystemIcon(ICON_QUIT, 16);
      else if (app == TEXT("lock"))
        icon = EGGetSystemIcon(ICON_LOCK, 16);
      break;
    case IT_SPECIAL_FOLDER:
        {
          UINT specialFolder = ELGetSpecialFolderIDFromPath(menuItemData.value);
          icon = EGGetSpecialFolderIcon(specialFolder, 16);
        }
      break;
    case IT_FILE_MENU:
    case IT_FILE_SUBMENU:
      app = menuItemData.value;
      app = ELToLower(app.substr(0, app.find_first_of(TEXT("|"))));
      if ((app == TEXT("%documents%")) ||
          (app == TEXT("%commondocuments%")))
        icon = EGGetSpecialFolderIcon(CSIDL_PERSONAL, 16);
      else if ((app == TEXT("%desktop%")) ||
               (app == TEXT("%commondesktop%")))
        icon = EGGetSpecialFolderIcon(CSIDL_DESKTOP, 16);
      else
        icon = EGGetFileIcon(app.c_str(), 16);
      if (icon != NULL)
        break;
    case IT_XML_MENU:
    case IT_TASKS_MENU:
      wcscpy(command, ELGetCurrentPath().c_str());
      icon = EGGetFileIcon(command, 16);
      break;
    case IT_SETTINGS_MENU:
      icon = EGGetSystemIcon(ICON_EMERGE, 16);
      break;
    case IT_HELP_MENU:
      icon = EGGetSystemIcon(ICON_QUESTION, 16);
      break;
    }
}

void MenuItem::SetValue(WCHAR *value)
{
  wcscpy(menuItemData.value, value);
}

void MenuItem::SetName(WCHAR *name)
{
  wcscpy(menuItemData.name, name);
}

void MenuItem::SetElement(TiXmlElement *element)
{
  menuItemData.element = element;
}
