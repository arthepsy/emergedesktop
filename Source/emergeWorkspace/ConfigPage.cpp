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

#include "Config.h"

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
      return pConfigPage->DoSettingsCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pConfigPage->DoSettingsNotify(hwndDlg, lParam);
    }

  return FALSE;
}

ConfigPage::ConfigPage(HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  dialogVisible = false;
  this->pSettings = pSettings;

  InitCommonControls();

  toolWnd = CreateWindowEx(
              0,
              TOOLTIPS_CLASS,
              NULL,
              TTS_ALWAYSTIP|WS_POPUP|TTS_NOPREFIX,
              CW_USEDEFAULT, CW_USEDEFAULT,
              CW_USEDEFAULT, CW_USEDEFAULT,
              NULL,
              NULL,
              hInstance,
              NULL);

  if (toolWnd)
    {
      SendMessage(toolWnd, TTM_SETMAXTIPWIDTH, 0, 300);
      SetWindowPos(toolWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |
                   SWP_NOACTIVATE);
    }
}

ConfigPage::~ConfigPage()
{
  DestroyWindow(toolWnd);
}

BOOL ConfigPage::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;

  HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);
  HWND aeroMenuWnd = GetDlgItem(hwndDlg, IDC_AEROMENUS);

  rect = ELGetWindowRect(hwndDlg);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  SetDlgItemInt(hwndDlg, IDC_TOPBORDER, pSettings->GetTopBorder(), false);
  SetDlgItemInt(hwndDlg, IDC_LEFTBORDER, pSettings->GetLeftBorder(), false);
  SetDlgItemInt(hwndDlg, IDC_RIGHTBORDER, pSettings->GetRightBorder(), false);
  SetDlgItemInt(hwndDlg, IDC_BOTTOMBORDER, pSettings->GetBottomBorder(), false);

  SendDlgItemMessage(hwndDlg, IDC_TOPUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)1000);
  SendDlgItemMessage(hwndDlg, IDC_LEFTUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)1000);
  SendDlgItemMessage(hwndDlg, IDC_RIGHTUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)1000);
  SendDlgItemMessage(hwndDlg, IDC_BOTTOMUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)1000);

  SetDlgItemInt(hwndDlg, IDC_MENUALPHA, pSettings->GetMenuAlpha(), false);

  if (pSettings->GetMenuIcons())
    SendDlgItemMessage(hwndDlg, IDC_MENUICONS, BM_SETCHECK, BST_CHECKED, 0);

  if (!IsWindowsVistaOrGreater()) //Aero is only available in Windows Vista on
    {
      EnableWindow(aeroMenuWnd, FALSE);
      SendMessage(aeroMenuWnd, BM_SETCHECK, BST_UNCHECKED, 0);
    }
  else if (pSettings->GetAeroMenus())
    SendDlgItemMessage(hwndDlg, IDC_AEROMENUS, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetBorderPrimary())
    SendDlgItemMessage(hwndDlg, IDC_BORDERPRI, BM_SETCHECK, BST_CHECKED, 0);

  SendMessage(sliderWnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(20, 100));
  SendMessage(sliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pSettings->GetMenuAlpha());

  return TRUE;
}

bool ConfigPage::GetVisible()
{
  return dialogVisible;
}

BOOL ConfigPage::DoSettingsCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDC_MENUALPHA:
      return DoSettingsChange(hwndDlg, wParam);
    }

  return FALSE;
}

bool ConfigPage::UpdateSettings(HWND hwndDlg)
{
  UINT result;
  BOOL success;

  if (SendDlgItemMessage(hwndDlg, IDC_MENUICONS, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      if (!pSettings->GetMenuIcons())
        pSettings->SetMenuIcons(true);
    }
  else if (SendDlgItemMessage(hwndDlg, IDC_MENUICONS, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
      if (pSettings->GetMenuIcons())
        pSettings->SetMenuIcons(false);
    }

  if (SendDlgItemMessage(hwndDlg, IDC_AEROMENUS, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      if (!pSettings->GetAeroMenus())
        pSettings->SetAeroMenus(true);
    }
  else if (SendDlgItemMessage(hwndDlg, IDC_AEROMENUS, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
      if (pSettings->GetAeroMenus())
        pSettings->SetAeroMenus(false);
    }

  if (SendDlgItemMessage(hwndDlg, IDC_BORDERPRI, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      if (!pSettings->GetBorderPrimary())
        pSettings->SetBorderPrimary(true);
    }
  else if (SendDlgItemMessage(hwndDlg, IDC_BORDERPRI, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
      if (pSettings->GetBorderPrimary())
        pSettings->SetBorderPrimary(false);
    }

  result = GetDlgItemInt(hwndDlg, IDC_TOPBORDER, &success, false);
  if (success)
    pSettings->SetTopBorder(result);
  else
    {
      ELMessageBox(hwndDlg, TEXT("Invalid value for top border"), TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_TOPBORDER, pSettings->GetTopBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_LEFTBORDER, &success, false);
  if (success)
    pSettings->SetLeftBorder(result);
  else
    {
      ELMessageBox(hwndDlg, TEXT("Invalid value for left border"), TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_LEFTBORDER, pSettings->GetLeftBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_RIGHTBORDER, &success, false);
  if (success)
    pSettings->SetRightBorder(result);
  else
    {
      ELMessageBox(hwndDlg, TEXT("Invalid value for right border"), TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_RIGHTBORDER, pSettings->GetRightBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_BOTTOMBORDER, &success, false);
  if (success)
    pSettings->SetBottomBorder(result);
  else
    {
      ELMessageBox(hwndDlg, TEXT("Invalid value for bottom border"), TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_BOTTOMBORDER, pSettings->GetBottomBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_MENUALPHA, &success, false);
  if (success)
    pSettings->SetMenuAlpha(result);
  else
    {
      ELMessageBox(hwndDlg, TEXT("Invalid value for menu alpha"), TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_MENUALPHA, pSettings->GetMenuAlpha(), false);
      return false;
    }

  pSettings->WriteSettings();

  return true;
}

BOOL ConfigPage::DoSettingsNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);
  NMHDR *phdr = (NMHDR*)lParam;

  if (phdr->hwndFrom == sliderWnd)
    {
      UINT sliderValue = (UINT)SendMessage(sliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_MENUALPHA, sliderValue, false);

      return 1;
    }

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

BOOL ConfigPage::DoSettingsChange(HWND hwndDlg, WPARAM wParam)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);
  HWND alphaWnd = GetDlgItem(hwndDlg, IDC_MENUALPHA);

  if (HIWORD(wParam) == EN_CHANGE)
    {
      int tmpLength = GetWindowText(alphaWnd, tmp, MAX_LINE_LENGTH);
      if (tmpLength > 1)
        {
          SendMessage(sliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)_wtoi(tmp));
          SendMessage(alphaWnd, EM_SETSEL, (WPARAM)tmpLength, (LPARAM)tmpLength);

          return TRUE;
        }
    }

  if (HIWORD(wParam) == EN_KILLFOCUS)
    {
      GetWindowText(alphaWnd, tmp, MAX_LINE_LENGTH);
      SendMessage(sliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)_wtoi(tmp));

      return TRUE;
    }

  return FALSE;
}
