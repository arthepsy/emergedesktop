//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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
}

void Settings::DoReadSettings(IOHelper& helper)
{
  // Clear the logFont on read so there isn't any font style transposing between themes
  ZeroMemory(&logFont, sizeof(LOGFONT));

  BaseSettings::DoReadSettings(helper);
  helper.ReadString(TEXT("TextAlign"), clockTextAlign, TEXT("left"));
  helper.ReadString(TEXT("VerticalAlign"), clockVerticalAlign, TEXT("top"));
  helper.ReadString(TEXT("Text"), textFormat, TEXT("Hello World!"));
  helper.ReadString(TEXT("Tip"), tipFormat, TEXT("Welcome to%_My World!"));
  helper.ReadString(TEXT("Font"), fontString, TEXT("Tahoma-12"));
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteString(TEXT("TextAlign"), clockTextAlign);
  helper.WriteString(TEXT("VerticalAlign"), clockVerticalAlign);
  helper.WriteString(TEXT("Text"), textFormat);
  helper.WriteString(TEXT("Tip"), tipFormat);

  EGFontToString(logFont, fontString);
  helper.WriteString(TEXT("Font"), fontString);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();

  EGStringToFont(fontString, logFont);

  wcscpy(displayTextFormat, textFormat);
  displayLines = ELStringReplace(displayTextFormat, (WCHAR*)TEXT("%_"), (WCHAR*)TEXT("\n"), false) + 1;
  wcscpy(displayTipFormat, tipFormat);
  ELStringReplace(displayTipFormat, (WCHAR*)TEXT("%_"), (WCHAR*)TEXT("\n"), false);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  wcscpy(clockTextAlign, (WCHAR*)TEXT("left"));
  wcscpy(clockVerticalAlign, (WCHAR*)TEXT("top"));
  wcscpy(textFormat, (WCHAR*)TEXT("Hello World!"));
  wcscpy(tipFormat, (WCHAR*)TEXT("Welcome to%_My World!"));
  wcscpy(fontString, (WCHAR*)TEXT("Tahoma-12"));
  x = 400;
  y = 400;
}


UINT Settings::GetDisplayLines()
{
  return displayLines;
}

bool Settings::SetFont(LOGFONT *logFont)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  EGFontToString(*logFont, tmp);
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

WCHAR *Settings::GetTextAlign()
{
  return clockTextAlign;
}

WCHAR *Settings::GetVerticalAlign()
{
  return clockVerticalAlign;
}

WCHAR *Settings::GetTextFormat()
{
  return textFormat;
}

WCHAR *Settings::GetDisplayTextFormat()
{
  return displayTextFormat;
}

WCHAR *Settings::GetDisplayTipFormat()
{
  return displayTipFormat;
}

WCHAR *Settings::GetTipFormat()
{
  return tipFormat;
}

bool Settings::SetTextAlign(WCHAR *clockTextAlign)
{
  if (_wcsicmp(this->clockTextAlign, clockTextAlign) != 0)
    {
      wcscpy(this->clockTextAlign, clockTextAlign);
      SetModified();
    }
  return true;
}

bool Settings::SetVerticalAlign(WCHAR *clockVerticalAlign)
{
  if (_wcsicmp(this->clockVerticalAlign, clockVerticalAlign) != 0)
    {
      wcscpy(this->clockVerticalAlign, clockVerticalAlign);
      SetModified();
    }
  return true;
}

bool Settings::SetTextFormat(WCHAR *textFormat)
{
  if (_wcsicmp(this->textFormat, textFormat) != 0)
    {
      wcscpy(this->textFormat, textFormat);
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

