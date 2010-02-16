// vim: tags+=../emergeLib/tags
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

#include "LaunchPage.h"

INT_PTR CALLBACK LaunchPage::LaunchPageDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static LaunchPage *pLaunchPage = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pLaunchPage = reinterpret_cast<LaunchPage*>(psp->lParam);
      if (!pLaunchPage)
        break;
      return pLaunchPage->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pLaunchPage->DoCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pLaunchPage->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

LaunchPage::LaunchPage(HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings)
{
  this->hInstance = hInstance;
  this->pSettings = pSettings;
  itemMoved = false;
  edit = false;

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

  addIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ADD), IMAGE_ICON, 16, 16, 0);
  editIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_EDIT), IMAGE_ICON, 16, 16, 0);
  delIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_DEL), IMAGE_ICON, 16, 16, 0);
  upIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_UP), IMAGE_ICON, 16, 16, 0);
  downIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_DOWN), IMAGE_ICON, 16, 16, 0);
  saveIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_SAVE), IMAGE_ICON, 16, 16, 0);
  abortIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ABORT), IMAGE_ICON, 16, 16, 0);
  browseIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_BROWSE), IMAGE_ICON, 16, 16, 0);
}

LaunchPage::~LaunchPage()
{
  if (addIcon)
    DestroyIcon(addIcon);
  if (editIcon)
    DestroyIcon(editIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (upIcon)
    DestroyIcon(upIcon);
  if (downIcon)
    DestroyIcon(downIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (abortIcon)
    DestroyIcon(abortIcon);
  if (browseIcon)
    DestroyIcon(browseIcon);
}

bool LaunchPage::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("All currently modifications will be lost.  To save and exit press OK.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeLauncher"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool LaunchPage::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);

  if ((!IsWindowEnabled(commandWnd)) && (!IsWindowEnabled(internalWnd)))
    return true;

  if ((GetDlgItemText(hwndDlg, IDC_COMMAND, tmp, MAX_LINE_LENGTH) != 0) ||
      (GetDlgItemText(hwndDlg, IDC_INTERNAL, tmp, MAX_LINE_LENGTH) != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("The current command will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeLauncher"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

BOOL LaunchPage::DoInitDialog(HWND hwndDlg)
{
  int iRet;
  LVCOLUMN lvCol;
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));
  DWORD dwRet;

  saveCount = 0;
  deleteCount = 0;

  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND iconWnd = GetDlgItem(hwndDlg, IDC_ICONPATH);
  HWND iconTextWnd = GetDlgItem(hwndDlg, IDC_ICONTEXT);
  HWND tipWnd = GetDlgItem(hwndDlg, IDC_TIP);
  HWND tipTextWnd = GetDlgItem(hwndDlg, IDC_TIPTEXT);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPITEM);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNITEM);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND browseIconWnd = GetDlgItem(hwndDlg, IDC_BROWSEICON);
  HWND browseCommandWnd = GetDlgItem(hwndDlg, IDC_BROWSECOMMAND);
  HWND browseWorkingDirWnd = GetDlgItem(hwndDlg, IDC_BROWSEWORKINGDIR);
  HWND workingDirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND exeButtonWnd = GetDlgItem(hwndDlg, IDC_EXEBUTTON);
  HWND comButtonWnd = GetDlgItem(hwndDlg, IDC_COMBUTTON);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_TYPE);
  HWND typeLabelWnd = GetDlgItem(hwndDlg, IDC_STATIC9);
  HWND specialFolderWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDER);
  HWND specialFolderLabelWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDERBUTTON);
  HWND separatorWnd = GetDlgItem(hwndDlg, IDC_SEPARATOR);
  HWND separatorLabelWnd = GetDlgItem(hwndDlg, IDC_SEPARATORBUTTON);

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (editIcon)
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (upIcon)
    SendMessage(upWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)upIcon);
  if (downIcon)
    SendMessage(downWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)downIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);
  if (browseIcon)
    {
      SendMessage(browseCommandWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
      SendMessage(browseIconWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
      SendMessage(browseWorkingDirWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
    }

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
  lvCol.pszText = (WCHAR*)TEXT("Type");
  lvCol.cx = 160;
  iRet = ListView_InsertColumn(listWnd, 0, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Command");
  lvCol.cx = 160;
  iRet = ListView_InsertColumn(listWnd, 1, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Working Directory");
  lvCol.cx = 160;
  iRet = ListView_InsertColumn(listWnd, 2, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Icon");
  lvCol.cx = 100;
  iRet = ListView_InsertColumn(listWnd, 3, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Tip");
  lvCol.cx = 160;
  iRet = ListView_InsertColumn(listWnd, 4, &lvCol);

  dwRet = ListView_SetExtendedListViewStyle(listWnd,  LVS_EX_FULLROWSELECT);

  PopulateList(listWnd);
  PopulateComboBoxes(hwndDlg);

  EnableWindow(commandWnd, false);
  EnableWindow(iconWnd, false);
  EnableWindow(iconTextWnd, false);
  EnableWindow(tipWnd, false);
  EnableWindow(tipTextWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(browseIconWnd, false);
  EnableWindow(browseCommandWnd, false);
  EnableWindow(editWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(upWnd, false);
  EnableWindow(downWnd, false);
  EnableWindow(workingDirWnd, false);
  EnableWindow(workingDirTextWnd, false);
  EnableWindow(browseWorkingDirWnd, false);
  EnableWindow(exeButtonWnd, false);
  EnableWindow(comButtonWnd, false);
  EnableWindow(internalWnd, false);
  EnableWindow(typeWnd, false);
  EnableWindow(typeLabelWnd, false);
  EnableWindow(specialFolderWnd, false);
  EnableWindow(specialFolderLabelWnd, false);
  EnableWindow(separatorWnd, false);
  EnableWindow(separatorLabelWnd, false);
  ShowWindow(comButtonWnd, SW_HIDE);
  ShowWindow(internalWnd, SW_HIDE);
  ShowWindow(specialFolderWnd, SW_HIDE);
  ShowWindow(specialFolderLabelWnd, SW_HIDE);
  ShowWindow(separatorWnd, SW_HIDE);
  ShowWindow(separatorLabelWnd, SW_HIDE);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add Item");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Item");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit Item");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = upWnd;
  ti.uId = (ULONG_PTR)upWnd;
  ti.lpszText = (WCHAR*)TEXT("Move Item Up");
  GetClientRect(upWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = downWnd;
  ti.uId = (ULONG_PTR)downWnd;
  ti.lpszText = (WCHAR*)TEXT("Move Item Down");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = saveWnd;
  ti.uId = (ULONG_PTR)saveWnd;
  ti.lpszText = (WCHAR*)TEXT("Save Item");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = abortWnd;
  ti.uId = (ULONG_PTR)abortWnd;
  ti.lpszText = (WCHAR*)TEXT("Discard Item");
  GetClientRect(abortWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = browseCommandWnd;
  ti.uId = (ULONG_PTR)browseCommandWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse For An Application");
  GetClientRect(browseCommandWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = browseIconWnd;
  ti.uId = (ULONG_PTR)browseIconWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse For An Icon");
  GetClientRect(browseIconWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  return TRUE;
}

BOOL LaunchPage::DoCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  bool exeButton = false;

  switch (LOWORD(wParam))
    {
    case IDC_EDITITEM:
      return DoEdit(hwndDlg);
    case IDC_ADDITEM:
      return DoAdd(hwndDlg);
    case IDC_DELITEM:
      return DelItem(hwndDlg);
    case IDC_UPITEM:
      return MoveItem(hwndDlg, true);
    case IDC_DOWNITEM:
      return MoveItem(hwndDlg, false);
    case IDC_SAVEITEM:
      SaveItem(hwndDlg);
      return EnableFields(hwndDlg, false);
    case IDC_ABORTITEM:
      AbortItem(hwndDlg);
      return EnableFields(hwndDlg, false);
    case IDC_BROWSECOMMAND:
      return Browse(hwndDlg, BROWSE_COMMAND);
    case IDC_BROWSEICON:
      return GetIcon(hwndDlg);
    case IDC_BROWSEWORKINGDIR:
      return Browse(hwndDlg, BROWSE_WORKINGDIR);
    case IDC_EXEBUTTON:
      exeButton = true;
    case IDC_COMBUTTON:
      return DoExeCom(hwndDlg, exeButton);
    case IDC_TYPE:
      if (HIWORD(wParam) == CBN_SELCHANGE)
        return ToggleFields(hwndDlg);
    case IDC_SEPARATOR:
      if (HIWORD(wParam) == CBN_SELCHANGE)
        return ToggleIconFields(hwndDlg);
      break;
    }

  return FALSE;
}

BOOL LaunchPage::ToggleIconFields(HWND hwndDlg)
{
  HWND iconWnd = GetDlgItem(hwndDlg, IDC_ICONPATH);
  HWND iconTextWnd = GetDlgItem(hwndDlg, IDC_ICONTEXT);
  HWND browseIconWnd = GetDlgItem(hwndDlg, IDC_BROWSEICON);
  WCHAR separatorName[MAX_LINE_LENGTH];

  if (GetDlgItemText(hwndDlg, IDC_SEPARATOR, separatorName, MAX_LINE_LENGTH) != 0)
    {
      if (wcsicmp(separatorName, TEXT("Custom")) == 0)
        {
          EnableWindow(iconWnd, true);
          EnableWindow(iconTextWnd, true);
          EnableWindow(browseIconWnd, true);
        }
      else
        {
          EnableWindow(iconWnd, false);
          EnableWindow(iconTextWnd, false);
          EnableWindow(browseIconWnd, false);
        }

      return TRUE;
    }

  return FALSE;
}

BOOL LaunchPage::ToggleFields(HWND hwndDlg)
{
  HWND iconWnd = GetDlgItem(hwndDlg, IDC_ICONPATH);
  HWND iconTextWnd = GetDlgItem(hwndDlg, IDC_ICONTEXT);
  HWND tipWnd = GetDlgItem(hwndDlg, IDC_TIP);
  HWND tipTextWnd = GetDlgItem(hwndDlg, IDC_TIPTEXT);
  HWND browseIconWnd = GetDlgItem(hwndDlg, IDC_BROWSEICON);
  HWND browseWorkingDirWnd = GetDlgItem(hwndDlg, IDC_BROWSEWORKINGDIR);
  HWND workingDirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND exeButtonWnd = GetDlgItem(hwndDlg, IDC_EXEBUTTON);
  HWND comButtonWnd = GetDlgItem(hwndDlg, IDC_COMBUTTON);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND browseCommandWnd = GetDlgItem(hwndDlg, IDC_BROWSECOMMAND);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND specialFolderWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDER);
  HWND specialFolderLabelWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDERBUTTON);
  HWND separatorWnd = GetDlgItem(hwndDlg, IDC_SEPARATOR);
  HWND separatorLabelWnd = GetDlgItem(hwndDlg, IDC_SEPARATORBUTTON);
  WCHAR typeName[MAX_LINE_LENGTH];

  if (GetDlgItemText(hwndDlg, IDC_TYPE, typeName, MAX_LINE_LENGTH) != 0)
    {
      if (wcsicmp(typeName, TEXT("Separator")) == 0)
        {
          EnableWindow(separatorWnd, true);
          EnableWindow(separatorLabelWnd, true);
          ShowWindow(separatorWnd, SW_SHOW);
          ShowWindow(separatorLabelWnd, SW_SHOW);
          EnableWindow(specialFolderWnd, false);
          EnableWindow(specialFolderLabelWnd, false);
          ShowWindow(specialFolderWnd, SW_HIDE);
          ShowWindow(specialFolderLabelWnd, SW_HIDE);
          EnableWindow(exeButtonWnd, false);
          EnableWindow(browseCommandWnd, false);
          EnableWindow(commandWnd, false);
          EnableWindow(comButtonWnd, false);
          ShowWindow(comButtonWnd, SW_HIDE);
          ShowWindow(internalWnd, SW_HIDE);
          ShowWindow(exeButtonWnd, SW_HIDE);
          ShowWindow(browseCommandWnd, SW_HIDE);
          ShowWindow(commandWnd, SW_HIDE);
          EnableWindow(internalWnd, false);
          EnableWindow(tipWnd, false);
          EnableWindow(tipTextWnd, false);
          EnableWindow(workingDirWnd, false);
          EnableWindow(workingDirTextWnd, false);
          EnableWindow(browseWorkingDirWnd, false);
          EnableWindow(iconWnd, false);
          EnableWindow(iconTextWnd, false);
          EnableWindow(browseIconWnd, false);
        }
      else if (wcsicmp(typeName, TEXT("Executable")) == 0)
        {
          EnableWindow(separatorWnd, false);
          EnableWindow(separatorLabelWnd, false);
          ShowWindow(separatorWnd, SW_HIDE);
          ShowWindow(separatorLabelWnd, SW_HIDE);
          EnableWindow(specialFolderWnd, false);
          EnableWindow(specialFolderLabelWnd, false);
          ShowWindow(specialFolderWnd, SW_HIDE);
          ShowWindow(specialFolderLabelWnd, SW_HIDE);
          EnableWindow(exeButtonWnd, true);
          EnableWindow(browseCommandWnd, true);
          EnableWindow(commandWnd, true);
          ShowWindow(exeButtonWnd, SW_SHOW);
          ShowWindow(browseCommandWnd, SW_SHOW);
          ShowWindow(commandWnd, SW_SHOW);
          ShowWindow(comButtonWnd, SW_HIDE);
          ShowWindow(internalWnd, SW_HIDE);
          EnableWindow(tipWnd, true);
          EnableWindow(tipTextWnd, true);
          EnableWindow(workingDirWnd, true);
          EnableWindow(workingDirTextWnd, true);
          EnableWindow(browseWorkingDirWnd, true);
          EnableWindow(iconWnd, true);
          EnableWindow(iconTextWnd, true);
          EnableWindow(browseIconWnd, true);
        }
      else if (wcsicmp(typeName, TEXT("Internal Command")) == 0)
        {
          EnableWindow(separatorWnd, false);
          EnableWindow(separatorLabelWnd, false);
          ShowWindow(separatorWnd, FALSE);
          ShowWindow(separatorLabelWnd, FALSE);
          EnableWindow(specialFolderWnd, false);
          EnableWindow(specialFolderLabelWnd, false);
          ShowWindow(specialFolderWnd, SW_HIDE);
          ShowWindow(specialFolderLabelWnd, SW_HIDE);
          EnableWindow(comButtonWnd, true);
          EnableWindow(internalWnd, true);
          ShowWindow(comButtonWnd, SW_SHOW);
          ShowWindow(internalWnd, SW_SHOW);
          ShowWindow(exeButtonWnd, SW_HIDE);
          ShowWindow(browseCommandWnd, SW_HIDE);
          ShowWindow(commandWnd, SW_HIDE);
          EnableWindow(tipWnd, true);
          EnableWindow(tipTextWnd, true);
          EnableWindow(workingDirWnd, false);
          EnableWindow(workingDirTextWnd, false);
          EnableWindow(browseWorkingDirWnd, false);
          EnableWindow(iconWnd, true);
          EnableWindow(iconTextWnd, true);
          EnableWindow(browseIconWnd, true);
        }
      else if (wcsicmp(typeName, TEXT("Special Folder")) == 0)
        {
          EnableWindow(separatorWnd, false);
          EnableWindow(separatorLabelWnd, false);
          ShowWindow(separatorWnd, FALSE);
          ShowWindow(separatorLabelWnd, FALSE);
          EnableWindow(specialFolderWnd, true);
          EnableWindow(specialFolderLabelWnd, true);
          ShowWindow(specialFolderWnd, SW_SHOW);
          ShowWindow(specialFolderLabelWnd, SW_SHOW);
          EnableWindow(comButtonWnd, false);
          EnableWindow(internalWnd, false);
          ShowWindow(comButtonWnd, SW_HIDE);
          ShowWindow(internalWnd, SW_HIDE);
          ShowWindow(exeButtonWnd, SW_HIDE);
          ShowWindow(browseCommandWnd, SW_HIDE);
          ShowWindow(commandWnd, SW_HIDE);
          EnableWindow(tipWnd, true);
          EnableWindow(tipTextWnd, true);
          EnableWindow(workingDirWnd, false);
          EnableWindow(workingDirTextWnd, false);
          EnableWindow(browseWorkingDirWnd, false);
          EnableWindow(iconWnd, true);
          EnableWindow(iconTextWnd, true);
          EnableWindow(browseIconWnd, true);
        }
      return TRUE;
    }

  return FALSE;
}

BOOL LaunchPage::GetIcon(HWND hwndDlg)
{
  WCHAR unexpanded[MAX_PATH], iconPath[MAX_PATH], *token;
  WCHAR source[MAX_LINE_LENGTH];
  WCHAR tmp[MAX_LINE_LENGTH];
  int iconIndex = 0;

  ZeroMemory(unexpanded, MAX_PATH);
  ZeroMemory(iconPath, MAX_PATH);
  ZeroMemory(source, MAX_LINE_LENGTH);
  ZeroMemory(tmp, MAX_LINE_LENGTH);

  if (GetDlgItemText(hwndDlg, IDC_ICONPATH, iconPath, MAX_LINE_LENGTH) != 0)
    {
      wcscpy(tmp, iconPath);
      token = wcstok(tmp, TEXT(","));
      if (token != NULL)
        wcscpy(iconPath, token);
      token = wcstok(NULL, TEXT(","));
      if (token != NULL)
        iconIndex = _wtoi(token);
    }
  else if (GetDlgItemText(hwndDlg, IDC_COMMAND, iconPath, MAX_LINE_LENGTH) != 0)
    {
      ELParseCommand(iconPath, source, tmp);
      wcscpy(iconPath, source);
    }

  if (EGGetIconDialogue(hwndDlg, iconPath, iconIndex))
    {
      ELUnExpandVars(iconPath);
      SetDlgItemText(hwndDlg, IDC_ICONPATH, iconPath);
    }
  return TRUE;
}

void LaunchPage::PopulateComboBoxes(HWND hwndDlg)
{
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_TYPE);
  HWND specialFolderWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDER);
  HWND separatorWnd = GetDlgItem(hwndDlg, IDC_SEPARATOR);
  WCHAR tmp[MAX_PATH];

  if (ELGetSpecialFolder(CSIDL_PERSONAL, tmp))
    SendMessage(specialFolderWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_DRIVES, tmp))
    SendMessage(specialFolderWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_NETWORK, tmp))
    SendMessage(specialFolderWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_CONTROLS, tmp))
    SendMessage(specialFolderWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_BITBUCKET, tmp))
    SendMessage(specialFolderWnd, CB_ADDSTRING, 0, (LPARAM)tmp);

  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Executable"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Internal Command"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Separator"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Special Folder"));

  SendMessage(separatorWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Blank"));
  SendMessage(separatorWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Custom"));
  SendMessage(separatorWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Double"));
  SendMessage(separatorWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Single"));

  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RightDeskMenu"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("MidDeskMenu"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Quit"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Run"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Shutdown"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Lock"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Hide"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Show"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_1"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_2"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_3"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_4"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_5"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_6"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_7"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_8"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_9"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMUp"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMDown"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMLeft"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMRight"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMGather"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("EmptyBin"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Logoff"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Reboot"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Halt"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Suspend"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Hibernate"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Disconnect"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("ShowDesktop"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeUp"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeDown"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeMute"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("DesktopSettings"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("DesktopMenuEditor"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CoreLaunchEditor"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CoreShellChanger"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Homepage"));
  SendMessage(internalWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Tutorial"));
}

bool LaunchPage::DoExeCom(HWND hwndDlg, bool exeButton)
{
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSECOMMAND);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND iconWnd = GetDlgItem(hwndDlg, IDC_ICONPATH);
  HWND browseWorkingDirWnd = GetDlgItem(hwndDlg, IDC_BROWSEWORKINGDIR);
  HWND workingDirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);

  if (!IsWindowEnabled(iconWnd))
    return false;

  if (exeButton)
    {
      SendMessage(internalWnd, CB_SETCURSEL, (WPARAM)-1, 0);
      EnableWindow(internalWnd, false);
      EnableWindow(browseWnd, true);
      EnableWindow(commandWnd, true);
      EnableWindow(browseWorkingDirWnd, true);
      EnableWindow(workingDirWnd, true);
    }
  else
    {
      SetDlgItemText(hwndDlg, IDC_COMMAND, TEXT(""));
      EnableWindow(internalWnd, true);
      EnableWindow(browseWnd, false);
      EnableWindow(commandWnd, false);
      EnableWindow(browseWorkingDirWnd, false);
      EnableWindow(workingDirWnd, false);
    }

  return true;
}

bool LaunchPage::UpdateSettings(HWND hwndDlg)
{
  WCHAR command[MAX_LINE_LENGTH], iconPath[MAX_LINE_LENGTH], tip[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  WCHAR typeName[MAX_LINE_LENGTH];
  int type = IT_EXECUTABLE;
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  bool listModified = ((saveCount != 0) || (deleteCount != 0) || itemMoved);

  if (listModified)
    pSettings->SetModified();
  pSettings->WriteSettings();

  if (listModified)
    {
      int i = 0;
      pSettings->DeleteItems(true);
      while (i < ListView_GetItemCount(listWnd))
        {
          ListView_GetItemText(listWnd, i, 0, typeName, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, i, 1, command, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, i, 2, workingDir, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, i, 3, iconPath, MAX_LINE_LENGTH);
          ListView_GetItemText(listWnd, i, 4, tip, MAX_LINE_LENGTH);

          if (wcsicmp(typeName, TEXT("separator")) == 0)
            type = IT_SEPARATOR;
          else if (wcsicmp(typeName, TEXT("internal command")) == 0)
            type = IT_INTERNAL_COMMAND;
          else if (wcsicmp(typeName, TEXT("special folder")) == 0)
            type = IT_SPECIAL_FOLDER;
          else
            type = IT_EXECUTABLE;

          pSettings->WriteItem(type, command, iconPath, tip, workingDir);

          i++;
        }
      pSettings->PopulateItems();
    }

  return true;
}

void LaunchPage::PopulateList(HWND listWnd)
{
  LVITEM lvItem;
  lvItem.mask = LVIF_TEXT;
  int iRet;

  for (UINT i = 0; i < pSettings->GetItemListSize(); i++)
    {
      lvItem.iItem = i;
      lvItem.iSubItem = 0;
      switch (pSettings->GetItem(i)->GetType())
        {
        case IT_SEPARATOR:
          lvItem.pszText = (WCHAR*)TEXT("Separator");
          break;
        case IT_INTERNAL_COMMAND:
          lvItem.pszText = (WCHAR*)TEXT("Internal Command");
          break;
        case IT_SPECIAL_FOLDER:
          lvItem.pszText = (WCHAR*)TEXT("Special Folder");
          break;
        default:
          lvItem.pszText = (WCHAR*)TEXT("Executable");
          break;
        }
      lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
      iRet = ListView_InsertItem(listWnd, &lvItem);
      ListView_SetItemText(listWnd, lvItem.iItem, 1, pSettings->GetItem(i)->GetApp());
      ListView_SetItemText(listWnd, lvItem.iItem, 2, pSettings->GetItem(i)->GetWorkingDir());
      ListView_SetItemText(listWnd, lvItem.iItem, 3, pSettings->GetItem(i)->GetIconPath());
      ListView_SetItemText(listWnd, lvItem.iItem, 4, pSettings->GetItem(i)->GetTip());
    }
}

bool LaunchPage::DelItem(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  bool ret = false;
  int i = 0, prevItem = 0;
  BOOL bRet;

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only delete one item at a time."),
                   (WCHAR*)TEXT("emergeLauncher"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          ret = true;
          prevItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          bRet = ListView_DeleteItem(listWnd, i);
          deleteCount++;

          ListView_SetItemState(listWnd, i, LVIS_SELECTED,
                                LVIS_SELECTED);
          if (!ListView_EnsureVisible(listWnd, i, FALSE))
            {
              if (prevItem != -1)
                {
                  ListView_SetItemState(listWnd, prevItem, LVIS_SELECTED,
                                        LVIS_SELECTED);
                  bRet = ListView_EnsureVisible(listWnd, prevItem, FALSE);
                }
            }

          break;
        }
      else
        i++;
    }

  if (ListView_GetItemCount(listWnd) == 0)
    EnableFields(hwndDlg, false);

  return ret;
}

bool LaunchPage::MoveItem(HWND hwndDlg, bool up)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  int i = 0;
  bool ret = false;
  LVITEM lvItem;
  WCHAR command[MAX_PATH], workingDir[MAX_PATH], icon[MAX_PATH], tip[MAX_LINE_LENGTH], typeName[MAX_LINE_LENGTH];
  BOOL bRet;
  int iRet;

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only move one item at a time."), (WCHAR*)TEXT("emergeLauncher"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  lvItem.mask = LVIF_TEXT;

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          ret = true;

          ListView_GetItemText(listWnd, i, 0, typeName, MAX_PATH);
          ListView_GetItemText(listWnd, i, 1, command, MAX_PATH);
          ListView_GetItemText(listWnd, i, 2, workingDir, MAX_PATH);
          ListView_GetItemText(listWnd, i, 3, icon, MAX_PATH);
          ListView_GetItemText(listWnd, i, 4, tip, MAX_LINE_LENGTH);

          if (up)
            lvItem.iItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          else
            lvItem.iItem = ListView_GetNextItem(listWnd, i, LVNI_BELOW);

          if (lvItem.iItem == -1)
            break;

          lvItem.iSubItem = 0;
          lvItem.pszText = typeName;
          lvItem.cchTextMax = MAX_PATH;

          bRet = ListView_DeleteItem(listWnd, i);

          itemMoved = true;
          iRet = ListView_InsertItem(listWnd, &lvItem);
          ListView_SetItemText(listWnd, lvItem.iItem, 1, command);
          ListView_SetItemText(listWnd, lvItem.iItem, 2, workingDir);
          ListView_SetItemText(listWnd, lvItem.iItem, 3, icon);
          ListView_SetItemText(listWnd, lvItem.iItem, 4, tip);

          ListView_SetItemState(listWnd, lvItem.iItem, LVIS_SELECTED, LVIS_SELECTED);
          bRet = ListView_EnsureVisible(listWnd, lvItem.iItem, FALSE);

          break;
        }
      else
        i++;
    }

  return ret;
}

bool LaunchPage::DoAdd(HWND hwndDlg)
{
  EnableFields(hwndDlg, true);

  SendDlgItemMessage(hwndDlg, IDC_INTERNAL, CB_SETCURSEL, (WPARAM)-1, 0);

  SetDlgItemText(hwndDlg, IDC_COMMAND, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_WORKINGDIR, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_ICONPATH, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_TIP, TEXT(""));

  return true;
}

bool LaunchPage::DoEdit(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only edit one item at a time."),
                   (WCHAR*)TEXT("emergeLauncher"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return false;
    }

  edit = true;

  EnableFields(hwndDlg, true);

  return true;
}

bool LaunchPage::EnableFields(HWND hwndDlg, bool enable)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND iconWnd = GetDlgItem(hwndDlg, IDC_ICONPATH);
  HWND iconTextWnd = GetDlgItem(hwndDlg, IDC_ICONTEXT);
  HWND tipWnd = GetDlgItem(hwndDlg, IDC_TIP);
  HWND tipTextWnd = GetDlgItem(hwndDlg, IDC_TIPTEXT);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPITEM);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNITEM);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND browseIconWnd = GetDlgItem(hwndDlg, IDC_BROWSEICON);
  HWND browseCommandWnd = GetDlgItem(hwndDlg, IDC_BROWSECOMMAND);
  HWND workingDirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND browseWorkingDirWnd = GetDlgItem(hwndDlg, IDC_BROWSEWORKINGDIR);
  HWND exeButtonWnd = GetDlgItem(hwndDlg, IDC_EXEBUTTON);
  HWND comButtonWnd = GetDlgItem(hwndDlg, IDC_COMBUTTON);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_TYPE);
  HWND typeLabelWnd = GetDlgItem(hwndDlg, IDC_STATIC9);
  HWND specialFolderWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDER);
  HWND specialFolderButtonWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDERBUTTON);
  HWND separatorWnd = GetDlgItem(hwndDlg, IDC_SEPARATOR);
  HWND separatorButtonWnd = GetDlgItem(hwndDlg, IDC_SEPARATORBUTTON);
  int i = 0;
  bool selected = false;

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        selected = true;

      i++;
    }

  if (enable)
    {
      EnableWindow(typeWnd, true);
      EnableWindow(typeLabelWnd, true);
      EnableWindow(listWnd, false);
      ToggleFields(hwndDlg);
      EnableWindow(addWnd, false);
      EnableWindow(editWnd, false);
      EnableWindow(delWnd, false);
      EnableWindow(downWnd, false);
      EnableWindow(upWnd, false);
      EnableWindow(saveWnd, true);
      EnableWindow(abortWnd, true);
    }
  else
    {
      EnableWindow(listWnd, true);
      EnableWindow(commandWnd, false);
      EnableWindow(iconWnd, false);
      EnableWindow(iconTextWnd, false);
      EnableWindow(tipWnd, false);
      EnableWindow(tipTextWnd, false);
      EnableWindow(addWnd, true);
      if (selected)
        {
          EnableWindow(editWnd, true);
          EnableWindow(delWnd, true);
          EnableWindow(downWnd, true);
          EnableWindow(upWnd, true);
        }
      else
        {
          EnableWindow(editWnd, false);
          EnableWindow(delWnd, false);
          EnableWindow(downWnd, false);
          EnableWindow(upWnd, false);
        }
      EnableWindow(saveWnd, false);
      EnableWindow(abortWnd, false);
      EnableWindow(browseIconWnd, false);
      EnableWindow(browseCommandWnd, false);
      EnableWindow(workingDirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(browseWorkingDirWnd, false);
      EnableWindow(exeButtonWnd, false);
      EnableWindow(comButtonWnd, false);
      EnableWindow(internalWnd, false);
      EnableWindow(typeWnd, false);
      EnableWindow(typeLabelWnd, false);
      EnableWindow(specialFolderWnd, false);
      EnableWindow(specialFolderButtonWnd, false);
      EnableWindow(separatorWnd, false);
      EnableWindow(separatorButtonWnd, false);
    }

  return true;
}

bool LaunchPage::AbortItem(HWND hwndDlg)
{
  SetDlgItemText(hwndDlg, IDC_COMMAND, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_ICONPATH, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_TIP, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_WORKINGDIR, TEXT(""));

  SendDlgItemMessage(hwndDlg, IDC_INTERNAL, CB_SETCURSEL, (WPARAM)-1, 0);
  SendDlgItemMessage(hwndDlg, IDC_TYPE, CB_SETCURSEL, (WPARAM)-1, 0);

  edit = false;

  return true;
}

bool LaunchPage::SaveItem(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  WCHAR command[MAX_LINE_LENGTH], iconPath[MAX_LINE_LENGTH], tip[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  WCHAR typeName[MAX_LINE_LENGTH];
  int i = 0, iRet;
  LVITEM lvItem;
  BOOL bRet;

  lvItem.mask = LVIF_TEXT;

  if (GetDlgItemText(hwndDlg, IDC_TYPE, typeName, MAX_LINE_LENGTH) == 0)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("Type cannot be empty"), (WCHAR*)TEXT("emergeLauncher"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }

  if (wcsicmp(typeName, TEXT("Executable")) == 0)
    {
      if (GetDlgItemText(hwndDlg, IDC_COMMAND, command, MAX_LINE_LENGTH) == 0)
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("Command cannot be empty"), (WCHAR*)TEXT("emergeLauncher"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }
  else if (wcsicmp(typeName, TEXT("Separator")) == 0)
    {
      if (GetDlgItemText(hwndDlg, IDC_SEPARATOR, command, MAX_LINE_LENGTH) == 0)
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("Style cannot be empty"), (WCHAR*)TEXT("emergeLauncher"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }
  else if (wcsicmp(typeName, TEXT("Internal Command")) == 0)
    {
      if (GetDlgItemText(hwndDlg, IDC_INTERNAL, command, MAX_LINE_LENGTH) == 0)
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("Command cannot be empty"), (WCHAR*)TEXT("emergeLauncher"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }
  else if (wcsicmp(typeName, TEXT("Special Folder")) == 0)
    {
      if (GetDlgItemText(hwndDlg, IDC_SPECIALFOLDER, command, MAX_LINE_LENGTH) == 0)
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("Command cannot be empty"), (WCHAR*)TEXT("emergeLauncher"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }
  else
    ZeroMemory(command, MAX_LINE_LENGTH);

  GetDlgItemText(hwndDlg, IDC_ICONPATH, iconPath, MAX_LINE_LENGTH);
  GetDlgItemText(hwndDlg, IDC_TIP, tip, MAX_LINE_LENGTH);
  GetDlgItemText(hwndDlg, IDC_WORKINGDIR, workingDir, MAX_LINE_LENGTH);

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        break;

      i++;
    }

  if (edit)
    bRet = ListView_DeleteItem(listWnd, i);

  lvItem.iItem = i;
  lvItem.iSubItem = 0;
  lvItem.pszText = typeName;
  lvItem.cchTextMax = (int)wcslen(command);
  iRet = ListView_InsertItem(listWnd, &lvItem);
  ListView_SetItemText(listWnd, lvItem.iItem, 1, command);
  ListView_SetItemText(listWnd, lvItem.iItem, 2, workingDir);
  ListView_SetItemText(listWnd, lvItem.iItem, 3, iconPath);
  ListView_SetItemText(listWnd, lvItem.iItem, 4, tip);

  SetDlgItemText(hwndDlg, IDC_COMMAND, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_ICONPATH, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_TIP, TEXT(""));
  SetDlgItemText(hwndDlg, IDC_WORKINGDIR, TEXT(""));

  SendDlgItemMessage(hwndDlg, IDC_INTERNAL, CB_SETCURSEL, (WPARAM)-1, 0);

  saveCount++;
  edit = false;

  return true;
}

bool LaunchPage::Browse(HWND hwndDlg, UINT type)
{
  bool ret = false;
  OPENFILENAME ofn;
  WCHAR tmp[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  BROWSEINFO bi;
  std::wstring workingPath;

  ZeroMemory(tmp, MAX_PATH);

  if (type == BROWSE_WORKINGDIR)
    {
      GetDlgItemText(hwndDlg, IDC_WORKINGDIR, tmp, MAX_PATH);
      ZeroMemory(&bi, sizeof(BROWSEINFO));
      bi.hwndOwner = hwndDlg;
      bi.ulFlags = BIF_NEWDIALOGSTYLE;
      bi.lpszTitle = TEXT("Select a folder:");

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

              SetDlgItemText(hwndDlg, IDC_WORKINGDIR, tmp);

              ret = true;
            }
        }
    }
  else
    {
      ZeroMemory(&ofn, sizeof(ofn));

      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hwndDlg;
      GetDlgItemText(hwndDlg, IDC_COMMAND, tmp, MAX_PATH);
      workingPath = tmp;
      workingPath = ELExpandVars(workingPath);
      if (ELParseCommand(workingPath.c_str(), program, arguments))
        wcscpy(tmp, program);
      ofn.lpstrFile = tmp;
      ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0\0");
      ofn.nMaxFile = MAX_PATH;
      ofn.lpstrTitle = TEXT("Browse For An Application");
      ofn.lpstrDefExt = NULL;
      //      ofn.lpstrInitialDir = path;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

      if (GetOpenFileName(&ofn))
        {
          ELUnExpandVars(tmp);
          SetDlgItemText(hwndDlg, IDC_COMMAND, tmp);
          ret = true;
        }
    }

  return ret;
}

BOOL LaunchPage::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPITEM);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNITEM);

  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
    {
    case LVN_ITEMCHANGED:
      EnableWindow(delWnd, true);
      EnableWindow(editWnd, true);
      EnableWindow(upWnd, true);
      EnableWindow(downWnd, true);
      return PopulateFields(hwndDlg, ((LPNMLISTVIEW)lParam)->iItem);

    case PSN_APPLY:
      if (CheckFields(hwndDlg) && UpdateSettings(hwndDlg))
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
      else
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return TRUE;

    case PSN_SETACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return TRUE;

    case PSN_KILLACTIVE:
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
      return TRUE;

    case PSN_QUERYCANCEL:
      if (CheckFields(hwndDlg) && CheckSaveCount(hwndDlg))
        SetWindowLong(hwndDlg,DWLP_MSGRESULT,FALSE);
      else
        SetWindowLong(hwndDlg,DWLP_MSGRESULT,TRUE);
      return TRUE;
    }

  return FALSE;
}

BOOL LaunchPage::PopulateFields(HWND hwndDlg, int itemIndex)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLIST);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_TYPE);
  HWND internalWnd = GetDlgItem(hwndDlg, IDC_INTERNAL);
  HWND internalButtonWnd = GetDlgItem(hwndDlg, IDC_COMBUTTON);
  HWND specialFolderWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDER);
  HWND specialFolderButtonWnd = GetDlgItem(hwndDlg, IDC_SPECIALFOLDERBUTTON);
  HWND separatorWnd = GetDlgItem(hwndDlg, IDC_SEPARATOR);
  HWND separatorButtonWnd = GetDlgItem(hwndDlg, IDC_SEPARATORBUTTON);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_COMMAND);
  HWND commandButtonWnd = GetDlgItem(hwndDlg, IDC_EXEBUTTON);
  WCHAR command[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH], iconPath[MAX_LINE_LENGTH];
  WCHAR tip[MAX_LINE_LENGTH], typeName[MAX_LINE_LENGTH];

  if (itemIndex == -1)
    return TRUE;

  if (ListView_GetItemState(listWnd, itemIndex, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, itemIndex, 0, typeName, MAX_LINE_LENGTH);
      int typeIndex = (int)SendMessage(typeWnd, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                       (LPARAM)typeName);
      if (typeIndex != CB_ERR)
        {
          SendMessage(typeWnd, CB_SETCURSEL, typeIndex, 0);
          ListView_GetItemText(listWnd, itemIndex, 1, command, MAX_LINE_LENGTH);
          if (wcsicmp(typeName, TEXT("internal command")) == 0)
            {
              ShowWindow(internalButtonWnd, SW_SHOW);
              ShowWindow(internalWnd, SW_SHOW);
              ShowWindow(commandWnd, SW_HIDE);
              ShowWindow(commandButtonWnd, SW_HIDE);
              ShowWindow(specialFolderWnd, SW_HIDE);
              ShowWindow(specialFolderButtonWnd, SW_HIDE);
              ShowWindow(separatorWnd, SW_HIDE);
              ShowWindow(separatorButtonWnd, SW_HIDE);

              int commandIndex = (int)SendMessage(internalWnd, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                                  (LPARAM)command);
              if (commandIndex != CB_ERR)
                SendMessage(internalWnd, CB_SETCURSEL, commandIndex, 0);
            }
          else if (wcsicmp(typeName, TEXT("special folder")) == 0)
            {
              ShowWindow(internalButtonWnd, SW_HIDE);
              ShowWindow(internalWnd, SW_HIDE);
              ShowWindow(commandWnd, SW_HIDE);
              ShowWindow(commandButtonWnd, SW_HIDE);
              ShowWindow(specialFolderWnd, SW_SHOW);
              ShowWindow(specialFolderButtonWnd, SW_SHOW);
              ShowWindow(separatorWnd, SW_HIDE);
              ShowWindow(separatorButtonWnd, SW_HIDE);

              int specialIndex = (int)SendMessage(specialFolderWnd, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                                  (LPARAM)command);
              if (specialIndex != CB_ERR)
                SendMessage(specialFolderWnd, CB_SETCURSEL, specialIndex, 0);
            }
          else if (wcsicmp(typeName, TEXT("separator")) == 0)
            {
              ShowWindow(internalButtonWnd, SW_HIDE);
              ShowWindow(internalWnd, SW_HIDE);
              ShowWindow(commandWnd, SW_HIDE);
              ShowWindow(commandButtonWnd, SW_HIDE);
              ShowWindow(specialFolderWnd, SW_HIDE);
              ShowWindow(specialFolderButtonWnd, SW_HIDE);
              ShowWindow(separatorWnd, SW_SHOW);
              ShowWindow(separatorButtonWnd, SW_SHOW);

              int separatorIndex = (int)SendMessage(separatorWnd, CB_FINDSTRINGEXACT, (WPARAM)-1,
                                                    (LPARAM)command);
              if (separatorIndex != CB_ERR)
                SendMessage(separatorWnd, CB_SETCURSEL, separatorIndex, 0);
            }
          else
            {
              ShowWindow(internalButtonWnd, SW_HIDE);
              ShowWindow(internalWnd, SW_HIDE);
              ShowWindow(commandWnd, SW_SHOW);
              ShowWindow(commandButtonWnd, SW_SHOW);
              ShowWindow(specialFolderWnd, SW_HIDE);
              ShowWindow(specialFolderButtonWnd, SW_HIDE);
              ShowWindow(separatorWnd, SW_HIDE);
              ShowWindow(separatorButtonWnd, SW_HIDE);

              SetDlgItemText(hwndDlg, IDC_COMMAND, command);
            }

          ListView_GetItemText(listWnd, itemIndex, 2, workingDir, MAX_LINE_LENGTH);
          SetDlgItemText(hwndDlg, IDC_WORKINGDIR, workingDir);
          ListView_GetItemText(listWnd, itemIndex, 3, iconPath, MAX_LINE_LENGTH);
          SetDlgItemText(hwndDlg, IDC_ICONPATH, iconPath);
          ListView_GetItemText(listWnd, itemIndex, 4, tip, MAX_LINE_LENGTH);
          SetDlgItemText(hwndDlg, IDC_TIP, tip);

          SetFocus(listWnd);
        }
    }

  return TRUE;
}
