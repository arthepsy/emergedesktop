//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2009  The Emerge Desktop Development Team
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
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  helper.ReadString(TEXT("Font"), fontString, TEXT("Tahoma-12"));
  helper.ReadInt(TEXT("UpdateInterval"), updateInterval, 1000);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  EGFontToString(logFont, fontString);
  helper.WriteString(TEXT("Font"), fontString);
  helper.WriteInt(TEXT("UpdateInterval"), updateInterval);
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
  updateInterval = 1000;
  SetHorizontalDirection((WCHAR*)TEXT("center"));
  SetVerticalDirection((WCHAR*)TEXT("center"));
}

void Settings::SetFont(LOGFONT *logFont)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*logFont, tmp);
  if (!EGEqualLogFont(this->logFont, *logFont))
    {
      wcscpy(fontString, tmp);
      this->logFont = *logFont;
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

LOGFONT *Settings::GetFont()
{
  return &logFont;
}

int Settings::GetUpdateInterval()
{
  return updateInterval;
}
