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
  ZeroMemory(fontString, MAX_LINE_LENGTH);
  ZeroMemory(textHorizontalAlign, MAX_LINE_LENGTH);
  ZeroMemory(textVerticalAlign, MAX_LINE_LENGTH);

  updateInterval = 60;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  // Clear the logFont on read so there isn't any font style transposing between themes
  ZeroMemory(&logFont, sizeof(LOGFONT));

  BaseSettings::DoReadSettings(helper);
  helper.ReadString(TEXT("Font"), fontString, TEXT("Tahoma-12"));
  helper.ReadInt(TEXT("UpdateInterval"), updateInterval, 60);
  helper.ReadString(TEXT("TextHorizontalAlign"), textHorizontalAlign, TEXT("left"));
  helper.ReadString(TEXT("TextVerticalAlign"), textVerticalAlign, TEXT("top"));
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  EGFontToString(logFont, fontString);
  helper.WriteString(TEXT("Font"), fontString);
  helper.WriteInt(TEXT("UpdateInterval"), updateInterval);
  helper.WriteString(TEXT("TextHorizontalAlign"), textHorizontalAlign);
  helper.WriteString(TEXT("TextVerticalAlign"), textVerticalAlign);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();
  EGStringToFont(fontString, logFont);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  wcscpy(fontString, (WCHAR*)TEXT("Tahoma-12"));
  updateInterval = 60;
}

void Settings::SetFont(LOGFONT *logFont)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*logFont, tmp);
  if (!EGEqualLogFont(this->logFont, *logFont))
    {
      wcscpy(fontString, tmp);
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

WCHAR *Settings::GetHorizontalAlign()
{
  return textHorizontalAlign;
}

bool Settings::SetHorizontalAlign(WCHAR *horizontalAlign)
{
  if (_wcsicmp(textHorizontalAlign, horizontalAlign) != 0)
    {
      wcscpy(textHorizontalAlign, horizontalAlign);
      SetModified();
    }
  return true;
}

WCHAR *Settings::GetVerticalAlign()
{
  return textVerticalAlign;
}

bool Settings::SetVerticalAlign(WCHAR *horizontalAlign)
{
  if (_wcsicmp(textVerticalAlign, horizontalAlign) != 0)
    {
      wcscpy(textVerticalAlign, horizontalAlign);
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
