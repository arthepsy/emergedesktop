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
#include "Applet.h"
#include "TrayIcon.h"

Settings::Settings(LPARAM lParam)
  : BaseSettings(true)
{
  unhideIcons = true;
  xmlFile = TEXT("%EmergeDir%\\files\\emergeTray.xml");
  this->lParam = lParam;
  infoFontString = TEXT("");
  infoTitleFontString = TEXT("");
  gradientMethod = TEXT("");
  borderColour = RGB(0, 0, 0);
  textColour = RGB(0, 0, 0);
  gradientTo = RGB(0, 0, 0);
  gradientFrom = RGB(0, 0, 0);
  alpha = 100;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  unhideIcons = helper.ReadBool(TEXT("UnhideIcons"), true);
  infoFontString = helper.ReadString(TEXT("InfoFont"), TEXT("Tahoma-10"));
  infoTitleFontString = helper.ReadString(TEXT("InfoTitleFont"), TEXT("Tahoma-Bold-12"));
  borderColour = helper.ReadColor(TEXT("BorderColor"), RGB(0, 0, 0));
  textColour = helper.ReadColor(TEXT("TextColor"), RGB(0, 0, 0));
  gradientFrom = helper.ReadColor(TEXT("GradientFrom"), RGB(255, 255, 255));
  gradientTo = helper.ReadColor(TEXT("GradientTo"), RGB(255, 255, 128));
  alpha = helper.ReadInt(TEXT("Alpha"), 100);
  gradientMethod = helper.ReadString(TEXT("GradientMethod"), TEXT("Vertical"));
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();

  infoLogFont = EGStringToFont(infoFontString);
  infoTitleLogFont = EGStringToFont(infoTitleFontString);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteBool(TEXT("UnhideIcons"), unhideIcons);
  infoFontString = EGFontToString(infoLogFont);
  helper.WriteString(TEXT("InfoFont"), infoFontString);
  infoTitleFontString = EGFontToString(infoTitleLogFont);
  helper.WriteString(TEXT("InfoTitleFont"), infoTitleFontString);
  helper.WriteColor(TEXT("BorderColor"), borderColour);
  helper.WriteColor(TEXT("TextColor"), textColour);
  helper.WriteColor(TEXT("GradientFrom"), gradientFrom);
  helper.WriteColor(TEXT("GradientTo"), gradientTo);
  helper.WriteInt(TEXT("Alpha"), alpha);
  helper.WriteString(TEXT("GradientMethod"), gradientMethod);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();

  unhideIcons = true;
  infoFontString = TEXT("Tahoma-10");
  infoTitleFontString = TEXT("Tahoma-Bold-12");
  borderColour = RGB(0, 0, 0);
  textColour =  RGB(0, 0, 0);
  gradientFrom = RGB(255, 255, 255);
  gradientTo =  RGB(255, 255, 128);
  alpha =  100;
  gradientMethod = TEXT("Vertical");

  x = -72;
  y = -1;
  width = 156;
  height = 32;
  zPosition = TEXT("Top");
  horizontalDirection = TEXT("left");
  verticalDirection = TEXT("up");
  directionOrientation = TEXT("horizontal");
  autoSize = false;
  iconSize = 16;
  iconSpacing = 3;
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  anchorPoint = TEXT("BottomRight");
}

bool Settings::GetUnhideIcons()
{
  return unhideIcons;
}

LOGFONT* Settings::GetInfoFont()
{
  return &infoLogFont;
}

COLORREF Settings::GetTextColor()
{
  return textColour;
}

COLORREF Settings::GetBorderColor()
{
  return borderColour;
}

COLORREF Settings::GetGradientFrom()
{
  return gradientFrom;
}

COLORREF Settings::GetGradientTo()
{
  return gradientTo;
}

int Settings::GetAlpha()
{
  return alpha;
}

std::wstring Settings::GetGradientMethod()
{
  return gradientMethod;
}

bool Settings::SetGradientMethod(std::wstring gradientMethod)
{
  if (ELToLower(this->gradientMethod) != ELToLower(gradientMethod))
  {
    this->gradientMethod = gradientMethod;
    SetModified();
  }
  return true;
}

LOGFONT* Settings::GetInfoTitleFont()
{
  return &infoTitleLogFont;
}

bool Settings::SetGradientFrom(COLORREF gradientFrom)
{
  if (this->gradientFrom != gradientFrom)
  {
    this->gradientFrom = gradientFrom;
    SetModified();
  }
  return true;
}

bool Settings::SetGradientTo(COLORREF gradientTo)
{
  if (this->gradientTo != gradientTo)
  {
    this->gradientTo = gradientTo;
    SetModified();
  }
  return true;
}

bool Settings::SetTextColor(COLORREF textColour)
{
  if (this->textColour != textColour)
  {
    this->textColour = textColour;
    SetModified();
  }
  return true;
}

bool Settings::SetBorderColor(COLORREF borderColour)
{
  if (this->borderColour != borderColour)
  {
    this->borderColour = borderColour;
    SetModified();
  }
  return true;
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

bool Settings::SetAlpha(int alpha)
{
  if (this->alpha != alpha)
  {
    this->alpha = alpha;
    SetModified();
  }
  return true;
}

bool Settings::SetInfoFont(LOGFONT* infoLogFont)
{
  if (!EGEqualLogFont(this->infoLogFont, *infoLogFont))
  {
    infoFontString = EGFontToString(*infoLogFont);
    CopyMemory(&this->infoLogFont, infoLogFont, sizeof(LOGFONT));
    SetModified();
  }
  return true;
}

bool Settings::SetInfoTitleFont(LOGFONT* infoTitleLogFont)
{
  if (!EGEqualLogFont(this->infoTitleLogFont, *infoTitleLogFont))
  {
    infoTitleFontString = EGFontToString(*infoTitleLogFont);
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
  TiXmlElement* section;
  std::wstring data;

  if (configXML)
  {
    // Clear the stickyList vector
    hideList.clear();
    section = ELGetXMLSection(configXML.get(), TEXT("Hide"), false);

    if (section)
    {
      IOHelper userIO(section);

      while (userIO.GetElement())
      {
        data = userIO.ReadString(TEXT("Icon"), TEXT(""));
        if (!data.empty())
        {
          hideList.push_back(data);
        }
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
bool Settings::CheckHide(WCHAR* iconTip)
{
  UINT i;
  bool tipMatch = false;
  WCHAR tmpiconTip[TIP_SIZE], tmphideList[TIP_SIZE];
  ZeroMemory(tmpiconTip, TIP_SIZE);
  ZeroMemory(tmphideList, TIP_SIZE);

  wcscpy(tmpiconTip, iconTip);

  for (i = 0; i < hideList.size(); i++)
  {
    wcscpy(tmphideList, (WCHAR*)hideList[i].c_str());
    if (wcsstr(_wcslwr(tmpiconTip), _wcslwr(tmphideList)) != NULL)
    {
      tipMatch = true;
    }
  }

  return tipMatch;
}


UINT Settings::GetHideListSize()
{
  return (UINT)hideList.size();
}


WCHAR* Settings::GetHideListItem(UINT item)
{
  return (WCHAR*)hideList[item].c_str();
}

void Settings::DeleteHideListItem(WCHAR* itemText)
{
  UINT i = 0;
  Applet* pApplet = reinterpret_cast<Applet*>(lParam);
  std::vector<std::wstring>::iterator iter = hideList.begin();

  while (i < pApplet->GetTrayIconListSize())
  {
    if (wcsstr(pApplet->GetTrayIconListItem(i)->GetTip(), itemText))
    {
      pApplet->GetTrayIconListItem(i)->SetHidden(false);
    }

    i++;
  }

  /**< Using a '!=' caused a crash here, switching to '<' fixed the crash - odd */
  while (iter < hideList.end())
  {
    if ((*iter) == itemText)
    {
      hideList.erase(iter);
    }
    iter++;
  }
}

void Settings::ModifyHideListItem(WCHAR* oldText, WCHAR* newText)
{
  UINT i = 0;
  Applet* pApplet = reinterpret_cast<Applet*>(lParam);
  std::vector<std::wstring>::iterator iter = hideList.begin();

  while (i < pApplet->GetTrayIconListSize())
  {
    if (wcsstr(pApplet->GetTrayIconListItem(i)->GetTip(), oldText))
    {
      pApplet->GetTrayIconListItem(i)->SetHidden(false);
    }

    i++;
  }

  while (iter < hideList.end())
  {
    if ((*iter) == oldText)
    {
      (*iter) = newText;
    }
    iter++;
  }
}

void Settings::AddHideListItem(WCHAR* item)
{
  std::vector<TrayIcon*>::iterator iter;
  Applet* pApplet = reinterpret_cast<Applet*>(lParam);
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
  TiXmlElement* section;

  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), TEXT("Hide"), true);

    if (section)
    {
      IOHelper userIO(section);

      userIO.Clear();
      for (UINT i = 0; i < hideList.size(); i++)
      {
        if (userIO.SetElement(TEXT("item")))
        {
          userIO.WriteString(TEXT("Icon"), hideList[i]);
        }
      }
    }

    ELWriteXMLConfig(configXML.get());
  }
}
