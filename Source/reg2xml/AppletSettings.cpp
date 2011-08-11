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

#include "AppletSettings.h"

AppletSettings::AppletSettings(HKEY key)
  :BaseSettings(false)
{
  (*this).key = key;
}

bool AppletSettings::ConvertReg()
{
  std::wstring themeFile = TEXT("%ThemeDir%\\") + appletName;
  WCHAR settingsKeyName[MAX_LINE_LENGTH];
  HKEY settingsKey;
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig = ELOpenXMLConfig(themeFile, true);
  TiXmlElement *section = NULL;

  wcscpy(settingsKeyName, TEXT("Settings"));
  if (RegOpenKeyEx(key, settingsKeyName, 0, KEY_READ, &settingsKey) != ERROR_SUCCESS)
    return false;

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Settings"), true);
      if (section)
        {
          IOHelper keyHelper(settingsKey), xmlHelper(section);
          ConvertSettings(keyHelper, xmlHelper);
          ELWriteXMLConfig(xmlConfig.get());
        }
    }

  return true;
}

void AppletSettings::ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper)
{
  WCHAR styleFile[MAX_PATH], origStyle[MAX_PATH];
  std::wstring themeStyle = TEXT("%ThemeDir%\\Styles\\");
  DoReadSettings(keyHelper);
  wcscpy(origStyle, GetStyleFile());
  if (ELPathFileExists(origStyle))
    {
      if (!ELPathIsDirectory(themeStyle.c_str()))
        ELCreateDirectory(themeStyle);
      wcscpy(styleFile, PathFindFileName(origStyle));
      themeStyle += styleFile;
      SetStyleFile(themeStyle.c_str());
      themeStyle = ELExpandVars(themeStyle);
      CopyFile(origStyle, themeStyle.c_str(), TRUE);
    }
  DoWriteSettings(xmlHelper);
}

