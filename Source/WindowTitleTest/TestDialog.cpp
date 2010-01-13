// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009  The Emerge Desktop Development Team
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

#include "TestDialog.h"

extern TestDialog *pTestDialog;

BOOL CALLBACK TestDialog::TestDialogDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static TestDialog *pTestDialog = NULL;

  switch (message) {
  case WM_INITDIALOG:
    pTestDialog = reinterpret_cast<TestDialog*>(lParam);
    if (!pTestDialog)
      break;
    return TRUE;

  case WM_COMMAND:
    return pTestDialog->DoCommand(hwndDlg, wParam, lParam);
  }

  return FALSE;
}

TestDialog::TestDialog(HINSTANCE hInstance)
{
  (*this).hInstance = hInstance;
}

TestDialog::~TestDialog()
{
}

int TestDialog::Show()
{
  return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)TestDialogDlgProc, (LPARAM)this);
}

void TestDialog::SetTitle(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];

  for (UINT i = 0; i < 1000000 ; i++)
    {
      swprintf(tmp, TEXT("%d"), i);
      SetWindowText(hwndDlg, tmp);
    }
}

BOOL TestDialog::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam)) {
  case IDC_TEST:
    SetTitle(hwndDlg);
    return TRUE;
  case IDOK:
    EndDialog(hwndDlg, wParam);
    return TRUE;
  }

  return FALSE;
}

