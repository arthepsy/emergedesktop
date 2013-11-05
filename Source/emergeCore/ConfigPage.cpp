//----  --------------------------------------------------------------------------------------------------------
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

    case WM_COMMAND:
      return pConfigPage->DoCommand(hwndDlg, wParam);

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
{}

INT_PTR ConfigPage::DoInitPage(HWND hwndDlg)
{
  HWND warningWnd = GetDlgItem(hwndDlg, IDC_WARNING);
  HWND explorerWnd = GetDlgItem(hwndDlg, IDC_EXPLORERDESKTOP);

  if (pSettings->GetEnableExplorerDesktop())
    {
      SendDlgItemMessage(hwndDlg, IDC_ENABLEEXPLORERDESKTOP, BM_SETCHECK, BST_CHECKED, 0);
      EnableWindow(explorerWnd, TRUE);
    }
  else
    EnableWindow(explorerWnd, FALSE);

  if (pSettings->GetShowExplorerDesktop())
    SendDlgItemMessage(hwndDlg, IDC_EXPLORERDESKTOP, BM_SETCHECK, BST_CHECKED, 0);

  std::wstring warningMessage = TEXT("Notes:\n\n");
  warningMessage += TEXT("When 'Show Explorer Desktop' is enabled:\n\n");
  warningMessage += TEXT("- Right-clicking on the desktop displays the Explorer right-click menu, and\n\n");
  warningMessage += TEXT("- The emergeWorkspace menus can no longer be accessed by right-clicking on the desktop. They still can be accessed via emergeHotkeys, emergeCommand and emergeLauncher.\n\n");

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

INT_PTR ConfigPage::DoCommand(HWND hwndDlg, WPARAM wParam)
{
  switch (LOWORD(wParam))
    {
    case IDC_ENABLEEXPLORERDESKTOP:
      EnableWindow((HWND)GetDlgItem(hwndDlg, IDC_EXPLORERDESKTOP),
                   (SendDlgItemMessage(hwndDlg, IDC_ENABLEEXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_CHECKED));
      return true;
    }

  return false;
}

bool ConfigPage::UpdateSettings(HWND hwndDlg)
{
  if (SendDlgItemMessage(hwndDlg, IDC_EXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetShowExplorerDesktop(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_EXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetShowExplorerDesktop(false);

  if (SendDlgItemMessage(hwndDlg, IDC_ENABLEEXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetEnableExplorerDesktop(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_ENABLEEXPLORERDESKTOP, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetEnableExplorerDesktop(false);

  return true;
}

