//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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

#ifndef __ETR_SETTINGS_H
#define __ETR_SETTINGS_H

#include "../emergeLib/emergeLib.h"
#include "../emergeBaseClasses/BaseSettings.h"

#include <string>
#include <vector>

class Settings : public BaseSettings
{
public:
  Settings(LPARAM lParam);
  bool GetUnhideIcons();
  bool SetUnhideIcons(bool unhideIcons);
  UINT GetHideListSize();
  WCHAR *GetHideListItem(UINT item);
  void DeleteHideListItem(UINT item);
  void ModifyHideListItem(UINT item, WCHAR *itemText);
  bool CheckHide(WCHAR *iconTip);
  void AddHideListItem(WCHAR *item);
  void WriteHideList();
  void BuildHideList();

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void ResetDefaults();

private:
  LPARAM lParam;
  std::vector<std::wstring> hideList;
  std::wstring xmlFile;
  bool unhideIcons;
};

#endif
