/*!
  @file IconHidePage.h
  @brief header for emergeTray
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

#ifndef __GUARD_4511a4aa_a7c1_4977_b28b_e0186984c00b
#define __GUARD_4511a4aa_a7c1_4977_b28b_e0186984c00b

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE	0x600

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <shlwapi.h>
#include "resource.h"
#include "Settings.h"

typedef struct tagLISTVIEWSORTINFO
{
  HWND listWnd;
  SORTINFO sortInfo;
}
LISTVIEWSORTINFO, *PLISTVIEWSORTINFO;

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
  static int CALLBACK ListViewCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

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
  bool edit, toggleSort[1];
  LISTVIEWSORTINFO lvSortInfo;
  WCHAR myName[MAX_LINE_LENGTH];
};

#endif
