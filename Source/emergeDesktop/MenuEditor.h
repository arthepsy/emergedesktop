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

#ifndef __ED_MENUEDITOR_H
#define __ED_MENUEDITOR_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include "Settings.h"
#include <shlwapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include "resource.h"
#include <map>
#include <stdio.h>

typedef struct _MENUTREEITEM
{
  WCHAR name[MAX_LINE_LENGTH];
  DWORD type;
  WCHAR value[MAX_LINE_LENGTH];
  WCHAR workingDir[MAX_PATH];
}
MENUTREEITEM;

typedef std::map<HTREEITEM, MENUTREEITEM> TreeItemMap;
typedef std::pair<HTREEITEM, MENUTREEITEM> TreeItem;

class MenuEditor
{
public:
  MenuEditor(HINSTANCE hInstance);
  ~MenuEditor();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoMenuCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoMenuNotify(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoMenuMouseMove(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoMenuTimer(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoMenuMouseLButtonUp(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoSettingsNotify(HWND hwndDlg, LPARAM lParam);
  BOOL DoSettingsChange(HWND hwndDlg, WPARAM wParam);
  bool UpdateLaunch(HWND hwndDlg);
  bool UpdateSettings(HWND hwndDlg);
  bool UpdateMenu(HWND hwndDlg);
  bool GetVisible();
  static INT_PTR CALLBACK MenuEditorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  bool CheckFields(HWND hwndDlg);
  bool DoSelChange(HWND hwndDlg, HWND typeWnd);
  bool EnableFields(HWND hwndDlg, UINT index);
  bool ShowFields(HWND hwndDlg, UINT index);
  bool PopulateList(HWND listWnd);
  bool PopulateTree(HWND treeWnd);
  bool DoLaunchAdd(HWND listWnd);
  bool DoLaunchDelete(HWND listWnd);
  bool DoLaunchBrowse(HWND listWnd);
  bool DoDelItem(HWND hwndDlg);
  bool DoEditItem(HWND hwndDlg);
  bool DoBrowseItem(HWND hwndDlg, bool workingDir);
  bool DoAddItem(HWND hwndDlg);
  bool DoUpdateFields(HWND hwndDlg);
  bool DoSaveItem(HWND hwndDlg);
  bool DoAbortItem(HWND hwndDlg);
  bool BuildMenuTree(HWND treeWnd, HTREEITEM parent, WCHAR *rootElement);
  bool BuildMenuTreeHelper (HWND treeWnd, HTREEITEM parent, TiXmlElement *menu);
  bool WriteMenu(HWND treeWnd, HTREEITEM parent, WCHAR *menu);
  bool WriteMenuHelper(TiXmlElement *section, HWND treeWnd, HTREEITEM parent);
  bool DeleteItem(HWND treeWnd, HTREEITEM item);
  int GetTypeValue(UINT type);
  void PopulateCommands(HWND commandWnd);
  void PopulateSpecialFolders(HWND specialFoldersWnd);
  void SetTooltip(HWND browseWnd, UINT type);
  void MoveSubmenu(HWND treeWnd, HTREEITEM target, HTREEITEM source);
  bool CheckSaveCount(HWND hwndDlg);
  UINT GetValueType(int value);
  HINSTANCE hInstance;
  HWND toolWnd;
  HTREEITEM rightRoot, midRoot, selected, hoverItem;
  bool dragging, edit;
  HICON addIcon, editIcon, delIcon, saveIcon, abortIcon, browseIcon, fileIcon;
  UINT saveCount, deleteCount;
  bool dialogVisible;
  std::wstring xmlFile;
};

#endif

