//---
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
//---

#ifndef __ESM_DISPLAYPAGE_H
#define __ESM_DISPLAYPAGE_H

#include "Settings.h"
#include "resource.h"

class DisplayPage
{
public:
  DisplayPage(std::tr1::shared_ptr<Settings> pSettings);
  ~DisplayPage();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  bool DoFontChooser(HWND hwndDlg);
  BOOL DoColourChooser(COLORREF *colour, HWND hwndDlg);
  void UpdateEnabledStates(HWND hwndDlg);
  INT_PTR DoNotify(HWND hwndDlg, LPARAM lParam);
  static INT_PTR CALLBACK DisplayPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  LOGFONT newFont;
  HFONT buttonFont;
  std::tr1::shared_ptr<Settings> pSettings;
  RECT colourRect;
  COLORREF CPUColourFrom;
  COLORREF CPUColourTo;
  COLORREF MemColourFrom;
  COLORREF MemColourTo;
  HBITMAP hbmCPUColourFrom;
  HBITMAP hbmCPUColourTo;
  HBITMAP hbmMemColourFrom;
  HBITMAP hbmMemColourTo;
  void UpdateColorButton(HWND hwndDlg, HBITMAP bitmap, COLORREF colour, int controlId);
};

#endif

