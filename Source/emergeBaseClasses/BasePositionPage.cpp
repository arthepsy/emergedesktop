//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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

#include "BasePositionPage.h"

INT_PTR CALLBACK BasePositionPage::BasePositionPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static BasePositionPage *pBasePositionPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pBasePositionPage = reinterpret_cast<BasePositionPage*>(psp->lParam);
      if (!pBasePositionPage)
        break;
      return pBasePositionPage->DoInitPage(hwndDlg);

    case WM_COMMAND:
      if (!pBasePositionPage)
        break;
      return pBasePositionPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pBasePositionPage)
        break;
      return pBasePositionPage->DoNotify(hwndDlg, lParam);
    }

  return 0;
}

BasePositionPage::BasePositionPage(std::tr1::shared_ptr<BaseSettings> pSettings, UINT flags)
{
  this->pSettings = pSettings;
  this->flags = flags;
}

BasePositionPage::~BasePositionPage()
{}

INT_PTR BasePositionPage::DoInitPage(HWND hwndDlg)
{
  int anchorIndex;
  bool orShow = false, hShow = false, vShow = true;

  HWND zpLabel = GetDlgItem(hwndDlg, IDC_STATIC6);
  HWND zpTop = GetDlgItem(hwndDlg, IDC_TOP);
  HWND zpNormal = GetDlgItem(hwndDlg, IDC_NORMAL);
  HWND zpBottom = GetDlgItem(hwndDlg, IDC_BOTTOM);
  HWND orLabel = GetDlgItem(hwndDlg, IDC_STATIC8);
  HWND orHorizontal = GetDlgItem(hwndDlg, IDC_HORIZONTAL);
  HWND orVertical = GetDlgItem(hwndDlg, IDC_VERTICAL);
  HWND hLabel = GetDlgItem(hwndDlg, IDC_STATIC9);
  HWND hRight = GetDlgItem(hwndDlg, IDC_RIGHT);
  HWND hCentre = GetDlgItem(hwndDlg, IDC_HCENTER);
  HWND hLeft = GetDlgItem(hwndDlg, IDC_LEFT);
  HWND vLabel = GetDlgItem(hwndDlg, IDC_STATIC10);
  HWND vTop = GetDlgItem(hwndDlg, IDC_DOWN);
  HWND vCentre = GetDlgItem(hwndDlg, IDC_VCENTER);
  HWND vBottom = GetDlgItem(hwndDlg, IDC_UP);
  HWND dirLabel = GetDlgItem(hwndDlg, IDC_STATIC7);


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

  if ((flags & BPP_ORIENTATION) == BPP_ORIENTATION)
    {
      if (_wcsicmp(pSettings->GetDirectionOrientation(), TEXT("vertical")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_VERTICAL, BM_SETCHECK, BST_CHECKED, 0);
      else
        SendDlgItemMessage(hwndDlg, IDC_HORIZONTAL, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    {
      ShowWindow(orLabel, SW_HIDE);
      ShowWindow(orVertical, SW_HIDE);
      ShowWindow(orHorizontal, SW_HIDE);
      orShow = false;
    }

  if ((flags & BPP_HORIZONTAL) == BPP_HORIZONTAL)
    {
      if (_wcsicmp(pSettings->GetHorizontalDirection(), TEXT("left")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_RIGHT, BM_SETCHECK, BST_CHECKED, 0);
      else if (_wcsicmp(pSettings->GetHorizontalDirection(), TEXT("center")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_HCENTER, BM_SETCHECK, BST_CHECKED, 0);
      else
        SendDlgItemMessage(hwndDlg, IDC_LEFT, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    {
      ShowWindow(hLabel, SW_HIDE);
      ShowWindow(hRight, SW_HIDE);
      ShowWindow(hCentre, SW_HIDE);
      ShowWindow(hLeft, SW_HIDE);
      hShow = false;
    }

  if ((flags & BPP_VERTICAL) == BPP_VERTICAL)
    {
      if (_wcsicmp(pSettings->GetVerticalDirection(), TEXT("up")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_UP, BM_SETCHECK, BST_CHECKED, 0);
      else if (_wcsicmp(pSettings->GetVerticalDirection(), TEXT("center")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_VCENTER, BM_SETCHECK, BST_CHECKED, 0);
      else
        SendDlgItemMessage(hwndDlg, IDC_DOWN, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    {
      ShowWindow(vLabel, SW_HIDE);
      ShowWindow(vTop, SW_HIDE);
      ShowWindow(vCentre, SW_HIDE);
      ShowWindow(vBottom, SW_HIDE);
      vShow = false;
    }

  if (!orShow && !hShow && !vShow)
    ShowWindow(dirLabel, SW_HIDE);

  if ((flags & BPP_ZORDER) == BPP_ZORDER)
    {
      if (_wcsicmp(pSettings->GetZPosition(), TEXT("top")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_TOP, BM_SETCHECK, BST_CHECKED, 0);
      else if (_wcsicmp(pSettings->GetZPosition(), TEXT("bottom")) == 0)
        SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_SETCHECK, BST_CHECKED, 0);
      else
        SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_SETCHECK, BST_CHECKED, 0);
    }
  else
    {
      ShowWindow(zpLabel, false);
      ShowWindow(zpTop, false);
      ShowWindow(zpNormal, false);
      ShowWindow(zpBottom, false);
    }

  return 1;
}

INT_PTR BasePositionPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND anchorWnd = GetDlgItem(hwndDlg, IDC_ANCHOR);

  switch (LOWORD(wParam))
    {
    case IDC_DYNAMICPOSITIONING:
      if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
        EnableWindow(anchorWnd, true);
      else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
        EnableWindow(anchorWnd, false);
      return 1;
    }

  return 0;
}

INT_PTR BasePositionPage::DoNotify(HWND hwndDlg, LPARAM lParam)
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

bool BasePositionPage::UpdateSettings(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetDynamicPositioning(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetDynamicPositioning(false);

  if ((flags & BPP_VERTICAL) == BPP_VERTICAL)
    {
      if (SendDlgItemMessage(hwndDlg, IDC_UP, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetVerticalDirection((WCHAR*)TEXT("Up"));
      if (SendDlgItemMessage(hwndDlg, IDC_VCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetVerticalDirection((WCHAR*)TEXT("Center"));
      if (SendDlgItemMessage(hwndDlg, IDC_DOWN, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetVerticalDirection((WCHAR*)TEXT("Down"));
    }

  if ((flags & BPP_ORIENTATION) == BPP_ORIENTATION)
    {
      if (SendDlgItemMessage(hwndDlg, IDC_VERTICAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetDirectionOrientation((WCHAR*)TEXT("Vertical"));
      if (SendDlgItemMessage(hwndDlg, IDC_HORIZONTAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetDirectionOrientation((WCHAR*)TEXT("Horizontal"));
    }

  if ((flags & BPP_HORIZONTAL) == BPP_HORIZONTAL)
    {
      if (SendDlgItemMessage(hwndDlg, IDC_RIGHT, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetHorizontalDirection((WCHAR*)TEXT("Left"));
      if (SendDlgItemMessage(hwndDlg, IDC_HCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetHorizontalDirection((WCHAR*)TEXT("Center"));
      if (SendDlgItemMessage(hwndDlg, IDC_LEFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetHorizontalDirection((WCHAR*)TEXT("Right"));
    }

  if ((flags & BPP_ZORDER) == BPP_ZORDER)
    {
      if (SendDlgItemMessage(hwndDlg, IDC_TOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetZPosition((WCHAR*)TEXT("Top"));
      if (SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetZPosition((WCHAR*)TEXT("Bottom"));
      if (SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pSettings->SetZPosition((WCHAR*)TEXT("Normal"));
    }

  if (GetDlgItemText(hwndDlg, IDC_ANCHOR, tmp, MAX_LINE_LENGTH) != 0)
    {
      if (wcscmp(tmp, pSettings->GetAnchorPoint()) != 0)
        pSettings->SetAnchorPoint(tmp);
    }

  return true;
}

LPCTSTR BasePositionPage::GetTemplate()
{
  return MAKEINTRESOURCE(IDD_POSITION_PAGE);
}

