//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

#include "InternalCommandEngine.h"

bool ELIsInternalCommand(std::wstring commandName)
{
  return (ELGetInternalCommandValue(commandName) != INVALID_COMMAND);
}

std::wstring ELGetInternalCommandName(int commandValue)
{
  std::vector<CommandInfoStruct> internalCommands = LoadEmergeInternalCommands();
  std::vector<CommandInfoStruct>::const_iterator tempIter;
  CommandInfoStruct tempValue;

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    if (tempValue.commandValue == commandValue)
    {
      return tempValue.commandName;
    }
  }

  return TEXT("");
}

int ELGetInternalCommandValue(std::wstring commandName)
{
  std::vector<CommandInfoStruct> internalCommands = LoadEmergeInternalCommands();
  std::vector<CommandInfoStruct>::const_iterator tempIter;
  CommandInfoStruct tempValue;

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    if (ELToLower(tempValue.commandName) == ELToLower(commandName))
    {
      return tempValue.commandValue;
    }
  }

  return INVALID_COMMAND;
}

bool ELExecuteInternalCommand(std::wstring commandName, std::wstring arguments)
{
  CommandInfoStruct command;
  HMODULE commandProviderDLL;
  std::string commandHandlerName;
  IEMERGEINTERNALCOMMANDHANDLERPROC commandHandlerCallback;
  std::vector<std::wstring> args;
  std::wstring argSplitters = TEXT(" ,\t"); //space, comma, tab
  size_t firstArgSplitPos = 0, secondArgSplitPos;
  bool returnValue = false;

  if (commandName.empty())
  {
    return false;
  }

  command = FindCommandHandler(commandName);
  if (command.commandHandlerFunctionName == TEXT(""))
  {
    return false;
  }

  secondArgSplitPos = arguments.find_first_of(argSplitters);
  if ((secondArgSplitPos == std::wstring::npos) || (secondArgSplitPos == arguments.length()))
  {
    args.push_back(arguments);
  }
  else
  {
    while ((firstArgSplitPos != std::wstring::npos) && (firstArgSplitPos != arguments.length()))
    {
      args.push_back(arguments.substr(firstArgSplitPos, secondArgSplitPos));

      firstArgSplitPos = secondArgSplitPos;

      secondArgSplitPos = arguments.find(argSplitters, secondArgSplitPos + 1);
      if (secondArgSplitPos == std::wstring::npos)
      {
        secondArgSplitPos = arguments.length();
      }
    }
  }

  commandProviderDLL = ELLoadEmergeLibrary(command.commandProviderDLLName);
  if (commandProviderDLL)
  {
    commandHandlerName = ELwstringTostring(command.commandHandlerFunctionName);
    commandHandlerCallback = (IEMERGEINTERNALCOMMANDHANDLERPROC)GetProcAddress(commandProviderDLL, commandHandlerName.c_str());

    if (commandHandlerCallback)
    {
      returnValue = commandHandlerCallback(args);
    }

    FreeLibrary(commandProviderDLL);
  }

  return returnValue;
}

bool ELPopulateInternalCommandList(HWND hwnd)
{
  std::vector<CommandInfoStruct> internalCommands = LoadEmergeInternalCommands();
  std::vector<CommandInfoStruct>::const_iterator tempIter;
  CommandInfoStruct tempValue;
  std::wstring tempCommandName;

  if (!IsWindow(hwnd))
  {
    return false;
  }

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    tempCommandName = tempValue.commandName;
    SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)tempCommandName.c_str());
  }

  return true;
}

std::vector<CommandInfoStruct> LoadEmergeInternalCommands()
{
  static std::vector<CommandInfoStruct> internalCommands;
  static time_t lastLoadTime;
  time_t currentTime = time(NULL);
  CommandInfoStruct internalCommand;
  std::wstring dllSearchLocation;
  std::vector<std::wstring> dllFiles;
  std::vector<std::wstring>::const_iterator dllFileIter;
  std::string dllName;

  if (abs(difftime(lastLoadTime, currentTime)) < 1800) //difftime returns a seconds value. 30m * 60s/m = 1800s
  {
    return internalCommands;
  }

  lastLoadTime = time(NULL); //get a current timestamp for the newly loaded commands

  dllSearchLocation = ELGetCurrentPath();
  dllSearchLocation = dllSearchLocation + TEXT("\\Plugins\\");
  dllFiles = ELGetFilesInFolder(dllSearchLocation, TEXT("*.dll"), true);

  if (!dllFiles.empty())
  {
    internalCommands.clear();
  }

  for (dllFileIter = dllFiles.begin(); dllFileIter != dllFiles.end(); ++dllFileIter)
  {
    dllName = ELwstringTostring(*dllFileIter);

    auto provider = CommandProvider::dynamic_creator(dllName, "CommandProvider")();

    for (auto& commandIterator : provider.GetEmergeInternalCommands())
    {
      internalCommand.commandName = std::get<0>(commandIterator);
      internalCommand.commandValue = std::get<1>(commandIterator);
      internalCommand.commandProviderDLLName = *dllFileIter;
      internalCommand.commandHandlerFunctionName = std::get<2>(commandIterator);
      internalCommands.push_back(internalCommand);
    }
  }

  return internalCommands;
}

CommandInfoStruct FindCommandHandler(std::wstring commandName)
{
  std::vector<CommandInfoStruct> internalCommands = LoadEmergeInternalCommands();
  std::vector<CommandInfoStruct>::const_iterator tempIter;
  CommandInfoStruct tempValue;

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    if (ELToLower(tempValue.commandName) == ELToLower(commandName))
    {
      return tempValue;
    }
  }

  //If we didn't return the command handler already, there isn't one. Return an error value instead.
  tempValue.commandName = TEXT("");
  tempValue.commandValue = INVALID_COMMAND;
  return tempValue;
}
