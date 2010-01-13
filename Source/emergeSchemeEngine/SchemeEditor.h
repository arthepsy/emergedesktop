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

#ifndef __SCHEMEEDITOR_H
#define __SCHEMEEDITOR_H

#undef _WIN32_IE
#define _WIN32_IE	0x501

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#include "emergeSchemeEngine.h"
#include "resource.h"
#include "../emergeLib/emergeLib.h"
#include <stdio.h>
#include <wchar.h>
#include <shlwapi.h>
#include <map>
#include <set>

#ifdef EMERGESCHEMEENGINE_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

typedef std::set<HWND> PanelSet;

class DLL_EXPORT SchemeEditor
{
public:
  SchemeEditor(HWND mainWnd);
  ~SchemeEditor();
  int Edit(WCHAR *schemeName);
  BOOL DoInitDialog(HWND hwndDlg, bool updatePos);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  BOOL DoNotify(HWND hwndDlg, LPARAM lParam);
  BOOL DoColourChooser(COLORREF *colour, HWND hwndDlg);
  BOOL DoLoad(HWND hwndDlg);
  BOOL DoSwitch(HWND hwndDlg);
  void DoSaveAs(HWND hwndDlg);
  static INT_PTR CALLBACK SchemeEditorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  LPCTSTR GetTemplate();

private:
  std::map< HTREEITEM, std::tr1::shared_ptr<PanelSet> > panelMap;
  WCHAR tmpFile[MAX_PATH];
  RECT colourRect;
  COLORREF colourBackground, colourForeground, colourSelected, colourFrame, colourFont;
  COLORREF colourFrom, colourTo, colourBorder;
  HWND mainWnd, toolWnd;
  GUIINFO guiInfo, origGuiInfo, defaultGuiInfo;
  WCHAR scheme[MAX_PATH], font[MAX_PATH], origScheme[MAX_PATH];
  HBITMAP hbmColourBackground, hbmColourForeground, hbmColourSelected, hbmColourFrame, hbmColourFont;
  HBITMAP hbmColourFrom, hbmColourTo, hbmColourBorder;
  bool DoSaveScheme(HWND hwndDlg, WCHAR *fileName);
  BOOL DoDefaults(HWND hwndDlg);
  void BuildPanelMap(HWND hwndDlg);
  void ClearPanelMap();
  void ShowPanel(HTREEITEM panel);
  HTREEITEM hitemOpacity, hitemColor, hitemGradient, hitemMisc;
};

#endif

