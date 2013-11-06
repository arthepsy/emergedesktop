//---
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
//---

#include "Settings.h"
#include <stdio.h>

Settings::Settings()
  : BaseSettings(false)
{
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  fontString = helper.ReadString(TEXT("Font"), TEXT("Tahoma-12"));
  displayType = helper.ReadInt(TEXT("DisplayType"), 0);
  upperCase = helper.ReadBool(TEXT("UpperCase"), true);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  fontString = EGFontToString(logFont);
  helper.WriteString(TEXT("Font"), fontString);
  helper.WriteInt(TEXT("DisplayType"), displayType);
  helper.WriteBool(TEXT("UpperCase"), upperCase);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();
  logFont = EGStringToFont(fontString);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  fontString = TEXT("Tahoma-12");
  displayType = 0;
}

void Settings::SetFont(LOGFONT* logFont)
{
  if (!EGEqualLogFont(this->logFont, *logFont))
  {
    fontString = EGFontToString(*logFont);
    CopyMemory(&this->logFont, logFont, sizeof(LOGFONT));
    SetModified();
  }
}

LOGFONT* Settings::GetFont()
{
  return &logFont;
}

LCTYPE Settings::GetDisplayLCType()
{
  switch(displayType)
  {
  case 0:
    return LOCALE_SISO639LANGNAME;
  case 1:
    return LOCALE_SABBREVLANGNAME;
  case 2:
    return LOCALE_SISO3166CTRYNAME;
  default:
    return LOCALE_SISO639LANGNAME;
  }
}

int Settings::GetDisplayType()
{
  return displayType;
}

void Settings::SetDisplayType(int value)
{
  if (displayType != value)
  {
    displayType = value;
    SetModified();
  }
}

bool Settings::IsUpperCase()
{
  return upperCase;
}

void Settings::SetUpperCase(bool value)
{
  if (upperCase != value)
  {
    upperCase = value;
    SetModified();
  }
}
