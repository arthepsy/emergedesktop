/*!
@file Settings.h
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

#ifndef __GUARD_8ABFCC33_0EC8_4676_8326_B9E2B3B4CA13
#define __GUARD_8ABFCC33_0EC8_4676_8326_B9E2B3B4CA13

#define UNICODE 1

#include <vector>
#include "../emergeBaseClasses/BaseSettings.h"

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
