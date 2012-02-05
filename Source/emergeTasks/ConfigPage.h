//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2012  The Emerge Desktop Development Team
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

#ifndef __ET_CONFIGPAGE_H
#define __ET_CONFIGPAGE_H

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include "Settings.h"
#include "resource.h"

class ConfigPage
{
public:
  ConfigPage(std::tr1::shared_ptr<Settings> pSettings);
  ~ConfigPage();
  INT_PTR DoInitPage(HWND hwndDlg);
  INT_PTR DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  INT_PTR DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  static INT_PTR CALLBACK ConfigPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  std::tr1::shared_ptr<Settings> pSettings;
};

#endif

