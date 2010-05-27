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
  x = 0;
  y = -40;
  width = 281;
  height = 32;
  wcscpy(zPosition, TEXT("Top"));
  wcscpy(horizontalDirection, TEXT("right"));
  wcscpy(verticalDirection, TEXT("down"));
  wcscpy(directionOrientation, TEXT("horizontal"));
  autoSize = true;
  iconSize = 32;
  iconSpacing = 3;
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  wcscpy(anchorPoint, TEXT("BottomLeft"));
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
                  userIO.ReadInt(TEXT("Type"), type, IT_EXECUTABLE);
                  userIO.ReadString(TEXT("Command"), app, TEXT(""));
                  userIO.ReadString(TEXT("Icon"), icon, TEXT(""));
                  if (type != IT_SEPARATOR)
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

  // Populate default items
  if (!found)
    {
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(2, L"RightDeskMenu", L"%AppletDir%\\emergeCore.exe,0", L"Start Menu", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(0, L"Double", L"", L"", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(2, L"Homepage", L"%ProgramFiles%\\Internet Explorer\\iexplore.exe,0", L"Emerge Desktop Homepage", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(2, L"ShowDesktop", L"%WinDir%\\system32\\shell32.dll,34", L"Show Desktop", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(4, L"CSIDL_PERSONAL", L"", L"My Documents", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(1, L"%AppletDir%\\documentation\\Emerge Desktop.chm", L"%SystemRoot%\\system32\\shell32.dll,23", L"Emerge Desktop Help", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(2, L"Tutorial", L"%WinDir%\\system32\\shell32.dll,13", L"Emerge Desktop Online Tutorial", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(0, L"Single", L"", L"", L"")));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
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
