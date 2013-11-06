/*!
  @file Settings.h
  @brief header for emergeCommand
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef __GUARD_11a31f42_cfc4_4185_b922_d0c098ab42b9
#define __GUARD_11a31f42_cfc4_4185_b922_d0c098ab42b9

#define UNICODE 1

#define MAX_HISTORY 50

#include <string>
#include <vector>
#include "../emergeBaseClasses/BaseSettings.h"

class Settings : public BaseSettings
{
public:
  Settings();
  std::wstring GetTimeFormat();
  std::wstring GetDisplayTimeFormat();
  std::wstring GetTipFormat();
  std::wstring GetDisplayTipFormat();
  std::wstring GetClockTextAlign();
  std::wstring GetCommandTextAlign();
  std::wstring GetClockVerticalAlign();
  std::wstring GetCommandVerticalAlign();
  LOGFONT* GetFont();
  UINT GetDisplayLines();
  bool SetFont(LOGFONT* logFont);
  bool SetTimeFormat(std::wstring timeFormat);
  bool SetTipFormat(std::wstring timeFormat);
  bool SetClockTextAlign(std::wstring clockTextAlign);
  bool SetCommandTextAlign(std::wstring commandTextAlign);
  bool SetClockVerticalAlign(std::wstring clockVerticalAlign);
  bool SetCommandVerticalAlign(std::wstring commandVerticalAlign);
  bool GetAutoComplete();
  bool SetAutoComplete(bool autoComplete);
  void WriteHistoryList();
  void BuildHistoryList();
  void AddHistoryItem(std::wstring item);
  UINT GetHistoryListSize();
  std::wstring GetHistoryListItem(UINT index);

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void DoInitialize();
  virtual void ResetDefaults();

private:
  bool autoComplete;
  std::vector<std::wstring> historyList;
  std::wstring xmlFile;
  std::wstring timeFormat;
  std::wstring displayTimeFormat;
  std::wstring tipFormat;
  std::wstring displayTipFormat;
  std::wstring clockTextAlign;
  std::wstring commandTextAlign;
  std::wstring clockVerticalAlign;
  std::wstring commandVerticalAlign;
  std::wstring fontString;
  LOGFONT logFont;
  UINT displayLines;
};

#endif
