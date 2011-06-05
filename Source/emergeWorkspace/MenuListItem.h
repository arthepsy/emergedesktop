//---
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
//---

#ifndef __ED_MENULISTITEM_H
#define __ED_MENULISTITEM_H

#include "../emergeLib/emergeLib.h"
#include "MenuItem.h"
#include <vector>

class MenuListItem
{
public:
  MenuListItem(WCHAR *name, UINT type, WCHAR *value, TiXmlElement *section);
  ~MenuListItem();
  WCHAR *GetName();
  WCHAR *GetValue();
  TiXmlElement *GetSection();
  void SetValue(WCHAR *value);
  void SetName(WCHAR *name);
  void SetSection(TiXmlElement *section);
  UINT GetType();
  MenuItem *GetMenuItem(UINT index);
  UINT GetMenuItemCount();
  void AddMenuItem(MenuItem *menuItem);
  void DeleteMenuItem(UINT index);
  IDropTarget *GetDropTarget();

private:
  WCHAR value[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH];
  UINT type;
  IDropTarget *dropTarget;
  std::tr1::shared_ptr<CustomDropTarget> customDropTarget;
  TiXmlElement *section;
  std::vector< std::tr1::shared_ptr<MenuItem> > menuItems;
};

#endif

