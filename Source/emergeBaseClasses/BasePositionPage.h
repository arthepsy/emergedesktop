/*!
  @file BasePositionPage.h
  @brief header for emergeBaseClasses
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

#ifndef __GUARD_fddfdd4d_2f6d_4a5f_8783_5c7c35ba6411
#define __GUARD_fddfdd4d_2f6d_4a5f_8783_5c7c35ba6411

#define UNICODE 1

#ifdef _MSC_VER
#pragma warning(push) //store the existing compiler warning level
#pragma warning(disable: 4251) //'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#endif

#ifdef EMERGEBASECLASSES_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#define BPP_ZORDER      0x1
#define BPP_ORIENTATION 0x2
#define BPP_HORIZONTAL  0x4
#define BPP_VERTICAL    0x8

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include "BaseSettings.h"
#include "resource.h"

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

#ifdef _MSC_VER
#pragma warning(pop) //restore the old compiler warning level
#endif

#endif
