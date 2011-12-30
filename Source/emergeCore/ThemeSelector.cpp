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

#include "ThemeSelector.h"

BOOL CALLBACK ThemeSelector::ThemeSelectorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static ThemeSelector *pThemeSelector = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pThemeSelector = reinterpret_cast<ThemeSelector*>(lParam);
      if (!pThemeSelector)
        break;
      return pThemeSelector->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pThemeSelector->DoThemeCommand(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

ThemeSelector::ThemeSelector(HINSTANCE hInstance, HWND mainWnd)
{
  this->hInstance = hInstance;
  this->mainWnd = mainWnd;

  ExtractIconEx(TEXT("emergeIcons.dll"), 10, NULL, &saveasIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 3, NULL, &delIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 20, NULL, &exportIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 21, NULL, &importIcon, 1);

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

ThemeSelector::~ThemeSelector()
{
  if (saveasIcon)
    DestroyIcon(saveasIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (exportIcon)
    DestroyIcon(exportIcon);
  if (importIcon)
    DestroyIcon(importIcon);

  DestroyWindow(toolWnd);
}

int ThemeSelector::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_THEMESELECT), mainWnd, (DLGPROC)ThemeSelectorDlgProc, (LPARAM)this);
}

BOOL ThemeSelector::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  ELGetWindowRect(hwndDlg, &rect);

  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  HWND saveasWnd = GetDlgItem(hwndDlg, IDC_SAVEAS);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVETHEME);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELTHEME);
  HWND importWnd = GetDlgItem(hwndDlg, IDC_IMPORTTHEME);
  HWND exportWnd = GetDlgItem(hwndDlg, IDC_EXPORTTHEME);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  if (saveasIcon)
    SendMessage(saveasWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveasIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (exportIcon)
    SendMessage(exportWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)exportIcon);
  if (importIcon)
    SendMessage(importWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)importIcon);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = saveasWnd;
  ti.uId = (ULONG_PTR)saveasWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Save Theme As");
  GetClientRect(saveasWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
  ti.hwnd = saveWnd;
  ti.uId = (ULONG_PTR)saveWnd;
  ti.lpszText = (WCHAR*)TEXT("Save Theme");
  GetClientRect(saveWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Theme");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
  ti.hwnd = importWnd;
  ti.uId = (ULONG_PTR)importWnd;
  ti.lpszText = (WCHAR*)TEXT("Import Theme");
  GetClientRect(importWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
  ti.hwnd = exportWnd;
  ti.uId = (ULONG_PTR)exportWnd;
  ti.lpszText = (WCHAR*)TEXT("Export Theme");
  GetClientRect(exportWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  bool isModifiedTheme = ELIsModifiedTheme(ELGetThemeName().c_str());
  std::wstring defaultModifiedTheme = TEXT("default (modified)");
  bool isDefaultModifiedTheme = (ELToLower(ELGetThemeName()) == defaultModifiedTheme);
  std::wstring GBRYModifiedTheme = TEXT("gbry (modified)");
  bool isGBRYModifiedTheme = (ELToLower(ELGetThemeName()) == GBRYModifiedTheme);

  EnableWindow(saveWnd, isModifiedTheme && !isDefaultModifiedTheme && !isGBRYModifiedTheme);
  EnableWindow(delWnd, !isModifiedTheme);

  PopulateThemes(themeWnd, (WCHAR*)ELGetThemeName().c_str());

  pThemeSaver = std::tr1::shared_ptr<ThemeSaver>(new ThemeSaver(hInstance, hwndDlg));

  return TRUE;
}

void ThemeSelector::PopulateThemes(HWND themeWnd, WCHAR *currentTheme)
{
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  bool found = false;
  std::wstring themeDir;

  // Clear any existing items
  SendMessage(themeWnd, CB_RESETCONTENT, 0, 0);
  SendMessage(themeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Default"));

  themeDir = TEXT("%EmergeDir%\\themes\\*");
  themeDir = ELExpandVars(themeDir);
  fileHandle = FindFirstFile(themeDir.c_str(), &findData);
  if (fileHandle == INVALID_HANDLE_VALUE)
    return;
  else
    {
      int i = 0;

      do
        {
          // Skip hidden files
          if (wcscmp(findData.cFileName, TEXT(".")) == 0 ||
              wcscmp(findData.cFileName, TEXT("..")) == 0 ||
              (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
            continue;

          if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
              i++;
              SendMessage(themeWnd, CB_ADDSTRING, 0, (LPARAM)findData.cFileName);

              if (_wcsicmp(currentTheme, findData.cFileName) == 0)
                {
                  found = true;
                  SendMessage(themeWnd, CB_SETCURSEL, (WPARAM)i, 0);
                }
            }
        }
      while (FindNextFile(fileHandle, &findData));

      FindClose(fileHandle);
    }

  if ((_wcsicmp(currentTheme, TEXT("Default")) == 0) || !found)
    SendMessage(themeWnd, CB_SETCURSEL, (WPARAM)0, 0);
}

BOOL ThemeSelector::DoThemeCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  std::wstring themePath = TEXT("%ThemeDir%");
  themePath = ELExpandVars(themePath);

  switch (LOWORD(wParam))
    {
    case IDOK:
      SaveTheme(hwndDlg);
    case IDCANCEL:
      if (!ELPathIsDirectory(themePath.c_str()))
        wParam = IDOK;
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDC_THEMEITEM:
      if (HIWORD(wParam) == CBN_SELCHANGE)
        DoThemeCheck(hwndDlg);
      return TRUE;
    case IDC_SAVEAS:
      DoSaveAs(hwndDlg);
      return TRUE;
    case IDC_DELTHEME:
      DoDelTheme(hwndDlg);
      return TRUE;
    case IDC_SAVETHEME:
      DoSave(hwndDlg);
      DoThemeCheck(hwndDlg);
      return TRUE;
    case IDC_EXPORTTHEME:
      DoExport(hwndDlg);
      return TRUE;
    case IDC_IMPORTTHEME:
      DoImport(hwndDlg);
      return TRUE;
    }

  return FALSE;
}

void ThemeSelector::DoExport(HWND hwndDlg)
{
  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  WCHAR theme[MAX_PATH], tmp[MAX_PATH], message[MAX_LINE_LENGTH];
  BROWSEINFO bi;
  std::wstring themePath, target, themeRoot = TEXT("%EmergeDir%\\themes");

  if (GetWindowText(themeWnd, theme, MAX_PATH) != 0)
    {
      themePath = themeRoot + TEXT("\\") + theme;
      target += theme;
      target += TEXT(".zip");

      ZeroMemory(&bi, sizeof(BROWSEINFO));
      bi.hwndOwner = hwndDlg;
      bi.ulFlags = BIF_NEWDIALOGSTYLE;
      bi.lpszTitle = TEXT("Export folder:");

      LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bi);

      if (pItemIDList != NULL)
        {
          if (SHGetPathFromIDList(pItemIDList, tmp))
            {
              IMalloc* pMalloc = NULL;
              if (SUCCEEDED(SHGetMalloc(&pMalloc)))
                {
                  pMalloc->Free(pItemIDList);
                  pMalloc->Release();
                }

              target = tmp;
              if (target.at(target.length() - 1) != '\\')
                target += TEXT("\\");
              target += theme;
              target += TEXT(".zip");
              if (PathFileExists(target.c_str()))
                {
                  swprintf(message, TEXT("Do you want to replace '%ls'?"), target.c_str());
                  if (ELMessageBox(hwndDlg, message, TEXT("emergeCore"),
                                   ELMB_YESNO|ELMB_ICONQUESTION) == IDNO)
                    return;
                  else
                    ELFileOp(hwndDlg, FO_DELETE, target);
                }
              if (ELMakeZip(target, themeRoot, themePath) == 0)
                swprintf(message, TEXT("Successfully exported '%ls' theme to '%ls'."),
                         theme, target.c_str());
              else
                swprintf(message, TEXT("Failed to export '%ls'."), theme);
              ELMessageBox(hwndDlg, message, TEXT("emergeCore"), ELMB_OK|ELMB_MODAL|ELMB_ICONINFORMATION);
            }
        }
    }
}

void ThemeSelector::DoImport(HWND hwndDlg)
{
  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  OPENFILENAME ofn;
  std::wstring themesPath = TEXT("%EmergeDir%\\themes"), workingZip, themeName, themePath;
  WCHAR tmp[MAX_PATH], message[MAX_LINE_LENGTH];

  ZeroMemory(&ofn, sizeof(ofn));
  ZeroMemory(tmp, MAX_PATH);

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("All Files (*.zip)\0*.zip\0\0");
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFile = tmp;
  ofn.lpstrTitle = TEXT("Browse For Theme File");
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER |
              OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
    {
      workingZip = tmp;
      if (ELExtractZip(workingZip, themesPath) == 0)
        {
          swprintf(message, TEXT("Successfully imported '%ls'."), workingZip.c_str());
          PopulateThemes(themeWnd, (WCHAR*)ELGetThemeName().c_str());
        }
      else
        swprintf(message, TEXT("Failed to import '%ls'."), workingZip.c_str());
      ELMessageBox(hwndDlg, message, TEXT("emergeCore"), ELMB_OK|ELMB_MODAL|ELMB_ICONINFORMATION);
    }
}

void ThemeSelector::DoSave(HWND hwndDlg)
{
  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  WCHAR theme[MAX_PATH], source[MAX_PATH];
  std::wstring sourceTheme, destTheme, copySource;

  GetDlgItemText(hwndDlg, IDC_THEMEITEM, source, MAX_PATH);
  wcscpy(theme, source);
  ELStripModified(theme);

  sourceTheme = TEXT("%EmergeDir%\\themes\\");
  sourceTheme += source;
  copySource = sourceTheme + TEXT("\\*");
  destTheme = TEXT("%EmergeDir%\\themes\\");
  destTheme += theme;

  // If the destination theme is the same as the source theme, abort
  if (sourceTheme == destTheme)
    return;

  // If the destTheme directory exists, remove it and re-create it (to make
  // sure its empty.
  if (ELPathIsDirectory(ELExpandVars(destTheme).c_str()))
    ELFileOp(hwndDlg, FO_DELETE, destTheme);
  if (ELCreateDirectory(destTheme))
    {
      if (ELFileOp(hwndDlg, FO_COPY, copySource, destTheme))
        {
          ELFileOp(hwndDlg, FO_DELETE, sourceTheme);
          ELSetTheme(destTheme);
        }
    }

  PopulateThemes(themeWnd, theme);
}

void ThemeSelector::DoDelTheme(HWND hwndDlg)
{
  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  WCHAR theme[MAX_PATH];
  std::wstring themePath = TEXT("%EmergeDir%\\themes\\");

  GetWindowText(themeWnd, theme, MAX_PATH);
  themePath += theme;

  if (_wcsicmp(ELGetThemeName().c_str(), theme) == 0)
    {
      if (ELMessageBox(hwndDlg,
                       TEXT("You are about to delete the active theme.  This\nwill result in the 'Default' theme becoming the active theme.\nDo you wish to proceed?"),
                       TEXT("Theme Manager"),
                       ELMB_YESNO | ELMB_ICONWARNING) == IDNO)
        return;
    }
  if (ELFileOp(hwndDlg, FO_DELETE, themePath))
    PopulateThemes(themeWnd, (WCHAR*)ELGetThemeName().c_str());
}

void ThemeSelector::DoSaveAs(HWND hwndDlg)
{
  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  WCHAR theme[MAX_PATH];

  GetWindowText(themeWnd, theme, MAX_PATH);
  if (pThemeSaver->Show(theme) == IDOK)
    PopulateThemes(themeWnd, (WCHAR*)ELGetThemeName().c_str());
}

bool ThemeSelector::SaveTheme(HWND hwndDlg)
{
  WCHAR theme[MAX_PATH];
  std::wstring themePath = TEXT("%EmergeDir%\\themes\\");

  GetDlgItemText(hwndDlg, IDC_THEMEITEM, theme, MAX_PATH);
  themePath += theme;

  return ELSetTheme(themePath);
}

BOOL ThemeSelector::DoThemeCheck(HWND hwndDlg)
{
  HWND themeWnd = GetDlgItem(hwndDlg, IDC_THEMEITEM);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVETHEME);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELTHEME);
  HWND exportWnd = GetDlgItem(hwndDlg, IDC_EXPORTTHEME);
  WCHAR theme[MAX_PATH], errorText[MAX_LINE_LENGTH];
  std::wstring themePath = TEXT("%ThemeDir%");
  themePath = ELExpandVars(themePath);

  GetWindowText(themeWnd, theme, MAX_PATH);
  EnableWindow(delWnd, (_wcsicmp(theme, TEXT("Default")) != 0));
  EnableWindow(saveWnd, (ELIsModifiedTheme(theme) && (_wcsicmp(theme, TEXT("Default (Modified)")) != 0)));
  EnableWindow(exportWnd, (_wcsicmp(theme, TEXT("Default")) != 0));

  if (ELIsModifiedTheme(ELGetThemeName().c_str()) && ELPathIsDirectory(themePath.c_str()))
    {
      swprintf (errorText, TEXT("The existing '%ls' theme will be lost, save it?"), ELGetThemeName().c_str());
      if (ELMessageBox(hwndDlg, errorText, TEXT("Theme Selector"),
                       ELMB_ICONQUESTION | ELMB_YESNO) == IDYES)
        {
          if (pThemeSaver->Show((WCHAR*)ELGetThemeName().c_str()) == IDOK)
            PopulateThemes(themeWnd, theme);
        }
      else
        {
          if (ELFileOp(hwndDlg, FO_DELETE, themePath))
            PopulateThemes(themeWnd, theme);
        }
    }
  return TRUE;
}
