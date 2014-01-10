/*!
  @file EmergeInternalCommandInterface.h
  @brief header for emergeLib
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

#ifndef __GUARD_ac25cd3e_9130_4800_83d3_fecfa010a169
#define __GUARD_ac25cd3e_9130_4800_83d3_fecfa010a169

#include <tuple>
#include <vector>
#include "../../cppcomponents/cppcomponents/cppcomponents.hpp"

typedef std::tuple<std::wstring, int, std::wstring> Command; //commandName, commandValue, commandHandlerFunctionName

struct ICommandProvider:cppcomponents::define_interface<cppcomponents::uuid<0xf4b4056d, 0x37a8, 0x4f32, 0x9eea, 0x03a31ed55dfa>>
{
    std::vector<Command>GetEmergeInternalCommands();

    CPPCOMPONENTS_CONSTRUCT(ICommandProvider, GetEmergeInternalCommands)
};

inline std::string CommandProviderId(){ return "CommandProvider"; }
typedef cppcomponents::runtime_class<CommandProviderId, cppcomponents::object_interfaces<ICommandProvider>> CommandProvider_t;
typedef cppcomponents::use_runtime_class<CommandProvider_t> CommandProvider;

typedef bool (*IEMERGEINTERNALCOMMANDHANDLERPROC)(std::vector<std::wstring>); //the std::vector<std::wstring> contains arguments for the internal command

#endif
