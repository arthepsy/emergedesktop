/*!
  @file ConfigPage.h
  @brief header for emergeLauncher
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

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

#ifndef __GUARD_6c9ae101_f3c1_4f3e_b631_dbaeb1c93eba
#define __GUARD_6c9ae101_f3c1_4f3e_b631_dbaeb1c93eba

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE	0x600

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x501

#define BROWSE_COMMAND      1
#define BROWSE_WORKINGDIR   2

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include "../emergeLib/emergeOSLib.h"
#include "resource.h"
#include "Settings.h"

class ConfigPage
{
public:
  ConfigPage(std::tr1::shared_ptr<Settings> pSettings);
  ~ConfigPage();
  bool DoInitDialog(HWND hwndDlg);
  bool DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  bool DoNotify(HWND hwndDlg, LPARAM lParam);
  bool DoFontChooser(HWND hwndDlg);
  bool UpdateSettings(HWND hwndDlg);
  static INT_PTR CALLBACK ConfigPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
  LOGFONT newFont;
  HFONT buttonFont;
};

#endif
