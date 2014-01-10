//---
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
//---

#include <string>
#include <algorithm>
#include <cctype>
#include "ThumbnailPage.h"
#include "Applet.h"

INT_PTR CALLBACK ThumbnailPage::ThumbnailPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam UNUSED, LPARAM lParam)
{
  static ThumbnailPage* pThumbnailPage = NULL;
  PROPSHEETPAGE* psp;

  switch (message)
  {
  case WM_INITDIALOG:
    psp = (PROPSHEETPAGE*)lParam;
    pThumbnailPage = reinterpret_cast<ThumbnailPage*>(psp->lParam);
    if (!pThumbnailPage)
    {
      break;
    }
    return pThumbnailPage->DoInitDialog(hwndDlg);

  case WM_NOTIFY:
    if (!pThumbnailPage)
    {
      break;
    }
    return pThumbnailPage->DoNotify(hwndDlg, lParam);
  }

  return FALSE;
}

ThumbnailPage::ThumbnailPage(std::tr1::shared_ptr<Settings> pSettings)
{
  this->pSettings = pSettings;
}

ThumbnailPage::~ThumbnailPage()
{
}

BOOL ThumbnailPage::DoInitDialog(HWND hwndDlg)
{
  HWND alphaSliderWnd = GetDlgItem(hwndDlg, IDC_ALPHASLIDER);
  HWND scaleSliderWnd = GetDlgItem(hwndDlg, IDC_SCALESLIDER);
  HWND scaleValueWnd = GetDlgItem(hwndDlg, IDC_SCALEVALUE);
  HWND alphaValueWnd = GetDlgItem(hwndDlg, IDC_ALPHAVALUE);
  HWND enableWnd = GetDlgItem(hwndDlg, IDC_ENABLETHUMBNAIL);

  if (ELOSVersionInfo() < 6.0)
  {
    EnableWindow(enableWnd, FALSE);
    EnableWindow(alphaSliderWnd, FALSE);
    EnableWindow(scaleSliderWnd, FALSE);
    EnableWindow(alphaValueWnd, FALSE);
    EnableWindow(scaleValueWnd, FALSE);
    SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_SETCHECK, BST_UNCHECKED, 0);
  }
  else if (pSettings->GetEnableThumbnails())
  {
    SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_SETCHECK, BST_CHECKED, 0);
  }

  SendMessage(alphaSliderWnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendMessage(alphaSliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pSettings->GetThumbnailAlpha());
  SetDlgItemInt(hwndDlg, IDC_ALPHAVALUE, pSettings->GetThumbnailAlpha(), false);

  SendMessage(scaleSliderWnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));
  SendMessage(scaleSliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pSettings->GetThumbnailScale());
  SetDlgItemInt(hwndDlg, IDC_SCALEVALUE, pSettings->GetThumbnailScale(), false);

  return TRUE;
}

INT_PTR ThumbnailPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND alphaSliderWnd = GetDlgItem(hwndDlg, IDC_ALPHASLIDER);
  HWND scaleSliderWnd = GetDlgItem(hwndDlg, IDC_SCALESLIDER);
  NMHDR* phdr = (NMHDR*)lParam;

  if (phdr->hwndFrom == alphaSliderWnd)
  {
    UINT alphaSliderValue = (UINT)SendMessage(alphaSliderWnd, TBM_GETPOS, 0, 0);
    SetDlgItemInt(hwndDlg, IDC_ALPHAVALUE, alphaSliderValue, false);

    return 1;
  }

  if (phdr->hwndFrom == scaleSliderWnd)
  {
    UINT scaleSliderValue = (UINT)SendMessage(scaleSliderWnd, TBM_GETPOS, 0, 0);
    SetDlgItemInt(hwndDlg, IDC_SCALEVALUE, scaleSliderValue, false);

    return 1;
  }

  switch (phdr->code)
  {
  case PSN_APPLY:
    if (UpdateSettings(hwndDlg))
    {
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
    }
    else
    {
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
    }
    return 1;

  case PSN_SETACTIVE:
    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
    return 1;

  case PSN_KILLACTIVE:
    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
    return 1;
  }

  return 0;
}

bool ThumbnailPage::UpdateSettings(HWND hwndDlg)
{
  BOOL success;
  UINT result;

  if (SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetEnableThumbnails(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_ENABLETHUMBNAIL, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetEnableThumbnails(false);
  }

  result = GetDlgItemInt(hwndDlg, IDC_ALPHAVALUE, &success, false);
  if (success)
  {
    pSettings->SetThumbnailAlpha(result);
  }
  else
  {
    ELMessageBox(hwndDlg, TEXT("Invalid value for alpha"), TEXT("emergeTasks"),
                 ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    SetDlgItemInt(hwndDlg, IDC_ALPHAVALUE, pSettings->GetThumbnailAlpha(), false);
    return false;
  }

  result = GetDlgItemInt(hwndDlg, IDC_SCALEVALUE, &success, false);
  if (success)
  {
    pSettings->SetThumbnailScale(result);
  }
  else
  {
    ELMessageBox(hwndDlg, TEXT("Invalid value for scale"), TEXT("emergeTasks"),
                 ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    SetDlgItemInt(hwndDlg, IDC_SCALEVALUE, pSettings->GetThumbnailScale(), false);
    return false;
  }

  return true;
}

