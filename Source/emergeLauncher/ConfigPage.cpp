// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
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

bool ConfigPage::DoInitDialog(HWND hwndDlg)
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

  if (pSettings->GetStartHidden())
    SendDlgItemMessage(hwndDlg, IDC_STARTHIDDEN, BM_SETCHECK, BST_CHECKED, 0);

  SetDlgItemText(hwndDlg, IDC_TITLEBARTEXT, pSettings->GetTitleBarText());

  //CopyMemory(&newFont, pSettings->GetFont(), sizeof(LOGFONT));
  CopyMemory(&newFont, pSettings->GetTitleBarFont(), sizeof(LOGFONT));

  if (buttonFont)
    DeleteObject(buttonFont);
  buttonFont = CreateFontIndirect(&newFont);
  SendDlgItemMessage(hwndDlg, IDC_FONTBUTTON, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
  SetDlgItemText(hwndDlg, IDC_FONTBUTTON, newFont.lfFaceName);

  SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);

  SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("16x16"));
  SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("32x32"));
  SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("48x48"));
  if (ELVersionInfo() >= 6.0)
    {
      SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("128x128"));
      SendMessage(iconSizeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("256x256"));
    }

  WPARAM wParam = 0;
  switch (pSettings->GetIconSize())
    {
    case 256:
      wParam = 4;
      break;
    case 128:
      wParam = 3;
      break;
    case 48:
      wParam = 2;
      break;
    case 32:
      wParam = 1;
      break;
    }
  if ((ELVersionInfo() < 6.0) && (wParam > 2))
    wParam = 2;
  SendMessage(iconSizeWnd, CB_SETCURSEL, wParam, 0);

  SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Full"));
  SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Background"));

  if (pSettings->GetClickThrough() == 0)
    EnableWindow(clickThroughWnd, false);

  if (pSettings->GetClickThrough() == 1)
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_SETCURSEL, (WPARAM)0, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_SETCURSEL, (WPARAM)1, 0);

  return true;
}

bool ConfigPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  switch (LOWORD(wParam))
    {
    case IDC_FONTBUTTON:
      if (DoFontChooser(hwndDlg))
        {
          if (buttonFont)
            DeleteObject(buttonFont);
          buttonFont = CreateFontIndirect(&newFont);
          SendDlgItemMessage(hwndDlg, IDC_FONTBUTTON, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
          SetDlgItemText(hwndDlg, IDC_FONTBUTTON, newFont.lfFaceName);

          return true;
        }
      break;
    case IDC_CLICKTHROUGH:
      EnableWindow(clickThroughWnd, (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED));
      return true;
    }

  return false;
}

bool ConfigPage::DoFontChooser(HWND hwndDlg)
{
  CHOOSEFONT chooseFont;

  ZeroMemory(&chooseFont, sizeof(CHOOSEFONT));
  chooseFont.lStructSize = sizeof(CHOOSEFONT);
  chooseFont.hwndOwner = hwndDlg;
  chooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_NOSCRIPTSEL;
  chooseFont.lpLogFont = &newFont;

  return (ChooseFont(&chooseFont) == TRUE);
}

bool ConfigPage::UpdateSettings(HWND hwndDlg)
{
  int i, result, size = 16;
  HWND iconSizeWnd = GetDlgItem(hwndDlg, IDC_ICONSIZE);
  BOOL success;
  WCHAR tmp[MAX_LINE_LENGTH];

  success = (SendDlgItemMessage(hwndDlg, IDC_STARTHIDDEN, BM_GETCHECK, 0, 0) == BST_CHECKED);
  pSettings->SetStartHidden(success);

  success = (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_CHECKED);
  pSettings->SetSnapMove(success);

  success = (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED);
  pSettings->SetSnapSize(success);

  success = (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED);
  pSettings->SetAutoSize(success);

  GetDlgItemText(hwndDlg, IDC_TITLEBARTEXT, tmp, MAX_LINE_LENGTH);
  pSettings->SetTitleBarText(tmp);

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
  switch (i)
    {
    case 0:
      size = 16;
      break;
    case 1:
      size = 32;
      break;
    case 2:
      size = 48;
      break;
    case 3:
      size = 128;
      break;
    case 4:
      size = 256;
      break;
    }
  pSettings->SetIconSize(size);

  //pSettings->SetFont(&newFont);
  pSettings->SetTitleBarFont(&newFont);

  return true;
}

bool ConfigPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
    {
    case PSN_APPLY:
      if (UpdateSettings(hwndDlg))
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return true;

    case PSN_SETACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
      return true;

    case PSN_KILLACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return true;
    }

  return false;
}
