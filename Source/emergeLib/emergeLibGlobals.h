/*!
  @file emergeLibGlobals.h
  @brief internal header for emergeLib
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

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

#ifndef __GUARD_394aa20d_7aa1_4ac5_bf9e_664e76dac21d
#define __GUARD_394aa20d_7aa1_4ac5_bf9e_664e76dac21d

#define UNICODE 1

#include <windows.h>
#include "emergeOSLib.h"

class emergeLibGlobals
{
public:
  static HINSTANCE getEmergeLibInstance();
  static HMODULE getMprDLL();
  static HMODULE getShell32DLL();
  static HMODULE getUser32DLL();
  static HMODULE getKernel32DLL();
  static HMODULE getDwmapiDLL();
  static HMODULE getShlwapiDLL();

protected:
  HINSTANCE emergeLibInstance;
  HMODULE mprDLL;
  HMODULE shell32DLL;
  HMODULE user32DLL;
  HMODULE kernel32DLL;
  HMODULE dwmapiDLL;
  HMODULE shlwapiDLL;

private:
  emergeLibGlobals();
  ~emergeLibGlobals();
};

#endif
