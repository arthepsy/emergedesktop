//---
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
//---

#pragma once

#include <vector>
#include "../emergeLib/emergeLib.h"
#include "MenuItem.h"

class MenuListItem
{
public:
  MenuListItem(UINT type, WCHAR *value, TiXmlElement *section);
  ~MenuListItem();
  WCHAR *GetValue();
  TiXmlElement *GetSection();
  void SetValue(WCHAR *value);
  void SetSection(TiXmlElement *section);
  UINT GetType();
  MenuItem *GetMenuItem(UINT index);
  UINT GetMenuItemCount();
  void AddMenuItem(MenuItem *menuItem);
  void DeleteMenuItem(UINT index);
  MenuItem *FindMenuItem(UINT id);

private:
  WCHAR value[MAX_LINE_LENGTH];
  UINT type;
  TiXmlElement *section;
  std::vector< std::tr1::shared_ptr<MenuItem> > menuItems;
};
