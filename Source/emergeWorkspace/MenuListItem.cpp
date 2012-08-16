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

#include "MenuListItem.h"

MenuListItem::MenuListItem(UINT type, WCHAR *value, TiXmlElement *section)
{
  this->type = type;
  this->section = section;

  if (value)
    wcscpy(this->value, value);
  else
    wcscpy(this->value, (WCHAR*)TEXT("\0"));
}

MenuListItem::~MenuListItem()
{
}

WCHAR *MenuListItem::GetValue()
{
  return value;
}

TiXmlElement *MenuListItem::GetSection()
{
  return section;
}

void MenuListItem::SetValue(WCHAR *value)
{
  wcscpy((*this).value, value);
}

void MenuListItem::SetSection(TiXmlElement *section)
{
  this->section = section;
}

UINT MenuListItem::GetType()
{
  return type;
}
