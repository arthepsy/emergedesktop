//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2009  The Emerge Desktop Development Team
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
#include "Applet.h"

extern Config *pConfig;
extern Settings *pSettings;

BOOL CALLBACK Config::ConfigDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
      return pConfig->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pConfig->DoCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

Config::Config(HINSTANCE hInstance, HWND mainWnd)
{
  (*this).hInstance = hInstance;
  (*this).mainWnd = mainWnd;
}

Config::~Config()
{
  if (buttonFont)
    DeleteObject(buttonFont);
}

int Config::Show()
{
  return (int)DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONFIG), mainWnd, (DLGPROC)ConfigDlgProc);
}

BOOL Config::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  ELGetWindowRect(hwndDlg, &rect);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  if (pSettings->GetAutoSize())
    SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetSnapMove())
    SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetSnapSize())
    SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_SETCHECK, BST_CHECKED, 0);

  SetDlgItemInt(hwndDlg, IDC_UPDATEINTERVAL, pSettings->GetUpdateInterval(), false);
  SendDlgItemMessage(hwndDlg, IDC_UPDATEINTERVALUPDOWN, UDM_SETRANGE, (WPARAM)200, (LPARAM)3000);

  if (pSettings->GetClickThrough() != 0)
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetDynamicPositioning())
    SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetDirectionOrientation(), TEXT("vertical")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_VERTICAL, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_HORIZONTAL, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetHorizontalDirection(), TEXT("left")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_LEFT, BM_SETCHECK, BST_CHECKED, 0);
  else if (_wcsicmp(pSettings->GetHorizontalDirection(), TEXT("center")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_HCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_RIGHT, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetVerticalDirection(), TEXT("up")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_UP, BM_SETCHECK, BST_CHECKED, 0);
  else if (_wcsicmp(pSettings->GetVerticalDirection(), TEXT("center")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_VCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_DOWN, BM_SETCHECK, BST_CHECKED, 0);

  if (_wcsicmp(pSettings->GetZPosition(), TEXT("top")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_TOP, BM_SETCHECK, BST_CHECKED, 0);
  else if (_wcsicmp(pSettings->GetZPosition(), TEXT("bottom")) == 0)
    SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_SETCHECK, BST_CHECKED, 0);

  SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("16x16"));
  SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("32x32"));

  if (pSettings->GetIconSize() == 32)
    SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_SETCURSEL, (WPARAM)1, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_SETCURSEL, (WPARAM)0, 0);

  SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);

  if (buttonFont)
    DeleteObject(buttonFont);
  buttonFont = CreateFontIndirect(pSettings->GetFont());
  SendDlgItemMessage(hwndDlg, IDC_FONT, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
  SetDlgItemText(hwndDlg, IDC_FONT, pSettings->GetFont()->lfFaceName);

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

BOOL Config::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  switch (LOWORD(wParam))
    {
    case IDOK:
      if (!UpdateSettings(hwndDlg))
        break;
    case IDCANCEL:
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDC_FONT:
      if (DoFontChooser(hwndDlg))
        {
          if (buttonFont)
            DeleteObject(buttonFont);
          buttonFont = CreateFontIndirect(pSettings->GetFont());
          SendDlgItemMessage(hwndDlg, IDC_FONT, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
          SetDlgItemText(hwndDlg, IDC_FONT, pSettings->GetFont()->lfFaceName);

          return TRUE;
        }

      return FALSE;
    case IDC_CLICKTHROUGH:
      if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
        EnableWindow(clickThroughWnd, true);
      else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
        EnableWindow(clickThroughWnd, false);
      return TRUE;
    }

  return FALSE;
}

bool Config::DoFontChooser(HWND hwndDlg)
{
  CHOOSEFONT chooseFont;
  BOOL res;

  ZeroMemory(&chooseFont, sizeof(CHOOSEFONT));
  chooseFont.lStructSize = sizeof(CHOOSEFONT);
  chooseFont.hwndOwner = hwndDlg;
  chooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_NOSCRIPTSEL;
  chooseFont.lpLogFont = pSettings->GetFont();

  res = ChooseFont(&chooseFont);

  if (res)
    pSettings->SetFont(chooseFont.lpLogFont);

  return (res == TRUE);
}

bool Config::UpdateSettings(HWND hwndDlg)
{
  BOOL success;
  int result, index;
  const WCHAR *tmpValue = NULL;

  if (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetAutoSize(success);

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetSnapMove(success);

  if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetDynamicPositioning(success);

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    success = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    success = false;
  pSettings->SetSnapSize(success);

  if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      index = (int)SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_GETCURSEL, 0, 0);
      index++;
      pSettings->SetClickThrough(index);
    }
  else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetClickThrough(0);

  result = GetDlgItemInt(hwndDlg, IDC_UPDATEINTERVAL, &success, false);
  if (success)
    pSettings->SetUpdateInterval(result);
  else if (!success)
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid value for update interval"),
                   (WCHAR*)TEXT("emergeMediaCtrl"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      SetDlgItemInt(hwndDlg, IDC_UPDATEINTERVAL, pSettings->GetUpdateInterval(), false);
      return false;
    }

  if (SendDlgItemMessage(hwndDlg, IDC_UP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Up");
  if (SendDlgItemMessage(hwndDlg, IDC_VCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_DOWN, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Down");
  pSettings->SetVerticalDirection((WCHAR*)tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_VERTICAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Vertical");
  if (SendDlgItemMessage(hwndDlg, IDC_HORIZONTAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Horizontal");
  pSettings->SetDirectionOrientation((WCHAR*)tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_RIGHT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Left");
  if (SendDlgItemMessage(hwndDlg, IDC_HCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_LEFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Right");
  pSettings->SetHorizontalDirection((WCHAR*)tmpValue);

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

