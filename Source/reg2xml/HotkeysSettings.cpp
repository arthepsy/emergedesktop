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

#include "HotkeysSettings.h"

HotkeysSettings::HotkeysSettings(HKEY key)
  :BaseSettings(false)
{
  (*this).key = key;
}

bool HotkeysSettings::ConvertActions()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *section = NULL;
  BYTE data[MAX_LINE_LENGTH];
  WCHAR value[MAX_LINE_LENGTH];
  DWORD valueSize = MAX_LINE_LENGTH, dataSize = MAX_LINE_LENGTH, type;
  HKEY actionsKey;
  WCHAR shellKeyName[MAX_LINE_LENGTH];

  wcscpy(shellKeyName, TEXT("Actions"));
  if (!ELPathIsDirectory(ELExpandVars(userFile).c_str()))
    ELCreateDirectory(userFile);
  userFile += TEXT("emergeHotkeys.xml");
  xmlConfig = ELOpenXMLConfig(userFile, true);

  if (RegOpenKeyEx(key, shellKeyName, 0, KEY_READ, &actionsKey) != ERROR_SUCCESS)
    return false;

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Actions"), true);
      if (section)
        {
          size_t i = 0;
          IOHelper userIO(section);
          userIO.Clear();
          while (RegEnumValue(actionsKey, i, value, &valueSize, NULL, &type, data, &dataSize) == ERROR_SUCCESS)
            {
              if (type == REG_SZ)
                {
                  if (userIO.SetElement(TEXT("item")))
                    {
                      userIO.WriteString(TEXT("KeyCombo"), value);
                      userIO.WriteString(TEXT("Action"), (WCHAR*)data);
                    }
                }

              valueSize = MAX_LINE_LENGTH;
              dataSize = MAX_LINE_LENGTH;
              ZeroMemory(&value, MAX_LINE_LENGTH);
              ZeroMemory(&data, MAX_LINE_LENGTH);

              i++;
            }
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}

