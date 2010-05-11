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

#ifndef __ETR_ICONHIDEPAGE_H
#define __ETR_ICONHIDEPAGE_H

#undef _WIN32_IE
#define _WIN32_IE	0x600

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include "Settings.h"
#include <shlwapi.h>
#include "resource.h"

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

class IconHidePage
{
public:
  IconHidePage(HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings);
  ~IconHidePage();
  BOOL DoInitPage(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  static INT_PTR CALLBACK IconHidePageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  bool CheckSaveCount(HWND hwndDlg);
  std::tr1::shared_ptr<Settings> pSettings;
  HINSTANCE hInstance;
  bool CheckFields(HWND hwndDlg);
  bool PopulateList(HWND listWnd);
  HWND toolWnd;
  UINT saveCount, deleteCount;
  bool DoAdd(HWND listWnd);
  bool DoDelete(HWND listWnd);
  bool DoEdit(HWND listWnd);
  bool DoBrowse(HWND listWnd);
  bool DoSave(HWND listWnd);
  bool DoAbort(HWND listWnd);
  HICON addIcon, editIcon, delIcon, fileIcon, saveIcon, abortIcon;
  bool edit;
};

#endif

