//---
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
//---

#ifndef __SETTINGS_H
#define __SETTINGS_H

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"

#include <string>
#include <vector>

#define MAX_HISTORY 10

class Settings : public BaseSettings
{
public:
  Settings();
  WCHAR *GetTextFormat();
  WCHAR *GetDisplayTextFormat();
  WCHAR *GetTipFormat();
  WCHAR *GetDisplayTipFormat();
  WCHAR *GetTextAlign();
  WCHAR *GetVerticalAlign();
  LOGFONT *GetFont();
  UINT GetDisplayLines();
  bool SetFont(LOGFONT *logFont);
  bool SetTextFormat(WCHAR *textFormat);
  bool SetTipFormat(WCHAR *textFormat);
  bool SetTextAlign(WCHAR *clockTextAlign);
  bool SetVerticalAlign(WCHAR *clockVerticalAlign);

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void DoInitialize();
  virtual void ResetDefaults();

private:
  WCHAR textFormat[MAX_LINE_LENGTH];
  WCHAR displayTextFormat[MAX_LINE_LENGTH];
  WCHAR tipFormat[MAX_LINE_LENGTH];
  WCHAR displayTipFormat[MAX_LINE_LENGTH];
  WCHAR clockTextAlign[MAX_LINE_LENGTH];
  WCHAR clockVerticalAlign[MAX_LINE_LENGTH];
  WCHAR fontString[MAX_LINE_LENGTH];
  LOGFONT logFont;
  UINT displayLines;
};

#endif
