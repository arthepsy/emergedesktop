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

#ifndef __ESM_BALLOONPAGE_H
#define __ESM_BALLOONPAGE_H

#include "Settings.h"
#include "resource.h"

class BalloonPage
{
public:
  BalloonPage(std::tr1::shared_ptr<Settings> pSettings);
  ~BalloonPage();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  bool DoFontChooser(HWND hwndDlg, LOGFONT *font);
  BOOL DoColourChooser(COLORREF *colour, HWND hwndDlg);
  INT_PTR DoNotify(HWND hwndDlg, LPARAM lParam);
  static INT_PTR CALLBACK BalloonPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  LOGFONT newTitleFont, newInfoFont;
  HFONT buttonTitleFont, buttonInfoFont;
  std::tr1::shared_ptr<Settings> pSettings;
  RECT colourRect;
  COLORREF ColourFrom;
  COLORREF ColourTo;
  HBITMAP hbmColourFrom;
  HBITMAP hbmColourTo;
  COLORREF ColourText;
  COLORREF ColourBorder;
  HBITMAP hbmColourText;
  HBITMAP hbmColourBorder;
  void UpdateColorButton(HWND hwndDlg, HBITMAP bitmap, COLORREF colour, int controlId);
};

#endif

