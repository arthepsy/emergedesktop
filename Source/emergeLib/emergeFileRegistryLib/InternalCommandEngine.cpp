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

#include "InternalCommandEngine.h"

bool ELRegisterInternalCommand(std::wstring commandName, int commandValue, COMMANDHANDLERPROC commandHandler)
{
  CommandHandler newCommand;

  if (commandName.empty())
  {
    return false;
  }

  if (commandValue == INVALID_COMMAND)
  {
    return false;
  }

  //commandHandlerMap.insert(commandValue, std::pair<std::wstring, COMMANDHANDLERPROC>(commandName, commandHandler));
  newCommand.commandValue = commandValue;
  newCommand.commandName = commandName;
  newCommand.commandHandler = commandHandler;
  internalCommands.insert(internalCommands.end(), &newCommand);

  return true;
}

bool ELIsInternalCommand(std::wstring commandName)
{
  return (ELGetInternalCommandValue(commandName) != INVALID_COMMAND);
}

std::wstring ELGetInternalCommandName(int commandValue)
{
  std::vector<CommandHandler*>::iterator tempIter;
  CommandHandler* tempValue;

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    if (tempValue->commandValue == commandValue)
    {
      return tempValue->commandName;
    }
  }

  return TEXT("");
}

int ELGetInternalCommandValue(std::wstring commandName)
{
  std::vector<CommandHandler*>::iterator tempIter;
  CommandHandler* tempValue;

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    if (ELToLower(tempValue->commandName) == ELToLower(commandName))
    {
      return tempValue->commandValue;
    }
  }

  return INVALID_COMMAND;
}

bool ELExecuteInternalCommand(std::wstring commandName, std::wstring arguments)
{
  COMMANDHANDLERPROC commandHandlerCallback;
  std::vector<std::wstring> args;
  std::wstring argSplitters = TEXT(" ,\t"); //space, comma, tab
  size_t firstArgSplitPos = 0, secondArgSplitPos;

  if (commandName.empty())
  {
    return false;
  }

  commandHandlerCallback = GetCommandHandlerCallback(commandName);
  if (commandHandlerCallback == NULL)
  {
    return false;
  }

  secondArgSplitPos = arguments.find_first_of(argSplitters);
  if ((secondArgSplitPos == std::wstring::npos) || (secondArgSplitPos == arguments.length()))
  {
    args.insert(args.end(), arguments);
  }
  else
  {
    while ((firstArgSplitPos != std::wstring::npos) && (firstArgSplitPos != arguments.length()))
    {
      args.insert(args.end(), arguments.substr(firstArgSplitPos, secondArgSplitPos));

      firstArgSplitPos = secondArgSplitPos;

      secondArgSplitPos = arguments.find(argSplitters, secondArgSplitPos + 1);
      if (secondArgSplitPos == std::wstring::npos)
      {
        secondArgSplitPos = arguments.length();
      }
    }
  }

  return commandHandlerCallback(args);
}

bool ELPopulateInternalCommandList(HWND hwnd)
{
  std::vector<CommandHandler*>::iterator tempIter;
  CommandHandler* tempValue;
  std::wstring tempCommandName;

  if (!IsWindow(hwnd))
  {
    return false;
  }

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    tempCommandName = tempValue->commandName;
    SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)tempCommandName.c_str());
  }

  return true;
}

COMMANDHANDLERPROC GetCommandHandlerCallback(std::wstring commandName)
{
  int commandValue;

  if (commandName.empty())
  {
    return NULL;
  }

  commandValue = ELGetInternalCommandValue(commandName);

  if (commandValue == INVALID_COMMAND)
  {
    return NULL;
  }

  return GetCommandHandlerCallback(commandValue);
}

COMMANDHANDLERPROC GetCommandHandlerCallback(int commandValue)
{
  std::vector<CommandHandler*>::iterator tempIter;
  CommandHandler* tempValue;

  for (tempIter = internalCommands.begin(); tempIter != internalCommands.end(); ++tempIter)
  {
    tempValue = *tempIter;
    if (tempValue->commandValue == commandValue)
    {
      return tempValue->commandHandler;
    }
  }

  return NULL;
}
