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

#ifndef __EL_ITEM_H
#define __EL_ITEM_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <shlobj.h>
#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeUtilityLib.h"

// Define icon and tip settings
#define TIP_SIZE 256

#ifndef GIL_FORSHORTCUT
#define GIL_FORSHORTCUT 128
#endif

#ifndef GIL_DEFAULTICON
#define GIL_DEFAULTICON 64
#endif

typedef enum _ITEMTYPE {
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
  IT_HELP_ITEM,
  IT_XML_MENU = 100,
  IT_FILE_MENU,
  IT_TASKS_MENU,
  IT_SETTINGS_MENU,
  IT_HELP_MENU,
  IT_FILE_SUBMENU
} ITEMTYPE;

//====================
// The Item Class
class Item
{
public:
  Item(ITEMTYPE type, std::wstring app, std::wstring icon, std::wstring tip, std::wstring workingDir);
  ~Item();

  HWND GetWnd();
  HICON GetIcon();
  RECT *GetRect();
  void SetIcon(int iconSize, std::wstring orientation);
  void SetRect(RECT rect);
  void CreateNewIcon(BYTE foregroundAlpha, BYTE backgroundAlpha);
  std::wstring GetApp();
  std::wstring GetTip();
  ITEMTYPE GetType();
  std::wstring GetIconPath();
  std::wstring GetWorkingDir();
  bool GetActive();
  void SetActive(bool active);

private:
  std::wstring app, iconPath;
  HICON origIcon, newIcon;
  std::wstring tip;
  std::wstring workingDir;
  RECT rect;
  bool convertIcon, active;
  ITEMTYPE type;
};

//====================

#endif

