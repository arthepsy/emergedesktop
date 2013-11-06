/*!
  @file BalloonPage.h
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

#ifndef __GUARD_705d58c7_c475_4f98_9c65_5f6979ec4408
#define __GUARD_705d58c7_c475_4f98_9c65_5f6979ec4408

#define UNICODE 1

#include "resource.h"
#include "Settings.h"

class BalloonPage
{
public:
  BalloonPage(std::tr1::shared_ptr<Settings> pSettings);
  ~BalloonPage();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  bool DoFontChooser(HWND hwndDlg, LOGFONT* font);
  BOOL DoColourChooser(COLORREF* colour, HWND hwndDlg);
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
