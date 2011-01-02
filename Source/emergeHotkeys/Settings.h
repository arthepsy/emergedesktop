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

#ifndef __EH_SETTINGS_H
#define __EH_SETTINGS_H

#include "HotkeyCombo.h"
#include "../emergeBaseClasses/BaseSettings.h"
#include <vector>

class Settings : public BaseSettings
{
public:
  Settings(HWND mainWnd);
  ~Settings();
  bool BuildList(bool backup);
  UINT GetHotkeyListSize();
  UINT GetBackupListSize();
  HotkeyCombo *GetHotkeyListItem(UINT item);
  HotkeyCombo *GetBackupListItem(UINT item);
  void DeleteHotkeyListItem(UINT item);
  void AddHotkeyListItem(HotkeyCombo *item);
  UINT FindHotkeyListItem(WCHAR *tmpKey, WCHAR *tmpAction);
  UINT FindHotkeyListItem(UINT index);
  void WriteList(bool backup);

private:
  std::vector< std::tr1::shared_ptr<HotkeyCombo> > hotkeyList;
  std::vector< std::tr1::shared_ptr<HotkeyCombo> > backupList;
  std::wstring xmlFile;
  HWND mainWnd;
};

#endif
