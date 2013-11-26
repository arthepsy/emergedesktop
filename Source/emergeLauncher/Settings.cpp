// vim: tags+=../emergeLib/tags
//---
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
//---

#include "Settings.h"

Settings::Settings(): BaseSettings(true)
{
  InitializeCriticalSection(&itemListCS);
}

Settings::~Settings()
{
  EnterCriticalSection(&itemListCS);
  while (!itemList.empty())
  {
    itemList.erase(itemList.begin());
  }
  LeaveCriticalSection(&itemListCS);

  DeleteCriticalSection(&itemListCS);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  width = 281;
  height = 32;
  zPosition = TEXT("Top");
  horizontalDirection = TEXT("right");
  verticalDirection = TEXT("down");
  directionOrientation = TEXT("horizontal");
  autoSize = true;
  iconSize = 32;
  iconSpacing = 3;
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  // If appletCount > 0 assume this is a new instance and place it at the center
  // of the screen.
  if (appletCount > 0)
  {
    POINT origin;
    SIZE appletSize;

    appletSize.cx = width;
    appletSize.cy = height;

    origin = InstancePosition(appletSize);
    x = origin.x;
    y = origin.y;
  }
  else
  {
    x = 0;
    y = -40;
    anchorPoint = TEXT("BottomLeft");
  }
}

void Settings::PopulateItems()
{
  std::wstring app, icon, tip, workingDir;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* settingsSection, *launchSection;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");
  bool found = false;

  EnterCriticalSection(&itemListCS);
  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
  {
    settingsSection = ELGetXMLSection(configXML.get(), TEXT("Settings"), false);
    if (settingsSection)
    {
      launchSection = ELGetFirstXMLElementByName(settingsSection, TEXT("Launch"), false);

      if (launchSection)
      {
        IOHelper userIO(launchSection);
        int type;

        while (userIO.GetElement())
        {
          found = true;
          type = userIO.ReadInt(TEXT("Type"), IT_EXECUTABLE);
          app = userIO.ReadString(TEXT("Command"), TEXT(""));
          icon = userIO.ReadString(TEXT("Icon"), TEXT(""));
          tip = userIO.ReadString(TEXT("Tip"), TEXT(""));
          workingDir = userIO.ReadString(TEXT("WorkingDir"), TEXT(""));

          // Convert the iconValue to a full path if relative
          if (ELPathIsRelative(icon))
          {
            icon = ELGetAbsolutePath(icon, TEXT("%ThemeDir%\\"));
          }

          // Add new Folders item to the vector and set the icon size
          itemList.push_back(std::tr1::shared_ptr<Item>(new Item((ITEMTYPE)type, app, icon, tip, workingDir)));
          itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());

          if (type == IT_LIVE_FOLDER)
          {
            loadLiveFolder((WCHAR*)app.c_str());
          }
        }
      }
    }
  }

  // Populate default items
  if (!found)
  {
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_INTERNAL_COMMAND, TEXT("RightDeskMenu"), TEXT("%AppletDir%\\emergeCore.exe,0"), TEXT("Start Menu"), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_SEPARATOR, TEXT("Double"), TEXT(""), TEXT(""), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_INTERNAL_COMMAND, TEXT("Homepage"), TEXT("%ProgramFiles%\\Internet Explorer\\iexplore.exe,0"), TEXT("Emerge Desktop Homepage"), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_INTERNAL_COMMAND, TEXT("ShowDesktop"), TEXT("%WinDir%\\system32\\shell32.dll,34"), TEXT("Show Desktop"), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_SPECIAL_FOLDER, TEXT("CSIDL_PERSONAL"), TEXT(""), TEXT("My Documents"), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_EXECUTABLE, TEXT("%AppletDir%\\documentation\\Emerge Desktop.chm"), TEXT("%SystemRoot%\\system32\\shell32.dll,23"), TEXT("Emerge Desktop Help"), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_INTERNAL_COMMAND, TEXT("Tutorial"), TEXT("%WinDir%\\system32\\shell32.dll,13"), TEXT("Emerge Desktop Online Tutorial"), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_SEPARATOR, TEXT("Single"), TEXT(""), TEXT(""), TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
  }

  LeaveCriticalSection(&itemListCS);
}

void Settings::DeleteItems(bool clearXML)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* settingsSection, *launchSection;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");

  EnterCriticalSection(&itemListCS);
  if (clearXML)
  {
    configXML = ELOpenXMLConfig(xmlFile, false);
    if (configXML)
    {
      settingsSection = ELGetXMLSection(configXML.get(), TEXT("Settings"), false);
      if (settingsSection)
      {
        launchSection = ELGetFirstXMLElementByName(settingsSection, TEXT("Launch"), false);
        if (launchSection)
        {
          IOHelper userIO(launchSection);
          userIO.Clear();

          ELWriteXMLConfig(configXML.get());
        }
      }
    }
  }

  while (!itemList.empty())
  {
    itemList.erase(itemList.begin());
  }
  LeaveCriticalSection(&itemListCS);
}

void Settings::WriteItem(int type, WCHAR* command, WCHAR* iconPath, WCHAR* tip, WCHAR* workingDir)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* settingsSection, *launchSection;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");

  if (type == IT_ENTIRE_FOLDER)
  {
    writeEntireFolder(command);
    return;
  }
  else if (type == IT_LIVE_FOLDER_ITEM)
  {
    return;
  }

  configXML = ELOpenXMLConfig(xmlFile, true);

  if (configXML)
  {
    settingsSection = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);
    if (settingsSection)
    {
      launchSection = ELGetFirstXMLElementByName(settingsSection, (WCHAR*)TEXT("Launch"), true);

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

Item* Settings::GetItem(size_t index)
{
  return itemList.at(index).get();
}

void Settings::writeEntireFolder(WCHAR* folderName)
{
  std::wstring workingFolder = ELExpandVars(folderName);
  workingFolder = ELGetAbsolutePath(workingFolder);
  WIN32_FIND_DATA FindFileData;
  std::wstring searchFolderName = workingFolder + TEXT("\\*.*"), tmpPath;

  HANDLE hFind = FindFirstFile(searchFolderName.c_str(), &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE)
  {
    return;
  }

  do
  {
    tmpPath = workingFolder + TEXT("\\");
    tmpPath += FindFileData.cFileName;

    if ((_wcsicmp(TEXT("."), FindFileData.cFileName) != 0) && (_wcsicmp(TEXT(".."), FindFileData.cFileName) != 0))
    {
      ELStripFileArguments(FindFileData.cFileName);
      WriteItem(IT_EXECUTABLE, (WCHAR*)tmpPath.c_str(), (WCHAR*)TEXT(""), FindFileData.cFileName, (WCHAR*)TEXT(""));
    }
  }
  while (FindNextFile(hFind, &FindFileData) != 0);

  FindClose(hFind);
}

void Settings::loadLiveFolder(WCHAR* folderName)
{
  std::wstring workingFolder = ELExpandVars(folderName);
  workingFolder = ELGetAbsolutePath(workingFolder);
  WIN32_FIND_DATA FindFileData;
  std::wstring searchFolderName = workingFolder + TEXT("\\*.*"), tmpPath;

  HANDLE hFind = FindFirstFile(searchFolderName.c_str(), &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE)
  {
    // Add an empty icon for each non-existent live folder path
    itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_LIVE_FOLDER_ITEM,
                       (WCHAR*)workingFolder.c_str(),
                       (WCHAR*)TEXT(""),
                       (WCHAR*)TEXT(""),
                       (WCHAR*)TEXT(""))));
    itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    return;
  }

  do
  {
    tmpPath = workingFolder + TEXT("\\");
    tmpPath += FindFileData.cFileName;

    if ((_wcsicmp(TEXT("."), FindFileData.cFileName) != 0) &&
        (_wcsicmp(TEXT(".."), FindFileData.cFileName) != 0) &&
        ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN))
    {
      ELStripFileArguments(FindFileData.cFileName);
      itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_LIVE_FOLDER_ITEM,
                         (WCHAR*)tmpPath.c_str(),
                         (WCHAR*)TEXT(""),
                         FindFileData.cFileName,
                         (WCHAR*)TEXT(""))));
      itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
    }
  }
  while (FindNextFile(hFind, &FindFileData) != 0);

  FindClose(hFind);
}
