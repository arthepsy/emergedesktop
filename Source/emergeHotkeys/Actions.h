//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#ifndef __EH_ACTIONS_H
#define __EH_ACTIONS_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include "Settings.h"
#include "resource.h"
#include <shlobj.h>
#include <stdio.h>

class Actions
{
public:
  Actions(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings);
  ~Actions();
  int Show();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  void RegisterHotkeyList(bool showError);

private:
  std::tr1::shared_ptr<Settings> pSettings;
  bool KeyCheck(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  bool DoCancel(HWND hwndDlg, WPARAM wParam);
  bool DoDelete(HWND hwndDlg);
  bool DoModify(HWND hwndDlg);
  bool DoAdd(HWND hwndDlg);
  bool DoSave(HWND hwndDlg);
  bool DoAbort(HWND hwndDlg);
  bool CheckFields(HWND hwndDlg);
  bool EnableFields(HWND hwndDlg, bool enable);
  bool PopulateList(HWND listWnd);
  void PopulateKeys(HWND keyWnd);
  void PopulateCommands(HWND commandWnd);
  bool DoInternal(HWND hwndDlg);
  bool DoExternal(HWND hwndDlg);
  bool DoBrowse(HWND hwndDlg, bool folder);
  bool ClearFields(HWND hwndDlg);
  bool PopulateFields(HWND hwndDlg, int modIndex);
  bool DoAbout();
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd;
  HICON addIcon, delIcon, editIcon, fileIcon, folderIcon, saveIcon, abortIcon;
  std::vector<HotkeyCombo *> addList;
  bool CheckSaveCount(HWND hwndDlg);
  UINT saveCount, deleteCount;
  bool edit;
  bool dialogVisible;
  UINT editIndex;
  static BOOL CALLBACK ActionsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
