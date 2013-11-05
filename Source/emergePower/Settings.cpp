//---
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
//---

#include "Settings.h"
#include <stdio.h>

Settings::Settings(): BaseSettings(false)
{
  fontString = TEXT("");
  textHorizontalAlign = TEXT("");
  textVerticalAlign = TEXT("");

  updateInterval = 60;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  // Clear the logFont on read so there isn't any font style transposing between themes
  ZeroMemory(&logFont, sizeof(LOGFONT));

  BaseSettings::DoReadSettings(helper);
  fontString = helper.ReadString(TEXT("Font"), TEXT("Tahoma-12"));
  updateInterval = helper.ReadInt(TEXT("UpdateInterval"), 60);
  textHorizontalAlign = helper.ReadString(TEXT("TextHorizontalAlign"), TEXT("left"));
  textVerticalAlign = helper.ReadString(TEXT("TextVerticalAlign"), TEXT("top"));
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  fontString = EGFontToString(logFont);
  helper.WriteString(TEXT("Font"), fontString);
  helper.WriteInt(TEXT("UpdateInterval"), updateInterval);
  helper.WriteString(TEXT("TextHorizontalAlign"), textHorizontalAlign);
  helper.WriteString(TEXT("TextVerticalAlign"), textVerticalAlign);
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
  updateInterval = 60;
}

void Settings::SetFont(LOGFONT *logFont)
{
  if (!EGEqualLogFont(this->logFont, *logFont))
    {
      fontString = EGFontToString(*logFont);
      CopyMemory(&this->logFont, logFont, sizeof(LOGFONT));
      SetModified();
    }
}

void Settings::SetUpdateInterval(int interval)
{
  if (updateInterval != interval)
    {
      updateInterval = interval;
      SetModified();
    }
}

std::wstring Settings::GetHorizontalAlign()
{
  return textHorizontalAlign;
}

bool Settings::SetHorizontalAlign(std::wstring horizontalAlign)
{
  if (ELToLower(textHorizontalAlign) != ELToLower(horizontalAlign))
    {
      textHorizontalAlign = horizontalAlign;
      SetModified();
    }
  return true;
}

std::wstring Settings::GetVerticalAlign()
{
  return textVerticalAlign;
}

bool Settings::SetVerticalAlign(std::wstring verticalAlign)
{
  if (ELToLower(textVerticalAlign) == ELToLower(verticalAlign))
    {
      textVerticalAlign = verticalAlign;
      SetModified();
    }
  return true;
}

LOGFONT *Settings::GetFont()
{
  return &logFont;
}

int Settings::GetUpdateInterval()
{
  return updateInterval;
}
