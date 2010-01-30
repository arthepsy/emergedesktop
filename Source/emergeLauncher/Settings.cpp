// vim: tags+=../emergeLib/tags
//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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
//---

#include "Settings.h"

extern WCHAR myName[];

Settings::Settings(): BaseSettings(true)
{
  swprintf(keyString, TEXT("%s\\Settings"), myName);
  xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += myName;
  xmlFile += TEXT(".xml");
}

Settings::~Settings()
{
  while (!itemList.empty())
    itemList.erase(itemList.begin());
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  y = 104;
}

void Settings::PopulateItems()
{
  WCHAR app[MAX_LINE_LENGTH], icon[MAX_LINE_LENGTH], tip[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *settingsSection, *launchSection;
  bool found = false;
  int type;

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      settingsSection = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), false);
      if (settingsSection)
        {
          launchSection = ELGetFirstXMLElementByName(settingsSection, (WCHAR*)TEXT("Launch"));

          if (launchSection)
            {
              IOHelper userIO(launchSection);

              while (userIO.GetElement())
                {
                  found = true; // Existing user settings found
                  userIO.ReadInt(TEXT("Type"), type, 1);
                  userIO.ReadString(TEXT("Command"), app, TEXT(""));
                  userIO.ReadString(TEXT("Icon"), icon, TEXT(""));
                  if ((type != 0) && (type != 5))
                    userIO.ReadString(TEXT("Tip"), tip, TEXT(""));
                  userIO.ReadString(TEXT("WorkingDir"), workingDir, TEXT(""));

                  // Convert the iconValue to a full path if relative
                  if (ELPathIsRelative(icon))
                    ELConvertThemePath(icon, CTP_FULL);

                  // Add new Launcher item to the vector and set the icon size
                  itemList.push_back(std::tr1::shared_ptr<Item>(new Item(type, app, icon, tip, workingDir)));
                  itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
                }
            }
        }
    }
}

void Settings::DeleteItems(bool clearXML)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *settingsSection, *launchSection;

  if (clearXML)
    {
      configXML = ELOpenXMLConfig(xmlFile, false);
      if (configXML)
        {
          settingsSection = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);
          if (settingsSection)
            {
              launchSection = ELGetFirstXMLElementByName(settingsSection, (WCHAR*)TEXT("Launch"));
              if (launchSection)
                {
                  IOHelper userIO(launchSection);
                  userIO.Clear();
                }
            }

          ELWriteXMLConfig(configXML.get());
        }
    }

  while (!itemList.empty())
    itemList.erase(itemList.begin());
}

void Settings::WriteItem(int type, WCHAR *command, WCHAR *iconPath, WCHAR *tip, WCHAR *workingDir)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *settingsSection, *launchSection;

  configXML = ELOpenXMLConfig(xmlFile, true);

  if (configXML)
    {
      settingsSection = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);
      if (settingsSection)
        {
          launchSection = ELGetFirstXMLElementByName(settingsSection, (WCHAR*)TEXT("Launch"));

          if (launchSection)
            {
              IOHelper userIO(launchSection);
              if (userIO.SetElement(TEXT("item")))
                {
                  userIO.WriteInt(TEXT("Type"), type);
                  userIO.WriteString(TEXT("Command"), command);
                  userIO.WriteString(TEXT("Icon"), iconPath);
                  userIO.WriteString(TEXT("Tip"), tip);
                  userIO.WriteString(TEXT("WorkingDir"), workingDir);
                  ELWriteXMLConfig(configXML.get());
                }
            }
        }
    }
}

UINT Settings::GetItemListSize()
{
  return (UINT)itemList.size();
}

Item *Settings::GetItem(UINT index)
{
  return itemList[index].get();
}
