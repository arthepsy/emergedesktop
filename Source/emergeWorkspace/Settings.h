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

#ifndef __ED_SETTINGS_H
#define __ED_SETTINGS_H

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"
#include <vector>

class Settings : public BaseSettings
{
public:
  Settings();
  void GetDesktopRect(RECT *rect);
  int GetMenuAlpha();
  int GetTopBorder();
  int GetLeftBorder();
  int GetRightBorder();
  int GetBottomBorder();
  bool GetBorderPrimary();
  bool GetMenuIcons();
  bool GetAeroMenus();
  void SetTopBorder(int value);
  void SetLeftBorder(int value);
  void SetRightBorder(int value);
  void SetBottomBorder(int value);
  void SetBorderPrimary(bool value);
  void SetMenuAlpha(int value);
  void SetMenuIcons(bool value);
  void SetAeroMenus(bool value);

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void DoInitialize();
  virtual void ResetDefaults();

private:
  int menuAlpha, topBorder, bottomBorder, leftBorder, rightBorder;
  bool menuIcons,borderPrimary, aeroMenus;
};

#endif

