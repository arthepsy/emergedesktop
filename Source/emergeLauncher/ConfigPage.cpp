// vim: tags+=../emergeLib/tags
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

#include "ConfigPage.h"

INT_PTR CALLBACK ConfigPage::ConfigPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ConfigPage *pConfigPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pConfigPage = reinterpret_cast<ConfigPage*>(psp->lParam);
      if (!pConfigPage)
        break;
      return pConfigPage->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pConfigPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pConfigPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

ConfigPage::ConfigPage(std::tr1::shared_ptr<Settings> pSettings)
{
  this->pSettings = pSettings;
}

ConfigPage::~ConfigPage()
{
}

BOOL ConfigPage::DoInitDialog(HWND hwndDlg)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  HWND iconSizeWnd = GetDlgItem(hwndDlg, IDC_ICONSIZE);
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  SendDlgItemMessage(hwndDlg, IDC_ICONSPACINGUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);

  if (pSettings->GetSnapMove())
    SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetSnapSize())
    SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetAutoSize())
    SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetClickThrough())
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_SETCHECK, BST_CHECKED, 0);

  SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);

  SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("16x16"));
  SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("32x32"));
  SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("48x48"));

  if (pSettings->GetIconSize() == 48)
    SendMessage(iconSizeWnd, CB_SETCURSEL, (WPARAM)2, 0);
  else if (pSettings->GetIconSize() == 32)
    SendMessage(iconSizeWnd, CB_SETCURSEL, (WPARAM)1, 0);
  else
    SendMessage(iconSizeWnd, CB_SETCURSEL, (WPARAM)0, 0);

  SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Full"));
  SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Background"));

  if (pSettings->GetClickThrough() == 0)
    EnableWindow(clickThroughWnd, false);

  if (pSettings->GetClickThrough() == 1)
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_SETCURSEL, (WPARAM)0, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_SETCURSEL, (WPARAM)1, 0);

  return TRUE;
}

BOOL ConfigPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  switch (LOWORD(wParam))
    {
    case IDC_CLICKTHROUGH:
      if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
        EnableWindow(clickThroughWnd, true);
      else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
        EnableWindow(clickThroughWnd, false);
      return TRUE;
    }

  return FALSE;
}

bool ConfigPage::UpdateSettings(HWND hwndDlg)
{
  int i, result, size;
  HWND iconSizeWnd = GetDlgItem(hwndDlg, IDC_ICONSIZE);
  BOOL success;

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetSnapMove(success);

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetSnapSize(success);

  if (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetAutoSize(success);

  if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      int index = (int)SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_GETCURSEL, 0, 0);
      index++;
      pSettings->SetClickThrough(index);
    }
  else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
      pSettings->SetClickThrough(0);
    }

  result = GetDlgItemInt(hwndDlg, IDC_ICONSPACING, &success, false);
  if (success)
    pSettings->SetIconSpacing(result);
  else if (!success)
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid value for icon spacing"),
                   (WCHAR*)TEXT("emergeLauncher"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);
      return false;
    }

  i = (int)SendMessage(iconSizeWnd, CB_GETCURSEL, 0, 0);
  if (i == 2)
    size = 48;
  else if (i == 1)
    size = 32;
  else
    size = 16;
  pSettings->SetIconSize(size);

  return true;
}

BOOL ConfigPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
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

  return FALSE;
}
