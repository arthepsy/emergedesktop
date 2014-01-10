/*!
@file emergeItemDefinitions.h
@brief export header for emergeLib
@author The Emerge Desktop Development Team

@attention This file is part of Emerge Desktop.
@attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

@attention Emerge Desktop is free software; you can redistribute it and/or
modify  it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

@attention Emerge Desktop is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

@attention You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GUARD_C46E8EFA_762C_4DB0_BCBE_41696EA4CAD7
#define __GUARD_C46E8EFA_762C_4DB0_BCBE_41696EA4CAD7

#define UNICODE 1
#define MAX_LINE_LENGTH 4096

#ifdef EMERGELIB_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#include <windows.h>
#include "../tinyxml/tinyxml.h"

typedef enum _ITEMTYPE
{
  IT_SEPARATOR = 0,
  IT_EXECUTABLE,
  IT_INTERNAL_COMMAND,
  IT_DATE_TIME,
  IT_SPECIAL_FOLDER,
  IT_ENTIRE_FOLDER,
  IT_LIVE_FOLDER,
  IT_LIVE_FOLDER_ITEM,
  IT_FILE,
  IT_TASK,
  IT_SETTING_ITEM,
  IT_HELP_ITEM
} ITEMTYPE;

typedef enum _MENUTYPE
{
  IT_XML_MENU = 100,
  IT_FILE_MENU,
  IT_TASKS_MENU,
  IT_SETTINGS_MENU,
  IT_HELP_MENU,
  IT_FILE_SUBMENU
} MENUTYPE;

typedef struct _MENUITEMDATA
{
  WCHAR name[MAX_LINE_LENGTH];
  int type;
  WCHAR value[MAX_LINE_LENGTH];
  WCHAR workingDir[MAX_LINE_LENGTH];
  TiXmlElement *element;
  HMENU subMenu;
}
MENUITEMDATA, *LPMENUITEMDATA;

#endif
