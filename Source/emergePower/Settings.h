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

#ifndef __EP_SETTINGS_H
#define __EP_SETTINGS_H

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"

class Settings: public BaseSettings
{
public:
  Settings();
  void SetFont(LOGFONT *logFont);
  void SetUpdateInterval(int ival);
  LOGFONT *GetFont();
  int GetUpdateInterval();
  WCHAR *GetHorizontalAlign();
  bool SetHorizontalAlign(WCHAR *horizontalAlign);
  WCHAR *GetVerticalAlign();
  bool SetVerticalAlign(WCHAR *horizontalAlign);

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void DoInitialize();
  virtual void ResetDefaults();

private:
  int updateInterval;
  WCHAR fontString[MAX_LINE_LENGTH], textHorizontalAlign[MAX_LINE_LENGTH], textVerticalAlign[MAX_LINE_LENGTH];
  LOGFONT logFont;
};

#endif
