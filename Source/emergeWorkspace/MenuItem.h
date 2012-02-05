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

#ifndef __ED_MENUITEM_H
#define __ED_MENUITEM_H

#include "../emergeLib/emergeLib.h"
#include "../emergeGraphics/emergeGraphics.h"
#include "CustomDropTarget.h"

class MenuItem
{
public:
  MenuItem(WCHAR *name, UINT type, WCHAR *value, WCHAR *workingDir, TiXmlElement *element);
  ~MenuItem();
  UINT GetType();
  WCHAR *GetName();
  WCHAR *GetValue();
  WCHAR *GetWorkingDir();
  TiXmlElement *GetElement();
  HICON GetIcon();
  IDropTarget *GetDropTarget();
  void SetIcon();
  void SetValue(WCHAR *value);
  void SetName(WCHAR *name);

private:
  TiXmlElement *element;
  UINT type;
  WCHAR name[MAX_LINE_LENGTH];
  WCHAR value[MAX_LINE_LENGTH];
  WCHAR workingDir[MAX_PATH];
  HICON icon;
  IDropTarget *dropTarget;
  CustomDropTarget *customDropTarget;
};

#endif

