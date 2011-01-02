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
}

bool Settings::ReadSettings()
{
  std::wstring userFile = TEXT("%EmergeDir%\\files\\");
  std::tr1::shared_ptr<TiXmlDocument> xmlConfig;
  TiXmlElement *section = NULL;
  bool readSettings = false;

  if (!PathIsDirectory(ELExpandVars(userFile).c_str()))
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
          DoReadSettings(xmlHelper);
        }
    }

  // In the case where there is an issue accessing the XML file, use default values;
  if (!readSettings)
    ResetDefaults();

  return true;
}

void Settings::ResetDefaults()
{
  showStartupErrors = false;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  helper.ReadBool(TEXT("ShowStartupErrors"), showStartupErrors, false);
}

bool Settings::GetShowStartupErrors()
{
  return showStartupErrors;
}

void Settings::SetShowStartupErrors(bool showStartupErrors)
{
  if (this->showStartupErrors != showStartupErrors)
    {
      this->showStartupErrors = showStartupErrors;
      SetModified();
    }
}

bool Settings::WriteSettings()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  std::wstring xmlFile = TEXT("%EmergeDir%\\files\\");
  xmlFile += TEXT("emergeCore.xml");
  bool ret = false;

  if (GetModified())
    {
      configXML = ELOpenXMLConfig(xmlFile, true);
      if (configXML)
        {
          section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Settings"), true);

          if (section)
            {
              IOHelper helper(section);
              DoWriteSettings(helper);

              ret = ELWriteXMLConfig(configXML.get());
              ClearModified();
            }
        }
    }

  return ret;
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  helper.WriteBool(TEXT("ShowStartupErrors"), showStartupErrors);
}

