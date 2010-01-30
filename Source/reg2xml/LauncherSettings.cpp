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

#include "LauncherSettings.h"

LauncherSettings::LauncherSettings(HKEY key)
  :AppletSettings(key)
{
  appletName = TEXT("emergeLauncher.xml");
}

void LauncherSettings::ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper)
{
  int index = 1;
  size_t splitter;
  WCHAR commandValue[MAX_LINE_LENGTH], iconValue[MAX_LINE_LENGTH], tipValue[MAX_LINE_LENGTH], workingDirValue[MAX_LINE_LENGTH];
  WCHAR app[MAX_LINE_LENGTH], icon[MAX_LINE_LENGTH], tip[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  std::tr1::shared_ptr<IOHelper> launchHelper;
  TiXmlElement *launchSection, *subSection;
  std::wstring themeMedia, workingIcon, workingIconFile, workingIconIndex;

  AppletSettings::ConvertSettings(keyHelper, xmlHelper);

  swprintf(commandValue, TEXT("Command%d"), index);
  swprintf(iconValue, TEXT("Icon%d"), index);
  swprintf(tipValue, TEXT("Tip%d"), index);
  swprintf(workingDirValue, TEXT("WorkingDir%d"), index);

  while (keyHelper.ReadString(commandValue, app, TEXT("")))
    {
      ZeroMemory(icon, MAX_LINE_LENGTH * sizeof(WCHAR));
      ZeroMemory(tip, MAX_LINE_LENGTH * sizeof(WCHAR));
      ZeroMemory(workingDir, MAX_LINE_LENGTH * sizeof(WCHAR));

      keyHelper.ReadString(iconValue, icon, TEXT(""));
      keyHelper.ReadString(tipValue, tip, TEXT(""));
      keyHelper.ReadString(workingDirValue, workingDir, TEXT(""));

      workingIcon = icon;
      if (!workingIcon.empty())
        {
          splitter = workingIcon.find_last_of(TEXT(","));
          if (splitter != std::wstring::npos)
            {
              workingIconFile = workingIcon.substr(0, splitter);
              workingIconIndex = workingIcon.substr(splitter);
            }
          else
            workingIconFile = workingIcon;
          splitter = workingIconFile.find_last_of(TEXT("\\"));
          // Copy the icon to the theme if it exists
          if (ELPathFileExists(workingIconFile.c_str()))
            {
              themeMedia = TEXT("%ThemeDir%\\Media\\");
              if (!PathIsDirectory(themeMedia.c_str()))
                ELCreateDirectory(themeMedia);
              themeMedia += workingIconFile.substr(splitter + 1);
              CopyFile(ELExpandVars(workingIconFile).c_str(), ELExpandVars(themeMedia).c_str(), TRUE);
              themeMedia += workingIconIndex;
              wcscpy(icon, themeMedia.c_str());
            }
        }

      // Add the Launch item to the XML settings
      launchSection = reinterpret_cast<TiXmlElement*>(xmlHelper.GetElement((WCHAR*)TEXT("Launch")));
      if (launchSection)
        {
          subSection = ELSetFirstXMLElement(launchSection, TEXT("item"));
          launchHelper = std::tr1::shared_ptr<IOHelper>(new IOHelper(subSection));
          launchHelper->WriteInt(TEXT("Type"), 1);
          launchHelper->WriteString(TEXT("Command"), app);
          launchHelper->WriteString(TEXT("Icon"), icon);
          launchHelper->WriteString(TEXT("Tip"), tip);
          launchHelper->WriteString(TEXT("WorkingDir"), workingDir);
        }

      index++;
      swprintf(commandValue, TEXT("Command%d"), index);
      swprintf(iconValue, TEXT("Icon%d"), index);
      swprintf(tipValue, TEXT("Tip%d"), index);
      swprintf(workingDirValue, TEXT("WorkingDir%d"), index);
    }
}
