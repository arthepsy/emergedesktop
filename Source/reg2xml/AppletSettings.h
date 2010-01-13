//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009-2010  The Emerge Desktop Development Team
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

#ifndef __APPLETSETTINGS_H
#define __APPLETSETTINGS_H

#undef _WIN32_IE
#define _WIN32_IE	0x600

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"

class AppletSettings : public BaseSettings
{
public:
  AppletSettings(HKEY key);
  bool ConvertReg();

protected:
  virtual void ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper);
  std::wstring appletName;

private:
  HKEY key;
};

#endif
