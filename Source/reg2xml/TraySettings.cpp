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

#include "TraySettings.h"

TraySettings::TraySettings(HKEY key)
  :AppletSettings(key)
{
  this->key = key;
  appletName = TEXT("emergeTray.xml");
}

void TraySettings::ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper)
{
  bool unhideIcons;

  AppletSettings::ConvertSettings(keyHelper, xmlHelper);

  // Read Tray specific settings from the registry
  keyHelper.ReadBool(TEXT("UnhideIcons"), unhideIcons, true);

  // Write Tray specific settings to the XML file
  xmlHelper.WriteBool(TEXT("UnhideIcons"), unhideIcons);
}

bool TraySettings::ConvertHide()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *section = NULL;
  WCHAR hideKeyName[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH];
  BYTE data[MAX_LINE_LENGTH];
  DWORD type, valueSize = MAX_LINE_LENGTH, dataSize = MAX_LINE_LENGTH;
  HKEY hideKey;

  wcscpy(hideKeyName, TEXT("Hide"));
  if (RegOpenKeyEx(key, hideKeyName, 0, KEY_READ, &hideKey) != ERROR_SUCCESS)
    return false;

  if (!PathIsDirectory(ELExpandVars(userFile).c_str()))
    ELCreateDirectory(userFile);
  userFile += TEXT("emergeTray.xml");
  xmlConfig = ELOpenXMLConfig(userFile, true);

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Hide"), true);
      if (section)
        {
          DWORD index = 0;
          IOHelper xmlHelper(section);
          xmlHelper.Clear();
          while (RegEnumValue(hideKey, index, value, &valueSize, NULL, &type, data, &dataSize) == ERROR_SUCCESS)
            {
              // If it's a string, execute it
              if (type == REG_SZ)
                {
                  if (xmlHelper.SetElement(TEXT("item")))
                    xmlHelper.WriteString(TEXT("Icon"), (WCHAR*)data);
                }

              valueSize = MAX_LINE_LENGTH;
              dataSize = MAX_LINE_LENGTH;

              index++;
            }
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}
