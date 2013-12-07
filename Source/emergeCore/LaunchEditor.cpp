// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

#include "LaunchEditor.h"

INT_PTR CALLBACK LaunchEditor::LaunchDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static LaunchEditor* pLaunchEditor = NULL;
  PROPSHEETPAGE* psp;

  switch (message)
  {
  case WM_INITDIALOG:
    psp = (PROPSHEETPAGE*)lParam;
    pLaunchEditor = reinterpret_cast<LaunchEditor*>(psp->lParam);
    if (!pLaunchEditor)
    {
      break;
    }
    return pLaunchEditor->DoInitDialog(hwndDlg);

  case WM_COMMAND:
    return pLaunchEditor->DoLaunchCommand(hwndDlg, wParam, lParam);

  case WM_NOTIFY:
    return pLaunchEditor->DoNotify(hwndDlg, lParam);
  }

  return FALSE;
}

BOOL CALLBACK LaunchEditor::AppletCheck(HWND hwnd, LPARAM lParam)
{
  LaunchEditor* pLaunchEditor = reinterpret_cast<LaunchEditor*>(lParam);
  std::wstring fileName = ELGetWindowApp(hwnd, true);

  if (fileName.empty())
  {
    return true;
  }

  if (ELToLower(fileName) == ELToLower(pLaunchEditor->GetSelectedApplet()))
  {
    SendMessage(hwnd, WM_NCDESTROY, 0, 0);
  }

  return true;
}

BOOL CALLBACK LaunchEditor::GatherApplet(HWND hwnd, LPARAM lParam)
{
  LaunchEditor* pLaunchEditor = reinterpret_cast<LaunchEditor*>(lParam);
  std::wstring fileName = ELGetWindowApp(hwnd, true);
  POINT cursorPt;
  RECT appletRect;

  if (fileName.empty())
  {
    return true;
  }

  GetCursorPos(&cursorPt);
  GetClientRect(hwnd, &appletRect);

  if (ELToLower(fileName) == ELToLower(pLaunchEditor->GetSelectedApplet()))
    SetWindowPos(hwnd, HWND_TOPMOST, cursorPt.x - (appletRect.right / 2),
                 cursorPt.y - (appletRect.bottom / 2), 0, 0,
                 SWP_NOSIZE | SWP_NOSENDCHANGING);

  return true;
}

std::wstring LaunchEditor::GetSelectedApplet()
{
  return selectedApplet;
}

LaunchEditor::LaunchEditor(HINSTANCE hInstance, HWND mainWnd)
{
  (*this).hInstance = hInstance;
  (*this).mainWnd = mainWnd;
  saveCount = 0;
  deleteCount = 0;
  edit = false;

  InitCommonControls();

  toolWnd = CreateWindowEx(
              0,
              TOOLTIPS_CLASS,
              NULL,
              TTS_ALWAYSTIP | WS_POPUP | TTS_NOPREFIX,
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

  ExtractIconEx(TEXT("emergeIcons.dll"), 2, NULL, &addIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 5, NULL, &editIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 3, NULL, &delIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &browseIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 13, NULL, &upIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 4, NULL, &downIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 1, NULL, &abortIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 11, NULL, &startIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 12, NULL, &stopIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 8, NULL, &infoIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 7, NULL, &gatherIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 22, NULL, &configIcon, 1);

  xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += TEXT("emergeCore.xml");
}

LaunchEditor::~LaunchEditor()
{
  if (addIcon)
  {
    DestroyIcon(addIcon);
  }
  if (editIcon)
  {
    DestroyIcon(editIcon);
  }
  if (delIcon)
  {
    DestroyIcon(delIcon);
  }
  if (upIcon)
  {
    DestroyIcon(upIcon);
  }
  if (downIcon)
  {
    DestroyIcon(downIcon);
  }
  if (browseIcon)
  {
    DestroyIcon(browseIcon);
  }
  if (saveIcon)
  {
    DestroyIcon(saveIcon);
  }
  if (abortIcon)
  {
    DestroyIcon(abortIcon);
  }
  if (startIcon)
  {
    DestroyIcon(startIcon);
  }
  if (stopIcon)
  {
    DestroyIcon(stopIcon);
  }

  DestroyWindow(toolWnd);
}

BOOL LaunchEditor::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  LVCOLUMN lvCol;
  TOOLINFO ti;
  dlgWnd = hwndDlg;

  ZeroMemory(&ti, sizeof(TOOLINFO));

  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPAPP);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNAPP);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND startWnd = GetDlgItem(hwndDlg, IDC_STARTAPP);
  HWND stopWnd = GetDlgItem(hwndDlg, IDC_STOPAPP);
  HWND infoWnd = GetDlgItem(hwndDlg, IDC_INFOAPP);
  HWND gatherWnd = GetDlgItem(hwndDlg, IDC_GATHERAPP);
  HWND configWnd = GetDlgItem(hwndDlg, IDC_CONFIGAPP);

  saveCount = 0;
  deleteCount = 0;

  rect = ELGetWindowRect(hwndDlg);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
  lvCol.pszText = (WCHAR*)TEXT("State");
  lvCol.cx = 70;
  if (ListView_InsertColumn(listWnd, 0, &lvCol) == -1)
  {
    return FALSE;
  }

  lvCol.pszText = (WCHAR*)TEXT("Applet");
  lvCol.cx = MAX_PATH;
  if (ListView_InsertColumn(listWnd, 1, &lvCol) == -1)
  {
    return FALSE;
  }

  lvCol.pszText = (WCHAR*)TEXT("Version");
  lvCol.cx = 100;
  if (ListView_InsertColumn(listWnd, 2, &lvCol) == -1)
  {
    return FALSE;
  }

  if (ListView_SetExtendedListViewStyle(listWnd,  LVS_EX_FULLROWSELECT) != 0)
  {
    return FALSE;
  }

  if (addIcon)
  {
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  }
  if (editIcon)
  {
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);
  }
  if (delIcon)
  {
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  }
  if (browseIcon)
  {
    SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
  }
  if (upIcon)
  {
    SendMessage(upWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)upIcon);
  }
  if (downIcon)
  {
    SendMessage(downWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)downIcon);
  }
  if (saveIcon)
  {
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  }
  if (abortIcon)
  {
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);
  }
  if (startIcon)
  {
    SendMessage(startWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)startIcon);
  }
  if (stopIcon)
  {
    SendMessage(stopWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)stopIcon);
  }
  if (infoIcon)
  {
    SendMessage(infoWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)infoIcon);
  }
  if (gatherIcon)
  {
    SendMessage(gatherWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)gatherIcon);
  }
  if (configIcon)
  {
    SendMessage(configWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)configIcon);
  }

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add Launch Applet");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit Launch Applet");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Launch Applet");
  GetClientRect(delWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;
  ti.lpszText = (WCHAR*)TEXT("Browse for Launch Applet");
  GetClientRect(browseWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = upWnd;
  ti.uId = (ULONG_PTR)upWnd;
  ti.lpszText = (WCHAR*)TEXT("Move Launch Applet Up");
  GetClientRect(upWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = downWnd;
  ti.uId = (ULONG_PTR)downWnd;
  ti.lpszText = (WCHAR*)TEXT("Move Launch Applet Down");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = startWnd;
  ti.uId = (ULONG_PTR)startWnd;
  ti.lpszText = (WCHAR*)TEXT("Start Applet");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = stopWnd;
  ti.uId = (ULONG_PTR)stopWnd;
  ti.lpszText = (WCHAR*)TEXT("Stop Applet");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = infoWnd;
  ti.uId = (ULONG_PTR)infoWnd;
  ti.lpszText = (WCHAR*)TEXT("Applet Information");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = gatherWnd;
  ti.uId = (ULONG_PTR)gatherWnd;
  ti.lpszText = (WCHAR*)TEXT("Gather Applet");
  GetClientRect(downWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = configWnd;
  ti.uId = (ULONG_PTR)configWnd;
  ti.lpszText = (WCHAR*)TEXT("Configure Applet");
  GetClientRect(configWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = saveWnd;
  ti.uId = (ULONG_PTR)saveWnd;
  ti.lpszText = (WCHAR*)TEXT("Save Changes");
  GetClientRect(saveWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = abortWnd;
  ti.uId = (ULONG_PTR)abortWnd;
  ti.lpszText = (WCHAR*)TEXT("Discard Changes");
  GetClientRect(abortWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  PopulateList(listWnd);

  EnableWindow(editWnd, false);
  EnableWindow(upWnd, false);
  EnableWindow(downWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(startWnd, false);
  EnableWindow(stopWnd, false);
  EnableWindow(infoWnd, false);
  EnableWindow(gatherWnd, false);
  EnableWindow(configWnd, false);

  return TRUE;
}

bool LaunchEditor::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
  {
    if (ELMessageBox(hwndDlg,
                     TEXT("All current modifications will be lost.\n\nDo you wish to continue?"),
                     TEXT("emergeCore"),
                     ELMB_YESNO | ELMB_ICONQUESTION | ELMB_MODAL) == IDYES)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  return true;
}

bool LaunchEditor::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);

  if (!IsWindowEnabled(appletWnd))
  {
    return true;
  }

  if (GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_LINE_LENGTH) != 0)
  {
    if (ELMessageBox(hwndDlg,
                     TEXT("The current applet will be lost.\n\nDo you wish to continue?"),
                     TEXT("emergeCore"),
                     ELMB_YESNO | ELMB_ICONQUESTION | ELMB_MODAL) == IDYES)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  return true;
}

BOOL LaunchEditor::DoLaunchCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
  {
  case IDC_DELAPP:
    return DoLaunchDelete(hwndDlg);
  case IDC_ADDAPP:
    return DoLaunchAddEdit(hwndDlg);
  case IDC_EDITAPP:
    edit = true;
    return DoLaunchAddEdit(hwndDlg);
  case IDC_SAVEAPP:
    return DoLaunchSave(hwndDlg);
  case IDC_ABORTAPP:
    return DoLaunchAbort(hwndDlg);
  case IDC_BROWSE:
    return DoLaunchBrowse(hwndDlg);
  case IDC_UPAPP:
    return DoLaunchMove(hwndDlg, true);
  case IDC_DOWNAPP:
    return DoLaunchMove(hwndDlg, false);
  case IDC_STARTAPP:
    return DoMultiStart(hwndDlg);
  case IDC_STOPAPP:
    return DoMultiStop(hwndDlg);
  case IDC_GATHERAPP:
    return DoMultiGather(hwndDlg);
  case IDC_CONFIGAPP:
    return DoMultiConfig(hwndDlg);
  case IDC_INFOAPP:
    return DoMultiInfo(hwndDlg);
  }

  return FALSE;
}

BOOL LaunchEditor::DoMultiConfig(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int listCount = ListView_GetItemCount(listWnd);
  WCHAR name[MAX_PATH];
  BOOL ret = FALSE;
  std::wstring selectedName;

  for (int i = 0; i < listCount; i++)
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, i, 1, name, MAX_PATH);
      selectedName = name;
      selectedName = selectedName.substr(selectedName.rfind('\\') + 1, selectedName.rfind('.') - (selectedName.rfind('\\') + 1));
      ELDispatchCoreMessage(EMERGE_CORE, CORE_SHOWCONFIG, selectedName.c_str());
      ret = TRUE;
    }
  }

  return ret;
}

BOOL LaunchEditor::DoMultiInfo(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int listCount = ListView_GetItemCount(listWnd);
  WCHAR name[MAX_PATH], appletName[MAX_PATH], tmp[MAX_LINE_LENGTH];
  VERSIONINFO versionInfo;
  BOOL ret = FALSE;

  for (int i = 0; i < listCount; i++)
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, i, 1, name, MAX_PATH);
      wcscpy(appletName, PathFindFileName(name));
      PathRemoveExtension(appletName);
      if (ELAppletFileVersion(name, &versionInfo))
      {
        swprintf(tmp, TEXT("%ls\n\nVersion: %ls\n\nAuthor: %ls"),
                 versionInfo.Description,
                 versionInfo.Version,
                 versionInfo.Author);
        ELMessageBox(hwndDlg, tmp, appletName,
                     ELMB_ICONQUESTION | ELMB_OK | ELMB_MODAL);
        ret = TRUE;
      }
    }
  }

  return ret;
}

BOOL LaunchEditor::DoMultiGather(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int listCount = ListView_GetItemCount(listWnd);
  WCHAR name[MAX_PATH];
  BOOL ret = FALSE;

  for (int i = 0; i < listCount; i++)
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, i, 1, name, MAX_PATH);
      selectedApplet = name;
      selectedApplet = ELExpandVars(selectedApplet);
      selectedApplet = ELGetAbsolutePath(selectedApplet);
      EnumWindows(GatherApplet, reinterpret_cast<LPARAM>(this));
      ret = TRUE;
    }
  }

  return ret;
}

BOOL LaunchEditor::DoMultiStop(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int listCount = ListView_GetItemCount(listWnd);
  WCHAR name[MAX_PATH];
  BOOL ret = FALSE;

  for (int i = 0; i < listCount; i++)
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, i, 1, name, MAX_PATH);
      selectedApplet = name;
      selectedApplet = ELExpandVars(selectedApplet);
      selectedApplet = ELGetAbsolutePath(selectedApplet);
      EnumWindows(AppletCheck, reinterpret_cast<LPARAM>(this));
      ListView_SetItemText(listWnd, i, 0, (WCHAR*)TEXT("Not Loaded"));
      ret = TRUE;
    }
  }

  return ret;
}

BOOL LaunchEditor::DoMultiStart(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int listCount = ListView_GetItemCount(listWnd);
  WCHAR name[MAX_PATH];
  BOOL ret = FALSE;

  for (int i = 0; i < listCount; i++)
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, i, 1, name, MAX_PATH);
      if (ELExecuteFileOrCommand(name))
      {
        ret = TRUE;
        ListView_SetItemText(listWnd, i, 0, (WCHAR*)TEXT("Loaded"));
      }
    }
  }

  return ret;
}

bool LaunchEditor::DoLaunchMove(HWND hwndDlg, bool up)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int i = 0;
  bool ret = false;
  LVITEM lvItem;
  WCHAR applet[MAX_PATH], version[MAX_PATH], state[MAX_PATH];

  if (ListView_GetSelectedCount(listWnd) > 1)
  {
    ELMessageBox(hwndDlg, TEXT("You can only move one item at a time."),
                 TEXT("emergeCore"), ELMB_OK | MB_ICONERROR | ELMB_MODAL);

    return ret;
  }

  saveCount++;

  lvItem.mask = LVIF_TEXT;

  while (i < ListView_GetItemCount(listWnd))
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      ListView_GetItemText(listWnd, i, 0, state, MAX_PATH);
      ListView_GetItemText(listWnd, i, 1, applet, MAX_PATH);
      ListView_GetItemText(listWnd, i, 2, version, MAX_PATH);

      if (up)
      {
        lvItem.iItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
      }
      else
      {
        lvItem.iItem = ListView_GetNextItem(listWnd, i, LVNI_BELOW);
      }

      if (lvItem.iItem == -1)
      {
        break;
      }

      lvItem.iSubItem = 0;
      lvItem.pszText = state;
      lvItem.cchTextMax = MAX_PATH;

      if (ListView_DeleteItem(listWnd, i))
      {

        if (ListView_InsertItem(listWnd, &lvItem) != -1)
        {
          ListView_SetItemText(listWnd, lvItem.iItem, 1, applet);
          ListView_SetItemText(listWnd, lvItem.iItem, 2, version);
        }

        ListView_SetItemState(listWnd, lvItem.iItem, LVIS_SELECTED, LVIS_SELECTED);
        ret = (ListView_EnsureVisible(listWnd, lvItem.iItem, FALSE) == TRUE);
      }

      break;
    }
    else
    {
      i++;
    }
  }

  return ret;
}

bool LaunchEditor::DoLaunchStart(HWND listWnd, int index)
{
  bool ret = false;

  if (ELExecuteFileOrCommand(selectedApplet))
  {
    ListView_SetItemText(listWnd, index, 0, (WCHAR*)TEXT("Loaded"));
    ret = true;
  }

  return ret;
}

bool LaunchEditor::DoLaunchStop(HWND listWnd, int index)
{
  EnumWindows(AppletCheck, reinterpret_cast<LPARAM>(this));
  ListView_SetItemText(listWnd, index, 0, (WCHAR*)TEXT("Not Loaded"));

  return true;
}

bool LaunchEditor::GetLaunchAppletName(int index, WCHAR* applet)
{
  HWND listWnd = GetDlgItem(dlgWnd, IDC_APPLETLIST);
  std::wstring workingApplet;

  ListView_GetItemText(listWnd, index, 1, applet, MAX_PATH);
  workingApplet = applet;
  workingApplet = ELExpandVars(workingApplet);
  wcscpy(applet, workingApplet.c_str());

  return true;
}

bool LaunchEditor::UpdateLaunch(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  WCHAR applet[MAX_PATH];
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* first, *section = NULL, *settings;
  std::wstring theme = ELGetThemeName(), oldTheme, newThemePath, oldThemePath;

  if ((saveCount == 0) && (deleteCount == 0))
  {
    return true;
  }

  if (!ELIsModifiedTheme(theme))
  {
    oldTheme = theme;
  }

  if (ELSetModifiedTheme(theme))
  {
    oldThemePath = TEXT("%EmergeDir%\\themes\\") + oldTheme;
    oldThemePath += TEXT("\\*");
    newThemePath = TEXT("%ThemeDir%");

    if ((ELGetFileSpecialFlags(newThemePath) & SF_DIRECTORY) != SF_DIRECTORY)
    {
      if (ELCreateDirectory(newThemePath))
      {
        ELFileOp(mainWnd, false, FO_COPY, oldThemePath, newThemePath);
      }
    }
  }

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
  {
    /**< Remove the old 'Launch' top level section */
    section = ELGetXMLSection(configXML.get(), TEXT("Launch"), false);
    if (section)
    {
      ELRemoveXMLElement(section);
    }

    settings = ELGetXMLSection(configXML.get(), TEXT("Settings"), true);
    if (settings)
    {
      /**< Remove existing Launch list */
      section = ELGetFirstXMLElementByName(settings, TEXT("Launch"), false);
      if (section)
      {
        ELRemoveXMLElement(section);
      }
      section = ELSetFirstXMLElementByName(settings, TEXT("Launch"));
    }

    if (section)
    {
      // Loop while there are entries in the key
      int i = 0;
      while (i < ListView_GetItemCount(listWnd))
      {
        ListView_GetItemText(listWnd, i, 1, applet, MAX_PATH);
        first = ELSetFirstXMLElementByName(section, TEXT("item"));
        if (first)
        {
          ELWriteXMLStringValue(first, TEXT("Command"), applet);
        }

        i++;
      }
      ELWriteXMLConfig(configXML.get());
    }
  }

  return true;
}

bool LaunchEditor::PopulateList(HWND listWnd)
{
  bool found = false;
  std::wstring data;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement* first, *sibling, *section = NULL, *settings;

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
  {
    settings = ELGetXMLSection(configXML.get(), TEXT("Settings"), false);
    if (settings)
    {
      section = ELGetFirstXMLElementByName(settings, TEXT("Launch"), false);
    }
    if (section)
    {
      first = ELGetFirstXMLElement(section);

      if (first)
      {
        DWORD index = 0;

        data = ELReadXMLStringValue(first, TEXT("Command"), TEXT(""));
        if (!data.empty())
        {
          found = true;
          InsertListViewItem(listWnd, index, data.c_str());
        }

        sibling = ELGetSiblingXMLElement(first);
        while (sibling)
        {
          index++;
          first = sibling;

          data = ELReadXMLStringValue(first, TEXT("Command"), TEXT(""));
          if (!data.empty())
          {
            InsertListViewItem(listWnd, index, data.c_str());
          }

          sibling = ELGetSiblingXMLElement(first);
        }
      }
    }
  }

  if (!found)
  {
    InsertListViewItem(listWnd, 0, TEXT("emergeTasks.exe"));
    InsertListViewItem(listWnd, 1, TEXT("emergeTray.exe"));
    InsertListViewItem(listWnd, 2, TEXT("emergeWorkspace.exe"));
    InsertListViewItem(listWnd, 3, TEXT("emergeCommand.exe"));
    InsertListViewItem(listWnd, 4, TEXT("emergeLauncher.exe"));
    InsertListViewItem(listWnd, 5, TEXT("emergeHotkeys.exe"));
  }

  return found;
}

void LaunchEditor::InsertListViewItem(HWND listWnd, int index, const WCHAR* item)
{
  VERSIONINFO versionInfo;
  LVITEM lvItem;
  WCHAR tmp[MAX_PATH];
  std::wstring workingItem;

  wcscpy(tmp, item);

  lvItem.mask = LVIF_TEXT;
  lvItem.iItem = index;
  lvItem.iSubItem = 0;
  lvItem.pszText = GetLaunchItemState(tmp);
  lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
  if (ListView_InsertItem(listWnd, &lvItem) != -1)
  {
    ListView_SetItemText(listWnd, lvItem.iItem, 1, tmp);
  }

  if (ELAppletFileVersion(tmp, &versionInfo))
  {
    ListView_SetItemText(listWnd, lvItem.iItem, 2, versionInfo.Version);
  }
}

WCHAR* LaunchEditor::GetLaunchItemState(WCHAR* launchItem)
{
  if (ELIsAppletRunning(launchItem))
  {
    return (WCHAR*)TEXT("Loaded");
  }

  return (WCHAR*)TEXT("Not Loaded");
}

bool LaunchEditor::DoLaunchDelete(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPAPP);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNAPP);
  HWND startWnd = GetDlgItem(hwndDlg, IDC_STARTAPP);
  HWND stopWnd = GetDlgItem(hwndDlg, IDC_STOPAPP);
  HWND infoWnd = GetDlgItem(hwndDlg, IDC_INFOAPP);
  HWND gatherWnd = GetDlgItem(hwndDlg, IDC_GATHERAPP);
  HWND configWnd = GetDlgItem(hwndDlg, IDC_CONFIGAPP);
  bool ret = false;
  int i = 0, prevItem = 0;

  if (ListView_GetSelectedCount(listWnd) > 1)
  {
    ELMessageBox(hwndDlg, TEXT("You can only delete one item at a time."),
                 TEXT("emergeCore"), ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);

    return ret;
  }

  while (i < ListView_GetItemCount(listWnd))
  {
    if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
    {
      prevItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
      if (!ListView_DeleteItem(listWnd, i))
      {
        return ret;
      }
      deleteCount++;

      ListView_SetItemState(listWnd, i, LVIS_SELECTED,
                            LVIS_SELECTED);
      if (!ListView_EnsureVisible(listWnd, i, FALSE))
      {
        if (prevItem != -1)
        {
          ListView_SetItemState(listWnd, prevItem, LVIS_SELECTED,
                                LVIS_SELECTED);
          ret = (ListView_EnsureVisible(listWnd, prevItem, FALSE) == TRUE);
        }
      }

      break;
    }
    else
    {
      i++;
    }
  }

  if (ListView_GetItemCount(listWnd) == 0)
  {
    EnableWindow(editWnd, false);
    EnableWindow(delWnd, false);
    EnableWindow(upWnd, false);
    EnableWindow(downWnd, false);
    EnableWindow(startWnd, false);
    EnableWindow(stopWnd, false);
    EnableWindow(infoWnd, false);
    EnableWindow(gatherWnd, false);
    EnableWindow(configWnd, false);
  }

  return ret;
}

bool LaunchEditor::DoLaunchAddEdit(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);

  EnableWindow(appletWnd, true);
  EnableWindow(browseWnd, true);
  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);
  EnableWindow(listWnd, false);

  if (!edit)
  {
    SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
  }

  return true;
}

bool LaunchEditor::DoLaunchAbort(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);

  SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(listWnd, true);

  edit = false;

  return true;
}

bool LaunchEditor::FindListSubItem(HWND listWnd, int subItem, WCHAR* searchString)
{
  int listSize = ListView_GetItemCount(listWnd), i = 0;
  WCHAR item[MAX_LINE_LENGTH];
  bool ret = false;

  for (i = 0; i < listSize; i++)
  {
    ListView_GetItemText(listWnd, i, subItem, item, MAX_LINE_LENGTH);
    if (_wcsicmp(item, searchString) == 0)
    {
      ret = true;
      break;
    }
  }

  return ret;
}

bool LaunchEditor::DoLaunchSave(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  bool ret = false;
  LVITEM lvItem;
  WCHAR tmp[MAX_LINE_LENGTH], error[MAX_LINE_LENGTH];
  VERSIONINFO versionInfo;

  ZeroMemory(tmp, MAX_LINE_LENGTH);
  lvItem.mask = LVIF_TEXT;

  GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_LINE_LENGTH);
  if (wcslen(tmp) > 0)
  {
    if (edit)
    {
      lvItem.iItem = selectedItem;
      lvItem.iSubItem = 0;
      lvItem.pszText = GetLaunchItemState(tmp);
      lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
      if (ListView_DeleteItem(listWnd, selectedItem))
      {
        if (ListView_InsertItem(listWnd, &lvItem) != -1)
        {
          ListView_SetItemText(listWnd, lvItem.iItem, 1, tmp);

          if (ELAppletFileVersion(tmp, &versionInfo))
          {
            ListView_SetItemText(listWnd, lvItem.iItem, 2, versionInfo.Version);
          }

          saveCount++;
          deleteCount++;

          ret = true;
        }
      }
    }
    else
    {
      if (!FindListSubItem(listWnd, 1, tmp))
      {
        lvItem.iItem = ListView_GetItemCount(listWnd);
        lvItem.iSubItem = 0;
        lvItem.pszText = GetLaunchItemState(tmp);
        lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
        if (ListView_InsertItem(listWnd, &lvItem) != -1)
        {
          ListView_SetItemText(listWnd, lvItem.iItem, 1, tmp);

          if (ELAppletFileVersion(tmp, &versionInfo))
          {
            ListView_SetItemText(listWnd, lvItem.iItem, 2, versionInfo.Version);
          }

          saveCount++;
          deleteCount++;

          ret = true;
        }
      }
      else
      {
        swprintf(error, TEXT("%ls is already in the applet launch list"), tmp);
        ELMessageBox(hwndDlg, error, TEXT("emergeCore"),
                     ELMB_OK | ELMB_ICONERROR | ELMB_MODAL);
      }

      SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
    }
  }

  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(listWnd, true);

  edit = false;

  return ret;
}

bool LaunchEditor::DoLaunchBrowse(HWND hwndDlg)
{
  bool ret = false;
  OPENFILENAME ofn;
  std::wstring path;
  WCHAR tmp[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  std::wstring workingPath;

  ZeroMemory(tmp, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));

  path = ELGetCurrentPath();

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
  GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_PATH);
  workingPath = tmp;
  workingPath = ELExpandVars(workingPath);
  if (ELParseCommand(workingPath.c_str(), program, arguments))
  {
    wcscpy(tmp, program);
  }
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = TEXT("Browse For Launch Applet");
  ofn.lpstrDefExt = NULL;
  ofn.lpstrInitialDir = path.c_str();
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
  {
    ELUnExpandVars(tmp);
    std::wstring workingTmp = ELGetRelativePath(tmp);
    SetDlgItemText(hwndDlg, IDC_APPLET, workingTmp.c_str());

    ret = true;
  }

  return ret;
}

BOOL LaunchEditor::PopulateFields(HWND hwndDlg, int index)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  WCHAR applet[MAX_PATH];

  ListView_GetItemText(listWnd, index, 1, applet, MAX_PATH);
  SetDlgItemText(hwndDlg, IDC_APPLET, applet);

  return TRUE;
}

BOOL LaunchEditor::DoNotify(HWND hwndDlg, LPARAM lParam)
{
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPAPP);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNAPP);
  HWND startWnd = GetDlgItem(hwndDlg, IDC_STARTAPP);
  HWND stopWnd = GetDlgItem(hwndDlg, IDC_STOPAPP);
  HWND infoWnd = GetDlgItem(hwndDlg, IDC_INFOAPP);
  HWND gatherWnd = GetDlgItem(hwndDlg, IDC_GATHERAPP);
  HWND configWnd = GetDlgItem(hwndDlg, IDC_CONFIGAPP);

  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
  {
  case LVN_ITEMCHANGED:
    selectedItem = ((LPNMLISTVIEW)lParam)->iItem;
    EnableWindow(editWnd, true);
    EnableWindow(delWnd, true);
    EnableWindow(upWnd, true);
    EnableWindow(downWnd, true);
    EnableWindow(startWnd, true);
    EnableWindow(stopWnd, true);
    EnableWindow(infoWnd, true);
    EnableWindow(gatherWnd, true);
    EnableWindow(configWnd, true);
    return PopulateFields(hwndDlg, selectedItem);

    // Disable Right click menu for now
    //case NM_RCLICK:
    //return DoRightClick(hwndDlg, ((LPNMITEMACTIVATE)lParam)->iItem);

  case PSN_APPLY:
    if (!CheckFields(hwndDlg))
    {
      SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
      return 1;
    }

    if (UpdateLaunch(hwndDlg))
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

  case PSN_QUERYCANCEL:
    if (CheckFields(hwndDlg) && CheckSaveCount(hwndDlg))
    {
      SetWindowLong(hwndDlg, DWLP_MSGRESULT, FALSE);
    }
    else
    {
      SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
    }
    return TRUE;
  }

  return FALSE;
}

BOOL LaunchEditor::DoRightClick(HWND hwndDlg, int index)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  WCHAR tmp[MAX_LINE_LENGTH], applet[MAX_PATH], appletName[MAX_PATH];
  VERSIONINFO versionInfo;
  HMENU appletMenu = CreatePopupMenu();
  POINT mousePt;
  bool start = false;

  AppendMenu(appletMenu, MF_STRING, 1, TEXT("About"));
  AppendMenu(appletMenu, MF_STRING, 2, TEXT("Gather"));

  ListView_GetItemText(listWnd, index, 1, applet, MAX_PATH);
  ListView_GetItemText(listWnd, index, 0, tmp, MAX_LINE_LENGTH);
  if (_wcsicmp(tmp, TEXT("stopped")) == 0)
  {
    AppendMenu(appletMenu, MF_STRING, 3, TEXT("Start"));
    start = true;
  }
  else
  {
    AppendMenu(appletMenu, MF_STRING, 3, TEXT("Stop"));
  }

  wcscpy(appletName, PathFindFileName(applet));
  PathRemoveExtension(appletName);
  GetCursorPos(&mousePt);

  if (ELAppletFileVersion(applet, &versionInfo))
  {
    swprintf(tmp, TEXT("%ls\n\nVersion: %ls\n\nAuthor: %ls"),
             versionInfo.Description,
             versionInfo.Version,
             versionInfo.Author);
  }

  switch (TrackPopupMenuEx(appletMenu, TPM_RETURNCMD, mousePt.x, mousePt.y,
                           hwndDlg, NULL))
  {
  case 1:
    ELMessageBox(hwndDlg, tmp, appletName,
                 ELMB_ICONQUESTION | ELMB_OK | ELMB_MODAL);
    break;
  case 2:
    selectedApplet = applet;
    selectedApplet = ELExpandVars(selectedApplet);
    selectedApplet = ELGetAbsolutePath(selectedApplet);
    EnumWindows(GatherApplet, reinterpret_cast<LPARAM>(this));
    break;
  case 3:
    selectedApplet = applet;
    selectedApplet = ELExpandVars(selectedApplet);
    selectedApplet = ELGetAbsolutePath(selectedApplet);
    if (start)
    {
      DoLaunchStart(listWnd, index);
    }
    else
    {
      DoLaunchStop(listWnd, index);
    }
    break;
  }

  DestroyMenu(appletMenu);

  return TRUE;
}
