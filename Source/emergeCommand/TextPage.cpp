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

#include "TextPage.h"

INT_PTR CALLBACK TextPage::TextPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam UNUSED, LPARAM lParam)
{
  static TextPage *pTextPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pTextPage = reinterpret_cast<TextPage*>(psp->lParam);
      if (!pTextPage)
        break;
      return pTextPage->DoInitDialog(hwndDlg);

    case WM_NOTIFY:
      if (!pTextPage)
        break;
      return pTextPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

TextPage::TextPage(std::tr1::shared_ptr<Settings> pSettings)
{
  this->pSettings = pSettings;
}

TextPage::~TextPage()
{
}

INT_PTR TextPage::DoNotify(HWND hwndDlg, LPARAM lParam)
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

BOOL TextPage::DoInitDialog(HWND hwndDlg)
{
  int anchorIndex;

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
                                        (LPARAM)pSettings->GetAnchorPoint().c_str());
  SendDlgItemMessage(hwndDlg, IDC_ANCHOR, CB_SETCURSEL, anchorIndex, 0);

  if (ELToLower(pSettings->GetZPosition()) == TEXT("top"))
    SendDlgItemMessage(hwndDlg, IDC_TOP, BM_SETCHECK, BST_CHECKED, 0);
  else if (ELToLower(pSettings->GetZPosition()) == TEXT("bottom"))
    SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_SETCHECK, BST_CHECKED, 0);

  if (ELToLower(pSettings->GetClockTextAlign()) == TEXT("right"))
    SendDlgItemMessage(hwndDlg, IDC_CLOCKRIGHT, BM_SETCHECK, BST_CHECKED, 0);
  else if (ELToLower(pSettings->GetClockTextAlign()) == TEXT("center"))
    SendDlgItemMessage(hwndDlg, IDC_CLOCKHCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_CLOCKLEFT, BM_SETCHECK, BST_CHECKED, 0);

  if (ELToLower(pSettings->GetClockVerticalAlign()) == TEXT("bottom"))
    SendDlgItemMessage(hwndDlg, IDC_CLOCKBOTTOM, BM_SETCHECK, BST_CHECKED, 0);
  else if (ELToLower(pSettings->GetClockVerticalAlign()) == TEXT("center"))
    SendDlgItemMessage(hwndDlg, IDC_CLOCKVCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_CLOCKTOP, BM_SETCHECK, BST_CHECKED, 0);

  if (ELToLower(pSettings->GetCommandTextAlign()) == TEXT("right"))
    SendDlgItemMessage(hwndDlg, IDC_COMMANDRIGHT, BM_SETCHECK, BST_CHECKED, 0);
  else if (ELToLower(pSettings->GetCommandTextAlign()) == TEXT("center"))
    SendDlgItemMessage(hwndDlg, IDC_COMMANDHCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_COMMANDLEFT, BM_SETCHECK, BST_CHECKED, 0);

  if (ELToLower(pSettings->GetCommandVerticalAlign()) == TEXT("bottom"))
    SendDlgItemMessage(hwndDlg, IDC_COMMANDBOTTOM, BM_SETCHECK, BST_CHECKED, 0);
  else if (ELToLower(pSettings->GetCommandVerticalAlign()) == TEXT("center"))
    SendDlgItemMessage(hwndDlg, IDC_COMMANDVCENTER, BM_SETCHECK, BST_CHECKED, 0);
  else
    SendDlgItemMessage(hwndDlg, IDC_COMMANDTOP, BM_SETCHECK, BST_CHECKED, 0);

  return TRUE;
}

bool TextPage::UpdateSettings(HWND hwndDlg)
{
  const WCHAR* tmpValue;
  WCHAR tmp[MAX_LINE_LENGTH];

  if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_CHECKED)
    pSettings->SetDynamicPositioning(true);
  else if (SendDlgItemMessage(hwndDlg, IDC_DYNAMICPOSITIONING, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
    pSettings->SetDynamicPositioning(false);

  if (GetDlgItemText(hwndDlg, IDC_ANCHOR, tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELToLower(tmp) != ELToLower(pSettings->GetAnchorPoint()))
        pSettings->SetAnchorPoint(tmp);
    }

  if (SendDlgItemMessage(hwndDlg, IDC_TOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Top");
  if (SendDlgItemMessage(hwndDlg, IDC_BOTTOM, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Bottom");
  if (SendDlgItemMessage(hwndDlg, IDC_NORMAL, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Normal");
  pSettings->SetZPosition(tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_COMMANDLEFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Left");
  if (SendDlgItemMessage(hwndDlg, IDC_COMMANDHCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_COMMANDRIGHT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Right");
  pSettings->SetCommandTextAlign(tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_COMMANDTOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Top");
  if (SendDlgItemMessage(hwndDlg, IDC_COMMANDVCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_COMMANDBOTTOM, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Bottom");
  pSettings->SetCommandVerticalAlign(tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_CLOCKLEFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Left");
  if (SendDlgItemMessage(hwndDlg, IDC_CLOCKHCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_CLOCKRIGHT, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Right");
  pSettings->SetClockTextAlign(tmpValue);

  if (SendDlgItemMessage(hwndDlg, IDC_CLOCKTOP, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Top");
  if (SendDlgItemMessage(hwndDlg, IDC_CLOCKVCENTER, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Center");
  if (SendDlgItemMessage(hwndDlg, IDC_CLOCKBOTTOM, BM_GETCHECK, 0, 0) == BST_CHECKED)
    tmpValue = TEXT("Bottom");
  pSettings->SetClockVerticalAlign(tmpValue);

  // commit changes
  pSettings->WriteSettings();

  return true;
}

