//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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
//----  --------------------------------------------------------------------------------------------------------

#include "Welcome.h"
#include <stdio.h>

extern Welcome* pWelcome;

BOOL CALLBACK Welcome::WelcomeDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Welcome* pWelcome = NULL;

  switch (message)
  {
  case WM_INITDIALOG:
    pWelcome = reinterpret_cast<Welcome*>(lParam);
    if (!pWelcome)
    {
      break;
    }
    return pWelcome->DoInitDialog(hwndDlg);

  case WM_COMMAND:
    return pWelcome->DoCommand(hwndDlg, wParam, lParam);

  case WM_NOTIFY:
    return pWelcome->DoNotify(hwndDlg, lParam);
  }

  return FALSE;
}

Welcome::Welcome(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;
  this->pSettings = pSettings;

  pForumLink = std::tr1::shared_ptr<CHyperLink>(new CHyperLink());
  pWikiLink = std::tr1::shared_ptr<CHyperLink>(new CHyperLink());
  pTutorialLink = std::tr1::shared_ptr<CHyperLink>(new CHyperLink());
  pOfflineLink = std::tr1::shared_ptr<CHyperLink>(new CHyperLink());

  hIconsDLL = ELLoadEmergeLibrary(TEXT("emergeIcons.dll"));

  logoBMP = LoadBitmap((HINSTANCE)hIconsDLL, MAKEINTRESOURCE(IDB_LOGO));
}

Welcome::~Welcome()
{
  if (logoBMP)
  {
    DeleteObject(logoBMP);
  }

  FreeLibrary(hIconsDLL);
}

int Welcome::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_WELCOME), mainWnd, (DLGPROC)WelcomeDlgProc, (LPARAM)this);
}

BOOL Welcome::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;

  rect = ELGetWindowRect(hwndDlg);
  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  if (logoBMP)
  {
    SendDlgItemMessage(hwndDlg, IDC_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)logoBMP);
  }

  if (pSettings->GetShowWelcome())
  {
    SendDlgItemMessage(hwndDlg, IDC_SHOWWELCOME, BM_SETCHECK, BST_CHECKED, 0);
  }

  pForumLink->ConvertStaticToHyperlink(hwndDlg,
                                       IDC_FORUMLINK,
                                       (WCHAR*)TEXT("Homepage"));
  /*pWikiLink->ConvertStaticToHyperlink(hwndDlg,
                                      IDC_WIKILINK,
                                      (WCHAR*)TEXT("http://ed.xaerolimit.net/wiki/"));*/
  pTutorialLink->ConvertStaticToHyperlink(hwndDlg,
                                          IDC_TUTORIALLINK,
                                          (WCHAR*)TEXT("Tutorial"));
  pOfflineLink->ConvertStaticToHyperlink(hwndDlg,
                                         IDC_HELPLINK,
                                         (WCHAR*)TEXT("Help"));

  return TRUE;
}

BOOL Welcome::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
  {
  case IDOK:
    if (UpdateSettings(hwndDlg))
    {
      EndDialog(hwndDlg, wParam);
    }
    return TRUE;
  }

  return FALSE;
}

BOOL Welcome::DoNotify(HWND hwndDlg UNUSED, LPARAM lParam UNUSED)
{
  return FALSE;
}

bool Welcome::UpdateSettings(HWND hwndDlg)
{
  pSettings->SetShowWelcome(SendDlgItemMessage(hwndDlg, IDC_SHOWWELCOME,
                            BM_GETCHECK, 0, 0) == BST_CHECKED);
  pSettings->WriteUserSettings();
  return true;
}
