//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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

#include "PositionPage.h"
#include "Applet.h"

INT_PTR CALLBACK PositionPage::PositionPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static PositionPage *pPositionPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pPositionPage = reinterpret_cast<PositionPage*>(psp->lParam);
      if (!pPositionPage)
        break;
      return pPositionPage->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pPositionPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pPositionPage)
        break;
      return pPositionPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

PositionPage::PositionPage(std::tr1::shared_ptr<Settings> pSettings)
{
  this->pSettings = pSettings;
}

PositionPage::~PositionPage()
{
}

BOOL PositionPage::DoInitDialog(HWND hwndDlg)
{
  int anchorIndex;

  if (pSettings->GetDynamicPositioning())
    SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_SETCHECK, BST_CHECKED, 0);

  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("TopLeft"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("TopMiddle"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("TopRight"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("CenterLeft"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("CenterMiddle"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("CenterRight"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("BottomLeft"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("BottomMiddle"));
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_ADDSTRING, 0, (LPARAM)TEXT("BottomRight"));

  anchorIndex = (int)SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                        (LPARAM)pSettings->GetAnchorPoint());
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_SETCURSEL, anchorIndex, 0);

  if (_wcsicmp(pSettings->GetDirectionOrientation(), TEXT("vertical")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_VERTICAL, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_HORIZONTAL, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetHorizontalAlign(), TEXT("right")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_RIGHT, BM_SETCHECK, BST_CHECKED, 0);
  else if (_wcsicmp(pSettings->GetHorizontalAlign(), TEXT("center")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_HCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_LEFT, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetVerticalAlign(), TEXT("bottom")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_UP, BM_SETCHECK, BST_CHECKED, 0);
  else if (_wcsicmp(pSettings->GetVerticalAlign(), TEXT("center")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_VCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_DOWN, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetZPosition(), TEXT("top")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_TOP, BM_SETCHECK, BST_CHECKED, 0);
  else if (_wcsicmp(pSettings->GetZPosition(), TEXT("bottom")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_SETCHECK, BST_CHECKED, 0);

  return TRUE;
}

BOOL PositionPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND anchorWnd = GetDlgItem(hwndDlg, IDC_ANCHOR);

  switch (LOWORD(wParam))
    {
    case IDC_DYNAMICPOSITIONING:
      if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
        EnableWindow(anchorWnd, true);
      else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
        EnableWindow(anchorWnd, false);
      return TRUE;
    }

  return FALSE;
}

INT_PTR PositionPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  NMHDR *phdr = (NMHDR*)lParam;

  switch (phdr->code)
    {
    case PSN_APPLY:
      if (UpdateSettings(hwndDlg))
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
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

bool PositionPage::UpdateSettings(HWND hwndDlg)
{
  BOOL success = false;
  const WCHAR *tmpValue;
  WCHAR tmp[MAX_LINE_LENGTH];

  if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetDynamicPositioning(success);

  if (GetDlgItemText(hwndDlg, IDC_ANCHOR, tmp, MAX_LINE_LENGTH) != 0)
    {
      if (wcscmp(tmp, pSettings->GetAnchorPoint()) != 0)
        pSettings->SetAnchorPoint(tmp);
    }

  if (SendDlgItemMessage(hwndDlg, IDC_UP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Up");
  if (SendDlgItemMessage(hwndDlg, IDC_VCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_DOWN, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Down");
  pSettings->SetVerticalAlign((WCHAR*)tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_VERTICAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Vertical");
  if (SendDlgItemMessage(hwndDlg, IDC_HORIZONTAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Horizontal");
  pSettings->SetDirectionOrientation((WCHAR*)tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_RIGHT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Right");
  if (SendDlgItemMessage(hwndDlg, IDC_HCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_LEFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Left");
  pSettings->SetHorizontalAlign((WCHAR*)tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_TOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Top");
  if (SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Bottom");
  if (SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Normal");
  pSettings->SetZPosition((WCHAR*)tmpValue);

  // commit changes
  pSettings->WriteSettings();

  return true;
}

