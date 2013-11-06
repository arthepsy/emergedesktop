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
#include "Applet.h"

INT_PTR CALLBACK ConfigPage::ConfigPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ConfigPage* pConfigPage = NULL;
  PROPSHEETPAGE* psp;

  switch (message)
  {
  case WM_INITDIALOG:
    psp = (PROPSHEETPAGE*)lParam;
    pConfigPage = reinterpret_cast<ConfigPage*>(psp->lParam);
    if (!pConfigPage)
    {
      break;
    }
    return pConfigPage->DoInitPage(hwndDlg);

  case WM_COMMAND:
    if (!pConfigPage)
    {
      break;
    }
    return pConfigPage->DoCommand(hwndDlg, wParam, lParam);

  case WM_NOTIFY:
    if (!pConfigPage)
    {
      break;
    }
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
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);

  SetDlgItemInt(hwndDlg, IDC_FLASHINTERVAL, pSettings->GetFlashInterval(), false);
  SetDlgItemInt(hwndDlg, IDC_FLASHCOUNT, pSettings->GetFlashCount(), false);

  SendDlgItemMessage(hwndDlg, IDC_ICONSPACINGUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);
  SendDlgItemMessage(hwndDlg, IDC_FLASHINTERVALUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)10000);
  SendDlgItemMessage(hwndDlg, IDC_FLASHCOUNTUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);
  SendDlgItemMessage(hwndDlg, IDC_AUTOLIMITUPDOWN, UDM_SETRANGE, (WPARAM)0, (LPARAM)100);

  if (pSettings->GetAutoSize())
  {
    SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_SETCHECK, BST_CHECKED, 0);
  }

  if (pSettings->GetSnapMove())
  {
    SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_SETCHECK, BST_CHECKED, 0);
  }

  if (pSettings->GetSnapSize())
  {
    SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_SETCHECK, BST_CHECKED, 0);
  }

  if (pSettings->GetHiliteActive())
  {
    SendDlgItemMessage(hwndDlg, IDC_ACTIVETASK, BM_SETCHECK, BST_CHECKED, 0);
  }

  //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  if (pSettings->GetSameMonitorOnly())
  {
    SendDlgItemMessage(hwndDlg, IDC_SAMEMONITOR, BM_SETCHECK, BST_CHECKED, 0);
  }

  if (pSettings->GetEnableFlash())
  {
    SendDlgItemMessage(hwndDlg, IDC_ENABLEFLASH, BM_SETCHECK, BST_CHECKED, 0);
  }

  if (pSettings->GetClickThrough() != 0)
  {
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_SETCHECK, BST_CHECKED, 0);
  }

  SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("16x16"));
  SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_ADDSTRING, 0, (LPARAM)TEXT("32x32"));

  SetDlgItemText(hwndDlg, IDC_AUTOLIMIT, towstring(pSettings->GetAutoSizeLimit()).c_str());
  if (!pSettings->GetAutoSize())
  {
    EnableWindow((HWND)GetDlgItem(hwndDlg, IDC_AUTOLIMIT), FALSE);
  }

  if (pSettings->GetIconSize() == 32)
  {
    SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_SETCURSEL, (WPARAM)1, 0);
  }
  else
  {
    SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_SETCURSEL, (WPARAM)0, 0);
  }

  SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);

  SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Full"));
  SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_ADDSTRING, 0, (LPARAM)TEXT("Background"));

  if (pSettings->GetClickThrough() == 0)
  {
    EnableWindow(clickThroughWnd, false);
  }

  if (pSettings->GetClickThrough() == 1)
  {
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_SETCURSEL, (WPARAM)0, 0);
  }
  else
  {
    SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_SETCURSEL, (WPARAM)1, 0);
  }

  return 1;
}

INT_PTR ConfigPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  HWND clickThroughWnd = GetDlgItem(hwndDlg, IDC_CLICKTHROUGHMETHOD);
  HWND anchorWnd = GetDlgItem(hwndDlg, IDC_ANCHOR);

  switch (LOWORD(wParam))
  {
  case IDC_CLICKTHROUGH:
    if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      EnableWindow(clickThroughWnd, true);
    }
    else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
      EnableWindow(clickThroughWnd, false);
    }
    return 1;
  case IDC_DYNAMICPOSITIONING:
    if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
      EnableWindow(anchorWnd, true);
    }
    else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    {
      EnableWindow(anchorWnd, false);
    }
    return 1;
  case IDC_AUTOSIZE:
    EnableWindow((HWND)GetDlgItem(hwndDlg, IDC_AUTOLIMIT),
                 (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED));
    return true;
  }

  return 0;
}

INT_PTR ConfigPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  NMHDR* phdr = (NMHDR*)lParam;

  switch (phdr->code)
  {
  case PSN_APPLY:
    if (UpdateSettings(hwndDlg))
    {
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
    }
    else
    {
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
    }
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
  BOOL success;
  int result, size, index;
  WCHAR tmp[MAX_LINE_LENGTH];

  if (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetAutoSize(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_AUTOSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetAutoSize(false);
  }

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetSnapMove(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPMOVE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetSnapMove(false);
  }

  if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetSnapSize(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_SNAPSIZE, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetSnapSize(false);
  }

  if (SendDlgItemMessage(hwndDlg, IDC_ACTIVETASK, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetHiliteActive(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_ACTIVETASK, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetHiliteActive(false);
  }

  if (SendDlgItemMessage(hwndDlg, IDC_ENABLEFLASH, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetEnableFlash(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_ENABLEFLASH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetEnableFlash(false);
  }

  //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  if (SendDlgItemMessage(hwndDlg, IDC_SAMEMONITOR, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    pSettings->SetSameMonitorOnly(true);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_SAMEMONITOR, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetSameMonitorOnly(false);
  }

  result = GetDlgItemInt(hwndDlg, IDC_AUTOLIMIT, &success, false);
  if (success)
  {
    pSettings->SetAutoSizeLimit(result);
  }
  else if (!success)
  {
    ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid value for AutoSize wrap"),
                 (WCHAR*)TEXT("emergeTasks"), ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);
    return false;
  }

  if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_CHECKED)
  {
    index = (int)SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGHMETHOD, CB_GETCURSEL, 0, 0);
    index++;
    pSettings->SetClickThrough(index);
  }
  else if (SendDlgItemMessage(hwndDlg, IDC_CLICKTHROUGH, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
  {
    pSettings->SetClickThrough(0);
  }

  result = GetDlgItemInt(hwndDlg, IDC_FLASHINTERVAL, &success, false);
  if (success)
  {
    pSettings->SetFlashInterval(result);
  }
  else if (!success)
  {
    ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid value for flash interval"),
                 (WCHAR*)TEXT("emergeTasks"), ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    SetDlgItemInt(hwndDlg, IDC_FLASHINTERVAL, pSettings->GetFlashInterval(), false);
    return false;
  }

  result = GetDlgItemInt(hwndDlg, IDC_FLASHCOUNT, &success, false);
  if (success)
  {
    pSettings->SetFlashCount(result);
  }
  else if (!success)
  {
    ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid value for flash count"),
                 (WCHAR*)TEXT("emergeTasks"), ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    SetDlgItemInt(hwndDlg, IDC_FLASHCOUNT, pSettings->GetFlashCount(), false);
    return false;
  }

  index = (int)SendDlgItemMessage(hwndDlg, IDC_ICONSIZE, CB_GETCURSEL, 0, 0);
  if (index == 1)
  {
    size = 32;
  }
  else
  {
    size = 16;
  }
  pSettings->SetIconSize(size);

  result = GetDlgItemInt(hwndDlg, IDC_ICONSPACING, &success, false);
  if (success)
  {
    pSettings->SetIconSpacing(result);
  }
  else if (!success)
  {
    ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Invalid value for icon spacing"),
                 (WCHAR*)TEXT("emergeTasks"), ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
    SetDlgItemInt(hwndDlg, IDC_ICONSPACING, pSettings->GetIconSpacing(), false);
    return false;
  }

  if (GetDlgItemText(hwndDlg, IDC_ANCHOR, tmp, MAX_LINE_LENGTH) != 0)
  {
    if (wcscmp(tmp, pSettings->GetAnchorPoint().c_str()) != 0)
    {
      pSettings->SetAnchorPoint(tmp);
    }
  }

  return true;
}

