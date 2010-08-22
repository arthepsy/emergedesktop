// vim:tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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

#include "ThemeSaver.h"

BOOL CALLBACK ThemeSaver::ThemeSaverDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ThemeSaver *pThemeSaver = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pThemeSaver = reinterpret_cast<ThemeSaver*>(lParam);
      if (!pThemeSaver)
        break;
      return pThemeSaver->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pThemeSaver->DoThemeCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

ThemeSaver::ThemeSaver(HINSTANCE hInstance, HWND mainWnd)
{
  CoInitialize(NULL);

  this->hInstance = hInstance;
  this->mainWnd = mainWnd;

  ZeroMemory(title, sizeof(title));
  ZeroMemory(theme, sizeof(theme));

  InitCommonControls();
}

ThemeSaver::~ThemeSaver()
{
  CoUninitialize();
}

int ThemeSaver::Show(WCHAR *theme)
{
  wcscpy((*this).theme, theme);
  swprintf(title, TEXT("Save '%s' Theme as..."), theme);
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_THEMESAVE), mainWnd, (DLGPROC)ThemeSaverDlgProc, (LPARAM)this);
}

BOOL ThemeSaver::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;

  GetWindowRect(hwndDlg, &rect);

  SetWindowText(hwndDlg, title);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  return TRUE;
}

BOOL ThemeSaver::DoThemeCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      if (SaveTheme(hwndDlg))
        EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDCANCEL:
      EndDialog(hwndDlg, wParam);
      return TRUE;
    }

  return FALSE;
}

bool ThemeSaver::SaveTheme(HWND hwndDlg)
{
  WCHAR themeName[MAX_PATH], errorText[MAX_LINE_LENGTH];
  std::wstring themePath = TEXT("%EmergeDir%\\themes\\"), copySource, copyDest;
  themePath = ELExpandVars(themePath);

  GetDlgItemText(hwndDlg, IDC_THEMEITEM, themeName, MAX_PATH);
  if ((_wcsicmp(themeName, TEXT("Default")) == 0) || (_wcsicmp(themeName, TEXT("GBRY")) == 0))
    {
      swprintf(errorText, TEXT("'%s' cannot be used as a theme name"), themeName);
      ELMessageBox(hwndDlg, errorText, (WCHAR*)TEXT("Theme Manager"), ELMB_MODAL
                   | ELMB_OK | ELMB_ICONERROR);
      return false;
    }
  copyDest = themePath + themeName;
  copySource = themePath + theme;
  copySource += TEXT("\\*");

  if (PathIsDirectory(copyDest.c_str()))
    {
      swprintf(errorText, TEXT("The theme '%s' already exists, overwrite?"), themeName);
      if (ELMessageBox(hwndDlg, errorText, (WCHAR*)TEXT("Theme Manager"),
                       ELMB_MODAL | ELMB_YESNO | ELMB_ICONERROR) == IDNO)
        return false;
    }
  else
    ELCreateDirectory(copyDest);

  if (ELFileOp(hwndDlg, FO_COPY, copySource, copyDest))
    {
      if (ELIsModifiedTheme(ELGetThemeName().c_str()))
        {
          copySource = themePath + theme;
          ELFileOp(hwndDlg, FO_DELETE, copySource);
        }
    }
  if (PathIsDirectory(copyDest.c_str()))
    ELSetTheme(copyDest);

  return true;
}
