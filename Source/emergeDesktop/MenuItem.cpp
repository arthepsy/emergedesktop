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
  WCHAR app[MAX_PATH], args[MAX_PATH];

  switch (type)
    {
    case IT_SEPARATOR:
      icon = EGGetWindowIcon((HWND)_wtoi(value), true, true);
      break;
    case IT_EXECUTABLE:
      if ((_wcsicmp(value, TEXT("%documents%")) == 0) ||
          (_wcsicmp(value, TEXT("%commondocuments%")) == 0))
        icon = EGGetSystemIcon(ICON_MYDOCUMENTS, 16);
      else if ((_wcsicmp(value, TEXT("%desktop%")) == 0) ||
               (_wcsicmp(value, TEXT("%commondesktop%")) == 0))
        icon = EGGetSystemIcon(ICON_DESKTOP, 16);
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

      switch (specialFolder)
        {
        case CSIDL_PERSONAL:
          icon = EGGetSystemIcon(ICON_MYDOCUMENTS, 16);
          break;
        case CSIDL_DRIVES:
          icon = EGGetSystemIcon(ICON_MYCOMPUTER, 16);
          break;
        case CSIDL_CONTROLS:
          icon = EGGetSystemIcon(ICON_CONTROLPANEL, 16);
          break;
        case CSIDL_NETWORK:
          icon = EGGetSystemIcon(ICON_NETWORKPLACES, 16);
          break;
        case CSIDL_BITBUCKET:
          icon = EGGetSystemIcon(ICON_RECYCLEBIN, 16);
          break;
        }
    }
    break;
    case IT_FILE_MENU:
      icon = EGGetFileIcon(value, 16);
      if (icon != NULL)
        break;
    case IT_XML_MENU:
    case IT_TASKS_MENU:
    case IT_SETTINGS_MENU:
      ELGetCurrentPath(app);
      icon = EGGetFileIcon(app, 16);
      break;
    }
}

void MenuItem::SetValue(WCHAR *value)
{
  wcscpy((*this).value, value);
}

void MenuItem::SetName(WCHAR *name)
{
  wcscpy((*this).name, name);
}
