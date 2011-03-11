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

#ifndef __EV_SETTINGS_H
#define __EV_SETTINGS_H

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"
#include <string>
#include <vector>

class Settings : public BaseSettings
{
public:
  Settings();
  ~Settings();
  int GetDesktopRows();
  int GetDesktopColumns();
  bool SetDesktopRows(int rows);
  bool SetDesktopColumns(int columns);
  UINT GetStickyListSize();
  WCHAR *GetStickyListItem(UINT item);
  void DeleteStickyListItem(UINT item);
  bool CheckSticky(WCHAR *appName);
  void AddStickyListItem(WCHAR *item);
  void WriteStickyList();
  bool GetHideSticky();
  bool SetHideSticky(bool hideSticky);
  void BuildStickyList();

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void ResetDefaults();

private:
  std::vector<std::wstring> stickyList;
  std::wstring xmlFile;
  int rows, columns;
  bool hideSticky;
};

#endif

