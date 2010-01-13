//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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
//----  --------------------------------------------------------------------------------------------------------

#ifndef __HOTKEYCOMBO_H
#define __HOTKEYCOMBO_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include "../emergeLib/emergeLib.h"

class HotkeyCombo
{
public:
  HotkeyCombo(WCHAR *keyCombo, WCHAR *action);
  ~HotkeyCombo();
  WCHAR *GetHotkeyAction();
  UINT GetHotkeyModifiers();
  UINT GetHotkeyID();
  WCHAR GetHotkeyKey();
  WCHAR *GetHotkeyString();

private:
  WCHAR keyCombo[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH], key;
  UINT modifiers, ID;
  void ParseKeyCombo();
};

#endif
