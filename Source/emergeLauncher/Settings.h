/*!
  @file Settings.h
  @brief header for emergeLauncher
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

#ifndef __GUARD_5c943251_6a1e_4a83_8ae4_0c03b0225c95
#define __GUARD_5c943251_6a1e_4a83_8ae4_0c03b0225c95

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <stdio.h>
#include <string>
#include "../emergeBaseClasses/BaseSettings.h"
#include "Item.h"

class Settings: public BaseSettings
{
public:
  Settings();
  ~Settings();
  UINT GetItemListSize();
  Item* GetItem(size_t index);
  void PopulateItems();
  void DeleteItems(bool clearXML);
  void WriteItem(int type, WCHAR* command, WCHAR* iconPath, WCHAR* tip, WCHAR* workingDir);

protected:
  void ResetDefaults();

private:
  void writeEntireFolder(WCHAR* folderName);
  void loadLiveFolder(WCHAR* folderName);
  std::vector< std::tr1::shared_ptr<Item> > itemList;
  CRITICAL_SECTION itemListCS;
};

#endif
