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
      if (!pConfigPage)
        break;
      return pConfigPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      if (!pConfigPage)
        break;
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
  if (buttonFont)
    DeleteObject(buttonFont);
}

BOOL ConfigPage::DoInitDialog(HWND hwndDlg)
{
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  if (pSettings->GetSnapMove())
    SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetSnapSize())
    SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetClickThrough())
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetAutoComplete())
    SendDlgItemMessage(hwndDlg, IDC_AUTOCOMPLETE, BM_SETCHECK, BST_CHECKED, 0);

  if (pSettings->GetStartHidden())
    SendDlgItemMessage(hwndDlg, IDC_STARTHIDDEN, BM_SETCHECK, BST_CHECKED, 0);

  SetDlgItemText(hwndDlg, IDC_CLOCKTEXT, pSettings->GetTimeFormat());

  SetDlgItemText(hwndDlg, IDC_TIPTEXT, pSettings->GetTipFormat());

  CopyMemory(&newFont, pSettings->GetFont(), sizeof(LOGFONT));

  if (buttonFont)
    DeleteObject(buttonFont);
  buttonFont = CreateFontIndirect(&newFont);
  SendDlgItemMessage(hwndDlg, IDC_FONT_BUTTON, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
  SetDlgItemText(hwndDlg, IDC_FONT_BUTTON, newFont.lfFaceName);

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

INT_PTR ConfigPage::DoNotify(HWND hwndDlg, LPARAM lParam)
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

BOOL ConfigPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  switch (LOWORD(wParam))
    {
    case IDC_FONT_BUTTON:
      if (DoFontChooser(hwndDlg))
        {
          if (buttonFont)
            DeleteObject(buttonFont);
          buttonFont = CreateFontIndirect(&newFont);
          SendDlgItemMessage(hwndDlg, IDC_FONT_BUTTON, WM_SETFONT, (WPARAM)buttonFont, (LPARAM)TRUE);
          SetDlgItemText(hwndDlg, IDC_FONT_BUTTON, newFont.lfFaceName);

          return TRUE;
        }
      break;
    case IDC_CLICKTHROUGH:
      if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
        EnableWindow(clickThroughWnd, true);
      else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
        EnableWindow(clickThroughWnd, false);
      return TRUE;
    }

  return FALSE;
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
  WCHAR tmp[MAX_LINE_LENGTH];
  bool tmpBool = false;

  if (SendDlgItemMessage(hwndDlg, IDC_STARTHIDDEN, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpBool = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_STARTHIDDEN, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    tmpBool = false;
  pSettings->SetStartHidden(tmpBool);

  if (SendDlgItemMessage(hwndDlg, IDC_AUTOCOMPLETE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpBool = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_AUTOCOMPLETE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    tmpBool = false;
  pSettings->SetAutoComplete(tmpBool);

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpBool = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    tmpBool = false;
  pSettings->SetSnapMove(tmpBool);

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpBool = true;
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    tmpBool = false;
  pSettings->SetSnapSize(tmpBool);

  if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      int index = (int)SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_GETCURSEL, 0, 0);
      index++;
      pSettings->SetClickThrough(index);
    }
  else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetClickThrough(0);

  GetDlgItemText(hwndDlg, IDC_CLOCKTEXT, tmp, MAX_LINE_LENGTH);
  pSettings->SetTimeFormat(tmp);

  GetDlgItemText(hwndDlg, IDC_TIPTEXT, tmp, MAX_LINE_LENGTH);
  pSettings->SetTipFormat(tmp);

  pSettings->SetFont(&newFont);

  // commit changes
  pSettings->WriteSettings();

  return true;
}

