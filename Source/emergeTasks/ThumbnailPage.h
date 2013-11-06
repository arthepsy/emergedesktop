/*!
  @file ThumbnailPage.h
  @brief header for emergeTasks
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

#ifndef __GUARD_a4892ab5_88a9_46b2_acf6_d20be73a1b6f
#define __GUARD_a4892ab5_88a9_46b2_acf6_d20be73a1b6f

#define UNICODE 1

#include "resource.h"
#include "Settings.h"

class ThumbnailPage
{
public:
  ThumbnailPage(std::tr1::shared_ptr<Settings> pSettings);
  ~ThumbnailPage();
  BOOL DoInitDialog(HWND hwndDlg);
  bool UpdateSettings(HWND hwndDlg);
  INT_PTR DoNotify(HWND hwndDlg, LPARAM lParam);
  static INT_PTR CALLBACK ThumbnailPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
};

#endif
