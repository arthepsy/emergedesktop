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
#include "Applet.h"

extern WCHAR myName[];

Settings::Settings(LPARAM lParam)
  :BaseSettings(true)
{
  unhideIcons = true;
  xmlFile = TEXT("%EmergeDir%\\files\\emergeTray.xml");
  this->lParam = lParam;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  helper.ReadBool(TEXT("UnhideIcons"), unhideIcons, true);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteBool(TEXT("UnhideIcons"), unhideIcons);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  unhideIcons = true;
  x = 104;
}

bool Settings::GetUnhideIcons()
{
  return unhideIcons;
}

bool Settings::SetUnhideIcons(bool unhideIcons)
{
  if (this->unhideIcons != unhideIcons)
    {
      this->unhideIcons = unhideIcons;
      SetModified();
    }
  return true;
}

//-----
// Function:	BuildHideList
// Required:	Nothing
// Returns:	Nothing
// Purpose:	Enumerates the hide key and builds the hideList vector
//-----
void Settings::BuildHideList()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(xmlFile, false);
  TiXmlElement *section;
  WCHAR data[MAX_LINE_LENGTH];

  if (configXML)
    {
      // Clear the stickyList vector
      hideList.clear();
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Hide"), false);

      if (section)
        {
          IOHelper userIO(section);

          while (userIO.GetElement())
            {
              if (userIO.ReadString(TEXT("Icon"), data, TEXT("")))
                hideList.push_back(data);
            }
        }
    }
}

//-----
// Function:	CheckHide
// Required:	WCHAR *iconTip - Tooltip of icon to check
// Returns:	bool
// Purpose:	Checks to see if the icon should be hidden
//-----
bool Settings::CheckHide(WCHAR *iconTip)
{
  UINT i;
  bool tipMatch = false;
  WCHAR tmpiconTip[TIP_SIZE], tmphideList[TIP_SIZE];
  ZeroMemory(tmpiconTip,TIP_SIZE);
  ZeroMemory(tmphideList,TIP_SIZE);

  wcscpy(tmpiconTip, iconTip);

  for (i = 0; i < hideList.size(); i++)
    {
      wcscpy(tmphideList,(WCHAR *)hideList[i].c_str());
      if (wcsstr(_wcslwr(tmpiconTip), _wcslwr(tmphideList)) != NULL)
        tipMatch = true;
    }

  return tipMatch;
}


UINT Settings::GetHideListSize()
{
  return (UINT)hideList.size();
}


WCHAR *Settings::GetHideListItem(UINT item)
{
  return (WCHAR*)hideList[item].c_str();
}

void Settings::DeleteHideListItem(UINT item)
{
  UINT i = 0;
  Applet *pApplet = reinterpret_cast<Applet*>(lParam);

  while (i < pApplet->GetTrayIconListSize())
    {
      if (wcsstr(pApplet->GetTrayIconListItem(i)->GetTip(), hideList[item].c_str()))
        pApplet->GetTrayIconListItem(i)->SetHidden(false);

      i++;
    }

  hideList.erase(hideList.begin() + item);
}

void Settings::ModifyHideListItem(UINT item, WCHAR *itemText)
{
  hideList[item] = itemText;
}

void Settings::AddHideListItem(WCHAR *item)
{
  std::vector<TrayIcon*>::iterator iter;
  Applet *pApplet = reinterpret_cast<Applet*>(lParam);
  bool itemexists;
  UINT i = 0;

  itemexists = false;

  for (UINT i = 0; i < hideList.size(); i++)
    {
      if (_wcsicmp((WCHAR*)hideList[i].c_str(), item) == 0)
        {
          itemexists = true;
        }

    }

  if (!itemexists)
    {
      while (i < pApplet->GetTrayIconListSize())
        {
          if (wcsstr(pApplet->GetTrayIconListItem(i)->GetTip(), item))
            {
              pApplet->GetTrayIconListItem(i)->SetHidden(true);
              pApplet->ShowHiddenIcons(true, true);
            }

          i++;
        }

      hideList.push_back(item);
    }
}

void Settings::WriteHideList()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(xmlFile, true);
  TiXmlElement *section;

  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Hide"), true);

      if (section)
        {
          IOHelper userIO(section);

          userIO.Clear();
          for (UINT i = 0; i < hideList.size(); i++)
            {
              if (userIO.SetElement(TEXT("item")))
                userIO.WriteString(TEXT("Icon"), (WCHAR*)hideList[i].c_str());
            }
        }

      ELWriteXMLConfig(configXML.get());
    }
}
