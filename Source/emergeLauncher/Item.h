/*!
  @file Item.h
  @brief header for emergeLauncher
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

#ifndef __GUARD_b7da538b_7615_4761_85ce_927856fc94c2
#define __GUARD_b7da538b_7615_4761_85ce_927856fc94c2

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <vector>
#include "../emergeGraphics/emergeGraphics.h"
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeItemDefinitions.h"
#include "../emergeLib/emergeUtilityLib.h"

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
  Item(ITEMTYPE type, std::wstring app, std::wstring icon, std::wstring tip, std::wstring workingDir);
  ~Item();

  HWND GetWnd();
  HICON GetIcon();
  RECT* GetRect();
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
