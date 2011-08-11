// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009-2011  The Emerge Desktop Development Team
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

#include "CommandSettings.h"

CommandSettings::CommandSettings(HKEY key)
  :AppletSettings(key)
{
  this->key = key;
  appletName = TEXT("emergeCommand.xml");
}

void CommandSettings::ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper)
{
  WCHAR timeFormat[MAX_LINE_LENGTH];
  WCHAR tipFormat[MAX_LINE_LENGTH];
  WCHAR clockTextAlign[MAX_LINE_LENGTH];
  WCHAR commandTextAlign[MAX_LINE_LENGTH];
  WCHAR clockVerticalAlign[MAX_LINE_LENGTH];
  WCHAR commandVerticalAlign[MAX_LINE_LENGTH];
  WCHAR fontString[MAX_LINE_LENGTH];

  AppletSettings::ConvertSettings(keyHelper, xmlHelper);

  // Read Command specific settings from the registry
  keyHelper.ReadString(TEXT("ClockTextAlign"), clockTextAlign, TEXT("left"));
  keyHelper.ReadString(TEXT("CommandTextAlign"), commandTextAlign, TEXT("left"));
  keyHelper.ReadString(TEXT("CommandVerticalAlign"), commandVerticalAlign, TEXT("top"));
  keyHelper.ReadString(TEXT("ClockVerticalAlign"), clockVerticalAlign, TEXT("top"));
  keyHelper.ReadString(TEXT("TimeFormat"), timeFormat, TEXT("%A%_%x%_%X"));
  keyHelper.ReadString(TEXT("TipFormat"), tipFormat, TEXT("%#c"));
  keyHelper.ReadString(TEXT("Font"), fontString, TEXT("Tahoma-12"));

  // Write Command specific settings to the XML file
  xmlHelper.WriteString(TEXT("ClockTextAlign"), clockTextAlign);
  xmlHelper.WriteString(TEXT("CommandTextAlign"), commandTextAlign);
  xmlHelper.WriteString(TEXT("CommandVerticalAlign"), commandVerticalAlign);
  xmlHelper.WriteString(TEXT("ClockVerticalAlign"), clockVerticalAlign);
  xmlHelper.WriteString(TEXT("TimeFormat"), timeFormat);
  xmlHelper.WriteString(TEXT("TipFormat"), tipFormat);
  xmlHelper.WriteString(TEXT("Font"), fontString);
}

bool CommandSettings::ConvertHistory()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *section = NULL;
  WCHAR command[MAX_LINE_LENGTH];
  HKEY historyKey;
  WCHAR historyKeyName[MAX_LINE_LENGTH], historyKeyIndex[MAX_LINE_LENGTH];
  wcscpy(historyKeyName, TEXT("History"));

  if (RegOpenKeyEx(key, historyKeyName, 0, KEY_READ, &historyKey) != ERROR_SUCCESS)
    return false;

  if (!ELPathIsDirectory(ELExpandVars(userFile).c_str()))
    ELCreateDirectory(userFile);
  userFile += TEXT("emergeCommand.xml");
  xmlConfig = ELOpenXMLConfig(userFile, true);

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("History"), true);
      if (section)
        {
          size_t i = 0;
          IOHelper keyHelper(historyKey), xmlHelper(section);
          xmlHelper.Clear();
          swprintf(historyKeyIndex, TEXT("%d"), i);
          while (keyHelper.ReadString(historyKeyIndex, command, TEXT("")))
            {
              if (xmlHelper.SetElement(TEXT("item")))
                xmlHelper.WriteString(TEXT("Command"), command);
              i++;
              swprintf(historyKeyIndex, TEXT("%d"), i);
            }
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}
