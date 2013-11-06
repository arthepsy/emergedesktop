/*!
  @file Settings.h
  @brief header for emergeHotkeys
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

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

#ifndef __GUARD_da0f2823_f8d1_4354_930e_e75d209c5814
#define __GUARD_da0f2823_f8d1_4354_930e_e75d209c5814

#include <vector>
#include "../emergeBaseClasses/BaseSettings.h"
#include "HotkeyCombo.h"

class Settings : public BaseSettings
{
public:
  Settings();
  ~Settings();
  bool BuildList(HWND mainWnd, bool backup);
  UINT GetHotkeyListSize();
  UINT GetBackupListSize();
  HotkeyCombo* GetHotkeyListItem(UINT item);
  HotkeyCombo* GetBackupListItem(UINT item);
  void DeleteHotkeyListItem(UINT item);
  void AddHotkeyListItem(HotkeyCombo* item);
  UINT FindHotkeyListItem(WCHAR* tmpKey, WCHAR* tmpAction);
  UINT FindHotkeyListItem(UINT index);
  void WriteList(bool backup);
  bool IsValidHotkey(UINT index);
  int GetX();
  int GetY();

protected:
  void ResetDefaults();

private:
  std::vector< std::tr1::shared_ptr<HotkeyCombo> > hotkeyList;
  std::vector< std::tr1::shared_ptr<HotkeyCombo> > backupList;
  std::wstring xmlFile;
};

#endif
