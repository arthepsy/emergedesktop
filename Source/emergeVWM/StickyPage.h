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

#ifndef __EV_STICKYPAGE_H
#define __EV_STICKYPAGE_H

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

typedef struct tagLISTVIEWSORTINFO
{
  HWND listWnd;
  SORTINFO sortInfo;
}
LISTVIEWSORTINFO, *PLISTVIEWSORTINFO;

class StickyPage
{
public:
  StickyPage(HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings);
  ~StickyPage();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  static INT_PTR CALLBACK StickyPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  static int CALLBACK ListViewCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
  std::tr1::shared_ptr<Settings> pSettings;
  bool CheckFields(HWND hwndDlg);
  bool PopulateList(HWND listWnd);
  bool DoAdd(HWND listWnd);
  bool DoEdit(HWND listWnd);
  bool DoDelete(HWND listWnd);
  bool DoBrowse(HWND listWnd);
  bool DoSave(HWND listWnd);
  bool DoAbort(HWND listWnd);
  bool CheckSaveCount(HWND hwndDlg);
  UINT saveCount, deleteCount;
  HINSTANCE hInstance;
  HWND toolWnd;
  bool edit, toggleSort[1];
  HICON addIcon, delIcon, fileIcon, saveIcon, abortIcon, editIcon;
  LISTVIEWSORTINFO lvSortInfo;
  WCHAR myName[MAX_LINE_LENGTH];
};

#endif

