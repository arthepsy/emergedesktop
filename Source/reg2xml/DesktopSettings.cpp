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

#include "DesktopSettings.h"

DesktopSettings::DesktopSettings(HKEY key)
  :AppletSettings(key)
{
  this->key = key;
  appletName = TEXT("emergeWorkspace.xml");
}

void DesktopSettings::ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper)
{
  int topBorder, bottomBorder, leftBorder, rightBorder, menuAlpha;
  bool menuIcons, borderPrimary;

  // Read Desktop specific settings from the registry
  keyHelper.ReadInt(TEXT("TopBorder"), topBorder, 0);
  keyHelper.ReadInt(TEXT("BottomBorder"), bottomBorder, 0);
  keyHelper.ReadInt(TEXT("LeftBorder"), leftBorder, 0);
  keyHelper.ReadInt(TEXT("RightBorder"), rightBorder, 0);
  keyHelper.ReadInt(TEXT("MenuAlpha"), menuAlpha, 100);
  keyHelper.ReadBool(TEXT("MenuIcons"), menuIcons, true);
  keyHelper.ReadBool(TEXT("BorderPrimary"), borderPrimary, true);

  // Write Desktop specific settings to the XML file
  xmlHelper.WriteInt(TEXT("TopBorder"), topBorder);
  xmlHelper.WriteInt(TEXT("BottomBorder"), bottomBorder);
  xmlHelper.WriteInt(TEXT("LeftBorder"), leftBorder);
  xmlHelper.WriteInt(TEXT("RightBorder"), rightBorder);
  xmlHelper.WriteInt(TEXT("MenuAlpha"), menuAlpha);
  xmlHelper.WriteBool(TEXT("MenuIcons"), menuIcons);
  xmlHelper.WriteBool(TEXT("BorderPrimary"), borderPrimary);
}

bool DesktopSettings::ConvertMenus()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *menuSection = NULL, *rightSection = NULL, *midSection = NULL;
  WCHAR menuKeyName[MAX_LINE_LENGTH];
  HKEY menuKey;

  if (!PathIsDirectory(ELExpandVars(userFile).c_str()))
    ELCreateDirectory(userFile);
  userFile += TEXT("emergeWorkspace.xml");
  xmlConfig = ELOpenXMLConfig(userFile, true);

  if (xmlConfig)
    {
      menuSection = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Menus"), true);
      if (menuSection)
        {
          rightSection = ELGetFirstXMLElementByName(menuSection, (WCHAR*)TEXT("RightMenu"));
          if (rightSection)
            {
              wcscpy(menuKeyName, TEXT("RightMenu"));
              if (RegOpenKeyEx(key, menuKeyName, 0, KEY_READ, &menuKey) == ERROR_SUCCESS)
                {
                  IOHelper xmlHelper(rightSection), keyHelper(menuKey);
                  xmlHelper.Clear();
                  ImportRegMenuHelper(xmlHelper, keyHelper);
                  RegCloseKey(menuKey);
                }
            }
          midSection = ELGetFirstXMLElementByName(menuSection, (WCHAR*)TEXT("MidMenu"));
          if (midSection)
            {
              wcscpy(menuKeyName, TEXT("MidMenu"));
              if (RegOpenKeyEx(key, menuKeyName, 0, KEY_READ, &menuKey) == ERROR_SUCCESS)
                {
                  IOHelper xmlHelper(midSection), keyHelper(menuKey);
                  xmlHelper.Clear();
                  ImportRegMenuHelper(xmlHelper, keyHelper);
                  RegCloseKey(menuKey);
                }
            }
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}

bool DesktopSettings::ImportRegMenuHelper(IOHelper &xmlHelper, IOHelper &keyHelper)
{
  bool ret = false;
  int type;
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH], subkeyName[MAX_LINE_LENGTH];

  while (keyHelper.GetElement())
    {
      keyHelper.GetElementText(subkeyName);
      keyHelper.ReadString(TEXT("name"), name, TEXT(""));
      keyHelper.ReadInt(TEXT("type"), type, 0);
      keyHelper.ReadString(TEXT("value"), value, TEXT(""));
      keyHelper.ReadString(TEXT("workingdir"), workingDir, TEXT(""));

      if (xmlHelper.SetElement(TEXT("item")))
        {
          xmlHelper.WriteString(TEXT("Name"), name);
          xmlHelper.WriteString(TEXT("Value"), value);
          xmlHelper.WriteInt(TEXT("Type"), type);
          xmlHelper.WriteString(TEXT("WorkingDir"), workingDir);
          IOHelper itemHelper(reinterpret_cast<TiXmlElement*>(xmlHelper.GetTarget()));

          if (type == 100)
            {
              IOHelper subkeyHelper((HKEY)keyHelper.GetElement(subkeyName));
              IOHelper subMenuHelper(reinterpret_cast<TiXmlElement*>(itemHelper.GetElement((WCHAR*)TEXT("Submenu"))));
              ImportRegMenuHelper(subMenuHelper, subkeyHelper);
            }
        }
    }

  return ret;
}
