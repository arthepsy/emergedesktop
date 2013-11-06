//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#include "Settings.h"
#include <wchar.h>

Settings::Settings()
  : BaseSettings(false)
{
  xmlFile = TEXT("%EmergeDir%\\files\\emergeCommand.xml");
  autoComplete = true;
  timeFormat = TEXT("");
  displayTimeFormat = TEXT("");
  tipFormat = TEXT("");
  displayTipFormat = TEXT("");
  clockTextAlign = TEXT("");
  commandTextAlign = TEXT("");
  clockVerticalAlign = TEXT("");
  commandVerticalAlign = TEXT("");
  fontString = TEXT("");
  displayLines = 0;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  // Clear the logFont on read so there isn't any font style transposing between themes
  ZeroMemory(&logFont, sizeof(LOGFONT));

  BaseSettings::DoReadSettings(helper);
  clockTextAlign = helper.ReadString(TEXT("ClockTextAlign"), TEXT("left"));
  commandTextAlign = helper.ReadString(TEXT("CommandTextAlign"), TEXT("left"));
  commandVerticalAlign = helper.ReadString(TEXT("CommandVerticalAlign"), TEXT("top"));
  clockVerticalAlign = helper.ReadString(TEXT("ClockVerticalAlign"), TEXT("top"));
  timeFormat = helper.ReadString(TEXT("TimeFormat"), TEXT("%A%_%x%_%X"));
  tipFormat = helper.ReadString(TEXT("TipFormat"), TEXT("%#c"));
  fontString = helper.ReadString(TEXT("Font"), TEXT("Tahoma-12"));
  autoComplete = helper.ReadBool(TEXT("AutoComplete"), true);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteString(TEXT("ClockTextAlign"), clockTextAlign);
  helper.WriteString(TEXT("CommandTextAlign"), commandTextAlign);
  helper.WriteString(TEXT("CommandVerticalAlign"), commandVerticalAlign);
  helper.WriteString(TEXT("ClockVerticalAlign"), clockVerticalAlign);
  helper.WriteString(TEXT("TimeFormat"), timeFormat);
  helper.WriteString(TEXT("TipFormat"), tipFormat);
  helper.WriteBool(TEXT("AutoComplete"), autoComplete);

  fontString = EGFontToString(logFont);
  helper.WriteString(TEXT("Font"), fontString);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();

  WCHAR displayTimeFormatBuffer[MAX_LINE_LENGTH], displayTipFormatBuffer[MAX_LINE_LENGTH];

  logFont = EGStringToFont(fontString);

  wcscpy(displayTimeFormatBuffer, timeFormat.c_str());
  displayLines = ELStringReplace(displayTimeFormatBuffer, (WCHAR*)TEXT("%_"), (WCHAR*)TEXT("\n"), false) + 1;
  displayTimeFormat = displayTimeFormatBuffer;

  wcscpy(displayTipFormatBuffer, tipFormat.c_str());
  ELStringReplace(displayTipFormatBuffer, (WCHAR*)TEXT("%_"), (WCHAR*)TEXT("\n"), false);
  displayTipFormat = displayTipFormatBuffer;
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  clockTextAlign = TEXT("center");
  commandTextAlign = TEXT("left");
  commandVerticalAlign = TEXT("center");
  clockVerticalAlign = TEXT("center");
  timeFormat = TEXT("%#H:%M");
  tipFormat = TEXT("%#x");
  fontString = TEXT("Arial-16");
  autoComplete = true;
  width = 64;
  height = 32;
  zPosition = TEXT("Top");
  horizontalDirection = TEXT("right");
  verticalDirection = TEXT("down");
  directionOrientation = TEXT("horizontal");
  autoSize = false;
  iconSize = 16;
  iconSpacing = 1;
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
    x = -72;
    y = -40;
    anchorPoint = TEXT("BottomRight");
  }
}


UINT Settings::GetDisplayLines()
{
  return displayLines;
}

bool Settings::SetFont(LOGFONT* logFont)
{
  if (!EGEqualLogFont(this->logFont, *logFont))
  {
    fontString = EGFontToString(*logFont);
    CopyMemory(&this->logFont, logFont, sizeof(LOGFONT));
    SetModified();
  }
  return true;
}

LOGFONT* Settings::GetFont()
{
  return &logFont;
}

std::wstring Settings::GetClockTextAlign()
{
  return clockTextAlign;
}

bool Settings::GetAutoComplete()
{
  return autoComplete;
}

std::wstring Settings::GetClockVerticalAlign()
{
  return clockVerticalAlign;
}

std::wstring Settings::GetCommandVerticalAlign()
{
  return commandVerticalAlign;
}

std::wstring Settings::GetCommandTextAlign()
{
  return commandTextAlign;
}

std::wstring Settings::GetTimeFormat()
{
  return timeFormat;
}

std::wstring Settings::GetDisplayTimeFormat()
{
  return displayTimeFormat;
}

std::wstring Settings::GetDisplayTipFormat()
{
  return displayTipFormat;
}

std::wstring Settings::GetTipFormat()
{
  return tipFormat;
}

bool Settings::SetClockTextAlign(std::wstring clockTextAlign)
{
  if (ELToLower(this->clockTextAlign) != ELToLower(clockTextAlign))
  {
    this->clockTextAlign = clockTextAlign;
    SetModified();
  }
  return true;
}

bool Settings::SetAutoComplete(bool autoComplete)
{
  if (this->autoComplete != autoComplete)
  {
    this->autoComplete = autoComplete;
    SetModified();
  }
  return true;
}

bool Settings::SetClockVerticalAlign(std::wstring clockVerticalAlign)
{
  if (ELToLower(this->clockVerticalAlign) != ELToLower(clockVerticalAlign))
  {
    this->clockVerticalAlign = clockVerticalAlign;
    SetModified();
  }
  return true;
}

bool Settings::SetCommandTextAlign(std::wstring commandTextAlign)
{
  if (ELToLower(this->commandTextAlign) != ELToLower(commandTextAlign))
  {
    this->commandTextAlign = commandTextAlign;
    SetModified();
  }
  return true;
}

bool Settings::SetCommandVerticalAlign(std::wstring commandVerticalAlign)
{
  if (ELToLower(this->commandVerticalAlign) != ELToLower(commandVerticalAlign))
  {
    this->commandVerticalAlign = commandVerticalAlign;
    SetModified();
  }
  return true;
}

bool Settings::SetTimeFormat(std::wstring timeFormat)
{
  if (ELToLower(this->timeFormat) != ELToLower(timeFormat))
  {
    this->timeFormat = timeFormat;
    SetModified();
  }
  return true;
}

bool Settings::SetTipFormat(std::wstring tipFormat)
{
  if (ELToLower(this->tipFormat) == ELToLower(tipFormat))
  {
    this->tipFormat = tipFormat;
    SetModified();
  }
  return true;
}

void Settings::BuildHistoryList()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(xmlFile, false);
  TiXmlElement* section;
  std::wstring data;

  if (configXML)
  {
    // Clear the stickyList vector
    historyList.clear();
    section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("History"), false);

    if (section)
    {
      IOHelper userIO(section);

      while (userIO.GetElement())
      {
        data = userIO.ReadString(TEXT("Command"), TEXT(""));
        if (!data.empty())
        {
          historyList.push_back(data);
        }
      }
    }
  }
}

void Settings::WriteHistoryList()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(xmlFile, true);
  TiXmlElement* section;
  WCHAR command[MAX_LINE_LENGTH];

  if (configXML)
  {
    section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("History"), true);

    if (section)
    {
      IOHelper userIO(section);

      userIO.Clear();
      for (UINT i = 0; i < historyList.size(); i++)
      {
        if (userIO.SetElement(TEXT("item")))
        {
          wcscpy(command, historyList[i].c_str());
          userIO.WriteString(TEXT("Command"), command);
        }
      }
    }

    ELWriteXMLConfig(configXML.get());
  }
}

void Settings::AddHistoryItem(std::wstring item)
{
  for (UINT i = 0; i < historyList.size(); i++)
  {
    if (ELToLower(item) == ELToLower(historyList[i]))
    {
      return;
    }
  }

  historyList.push_back(item);
  if (historyList.size() > MAX_HISTORY)
  {
    historyList.erase(historyList.begin());
  }

  WriteHistoryList();
}

UINT Settings::GetHistoryListSize()
{
  return historyList.size();
}

std::wstring Settings::GetHistoryListItem(UINT index)
{
  return historyList[index];
}
