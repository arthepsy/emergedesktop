/*!
  @file InternalCommandEngine.h
  @brief internal header for emergeLib
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

#ifndef __GUARD_0c1a6dfe_ccd0_4681_80e5_c71012946e10
#define __GUARD_0c1a6dfe_ccd0_4681_80e5_c71012946e10

#define UNICODE 1

#define INVALID_COMMAND 0

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>
#include <string>
#include <vector>
#include "../emergeCoreLib.h"
#include "../emergeFileRegistryLib.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "EmergeInternalCommandInterface.h"

struct CommandInfoStruct //the Command tuple maps to this struct
{
  int commandValue;
  std::wstring commandName;
  std::wstring commandProviderDLLName; //this is the filename of the DLL with the handler function(s). It's a std::string because that's what GetProcAddress expects.
  std::wstring commandHandlerFunctionName; //this is the name of the function we call GetProcAddress on
};

//Helper functions
std::vector<CommandInfoStruct> LoadEmergeInternalCommands();
CommandInfoStruct FindCommandHandler(std::wstring commandName);

#endif
