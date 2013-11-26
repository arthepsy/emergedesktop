/*!
  @file MessageControl.h
  @brief header for emergeCore
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

  @note The message handling routine is based on MessageManager which is part
  of the BB4Win source code copyright 2001-2004 The Blackbox for Windows
  Development Team.
  */

#ifndef __GUARD_053a2c73_4076_469e_90bf_cd0ef2465907
#define __GUARD_053a2c73_4076_469e_90bf_cd0ef2465907

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#endif

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <windows.h>
#include <map>
#include <set>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeWindowLib.h"

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
  void Dispatch_Message(UINT type, UINT message, WCHAR* instanceName);
};

#endif
