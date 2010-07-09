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
#include "TrayIcon.h"

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
  helper.ReadString(TEXT("InfoFont"), infoFontString, TEXT("Tahoma-10"));
  helper.ReadString(TEXT("InfoTitleFont"), infoTitleFontString, TEXT("Tahoma-12"));
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();

  EGStringToFont(infoFontString, infoLogFont);
  EGStringToFont(infoTitleFontString, infoTitleLogFont);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteBool(TEXT("UnhideIcons"), unhideIcons);
  EGFontToString(infoLogFont, infoFontString);
  helper.WriteString(TEXT("InfoFont"), infoFontString);
  EGFontToString(infoTitleLogFont, infoTitleFontString);
  helper.WriteString(TEXT("InfoTitleFont"), infoTitleFontString);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  unhideIcons = true;
  x = -72;
  y = -1;
  width = 156;
  height = 32;
  wcscpy(zPosition, TEXT("Top"));
  wcscpy(horizontalDirection, TEXT("left"));
  wcscpy(verticalDirection, TEXT("up"));
  wcscpy(directionOrientation, TEXT("horizontal"));
  autoSize = false;
  iconSize = 16;
  iconSpacing = 3;
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  wcscpy(anchorPoint, TEXT("BottomRight"));
}

bool Settings::GetUnhideIcons()
{
  return unhideIcons;
}

LOGFONT *Settings::GetInfoFont()
{
  return &infoLogFont;
}

LOGFONT *Settings::GetInfoTitleFont()
{
  return &infoTitleLogFont;
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

bool Settings::SetInfoFont(LOGFONT *infoLogFont)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*infoLogFont, tmp);

  if (!EGEqualLogFont(this->infoLogFont, *infoLogFont))
    {
      wcscpy(infoFontString, tmp);
      CopyMemory(&this->infoLogFont, infoLogFont, sizeof(LOGFONT));
      SetModified();
    }
  return true;
}

bool Settings::SetInfoTitleFont(LOGFONT *infoTitleLogFont)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*infoTitleLogFont, tmp);

  if (!EGEqualLogFont(this->infoTitleLogFont, *infoTitleLogFont))
    {
      wcscpy(infoTitleFontString, tmp);
      CopyMemory(&this->infoTitleLogFont, infoTitleLogFont, sizeof(LOGFONT));
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
