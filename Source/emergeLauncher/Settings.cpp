// vim: tags+=../emergeLib/tags
//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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
    itemList.erase(itemList.begin());
  LeaveCriticalSection(&itemListCS);

  DeleteCriticalSection(&itemListCS);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
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
  // If appletCount > 0 assume this is a new instance and place it at the centre
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
      wcscpy(anchorPoint, TEXT("BottomLeft"));
    }
}

void Settings::PopulateItems()
{
  WCHAR app[MAX_LINE_LENGTH], icon[MAX_LINE_LENGTH], tip[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *settingsSection, *launchSection;
  int type;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");
  bool found = false;

  EnterCriticalSection(&itemListCS);
  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      settingsSection = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), false);
      if (settingsSection)
        {
          launchSection = ELGetFirstXMLElementByName(settingsSection, (WCHAR*)TEXT("Launch"), false);

          if (launchSection)
            {
              IOHelper userIO(launchSection);

              while (userIO.GetElement())
                {
                  found = true;
                  userIO.ReadInt(TEXT("Type"), type, IT_EXECUTABLE);
                  userIO.ReadString(TEXT("Command"), app, TEXT(""));
                  ELAbsPathFromRelativePath(app);
                  userIO.ReadString(TEXT("Icon"), icon, TEXT(""));
                  userIO.ReadString(TEXT("Tip"), tip, TEXT(""));
                  userIO.ReadString(TEXT("WorkingDir"), workingDir, TEXT(""));
                  ELAbsPathFromRelativePath(workingDir);

                  // Convert the iconValue to a full path if relative
                  if (ELPathIsRelative(icon))
                    ELConvertThemePath(icon, CTP_FULL);

                  // Add new Folders item to the vector and set the icon size
                  itemList.push_back(std::tr1::shared_ptr<Item>(new Item(type, app, icon, tip, workingDir)));
                  itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());

                  if (type == IT_LIVE_FOLDER)
                    loadLiveFolder(app);
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

  LeaveCriticalSection(&itemListCS);
}

void Settings::DeleteItems(bool clearXML)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *settingsSection, *launchSection;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");

  EnterCriticalSection(&itemListCS);
  if (clearXML)
    {
      configXML = ELOpenXMLConfig(xmlFile, false);
      if (configXML)
        {
          settingsSection = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), false);
          if (settingsSection)
            {
              launchSection = ELGetFirstXMLElementByName(settingsSection, (WCHAR*)TEXT("Launch"), false);
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
    itemList.erase(itemList.begin());
  LeaveCriticalSection(&itemListCS);
}

void Settings::WriteItem(int type, WCHAR *command, WCHAR *iconPath, WCHAR *tip, WCHAR *workingDir)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *settingsSection, *launchSection;
  std::wstring xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += appletName;
  xmlFile += TEXT(".xml");

  if (type == IT_ENTIRE_FOLDER)
    {
      writeEntireFolder(command);
      return;
    }
  else if (type == IT_LIVE_FOLDER_ITEM)
    return;

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
                  ELRelativePathFromAbsPath(command);
                  userIO.WriteString(TEXT("Command"), command);
                  userIO.WriteString(TEXT("Icon"), iconPath);
                  userIO.WriteString(TEXT("Tip"), tip);
                  ELRelativePathFromAbsPath(workingDir);
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

void Settings::writeEntireFolder(WCHAR *folderName)
{
  wcscpy(folderName, (ELExpandVars(folderName)).c_str());
  ELAbsPathFromRelativePath(folderName);

  WIN32_FIND_DATA FindFileData;
  WCHAR tmpFolderName[MAX_PATH], tmpPath[MAX_PATH];

  swprintf(tmpFolderName, TEXT("%s\\*.*"), folderName);

  HANDLE hFind = FindFirstFile(tmpFolderName, &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE)
    return;

  do
    {
      swprintf(tmpPath, TEXT("%s\\%s"), folderName, FindFileData.cFileName);

      if ((_wcsicmp(TEXT("."), FindFileData.cFileName) != 0) && (_wcsicmp(TEXT(".."), FindFileData.cFileName) != 0))
        {
          ELStripShortcutExtension(FindFileData.cFileName);
          WriteItem(IT_EXECUTABLE, tmpPath, (WCHAR*)TEXT(""), FindFileData.cFileName, (WCHAR*)TEXT(""));
        }
    }
  while (FindNextFile(hFind, &FindFileData) != 0);

  FindClose(hFind);
}

void Settings::loadLiveFolder(WCHAR *folderName)
{
  wcscpy(folderName, (ELExpandVars(folderName)).c_str());
  ELAbsPathFromRelativePath(folderName);

  WIN32_FIND_DATA FindFileData;
  WCHAR tmpFolderName[MAX_PATH], tmpPath[MAX_PATH];

  swprintf(tmpFolderName, TEXT("%s\\*.*"), folderName);

  HANDLE hFind = FindFirstFile(tmpFolderName, &FindFileData);

  if (hFind == INVALID_HANDLE_VALUE)
    return;

  do
    {
      swprintf(tmpPath, TEXT("%s\\%s"), folderName, FindFileData.cFileName);

      if ((_wcsicmp(TEXT("."), FindFileData.cFileName) != 0) &&
          (_wcsicmp(TEXT(".."), FindFileData.cFileName) != 0) &&
          ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN))
        {
          ELStripShortcutExtension(FindFileData.cFileName);
          itemList.push_back(std::tr1::shared_ptr<Item>(new Item(IT_LIVE_FOLDER_ITEM, tmpPath, (WCHAR*)TEXT(""), FindFileData.cFileName, (WCHAR*)TEXT(""))));
          itemList.back()->SetIcon(GetIconSize(), GetDirectionOrientation());
        }
    }
  while (FindNextFile(hFind, &FindFileData) != 0);

  FindClose(hFind);
}
