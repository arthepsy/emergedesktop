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

#ifndef __ED_ITEMEDITOR_H
#define __ED_ITEMEDITOR_H

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

class ItemEditor
{
public:
  ItemEditor(HINSTANCE hInstance, HWND mainWnd);
  ~ItemEditor();
  int Show(TiXmlElement *section, WCHAR *name, WCHAR *value, UINT type, WCHAR *workingDir);
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoMenuCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool GetVisible();

private:
  bool PopulateList(HWND listWnd);
  void PopulateSpecialFolders(HWND specialFoldersWnd);
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  TiXmlElement *section;
  UINT type;
  HINSTANCE hInstance;
  HWND mainWnd, toolWnd;
  HICON browseIcon, fileIcon;
  bool dialogVisible;
  static BOOL CALLBACK MenuDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  bool DoSelChange(HWND hwndDlg, HWND typeWnd);
  bool EnableFields(HWND hwndDlg);
  bool ShowFields(HWND hwndDlg, int index);
  int GetTypeValue(UINT type);
  UINT GetValueType(int value);
  void SetTooltip(HWND browseWnd, UINT type);
  bool DoSaveItem(HWND hwndDlg);
  bool DoBrowseItem(HWND hwndDlg, bool workingDir);
};

#endif

