//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------
//
// Note: The message handling routine is based on MessageManager which is part
// of the BB4Win source code copyright 2001-2004 The Blackbox for Windows
// Development Team.
//
//----  --------------------------------------------------------------------------------------------------------

#ifndef __EC_MESSAGECONTROL_H
#define __EC_MESSAGECONTROL_H

#include "../emergeLib/emergeLib.h"
#include <map>
#include <set>

typedef std::set<HWND> WindowSet;
typedef std::map< UINT, std::tr1::shared_ptr<WindowSet> > TypeMap;

class MessageControl
{
private:
  TypeMap types;
  UINT EmergeDesktopTypes[2];
  void DoAdd(HWND window, UINT message);
  void DoRemove(HWND window, UINT message);
public:
  MessageControl();
  ~MessageControl();
  void AddType(HWND window, UINT message);
  void RemoveType(HWND window, UINT message);
  void DispatchMessage(UINT type, UINT message, WCHAR *instanceName);
};

#endif

