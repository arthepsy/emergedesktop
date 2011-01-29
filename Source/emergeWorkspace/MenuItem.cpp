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

#include "MenuItem.h"

MenuItem::MenuItem(WCHAR *name, UINT type, WCHAR* value, WCHAR *workingDir, TiXmlElement *element)
{
  LPVOID lpVoid;

  customDropTarget = new CustomDropTarget();
  customDropTarget->QueryInterface(IID_IDropTarget, &lpVoid);
  dropTarget = reinterpret_cast <IDropTarget*> (lpVoid);

  this->element = element;
  this->type = type;
  icon = NULL;

  if (name)
    wcscpy(this->name, name);
  else
    wcscpy(this->name, (WCHAR*)TEXT("\0"));

  if (value)
    wcscpy(this->value, value);
  else
    wcscpy(this->value, (WCHAR*)TEXT("\0"));

  if (workingDir)
    wcscpy(this->workingDir, workingDir);
  else
    wcscpy(this->workingDir, (WCHAR*)TEXT("\0"));
}

MenuItem::~MenuItem()
{
  if (dropTarget)
    dropTarget->Release();

  DestroyIcon(icon);
}

UINT MenuItem::GetType()
{
  return type;
}

TiXmlElement *MenuItem::GetElement()
{
  return element;
}

WCHAR *MenuItem::GetValue()
{
  return value;
}

WCHAR *MenuItem::GetName()
{
  return name;
}

IDropTarget *MenuItem::GetDropTarget()
{
  return dropTarget;
}

WCHAR *MenuItem::GetWorkingDir()
{
  return workingDir;
}

HICON MenuItem::GetIcon()
{
  return icon;
}

void MenuItem::SetIcon()
{
  WCHAR app[MAX_PATH], args[MAX_PATH], *lwrValue = _wcslwr(_wcsdup(value));

  switch (type)
    {
    case IT_SEPARATOR:
      icon = EGGetWindowIcon((HWND)_wtoi(value), true, true);
      break;
    case IT_EXECUTABLE:
      if ((wcsstr(lwrValue, TEXT("%documents%")) != NULL) ||
          (wcsstr(lwrValue, TEXT("%commondocuments%")) != NULL))
        icon = EGGetSpecialFolderIcon(CSIDL_PERSONAL, 16);
      else if ((wcsstr(lwrValue, TEXT("%desktop%")) != NULL) ||
               (wcsstr(lwrValue, TEXT("%commondesktop%")) != NULL))
        icon = EGGetSpecialFolderIcon(CSIDL_DESKTOP, 16);
      else
        {
          ELParseCommand(value, app, args);
          icon = EGGetFileIcon(app, 16);
        }
      break;
    case IT_INTERNAL_COMMAND:
      if (_wcsicmp(value, TEXT("logoff")) == 0)
        icon = EGGetSystemIcon(ICON_LOGOFF, 16);
      else if (_wcsicmp(value, TEXT("shutdown")) == 0)
        icon = EGGetSystemIcon(ICON_SHUTDOWN, 16);
      else if (_wcsicmp(value, TEXT("run")) == 0)
        icon = EGGetSystemIcon(ICON_RUN, 16);
      else if (_wcsicmp(value, TEXT("quit")) == 0)
        icon = EGGetSystemIcon(ICON_QUIT, 16);
      else if (_wcsicmp(value, TEXT("lock")) == 0)
        icon = EGGetSystemIcon(ICON_LOCK, 16);
      break;
    case IT_SPECIAL_FOLDER:
        {
          UINT specialFolder = ELIsSpecialFolder(value);
          icon = EGGetSpecialFolderIcon(specialFolder, 16);
        }
      break;
    case IT_FILE_MENU:
      if ((wcsstr(lwrValue, TEXT("%documents%")) != NULL) ||
          (wcsstr(lwrValue, TEXT("%commondocuments%")) != NULL))
        icon = EGGetSpecialFolderIcon(CSIDL_PERSONAL, 16);
      else if ((wcsstr(lwrValue, TEXT("%desktop%")) != NULL) ||
               (wcsstr(lwrValue, TEXT("%commondesktop%")) != NULL))
        icon = EGGetSpecialFolderIcon(CSIDL_DESKTOP, 16);
      else
        icon = EGGetFileIcon(value, 16);
      if (icon != NULL)
        break;
    case IT_XML_MENU:
    case IT_TASKS_MENU:
      ELGetCurrentPath(app);
      icon = EGGetFileIcon(app, 16);
      break;
    case IT_SETTINGS_MENU:
      icon = EGGetSystemIcon(ICON_EMERGE, 16);
      break;
    }

  free(lwrValue);
}

void MenuItem::SetValue(WCHAR *value)
{
  wcscpy((*this).value, value);
}

void MenuItem::SetName(WCHAR *name)
{
  wcscpy((*this).name, name);
}
