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

#include "Settings.h"

Settings::Settings(): BaseSettings(false)
{
  userModified = false;
  showStartupErrors = false;
  showExplorerDesktop = false;
  showWelcome = true;
  wcscpy(appletName, TEXT("emergeCore"));
}

void Settings::DoReadSettings(IOHelper& helper)
{
  helper.ReadBool(TEXT("ShowExplorerDesktop"), showExplorerDesktop, false);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  helper.WriteBool(TEXT("ShowExplorerDesktop"), showExplorerDesktop);
}

void Settings::ResetDefaults()
{
  showExplorerDesktop = false;
}

bool Settings::GetShowExplorerDesktop()
{
  return showExplorerDesktop;
}

void Settings::SetShowExplorerDesktop(bool showExplorerDesktop)
{
  if (this->showExplorerDesktop != showExplorerDesktop)
    {
      this->showExplorerDesktop = showExplorerDesktop;
      SetModified();
    }
}

bool Settings::ReadUserSettings()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *section = NULL;
  bool readSettings = false;

  if (!ELPathIsDirectory(ELExpandVars(userFile).c_str()))
    ELCreateDirectory(userFile);
  userFile += TEXT("emergeCore.xml");
  xmlConfig = ELOpenXMLConfig(userFile, true);

  if (xmlConfig)
    {
      section = ELGetXMLSection(xmlConfig.get(), (WCHAR*)TEXT("Settings"), false);
      if (section)
        {
          readSettings = true;
          IOHelper xmlHelper(section);
          DoReadUserSettings(xmlHelper);
        }
    }

  // In the case where there is an issue accessing the XML file, use default values;
  if (!readSettings)
    ResetUserDefaults();

  return true;
}

void Settings::ResetUserDefaults()
{
  showStartupErrors = false;
  showWelcome = true;
}

void Settings::DoReadUserSettings(IOHelper& helper)
{
  helper.ReadBool(TEXT("ShowStartupErrors"), showStartupErrors, false);
  helper.ReadBool(TEXT("ShowWelcome"), showWelcome, true);
}

bool Settings::GetShowStartupErrors()
{
  return showStartupErrors;
}

bool Settings::GetShowWelcome()
{
  return showWelcome;
}

void Settings::SetShowWelcome(bool showWelcome)
{
  if (this->showWelcome != showWelcome)
    {
      this->showWelcome = showWelcome;
      userModified = true;
    }
}

void Settings::SetShowStartupErrors(bool showStartupErrors)
{
  if (this->showStartupErrors != showStartupErrors)
    {
      this->showStartupErrors = showStartupErrors;
      userModified = true;
    }
}

bool Settings::WriteUserSettings()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  std::wstring xmlFile = TEXT("%EmergeDir%\\files\\");
  xmlFile += TEXT("emergeCore.xml");
  bool ret = false;

  if (userModified)
    {
      configXML = ELOpenXMLConfig(xmlFile, true);
      if (configXML)
        {
          section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);

          if (section)
            {
              IOHelper helper(section);
              DoWriteUserSettings(helper);

              ret = ELWriteXMLConfig(configXML.get());
              userModified = false;
            }
        }
    }

  return ret;
}

void Settings::DoWriteUserSettings(IOHelper& helper)
{
  helper.WriteBool(TEXT("ShowStartupErrors"), showStartupErrors);
  helper.WriteBool(TEXT("ShowWelcome"), showWelcome);
}

