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

#ifndef __EL_ITEM_H
#define __EL_ITEM_H

#include "../emergeLib/emergeLib.h"
#include "../emergeGraphics/emergeGraphics.h"
#include <commctrl.h>
#include <vector>
#include <shlobj.h>

// Define icon and tip settings
#define TIP_SIZE 256

#ifndef GIL_FORSHORTCUT
#define GIL_FORSHORTCUT 128
#endif

#ifndef GIL_DEFAULTICON
#define GIL_DEFAULTICON 64
#endif

//====================
// The Item Class
class Item
{
public:
  Item(int type, LPCTSTR app, LPCTSTR icon, LPCTSTR tip, LPCTSTR workingDir);
  ~Item();

  HWND GetWnd();
  HICON GetIcon();
  RECT *GetRect();
  void SetIcon(int iconSize, WCHAR *orientation);
  void SetRect(RECT rect);
  void CreateNewIcon(BYTE foregroundAlpha, BYTE backgroundAlpha);
  WCHAR *GetApp();
  WCHAR *GetTip();
  int GetType();
  WCHAR *GetIconPath();
  WCHAR *GetWorkingDir();
  bool GetActive();
  void SetActive(bool active);

private:
  WCHAR app[MAX_PATH], iconPath[MAX_PATH];
  HICON origIcon, newIcon;
  WCHAR tip[TIP_SIZE];
  WCHAR workingDir[MAX_LINE_LENGTH];
  RECT rect;
  bool convertIcon, active;
  int type;
};

//====================

#endif

