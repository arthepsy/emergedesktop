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

#include "LaunchEditor.h"

BOOL CALLBACK LaunchEditor::LaunchDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static LaunchEditor *pLaunchEditor = NULL;

  switch (message)
    {
    case WM_INITDIALOG:
      pLaunchEditor = reinterpret_cast<LaunchEditor*>(lParam);
      if (!pLaunchEditor)
        break;
      return pLaunchEditor->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pLaunchEditor->DoLaunchCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pLaunchEditor->DoNotify(hwndDlg, lParam);
    }

  return FALSE;
}

BOOL CALLBACK LaunchEditor::StartedAppletCheck(HWND hwnd, LPARAM lParam)
{
  WindowList *windowList = reinterpret_cast<WindowList*>(lParam);

  if (ELIsApplet(hwnd))
    windowList->push_back(hwnd);

  return true;
}

BOOL CALLBACK LaunchEditor::AppletCheck(HWND hwnd, LPARAM lParam)
{
  LaunchEditor *pLaunchEditor = reinterpret_cast<LaunchEditor*>(lParam);
  WCHAR fileName[MAX_PATH];

  if (!ELGetWindowApp(hwnd, fileName, true))
    return true;

  if (_wcsicmp(fileName, pLaunchEditor->GetSelectedApplet().c_str()) == 0)
    SendMessage(hwnd, WM_NCDESTROY, 0, 0);

  return true;
}

BOOL CALLBACK LaunchEditor::GatherApplet(HWND hwnd, LPARAM lParam)
{
  LaunchEditor *pLaunchEditor = reinterpret_cast<LaunchEditor*>(lParam);
  WCHAR fileName[MAX_PATH];
  POINT cursorPt;
  RECT appletRect;

  if (!ELGetWindowApp(hwnd, fileName, true))
    return true;

  GetCursorPos(&cursorPt);
  GetClientRect(hwnd, &appletRect);

  if (_wcsicmp(fileName, pLaunchEditor->GetSelectedApplet().c_str()) == 0)
    SetWindowPos(hwnd, HWND_TOPMOST, cursorPt.x - (appletRect.right / 2),
                 cursorPt.y - (appletRect.bottom / 2), 0, 0,
                 SWP_NOSIZE|SWP_NOSENDCHANGING);

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
  delIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_DEL), IMAGE_ICON, 16, 16, 0);
  browseIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_BROWSE), IMAGE_ICON, 16, 16, 0);
  upIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_UP), IMAGE_ICON, 16, 16, 0);
  downIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_DOWN), IMAGE_ICON, 16, 16, 0);
  saveIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_SAVE), IMAGE_ICON, 16, 16, 0);
  abortIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ABORT), IMAGE_ICON, 16, 16, 0);

  xmlFile = TEXT("%ThemeDir%\\");
  xmlFile += TEXT("emergeCore.xml");
}

LaunchEditor::~LaunchEditor()
{
  if (addIcon)
    DestroyIcon(addIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (upIcon)
    DestroyIcon(upIcon);
  if (downIcon)
    DestroyIcon(downIcon);
  if (browseIcon)
    DestroyIcon(browseIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (abortIcon)
    DestroyIcon(abortIcon);

  DestroyWindow(toolWnd);
}

int LaunchEditor::Show()
{
  return (int)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_LAUNCH), mainWnd, (DLGPROC)LaunchDlgProc, (LPARAM)this);
}

BOOL LaunchEditor::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y, ret;
  LVCOLUMN lvCol;
  TOOLINFO ti;
  dlgWnd = hwndDlg;

  ZeroMemory(&ti, sizeof(TOOLINFO));

  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDAPP);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPAPP);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNAPP);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);

  saveCount = 0;
  deleteCount = 0;

  GetWindowRect(hwndDlg, &rect);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH;
  lvCol.pszText = (WCHAR*)TEXT("State");
  lvCol.cx = 50;
  ret = ListView_InsertColumn(listWnd, 0, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Applet");
  lvCol.cx = MAX_PATH;
  ret = ListView_InsertColumn(listWnd, 1, &lvCol);

  lvCol.pszText = (WCHAR*)TEXT("Version");
  lvCol.cx = 100;
  ret = ListView_InsertColumn(listWnd, 2, &lvCol);

  ret = ListView_SetExtendedListViewStyle(listWnd,  LVS_EX_FULLROWSELECT);

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (browseIcon)
    SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
  if (upIcon)
    SendMessage(upWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)upIcon);
  if (downIcon)
    SendMessage(downWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)downIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add Launch Applet");
  GetClientRect(addWnd, &ti.rect);
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

  EnableWindow(upWnd, false);
  EnableWindow(downWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);

  return TRUE;
}

bool LaunchEditor::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("All current modifications will be lost.  To save and exit press OK.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeCore"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool LaunchEditor::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);

  if (!IsWindowEnabled(appletWnd))
    return true;

  if (GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_LINE_LENGTH) != 0)
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("The current applet will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeCore"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

BOOL LaunchEditor::DoLaunchCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam UNUSED)
{
  switch (LOWORD(wParam))
    {
    case IDOK:
      if (!CheckFields(hwndDlg))
        break;
      if (!UpdateLaunch(hwndDlg))
        break;
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDCANCEL:
      if (!CheckFields(hwndDlg))
        break;
      if (!CheckSaveCount(hwndDlg))
        break;
      EndDialog(hwndDlg, wParam);
      return TRUE;
    case IDC_DELAPP:
      return DoLaunchDelete(hwndDlg);
    case IDC_ADDAPP:
      return DoLaunchAdd(hwndDlg);
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
    }

  return FALSE;
}

bool LaunchEditor::DoLaunchMove(HWND hwndDlg, bool up)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  int i = 0, lvret;
  bool ret = false;
  LVITEM lvItem;
  WCHAR applet[MAX_PATH], version[MAX_PATH], state[MAX_PATH];

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only move one item at a time."),
                   (WCHAR*)TEXT("emergeCore"), ELMB_OK|MB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  saveCount++;

  lvItem.mask = LVIF_TEXT;

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          ret = true;

          ListView_GetItemText(listWnd, i, 0, state, MAX_PATH);
          ListView_GetItemText(listWnd, i, 1, applet, MAX_PATH);
          ListView_GetItemText(listWnd, i, 2, version, MAX_PATH);

          if (up)
            lvItem.iItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          else
            lvItem.iItem = ListView_GetNextItem(listWnd, i, LVNI_BELOW);

          if (lvItem.iItem == -1)
            break;

          lvItem.iSubItem = 0;
          lvItem.pszText = state;
          lvItem.cchTextMax = MAX_PATH;

          lvret = ListView_DeleteItem(listWnd, i);

          lvret = ListView_InsertItem(listWnd, &lvItem);
          ListView_SetItemText(listWnd, lvItem.iItem, 1, applet);
          ListView_SetItemText(listWnd, lvItem.iItem, 2, version);

          ListView_SetItemState(listWnd, lvItem.iItem, LVIS_SELECTED, LVIS_SELECTED);
          lvret = ListView_EnsureVisible(listWnd, lvItem.iItem, FALSE);

          break;
        }
      else
        i++;
    }

  return ret;
}

bool LaunchEditor::DoLaunchStart(HWND listWnd, int index)
{
  bool ret = false;

  if (ELExecute((WCHAR*)selectedApplet.c_str()))
    {
      ListView_SetItemText(listWnd, index, 0, (WCHAR*)TEXT("Started"));
      ret = true;
    }

  return ret;
}

bool LaunchEditor::DoLaunchStop(HWND listWnd, int index)
{
  EnumWindows(AppletCheck, reinterpret_cast<LPARAM>(this));
  ListView_SetItemText(listWnd, index, 0, (WCHAR*)TEXT("Stopped"));

  return true;
}

bool LaunchEditor::GetLaunchAppletName(int index, WCHAR *applet)
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
  TiXmlElement *first, *section;
  std::wstring theme = ELGetThemeName(), oldTheme, newThemePath, oldThemePath;

  if ((saveCount == 0) && (deleteCount == 0))
    return true;

  if (!ELIsModifiedTheme(theme))
    oldTheme = theme;

  if (ELSetModifiedTheme(theme))
    {
      oldThemePath = TEXT("%EmergeDir%\\themes\\") + oldTheme;
      oldThemePath += TEXT("\\*");
      newThemePath = TEXT("%ThemeDir%");

      ELFileOp(mainWnd, FO_COPY, oldThemePath, newThemePath);
    }

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      int i = 0;
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Launch"), true);

      if (section)
        {
          first = ELGetFirstXMLElement(section);
          while (first)
            {
              ELRemoveXMLElement(first);
              first = ELGetFirstXMLElement(section);
            }
        }

      // Loop while there are entries in the key
      while (i < ListView_GetItemCount(listWnd))
        {
          ListView_GetItemText(listWnd, i, 1, applet, MAX_PATH);
          first = ELSetFirstXMLElement(section, TEXT("item"));
          if (first)
            ELWriteXMLStringValue(first, TEXT("Command"), applet);

          i++;
        }
      ELWriteXMLConfig(configXML.get());
    }

  return true;
}

bool LaunchEditor::PopulateList(HWND listWnd)
{
  bool found = false;
  WCHAR data[MAX_LINE_LENGTH];
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *first, *sibling, *section;

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Launch"), false);

      if (section)
        {
          first = ELGetFirstXMLElement(section);

          if (first)
            {
              DWORD index = 0;

              if (ELReadXMLStringValue(first, TEXT("Command"), data, TEXT("")))
                {
                  found = true;
                  InsertListViewItem(listWnd, index, data);
                }

              sibling = ELGetSiblingXMLElement(first);
              while (sibling)
                {
                  index++;
                  first = sibling;

                  if (ELReadXMLStringValue(first, TEXT("Command"), data, TEXT("")))
                    InsertListViewItem(listWnd, index, data);

                  sibling = ELGetSiblingXMLElement(first);
                }
            }
        }
    }

  if (!found)
    {
      InsertListViewItem(listWnd, 0, TEXT("emergeTasks.exe"));
      InsertListViewItem(listWnd, 1, TEXT("emergeTray.exe"));
      InsertListViewItem(listWnd, 2, TEXT("emergeDesktop.exe"));
    }

  return found;
}

void LaunchEditor::InsertListViewItem(HWND listWnd, int index, const WCHAR *item)
{
  VERSIONINFO versionInfo;
  LVITEM lvItem;
  WCHAR tmp[MAX_PATH];
  int ret;
  WindowList windowList;
  WindowList::iterator iter;
  std::wstring workingItem, expandedItem;

  wcscpy(tmp, item);
  if (ELPathIsRelative(tmp))
    ELConvertAppletPath(tmp, CTP_FULL);
  workingItem = tmp;
  expandedItem = ELExpandVars(workingItem);

  EnumWindows(StartedAppletCheck, (LPARAM)&windowList);

  iter = windowList.begin();
  while (iter != windowList.end())
    {
      if (ELGetWindowApp((*iter), tmp, true))
        {
          if (wcsicmp(expandedItem.c_str(), tmp) == 0)
            break;
        }
      iter++;
    }

  lvItem.mask = LVIF_TEXT;
  lvItem.iItem = index;
  lvItem.iSubItem = 0;
  if (iter == windowList.end())
    lvItem.pszText = (WCHAR*)TEXT("Stopped");
  else
    lvItem.pszText = (WCHAR*)TEXT("Started");
  lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
  ret = ListView_InsertItem(listWnd, &lvItem);

  ListView_SetItemText(listWnd, lvItem.iItem, 1, (WCHAR*)workingItem.c_str());

  if (ELAppletFileVersion(tmp, &versionInfo))
    ListView_SetItemText(listWnd, lvItem.iItem, 2, versionInfo.Version);

  windowList.clear();
}

bool LaunchEditor::DoLaunchDelete(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPAPP);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNAPP);
  bool ret = false;
  int i = 0, prevItem = 0, lvret;

  if (ListView_GetSelectedCount(listWnd) > 1)
    {
      ELMessageBox(hwndDlg, (WCHAR*)TEXT("You can only delete one item at a time."),
                   (WCHAR*)TEXT("emergeCore"), ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

      return ret;
    }

  while (i < ListView_GetItemCount(listWnd))
    {
      if (ListView_GetItemState(listWnd, i, LVIS_SELECTED))
        {
          ret = true;
          prevItem = ListView_GetNextItem(listWnd, i, LVNI_ABOVE);
          lvret = ListView_DeleteItem(listWnd, i);
          deleteCount++;

          ListView_SetItemState(listWnd, i, LVIS_SELECTED,
                                LVIS_SELECTED);
          if (!ListView_EnsureVisible(listWnd, i, FALSE))
            {
              if (prevItem != -1)
                {
                  ListView_SetItemState(listWnd, prevItem, LVIS_SELECTED,
                                        LVIS_SELECTED);
                  lvret = ListView_EnsureVisible(listWnd, prevItem, FALSE);
                }
            }

          break;
        }
      else
        i++;
    }

  if (ListView_GetItemCount(listWnd) == 0)
    {
      EnableWindow(delWnd, false);
      EnableWindow(upWnd, false);
      EnableWindow(downWnd, false);
    }

  return ret;
}

bool LaunchEditor::DoLaunchAdd(HWND hwndDlg)
{
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);

  EnableWindow(appletWnd, true);
  EnableWindow(browseWnd, true);
  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);

  SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));

  return true;
}

bool LaunchEditor::DoLaunchAbort(HWND hwndDlg)
{
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);

  SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);

  return true;
}

bool LaunchEditor::DoLaunchSave(HWND hwndDlg)
{
  HWND listWnd = GetDlgItem(hwndDlg, IDC_APPLETLIST);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEAPP);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTAPP);
  HWND appletWnd = GetDlgItem(hwndDlg, IDC_APPLET);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_BROWSE);
  bool ret = false;
  LVFINDINFO lvFI;
  LVITEM lvItem;
  WCHAR tmp[MAX_LINE_LENGTH], error[MAX_LINE_LENGTH];
  VERSIONINFO versionInfo;

  ZeroMemory(tmp, MAX_LINE_LENGTH);

  lvFI.flags = LVFI_STRING;
  lvItem.mask = LVIF_TEXT;

  GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_LINE_LENGTH);
  if (wcslen(tmp) > 0)
    {
      lvFI.psz = tmp;

      if (ListView_FindItem(listWnd, 0, &lvFI) == -1)
        {
          lvItem.iItem = ListView_GetItemCount(listWnd);
          lvItem.iSubItem = 0;
          lvItem.pszText = (WCHAR*)TEXT("Started");
          lvItem.cchTextMax = (int)wcslen(lvItem.pszText);
          if (ListView_InsertItem(listWnd, &lvItem) != -1)
            {
              ListView_SetItemText(listWnd, lvItem.iItem, 1, tmp);

              if (ELAppletFileVersion(tmp, &versionInfo))
                ListView_SetItemText(listWnd, lvItem.iItem, 1, versionInfo.Version);

              saveCount++;
              deleteCount++;

              ret = true;
            }
        }
      else
        {
          swprintf(error, TEXT("%s is already in the applet launch list"), tmp);
          ELMessageBox(hwndDlg, error, (WCHAR*)TEXT("emergeCore"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
        }

      SetDlgItemText(hwndDlg, IDC_APPLET, TEXT(""));
    }

  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(appletWnd, false);
  EnableWindow(browseWnd, false);

  return ret;
}

bool LaunchEditor::DoLaunchBrowse(HWND hwndDlg)
{
  bool ret = false;
  OPENFILENAME ofn;
  WCHAR tmp[MAX_PATH], path[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  std::wstring workingPath;

  ZeroMemory(tmp, MAX_PATH);
  ZeroMemory(&ofn, sizeof(ofn));

  ELGetCurrentPath(path);

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwndDlg;
  ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
  GetDlgItemText(hwndDlg, IDC_APPLET, tmp, MAX_PATH);
  workingPath = tmp;
  workingPath = ELExpandVars(workingPath);
  if (ELParseCommand(workingPath.c_str(), program, arguments))
    wcscpy(tmp, program);
  ofn.lpstrFile = tmp;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = TEXT("Browse For Launch Applet");
  ofn.lpstrDefExt = NULL;
  ofn.lpstrInitialDir = path;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

  if (GetOpenFileName(&ofn))
    {
      ELUnExpandVars(tmp);
      SetDlgItemText(hwndDlg, IDC_APPLET, tmp);

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
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELAPP);
  HWND upWnd = GetDlgItem(hwndDlg, IDC_UPAPP);
  HWND downWnd = GetDlgItem(hwndDlg, IDC_DOWNAPP);

  switch (((LPNMITEMACTIVATE)lParam)->hdr.code)
    {
    case LVN_ITEMCHANGED:
      EnableWindow(delWnd, true);
      EnableWindow(upWnd, true);
      EnableWindow(downWnd, true);
      return PopulateFields(hwndDlg, ((LPNMLISTVIEW)lParam)->iItem);

    case NM_RCLICK:
      return DoRightClick(hwndDlg, ((LPNMITEMACTIVATE)lParam)->iItem);
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
  if (wcsicmp(tmp, TEXT("stopped")) == 0)
    {
      AppendMenu(appletMenu, MF_STRING, 3, TEXT("Start"));
      start = true;
    }
  else
    AppendMenu(appletMenu, MF_STRING, 3, TEXT("Stop"));

  wcscpy(appletName, PathFindFileName(applet));
  PathRemoveExtension(appletName);
  GetCursorPos(&mousePt);

  if (ELAppletFileVersion(applet, &versionInfo))
    {
      swprintf(tmp, TEXT("%s\n\nVersion: %s\n\nAuthor: %s"),
               versionInfo.Description,
               versionInfo.Version,
               versionInfo.Author);
    }

  switch (TrackPopupMenuEx(appletMenu, TPM_RETURNCMD, mousePt.x, mousePt.y,
                           hwndDlg, NULL))
    {
    case 1:
      ELMessageBox(hwndDlg, tmp, appletName,
                   ELMB_ICONQUESTION|ELMB_OK|ELMB_MODAL);
      break;
    case 2:
      selectedApplet = applet;
      selectedApplet = ELExpandVars(selectedApplet);
      EnumWindows(GatherApplet, reinterpret_cast<LPARAM>(this));
      break;
    case 3:
      selectedApplet = applet;
      selectedApplet = ELExpandVars(selectedApplet);
      if (start)
        DoLaunchStart(listWnd, index);
      else
        DoLaunchStop(listWnd, index);
      break;
    }

  DestroyMenu(appletMenu);

  return TRUE;
}
