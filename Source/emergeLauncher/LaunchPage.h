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

#ifndef __EL_LAUNCHPAGE_H
#define __EL_LAUNCHPAGE_H

#undef _WIN32_IE
#define _WIN32_IE	0x600

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#define BROWSE_COMMAND      1
#define BROWSE_WORKINGDIR   2
#define BROWSE_ENTIREDIR    3

#include "Settings.h"
#include "resource.h"

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

class LaunchPage
{
public:
  LaunchPage(HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings);
  ~LaunchPage();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  void PopulateList(HWND listWnd);
  bool DelItem(HWND hwndDlg);
  bool MoveItem(HWND hwndDlg, bool up);
  bool EnableFields(HWND hwndDlg, bool enable);
  bool SaveItem(HWND hwndDlg);
  bool AbortItem(HWND hwndDlg);
  bool Browse(HWND hwndDlg, UINT type);
  bool CheckFields(HWND hwndDlg);
  bool CheckSaveCount(HWND hwndDlg);
  bool DoExeCom(HWND hwndDlg, bool exeButton);
  void PopulateComboBoxes(HWND internalWnd);
  bool DoAdd(HWND hwndDlg);
  bool DoEdit(HWND hwndDlg);
  BOOL PopulateFields(HWND hwndDlg, int itemIndex);
  static INT_PTR CALLBACK LaunchPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
  BOOL GetIcon(HWND hwndDlg);
  BOOL ToggleFields(HWND hwndDlg);
  BOOL ToggleIconFields(HWND hwndDlg);
  HINSTANCE hInstance;
  HWND toolWnd;
  HICON addIcon, editIcon, delIcon, upIcon, downIcon, saveIcon, abortIcon, browseIcon, fileIcon;
  bool edit, itemMoved;
  UINT saveCount, deleteCount;
};

#endif

