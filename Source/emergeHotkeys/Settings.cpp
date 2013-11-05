// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2012  The Emerge Desktop Development Team
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

Settings::Settings()
  :BaseSettings(false)
{
  xmlFile = TEXT("%EmergeDir%\\files\\emergeHotkeys.xml");
  appletName = TEXT("emergeHotkeys");
}

Settings::~Settings()
{
  // Clear the hotkeyList vector
  hotkeyList.clear();
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  x = 0;
  y = 0;
  width = 0;
  height = 0;
}

int Settings::GetX()
{
  return (GetSystemMetrics(SM_XVIRTUALSCREEN) - 200);
}

int Settings::GetY()
{
  return (GetSystemMetrics(SM_YVIRTUALSCREEN) - 200);
}

UINT Settings::GetHotkeyListSize()
{
  return (UINT)hotkeyList.size();
}

UINT Settings::GetBackupListSize()
{
  return (UINT)backupList.size();
}

HotkeyCombo *Settings::GetHotkeyListItem(UINT item)
{
  return hotkeyList[item].get();
}

HotkeyCombo *Settings::GetBackupListItem(UINT item)
{
  return backupList[item].get();
}

void Settings::DeleteHotkeyListItem(UINT item)
{
  hotkeyList.erase(hotkeyList.begin() + item);
  WriteList(false);
}

void Settings::AddHotkeyListItem(HotkeyCombo *item)
{
  hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(item) );
  WriteList(false);
}

void Settings::WriteList(bool backup)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Actions"), true);

      if (section)
        {
          IOHelper userIO(section);

          userIO.Clear();
          if (backup)
            {
              // Loop while there are entries in the key
              for (UINT i = 0; i < backupList.size(); i++)
                {
                  if (userIO.SetElement(TEXT("item")))
                    {
                      userIO.WriteString(TEXT("KeyCombo"),
                                         backupList[i]->GetHotkeyString());
                      userIO.WriteString(TEXT("Action"),
                                         backupList[i]->GetHotkeyAction());
                    }
                }
            }
          else
            {
              // Loop while there are entries in the key
              for (UINT i = 0; i < hotkeyList.size(); i++)
                {
                  if (userIO.SetElement(TEXT("item")))
                    {
                      userIO.WriteString(TEXT("KeyCombo"),
                                         hotkeyList[i]->GetHotkeyString());
                      userIO.WriteString(TEXT("Action"),
                                         hotkeyList[i]->GetHotkeyAction());
                    }
                }
            }

          ELWriteXMLConfig(configXML.get());
        }
    }
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	BuildList
// Required:	Nothing
// Returns:	bool
// Purpose:	Enumerates the Actions key and builds the actionsList vector
//----  --------------------------------------------------------------------------------------------------------
bool Settings::BuildList(HWND mainWnd, bool backup)
{
  bool found = false;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  std::wstring keyCombo, action;

  if (backup)
    backupList.clear();
  else
    {
      for (UINT i = 0; i < hotkeyList.size(); i++)
        UnregisterHotKey(mainWnd, hotkeyList[i]->GetHotkeyID());
      hotkeyList.clear();
    }

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), TEXT("Actions"), false);

      if (section)
        {
          IOHelper userIO(section);

          while (userIO.GetElement())
            {
              found = true;
              keyCombo = userIO.ReadString(TEXT("KeyCombo"), TEXT(""));
              action = userIO.ReadString(TEXT("Action"), TEXT(""));

              // Add the hotkey definition to the appropriate vector
              if (backup)
                backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(keyCombo, action, true)) );
              else
                hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(keyCombo, action, false)) );
            }
        }
    }

  if (!found)
    {
      if (backup)
        {
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Shift+Escape"), TEXT("activate emergeHotkeys"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Win+R"), TEXT("Run"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Win+D"), TEXT("ShowDesktop"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Win+E"), TEXT("explorer /e"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("VolumeMute"), TEXT("VolumeMute"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("VolumeUp"), TEXT("VolumeUp"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("VolumeDown"), TEXT("VolumeDown"), true)) );
          backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("LeftWinKey"), TEXT("RightDeskMenu"), true)) );
        }
      else
        {
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Shift+Escape"), TEXT("activate emergeHotkeys"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Win+R"), TEXT("Run"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Win+D"), TEXT("ShowDesktop"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("Win+E"), TEXT("explorer /e"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("VolumeMute"), TEXT("VolumeMute"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("VolumeUp"), TEXT("VolumeUp"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("VolumeDown"), TEXT("VolumeDown"), false)) );
          hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(TEXT("LeftWinKey"), TEXT("RightDeskMenu"), false)) );
        }
    }

  return found;
}

UINT Settings::FindHotkeyListItem(WCHAR *tmpKey, WCHAR *tmpAction)
{
  UINT i = 0;

  while (i < hotkeyList.size())
    {
      if (_wcsicmp(hotkeyList[i]->GetHotkeyString(), tmpKey) == 0)
        {
          if (_wcsicmp(hotkeyList[i]->GetHotkeyAction(), tmpAction) == 0)
            break;
        }

      i++;
    }

  return i;
}

bool Settings::IsValidHotkey(UINT index)
{
  return hotkeyList[index]->GetValid();
}

UINT Settings::FindHotkeyListItem(UINT index)
{
  UINT i = 0;

  while (i < hotkeyList.size())
    {
      if (hotkeyList[i]->GetHotkeyID() == index)
        break;

      i++;
    }

  return i;
}

