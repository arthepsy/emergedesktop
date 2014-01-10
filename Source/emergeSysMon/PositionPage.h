/*!
  @file PositionPage.h
  @brief header for emergeSysMon
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

#ifndef __GUARD_e792a237_0192_4d4d_83c7_f8abc58292c2
#define __GUARD_e792a237_0192_4d4d_83c7_f8abc58292c2

#define UNICODE 1

#include "resource.h"
#include "Settings.h"

class PositionPage
{
public:
  PositionPage(std::tr1::shared_ptr<Settings> pSettings);
  ~PositionPage();
  BOOL DoInitDialog(HWND hwndDlg);
  BOOL DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  void UpdateEnabledStates(HWND hwndDlg);
  static INT_PTR CALLBACK PositionPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  INT_PTR DoNotify(HWND hwndDlg, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
};

#endif
