/*!
  @file AliasEditor.h
  @brief header for emergeCore
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

#ifndef __GUARD_2fd29158_48f0_4b0e_9836_03c7c4b8baa1
#define __GUARD_2fd29158_48f0_4b0e_9836_03c7c4b8baa1

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#define HOTKEY_N  1
#define HOTKEY_E  2

#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <psapi.h>
#include <shlwapi.h>
#include <stdio.h>
#include <map>
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeWindowLib.h"
#include "resource.h"
#include "settings.h"

typedef struct tagLISTVIEWSORTINFO
{
  HWND listWnd;
  SORTINFO sortInfo;
}
LISTVIEWSORTINFO, *PLISTVIEWSORTINFO;

class AliasEditor
{
public:
  AliasEditor(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings);
  ~AliasEditor();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  BOOL DoHotkey(HWND hwndDlg, int hotkeyID);
  bool UpdateAliases(HWND hwndDlg);
  static INT_PTR CALLBACK AliasDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  static int CALLBACK ListViewCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  bool AliasCheck(HWND hwndDlg, WCHAR* alias);

private:
  bool CheckFields(HWND hwndDlg);
  bool PopulateList(HWND listWnd);
  bool DoAliasAdd(HWND hwndDlg);
  bool DoAliasSave(HWND listWnd);
  bool DoAliasAbort(HWND listWnd);
  bool DoAliasDelete(HWND hwndDlg);
  bool DoAliasBrowse(HWND listWnd);
  bool CheckSaveCount(HWND hwndDlg);
  void InsertListViewItem(HWND listWnd, int index, const WCHAR* item);
  BOOL PopulateFields(HWND hwndDlg, int index);
  UINT saveCount, deleteCount;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd, dlgWnd;
  HICON addIcon, delIcon, browseIcon, saveIcon, abortIcon, editIcon;
  bool edit, toggleSort[2];
  bool FindListSubItem(HWND listWnd, int subItem, WCHAR* searchString);
  std::tr1::shared_ptr<Settings> pSettings;
  LISTVIEWSORTINFO lvSortInfo;
  WCHAR myName[MAX_LINE_LENGTH];
  static LRESULT CALLBACK AliasProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK AppletProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
