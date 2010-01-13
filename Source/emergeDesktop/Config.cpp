//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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

BOOL CALLBACK Config::SettingsDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static Config *pConfig = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pConfig = reinterpret_cast<Config*>(lParam);
      if (!pConfig)
        break;
      return pConfig->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pConfig->DoSettingsCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pConfig->DoSettingsNotify(hwndDlg, lParam);
    }

  return FALSE;
}

Config::Config(HINSTANCE hInstance, HWND mainWnd, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;
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

Config::~Config()
{
  DestroyWindow(toolWnd);
}

int Config::Show()
{
  dialogVisible = true;
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), mainWnd, (DLGPROC)SettingsDlgProc, (LPARAM)this);
}

BOOL Config::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;

  HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);

  GetWindowRect(hwndDlg, &rect);

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

  if (pSettings->GetBorderPrimary())
    SendDlgItemMessage(hwndDlg, IDC_BORDERPRI, BM_SETCHECK, BST_CHECKED, 0);

  SendMessage(sliderWnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(20, 100));
  SendMessage(sliderWnd, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pSettings->GetMenuAlpha());

  return TRUE;
}

bool Config::GetVisible()
{
  return dialogVisible;
}

BOOL Config::DoSettingsCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      if (!UpdateSettings(hwndDlg))
        break;
    case IDCANCEL:
      dialogVisible = false;
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDC_MENUALPHA:
      return pConfig->DoSettingsChange(hwndDlg, wParam);
    }

  return FALSE;
}

bool Config::UpdateSettings(HWND hwndDlg)
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
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Invalid value for top border"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_TOPBORDER, pSettings->GetTopBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_LEFTBORDER, &success, false);
  if (success)
    pSettings->SetLeftBorder(result);
  else
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Invalid value for left border"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_LEFTBORDER, pSettings->GetLeftBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_RIGHTBORDER, &success, false);
  if (success)
    pSettings->SetRightBorder(result);
  else
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Invalid value for right border"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_RIGHTBORDER, pSettings->GetRightBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_BOTTOMBORDER, &success, false);
  if (success)
    pSettings->SetBottomBorder(result);
  else
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Invalid value for bottom border"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_BOTTOMBORDER, pSettings->GetBottomBorder(), false);
      return false;
    }

  result = GetDlgItemInt(hwndDlg, IDC_MENUALPHA, &success, false);
  if (success)
    pSettings->SetMenuAlpha(result);
  else
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Invalid value for menu alpha"), (WCHAR*)TEXT("emergeCore"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_MENUALPHA, pSettings->GetMenuAlpha(), false);
      return false;
    }

  pSettings->WriteSettings();

  return true;
}

BOOL Config::DoSettingsNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND sliderWnd = GetDlgItem(hwndDlg, IDC_SLIDER);

  NMHDR *nmhdr = (NMHDR*)lParam;

  if (nmhdr->hwndFrom == sliderWnd)
    {
      UINT sliderValue = (UINT)SendMessage(sliderWnd, TBM_GETPOS, 0, 0);
      SetDlgItemInt(hwndDlg, IDC_MENUALPHA, sliderValue, false);

      return TRUE;
    }

  return FALSE;
}

BOOL Config::DoSettingsChange(HWND hwndDlg, WPARAM wParam)
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

