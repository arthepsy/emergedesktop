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

INT_PTR CALLBACK ConfigPage::ConfigPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam UNUSED, LPARAM lParam)
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
      return pConfigPage->DoInitPage(hwndDlg);

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
{}

INT_PTR ConfigPage::DoInitPage(HWND hwndDlg)
{
  HWND warningWnd = GetDlgItem(hwndDlg, IDC_WARNING);

  if (pSettings->GetShowExplorerDesktop())
    SendDlgItemMessage(hwndDlg, IDC_EXPLORERDESKTOP, BM_SETCHECK, BST_CHECKED, 0);

  std::wstring warningMessage = L"Note:\n\n";
  warningMessage += L"1. With 'Explorer Desktop' enabled, right clicking on the desktop willl display the Explorer right click menu.\n";
  warningMessage += L"2. When disabling 'Explorer Desktop' (if previously enabled), an Explorer window will be displayed.\n";

  SetWindowText(warningWnd, warningMessage.c_str());

  return 1;
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

bool ConfigPage::UpdateSettings(HWND hwndDlg)
{
  if (SendDlgItemMessage(hwndDlg, IDC_EXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetShowExplorerDesktop(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_EXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetShowExplorerDesktop(false);

  return true;
}

