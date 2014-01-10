/*!
@file MenuListItem.h
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

#ifndef __GUARD_D68794F5_BD1A_4D42_88D3_1A35E0D0E994
#define __GUARD_D68794F5_BD1A_4D42_88D3_1A35E0D0E994

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#include <windows.h>
#include <vector>
#include "../tinyxml/tinyxml.h"

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

private:
  WCHAR value[MAX_LINE_LENGTH];
  UINT type;
  TiXmlElement *section;
};

#endif
