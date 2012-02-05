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

#ifndef __EB_BASEPOSITIONPAGE_H
#define __EB_BASEPOSITIONPAGE_H

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include "BaseSettings.h"
#include "resource.h"

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#define BPP_ZORDER      0x1
#define BPP_ORIENTATION 0x2
#define BPP_HORIZONTAL  0x4
#define BPP_VERTICAL    0x8

class DLL_EXPORT BasePositionPage
{
public:
  BasePositionPage(std::tr1::shared_ptr<BaseSettings> pSettings, UINT flags);
  ~BasePositionPage();
  INT_PTR DoInitPage(HWND hwndDlg);
  INT_PTR DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
  INT_PTR DoNotify(HWND hwndDlg, LPARAM lParam);
  bool UpdateSettings(HWND hwndDlg);
  static INT_PTR CALLBACK BasePositionPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
  LPCTSTR GetTemplate();

private:
  UINT flags;
  std::tr1::shared_ptr<BaseSettings> pSettings;
};

#endif

