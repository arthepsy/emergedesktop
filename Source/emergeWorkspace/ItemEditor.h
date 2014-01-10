/*!
@file ItemEditor.h
@brief header for emergeWorkspace
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

#ifndef __GUARD_307CAF15_4D5A_4A4F_9805_D4BFD362CA7F
#define __GUARD_307CAF15_4D5A_4A4F_9805_D4BFD362CA7F

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#define _CRT_NON_CONFORMING_SWPRINTFS 1 //suppress warnings about old swprintf format
#endif

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <map>
#include <string>
#include "../emergeLib/emergeItemDefinitions.h"
#include "resource.h"
#include "Settings.h"

class ItemEditor
{
public:
  ItemEditor(HINSTANCE hInstance, HWND mainWnd);
  ~ItemEditor();
  int Show(TiXmlElement *section, std::wstring name, std::wstring value, UINT type, std::wstring workingDir);
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoMenuCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool GetVisible();

private:
  bool PopulateList(HWND listWnd);
  void PopulateSpecialFolders(HWND specialFoldersWnd);
  std::wstring name, value, workingDir;
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
