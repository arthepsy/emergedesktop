/*!
@file MenuItem.h
@brief header for emergeWorkspace
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

#ifndef __GUARD_3C06BCA7_24FD_4F8D_9021_B6DF900197D0
#define __GUARD_3C06BCA7_24FD_4F8D_9021_B6DF900197D0

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#endif

#define MAX_LINE_LENGTH 4096

#include <windows.h>
#include <shlobj.h>
#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeItemDefinitions.h"
#include "../emergeLib/emergeOSLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "../tinyxml/tinyxml.h"
#include "CustomDropTarget.h"

class MenuItem
{
public:
  MenuItem(MENUITEMDATA menuItemData, HMENU menu);
  ~MenuItem();
  UINT GetType();
  WCHAR *GetName();
  WCHAR *GetValue();
  WCHAR *GetWorkingDir();
  HMENU GetMenu();
  TiXmlElement *GetElement();
  HICON GetIcon();
  IDropTarget *GetDropTarget();
  IDropSource *GetDropSource();
  void SetIcon();
  void SetValue(WCHAR *value);
  void SetName(WCHAR *name);
  void SetElement(TiXmlElement *element);
  UINT_PTR GetID();
  MENUITEMDATA *GetMenuItemData();

private:
  MENUITEMDATA menuItemData;
  HICON icon;
  IDropTarget *dropTarget;
  HMENU menu;
};

#endif
