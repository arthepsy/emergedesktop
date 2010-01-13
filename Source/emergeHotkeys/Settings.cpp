// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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

Settings::Settings(HWND mainWnd)
  :BaseSettings(true)
{
  (*this).mainWnd = mainWnd;
  xmlFile = TEXT("%EmergeDir%\\files\\emergeHotkeys.xml");
}

Settings::~Settings()
{
  // Clear the hotkeyList vector
  hotkeyList.clear();
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
                      userIO.WriteString(TEXT("KeyCombo"), backupList[i]->GetHotkeyString());
                      userIO.WriteString(TEXT("Action"), backupList[i]->GetHotkeyAction());
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
                      userIO.WriteString(TEXT("KeyCombo"), hotkeyList[i]->GetHotkeyString());
                      userIO.WriteString(TEXT("Action"), hotkeyList[i]->GetHotkeyAction());
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
bool Settings::BuildList(bool backup)
{
  bool found = false;
  UINT i;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section;
  WCHAR keyCombo[MAX_LINE_LENGTH], action[MAX_LINE_LENGTH];

  if (backup)
    {
      for (i = 0; i < backupList.size(); i++)
        {
          UnregisterHotKey(mainWnd, backupList[i]->GetHotkeyID());
          backupList.erase(backupList.begin() + i);
        }
      backupList.clear();
    }
  else
    {
      for (i = 0; i < hotkeyList.size(); i++)
        {
          UnregisterHotKey(mainWnd, hotkeyList[i]->GetHotkeyID());
          hotkeyList.erase(hotkeyList.begin() + i);
        }
      hotkeyList.clear();
    }

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Actions"), false);

      if (section)
        {
          IOHelper userIO(section);

          while (userIO.GetElement())
            {
              found = true;
              userIO.ReadString(TEXT("KeyCombo"), keyCombo, TEXT(""));
              userIO.ReadString(TEXT("Action"), action, TEXT(""));

              // Add the hotkey definition to the appropriate vector
              if (backup)
                backupList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(keyCombo, action)) );
              else
                hotkeyList.push_back( std::tr1::shared_ptr<HotkeyCombo>(new HotkeyCombo(keyCombo, action)) );
            }
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

