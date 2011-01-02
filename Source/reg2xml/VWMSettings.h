//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009-2011  The Emerge Desktop Development Team
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

#ifndef __VWMSETTINGS_H
#define __VWMSETTINGS_H

#include "AppletSettings.h"

class VWMSettings : public AppletSettings
{
public:
  VWMSettings(HKEY key);
  bool ConvertSticky();

protected:
  void ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper);

private:
  HKEY key;
};

#endif
