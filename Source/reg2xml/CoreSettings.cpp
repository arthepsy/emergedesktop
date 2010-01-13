// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009-2010  The Emerge Desktop Development Team
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

#include "CoreSettings.h"

CoreSettings::CoreSettings(HKEY key)
  :BaseSettings(false)
{
  (*this).key = key;
}

bool CoreSettings::ConvertShells()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *section = NULL;
  WCHAR name[MAX_LINE_LENGTH], command[MAX_LINE_LENGTH];
  HKEY shellsKey;
  WCHAR shellKeyName[MAX_LINE_LENGTH];

  wcscpy(shellKeyName, TEXT("Shells"));
  if (!PathIsDirectory(ELExpandVars(userFile).c_str()))
    ELCreateDirectory(userFile);
  userFile += TEXT("emergeCore.xml");
  xmlConfig = ELOpenXMLConfig(userFile, true);

  if (RegOpenKeyEx(key, shellKeyName, 0, KEY_READ, &shellsKey) != ERROR_SUCCESS)
    return false;

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Shells"), true);
      if (section)
        {
          IOHelper keyHelper(shellsKey), xmlHelper(section);
          xmlHelper.Clear();
          while (keyHelper.GetElement())
            {
              if (keyHelper.ReadString(TEXT("Command"), command, TEXT("")) &&
                  keyHelper.ReadString(TEXT("Name"), name, TEXT("")))
                {
                  if (xmlHelper.SetElement(TEXT("item")))
                    {
                      xmlHelper.WriteString(TEXT("Command"), command);
                      xmlHelper.WriteString(TEXT("Name"), name);
                    }
                }
            }
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}

bool CoreSettings::ConvertLaunch()
{
  std::wstring userFile = TEXT("%ThemeDir%\\emergeCore.xml");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig = ELOpenXMLConfig(userFile, true);
  TiXmlElement *section = NULL;
  WCHAR command[MAX_LINE_LENGTH];
  HKEY launchKey;
  WCHAR launchKeyName[MAX_LINE_LENGTH], launchKeyIndex[MAX_LINE_LENGTH];
  wcscpy(launchKeyName, TEXT("Launch"));
  std::wstring workingCommand;

  if (RegOpenKeyEx(key, launchKeyName, 0, KEY_READ, &launchKey) != ERROR_SUCCESS)
    return false;

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Launch"), true);
      if (section)
        {
          size_t i = 0;
          IOHelper keyHelper(launchKey), xmlHelper(section);
          xmlHelper.Clear();
          swprintf(launchKeyIndex, TEXT("%d"), 10 + i);
          while (keyHelper.ReadString(launchKeyIndex, command, TEXT("")))
            {
              workingCommand = TEXT("%AppletDir%\\");
              // Convert the command to a full path if relative
              if (ELPathIsRelative(command))
                workingCommand += command;
              else
                {
                  ELUnExpandVars(command);
                  workingCommand = command;
                }
              if (xmlHelper.SetElement(TEXT("item")))
                xmlHelper.WriteString(TEXT("Command"), (WCHAR*)workingCommand.c_str());
              i++;
              swprintf(launchKeyIndex, TEXT("%d"), 10 + i);
            }
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}
