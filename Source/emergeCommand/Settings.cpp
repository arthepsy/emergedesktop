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
#include <wchar.h>

Settings::Settings()
  :BaseSettings(false)
{
  ZeroMemory(&logFont, sizeof(LOGFONT));
  xmlFile = TEXT("%EmergeDir%\\files\\emergeCommand.xml");
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  helper.ReadString(TEXT("ClockTextAlign"), clockTextAlign, TEXT("left"));
  helper.ReadString(TEXT("CommandTextAlign"), commandTextAlign, TEXT("left"));
  helper.ReadString(TEXT("CommandVerticalAlign"), commandVerticalAlign, TEXT("top"));
  helper.ReadString(TEXT("ClockVerticalAlign"), clockVerticalAlign, TEXT("top"));
  helper.ReadString(TEXT("TimeFormat"), timeFormat, TEXT("%A%_%x%_%X"));
  helper.ReadString(TEXT("TipFormat"), tipFormat, TEXT("%#c"));
  helper.ReadString(TEXT("Font"), fontString, TEXT("Tahoma-12"));
  helper.ReadBool(TEXT("AutoComplete"), autoComplete, true);
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

  EGFontToString(logFont, fontString);
  helper.WriteString(TEXT("Font"), fontString);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();

  EGStringToFont(fontString, logFont);

  wcscpy(displayTimeFormat, timeFormat);
  displayLines = ELStringReplace(displayTimeFormat, (WCHAR*)TEXT("%_"), (WCHAR*)TEXT("\n"), false) + 1;
  wcscpy(displayTipFormat, tipFormat);
  ELStringReplace(displayTipFormat, (WCHAR*)TEXT("%_"), (WCHAR*)TEXT("\n"), false);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  wcscpy(clockTextAlign, (WCHAR*)TEXT("left"));
  wcscpy(commandTextAlign, (WCHAR*)TEXT("left"));
  wcscpy(commandVerticalAlign, (WCHAR*)TEXT("top"));
  wcscpy(clockVerticalAlign, (WCHAR*)TEXT("top"));
  wcscpy(timeFormat, (WCHAR*)TEXT("%A%_%x%_%X"));
  wcscpy(tipFormat, (WCHAR*)TEXT("%#c"));
  wcscpy(fontString, (WCHAR*)TEXT("Tahoma-12"));
  autoComplete = true;
  x = 208;
}


UINT Settings::GetDisplayLines()
{
  return displayLines;
}

bool Settings::SetFont(LOGFONT *logFont)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*logFont, tmp);

  std::wstring debug = tmp;
  ELWriteDebug(debug);
  debug = fontString;
  ELWriteDebug(debug);

  if (!EGEqualLogFont(this->logFont, *logFont))
    {
      wcscpy(fontString, tmp);
      CopyMemory(&this->logFont, logFont, sizeof(LOGFONT));
      SetModified();
    }
  return true;
}

LOGFONT *Settings::GetFont()
{
  return &logFont;
}

WCHAR *Settings::GetClockTextAlign()
{
  return clockTextAlign;
}

bool Settings::GetAutoComplete()
{
  return autoComplete;
}

WCHAR *Settings::GetClockVerticalAlign()
{
  return clockVerticalAlign;
}

WCHAR *Settings::GetCommandVerticalAlign()
{
  return commandVerticalAlign;
}

WCHAR *Settings::GetCommandTextAlign()
{
  return commandTextAlign;
}

WCHAR *Settings::GetTimeFormat()
{
  return timeFormat;
}

WCHAR *Settings::GetDisplayTimeFormat()
{
  return displayTimeFormat;
}

WCHAR *Settings::GetDisplayTipFormat()
{
  return displayTipFormat;
}

WCHAR *Settings::GetTipFormat()
{
  return tipFormat;
}

bool Settings::SetClockTextAlign(WCHAR *clockTextAlign)
{
  if (_wcsicmp(this->clockTextAlign, clockTextAlign) != 0)
    {
      wcscpy(this->clockTextAlign, clockTextAlign);
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

bool Settings::SetClockVerticalAlign(WCHAR *clockVerticalAlign)
{
  if (_wcsicmp(this->clockVerticalAlign, clockVerticalAlign) != 0)
    {
      wcscpy(this->clockVerticalAlign, clockVerticalAlign);
      SetModified();
    }
  return true;
}

bool Settings::SetCommandTextAlign(WCHAR *commandTextAlign)
{
  if (_wcsicmp(this->commandTextAlign, commandTextAlign) != 0)
    {
      wcscpy(this->commandTextAlign, commandTextAlign);
      SetModified();
    }
  return true;
}

bool Settings::SetCommandVerticalAlign(WCHAR *commandVerticalAlign)
{
  if (_wcsicmp(this->commandVerticalAlign, commandVerticalAlign) != 0)
    {
      wcscpy(this->commandVerticalAlign, commandVerticalAlign);
      SetModified();
    }
  return true;
}

bool Settings::SetTimeFormat(WCHAR *timeFormat)
{
  if (_wcsicmp(this->timeFormat, timeFormat) != 0)
    {
      wcscpy(this->timeFormat, timeFormat);
      SetModified();
    }
  return true;
}

bool Settings::SetTipFormat(WCHAR *tipFormat)
{
  if (_wcsicmp(this->tipFormat, tipFormat) != 0)
    {
      wcscpy(this->tipFormat, tipFormat);
      SetModified();
    }
  return true;
}

void Settings::BuildHistoryList()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(xmlFile, false);
  TiXmlElement *section;
  WCHAR data[MAX_LINE_LENGTH];

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
              if (userIO.ReadString(TEXT("Command"), data, TEXT("")))
                historyList.push_back(data);
            }
        }
    }
}

void Settings::WriteHistoryList()
{
  std::tr1::shared_ptr<TiXmlDocument> configXML = ELOpenXMLConfig(xmlFile, true);
  TiXmlElement *section;

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
                userIO.WriteString(TEXT("Command"), (WCHAR*)historyList[i].c_str());
            }
        }

      ELWriteXMLConfig(configXML.get());
    }
}

void Settings::AddHistoryItem(WCHAR *item)
{
  UINT size = historyList.size();

  for (UINT i = 0; i < size; i++)
    {
      if (_wcsicmp(item, historyList[i].c_str()) == 0)
        return;
    }

  historyList.push_back(item);
  if (historyList.size() > MAX_HISTORY)
    historyList.erase(historyList.begin());

  WriteHistoryList();
}

UINT Settings::GetHistoryListSize()
{
  return historyList.size();
}

WCHAR *Settings::GetHistoryListItem(UINT index)
{
  return (WCHAR*)historyList[index].c_str();
}
