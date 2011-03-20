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

#include "MenuEditor.h"

TreeItemMap treeMap;

INT_PTR CALLBACK MenuEditor::MenuEditorDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  static MenuEditor *pMenuEditor = NULL;
  PROPSHEETPAGE *psp;

  switch (message)
    {
    case WM_INITDIALOG:
      psp = (PROPSHEETPAGE*)lParam;
      pMenuEditor = reinterpret_cast<MenuEditor*>(psp->lParam);
      if (!pMenuEditor)
        break;
      return pMenuEditor->DoInitDialog(hwndDlg);

    case WM_COMMAND:
      return pMenuEditor->DoMenuCommand(hwndDlg, wParam, lParam);

    case WM_NOTIFY:
      return pMenuEditor->DoMenuNotify(hwndDlg, wParam, lParam);

    case WM_MOUSEMOVE:
      return pMenuEditor->DoMenuMouseMove(hwndDlg, wParam, lParam);

    case WM_TIMER:
      return pMenuEditor->DoMenuTimer(hwndDlg, wParam, lParam);

    case WM_LBUTTONUP:
      return pMenuEditor->DoMenuMouseLButtonUp(hwndDlg, wParam, lParam);
    }

  return FALSE;
}

MenuEditor::MenuEditor(HINSTANCE hInstance)
{
  (*this).hInstance = hInstance;
  edit = false;
  dragging = false;

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

  ExtractIconEx(TEXT("emergeIcons.dll"), 2, NULL, &addIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 5, NULL, &editIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 3, NULL, &delIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 9, NULL, &saveIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 1, NULL, &abortIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 6, NULL, &browseIcon, 1);
  ExtractIconEx(TEXT("emergeIcons.dll"), 18, NULL, &fileIcon, 1);

  dialogVisible = false;

  xmlFile = TEXT("%EmergeDir%\\files\\emergeWorkspace.xml");
}

MenuEditor::~MenuEditor()
{
  treeMap.clear();

  if (addIcon)
    DestroyIcon(addIcon);
  if (editIcon)
    DestroyIcon(editIcon);
  if (delIcon)
    DestroyIcon(delIcon);
  if (saveIcon)
    DestroyIcon(saveIcon);
  if (abortIcon)
    DestroyIcon(abortIcon);
  if (fileIcon)
    DestroyIcon(fileIcon);
  if (browseIcon)
    DestroyIcon(browseIcon);

  DestroyWindow(toolWnd);
}

BOOL MenuEditor::DoInitDialog(HWND hwndDlg)
{
  RECT rect;
  int x, y;
  TOOLINFO ti;

  ZeroMemory(&ti, sizeof(TOOLINFO));
  GetWindowRect(hwndDlg, &rect);

  saveCount = 0;
  deleteCount = 0;

  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND typeTextWnd = GetDlgItem(hwndDlg, IDC_TYPETEXT);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND valueTextWnd = GetDlgItem(hwndDlg, IDC_VALUETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND dirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND dirBrowseWnd = GetDlgItem(hwndDlg, IDC_DIRBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);

  x = (GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2);
  y = (GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2);
  SetWindowPos(hwndDlg, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
  ELStealFocus(hwndDlg);

  PopulateCommands(commandWnd);
  ShowWindow(commandWnd, false);

  PopulateSpecialFolders(specialFoldersWnd);
  ShowWindow(specialFoldersWnd, false);

  SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));

  if (addIcon)
    SendMessage(addWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)addIcon);
  if (editIcon)
    SendMessage(editWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)editIcon);
  if (delIcon)
    SendMessage(delWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)delIcon);
  if (saveIcon)
    SendMessage(saveWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)saveIcon);
  if (abortIcon)
    SendMessage(abortWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)abortIcon);
  if (fileIcon)
    SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
  if (browseIcon)
    SendMessage(dirBrowseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.uFlags = TTF_SUBCLASS;
  ti.hwnd = addWnd;
  ti.uId = (ULONG_PTR)addWnd;
  ti.hinst = hInstance;
  ti.lpszText = (WCHAR*)TEXT("Add Menu Item");
  GetClientRect(addWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = editWnd;
  ti.uId = (ULONG_PTR)editWnd;
  ti.lpszText = (WCHAR*)TEXT("Edit Menu Item");
  GetClientRect(editWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  ti.hwnd = delWnd;
  ti.uId = (ULONG_PTR)delWnd;
  ti.lpszText = (WCHAR*)TEXT("Delete Menu Item");
  GetClientRect(delWnd, &ti.rect);
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

  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;
  ti.lpszText = (WCHAR*)TEXT("");
  GetClientRect(browseWnd, &ti.rect);
  SendMessage(toolWnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

  PopulateTree(treeWnd);
  EnableWindow(nameWnd, false);
  EnableWindow(nameTextWnd, false);
  EnableWindow(typeWnd, false);
  EnableWindow(typeTextWnd, false);
  EnableWindow(valueWnd, false);
  EnableWindow(valueTextWnd, false);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(addWnd, false);
  EnableWindow(editWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(commandWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(dirWnd, false);
  EnableWindow(workingDirTextWnd, false);
  EnableWindow(dirBrowseWnd, false);
  EnableWindow(specialFoldersWnd, false);

  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Separator"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Executable"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Internal Command"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("DateTime"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Special Folder"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Submenu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Folder Menu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Tasks Menu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Settings Menu"));
  SendMessage(typeWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Help Menu"));

  edit = false;

  return TRUE;
}

void MenuEditor::PopulateCommands(HWND commandWnd)
{
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RightDeskMenu"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("MidDeskMenu"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Quit"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Run"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Shutdown"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Lock"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Hide"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Show"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_1"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_2"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_3"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_4"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_5"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_6"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_7"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_8"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWM_9"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMUp"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMDown"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMLeft"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMRight"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VWMGather"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("EmptyBin"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Logoff"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Reboot"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Halt"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Suspend"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Hibernate"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Disconnect"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("ShowDesktop"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeUp"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeDown"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("VolumeMute"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("WorkspaceSettings"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CoreSettings"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CoreShellChanger"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CoreAbout"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("CoreAliasEditor"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Homepage"));
  SendMessage(commandWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Tutorial"));
}

void MenuEditor::PopulateSpecialFolders(HWND specialFoldersWnd)
{
  WCHAR tmp[MAX_PATH];

  if (ELGetSpecialFolder(CSIDL_PERSONAL, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_DRIVES, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_NETWORK, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_CONTROLS, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
  if (ELGetSpecialFolder(CSIDL_BITBUCKET, tmp))
    SendMessage(specialFoldersWnd, CB_ADDSTRING, 0, (LPARAM)tmp);
}

bool MenuEditor::PopulateTree(HWND treeWnd)
{
  TVINSERTSTRUCT tvInsert;

  tvInsert.hParent = NULL;
  tvInsert.hInsertAfter=TVI_ROOT;
  tvInsert.item.mask = TVIF_TEXT;
  tvInsert.item.pszText = (WCHAR*)TEXT("Right");
  rightRoot = TreeView_InsertItem(treeWnd, &tvInsert);
  BuildMenuTree(treeWnd, rightRoot, (WCHAR*)TEXT("RightMenu"));

  tvInsert.item.pszText = (WCHAR*)TEXT("Middle");
  midRoot = TreeView_InsertItem(treeWnd, &tvInsert);
  BuildMenuTree(treeWnd, midRoot, (WCHAR*)TEXT("MidMenu"));

  return true;
}

bool MenuEditor::BuildMenuTreeHelper (HWND treeWnd, HTREEITEM parent, TiXmlElement *menu)
{
  bool ret = false;
  TiXmlElement *subMenu, *xmlItem, *tmp;
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  int type;
  MENUTREEITEM menuItem;
  TVINSERTSTRUCT tvInsert;
  HTREEITEM child;

  xmlItem = ELGetFirstXMLElement(menu);
  while (xmlItem)
    {
      ret = true;
      ELReadXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
      ELReadXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), &type, 0);
      ELReadXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0"));
      ELReadXMLStringValue(xmlItem, (WCHAR*)TEXT("WorkingDir"), workingDir, (WCHAR*)TEXT("\0"));

      wcscpy(menuItem.name, name);
      menuItem.type = type;
      wcscpy(menuItem.value, value);
      wcscpy(menuItem.workingDir, workingDir);

      tvInsert.hParent = parent;
      tvInsert.hInsertAfter=TVI_LAST;
      tvInsert.item.mask = TVIF_TEXT;
      if (type == IT_SEPARATOR)
        tvInsert.item.pszText = (WCHAR*)TEXT("[Separator]");
      else if (type == IT_DATE_TIME)
        tvInsert.item.pszText = (WCHAR*)TEXT("[DateTime]");
      else
        tvInsert.item.pszText = name;

      child = TreeView_InsertItem(treeWnd, &tvInsert);

      treeMap.insert(TreeItem(child, menuItem));

      if (type == IT_XML_MENU)
        {
          subMenu = ELGetFirstXMLElementByName(xmlItem, (WCHAR*)TEXT("Submenu"));
          if (subMenu)
            BuildMenuTreeHelper(treeWnd, child, subMenu);
        }

      tmp = xmlItem;
      xmlItem = ELGetSiblingXMLElement(tmp);
    }

  return ret;
}

bool MenuEditor::BuildMenuTree(HWND treeWnd, HTREEITEM parent, WCHAR *rootElement)
{
  bool ret = false;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *rootXML, *section;

  configXML = ELOpenXMLConfig(xmlFile, false);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Menus"), false);
      if (section)
        {
          rootXML = ELGetFirstXMLElementByName(section, rootElement);
          if (rootXML)
            ret = BuildMenuTreeHelper(treeWnd, parent, rootXML);
        }
    }

  return ret;
}

bool MenuEditor::CheckSaveCount(HWND hwndDlg)
{
  if ((saveCount != 0) || (deleteCount != 0))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("All current modifications will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeWorkspace"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool MenuEditor::CheckFields(HWND hwndDlg)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);

  if (((GetDlgItemText(hwndDlg, IDC_ITEMNAME, tmp, MAX_LINE_LENGTH) != 0) &&
       IsWindowEnabled(nameWnd)) ||
      ((GetDlgItemText(hwndDlg, IDC_ITEMTYPE, tmp, MAX_LINE_LENGTH) != 0) &&
       IsWindowEnabled(typeWnd)) ||
      ((GetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp, MAX_LINE_LENGTH) != 0) &&
       IsWindowEnabled(valueWnd)))
    {
      if (ELMessageBox(hwndDlg,
                       (WCHAR*)TEXT("The current menu item will be lost.\n\nDo you wish to continue?"),
                       (WCHAR*)TEXT("emergeWorkspace"),
                       ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
        return true;
      else
        return false;
    }

  return true;
}

bool MenuEditor::GetVisible()
{
  return dialogVisible;
}

BOOL MenuEditor::DoMenuCommand(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
  switch (LOWORD(wParam))
    {
    case IDC_DELITEM:
      return DoDelItem(hwndDlg);
    case IDC_EDITITEM:
      return DoEditItem(hwndDlg);
    case IDC_ADDITEM:
      return DoAddItem(hwndDlg);
    case IDC_SAVEITEM:
      return DoSaveItem(hwndDlg);
    case IDC_ABORTITEM:
      return DoAbortItem(hwndDlg);
    case IDC_ITEMTYPE:
      if (HIWORD(wParam) == CBN_SELCHANGE)
        return DoSelChange(hwndDlg, (HWND)lParam);
      break;
    case IDC_ITEMBROWSE:
      return DoBrowseItem(hwndDlg, false);
    case IDC_DIRBROWSE:
      return DoBrowseItem(hwndDlg, true);
    }

  return FALSE;
}

bool MenuEditor::UpdateMenu(HWND hwndDlg)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  WCHAR name[MAX_LINE_LENGTH];

  if (IsWindowEnabled(nameWnd) && IsWindowEnabled(typeWnd))
    {
      UINT size = GetDlgItemText(hwndDlg, IDC_ITEMNAME, name, MAX_LINE_LENGTH);
      int index = (int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0);
      if ((size > 0) && (index != CB_ERR))
        {
          if (ELMessageBox(hwndDlg,
                           (WCHAR*)TEXT("Do you wish to save this item?"),
                           (WCHAR*)TEXT("emergeWorkspace"),
                           ELMB_YESNO|ELMB_ICONQUESTION|ELMB_MODAL) == IDYES)
            DoSaveItem(hwndDlg);
        }
    }

  WriteMenu(treeWnd, rightRoot, (WCHAR*)TEXT("RightMenu"));
  WriteMenu(treeWnd, midRoot, (WCHAR*)TEXT("MidMenu"));

  treeMap.clear();

  return true;
}

bool MenuEditor::WriteMenuHelper(TiXmlElement *section, HWND treeWnd, HTREEITEM parent)
{
  TreeItemMap::iterator iter;
  UINT i = 0;
  HTREEITEM child;

  child = TreeView_GetChild(treeWnd, parent);

  TiXmlElement *item, *subItem;
  bool readXML = false;

  while (child)
    {
      iter = treeMap.find(child);

      if (iter->second.type == MAX_LINE_LENGTH)
        {
          child = TreeView_GetNextSibling(treeWnd, child);
          continue;
        }

      if (iter != treeMap.end())
        {
          item = ELSetFirstXMLElement(section, TEXT("item"));

          if (item)
            {
              readXML = true;

              if (wcslen(iter->second.name) > 0)
                ELWriteXMLStringValue(item, (WCHAR*)TEXT("Name"),
                                      iter->second.name);
              ELWriteXMLIntValue(item, (WCHAR*)TEXT("Type"), iter->second.type);
              if (wcslen(iter->second.value) > 0)
                ELWriteXMLStringValue(item, (WCHAR*)TEXT("Value"),
                                      iter->second.value);
              if (wcslen(iter->second.workingDir) > 0)
                ELWriteXMLStringValue(item, (WCHAR*)TEXT("WorkingDir"),
                                      iter->second.workingDir);

              if (iter->second.type == IT_XML_MENU)
                {
                  subItem = ELSetFirstXMLElement(item, TEXT("Submenu"));
                  WriteMenuHelper(subItem, treeWnd, child);
                }

              i++;
            }

          treeMap.erase(iter);
        }

      child = TreeView_GetNextSibling(treeWnd, child);
    }

  return readXML;
}

bool MenuEditor::WriteMenu(HWND treeWnd, HTREEITEM parent, WCHAR *menu)
{
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section, *menuRoot;
  bool readXML = false;

  xmlFile = TEXT("%EmergeDir%\\files\\emergeWorkspace.xml");

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Menus"), true);

      if (section)
        {
          menuRoot = ELGetFirstXMLElementByName(section, menu);

          if (menuRoot)
            {
              ELRemoveXMLElement(menuRoot);
              menuRoot = ELSetFirstXMLElement(section, menu);
              readXML = WriteMenuHelper(menuRoot, treeWnd, parent);
            }
        }

      ELWriteXMLConfig(configXML.get());
    }

  return readXML;
}

BOOL MenuEditor::DoMenuNotify(HWND hwndDlg, WPARAM wParam UNUSED, LPARAM lParam)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);
  TreeItemMap::iterator iter;
  TVITEMEX pItem;
  WCHAR value[MAX_LINE_LENGTH];

  switch (((LPNMHDR)lParam)->code)
    {
    case TVN_SELCHANGING:
    {
      selected = ((LPNMTREEVIEW)lParam)->itemNew.hItem;

      iter = treeMap.find(selected);

      if ((selected == rightRoot) || (selected == midRoot))
        {
          if (TreeView_SelectDropTarget(treeWnd, selected))
            {
              if (IsWindowEnabled(treeWnd))
                {
                  EnableWindow(addWnd, true);
                  EnableWindow(editWnd, false);
                  EnableWindow(delWnd, false);
                }
            }
        }
      else if (iter != treeMap.end())
        {
          if (IsWindowEnabled(treeWnd))
            {
              EnableWindow(addWnd, true);
              EnableWindow(editWnd, true);
              EnableWindow(delWnd, true);
            }

          if (TreeView_SelectDropTarget(treeWnd, iter->first))
            {
              SetDlgItemText(hwndDlg, IDC_ITEMNAME, iter->second.name);
              SetDlgItemText(hwndDlg, IDC_ITEMVALUE, iter->second.value);
              SetDlgItemText(hwndDlg, IDC_WORKINGDIR, iter->second.workingDir);
              SendMessage(typeWnd, CB_SETCURSEL,
                          (WPARAM)GetTypeValue(iter->second.type), 0);
              ShowFields(hwndDlg, GetTypeValue(iter->second.type));
              if (iter->second.type == IT_INTERNAL_COMMAND)
                {
                  SendMessage(commandWnd, CB_SETCURSEL,
                              SendMessage(commandWnd,
                                          CB_FINDSTRINGEXACT,
                                          (WPARAM)-1,
                                          (LPARAM)iter->second.value),
                              0);
                }
              else if (iter->second.type == IT_SPECIAL_FOLDER)
                {
                  int folder = ELIsSpecialFolder(iter->second.value);
                  if (ELGetSpecialFolder(folder, value))
                    SendMessage(specialFoldersWnd, CB_SETCURSEL,
                                SendMessage(specialFoldersWnd,
                                            CB_FINDSTRINGEXACT,
                                            (WPARAM)-1,
                                            (LPARAM)value),
                                0);
                }
              else
                {
                  SetDlgItemText(hwndDlg, IDC_ITEMVALUE, iter->second.value);
                  SetTooltip(browseWnd, iter->second.type);
                }
            }
        }
    }
    break;

    case TVN_BEGINDRAG:
    {
      selected = ((LPNMTREEVIEW)lParam)->itemNew.hItem;

      if ((selected == rightRoot) || (selected == midRoot))
        return FALSE;

      pItem.mask = TVIF_STATE | TVIF_HANDLE;
      pItem.stateMask = TVIS_BOLD;
      pItem.hItem = selected;
      pItem.state = TVIS_BOLD;
      SendMessage(treeWnd, TVM_SETITEM, 0, (LPARAM)&pItem);

      SetCapture(hwndDlg);
      dragging = true;

      return TRUE;
    }

    case PSN_APPLY:
      if (!CheckFields(hwndDlg))
        {
          SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
          return 1;
        }

      if (UpdateMenu(hwndDlg))
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

    case PSN_QUERYCANCEL:
      if (CheckFields(hwndDlg) && CheckSaveCount(hwndDlg))
        SetWindowLong(hwndDlg,DWLP_MSGRESULT,FALSE);
      else
        SetWindowLong(hwndDlg,DWLP_MSGRESULT,TRUE);
      return TRUE;
    }

  return FALSE;
}

int MenuEditor::GetTypeValue(UINT type)
{
  switch (type)
    {
    case IT_SEPARATOR:
      return 0;
    case IT_EXECUTABLE:
      return 1;
    case IT_INTERNAL_COMMAND:
      return 2;
    case IT_DATE_TIME:
      return 3;
    case IT_SPECIAL_FOLDER:
      return 4;
    case IT_XML_MENU:
      return 5;
    case IT_FILE_MENU:
      return 6;
    case IT_TASKS_MENU:
      return 7;
    case IT_SETTINGS_MENU:
      return 8;
    case IT_HELP_MENU:
      return 9;
    }

  return -1;
}

UINT MenuEditor::GetValueType(int value)
{
  switch (value)
    {
    case 0:
      return IT_SEPARATOR;
    case 1:
      return IT_EXECUTABLE;
    case 2:
      return IT_INTERNAL_COMMAND;
    case 3:
      return IT_DATE_TIME;
    case 4:
      return IT_SPECIAL_FOLDER;
    case 5:
      return IT_XML_MENU;
    case 6:
      return IT_FILE_MENU;
    case 7:
      return IT_TASKS_MENU;
    case 8:
      return IT_SETTINGS_MENU;
    case 9:
      return IT_HELP_MENU;
    }

  return 0;
}

BOOL MenuEditor::DoMenuMouseMove(HWND hwndDlg, WPARAM wParam UNUSED, LPARAM lParam)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);

  if (dragging)
    {
      TVHITTESTINFO hitTest;

      hitTest.pt.x = LOWORD(lParam);
      hitTest.pt.y = HIWORD(lParam);

      HTREEITEM htItem = TreeView_HitTest(treeWnd, &hitTest);

      if (htItem != NULL)
        {
          if (SetTimer(hwndDlg, 1, 500, NULL))
            hoverItem = hitTest.hItem;
          if (TreeView_SelectDropTarget(treeWnd, hitTest.hItem))
            {
              (void)TreeView_EnsureVisible(treeWnd, TreeView_GetNextVisible(treeWnd, hitTest.hItem));
              (void)TreeView_EnsureVisible(treeWnd, TreeView_GetPrevVisible(treeWnd, hitTest.hItem));
              return TRUE;
            }
        }
    }

  return FALSE;
}

BOOL MenuEditor::DoMenuTimer(HWND hwndDlg, WPARAM wParam UNUSED, LPARAM lParam UNUSED)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);

  if (dragging && TreeView_GetChild(treeWnd, hoverItem))
    (void)TreeView_Expand(treeWnd, hoverItem, TVE_EXPAND);

  KillTimer(hwndDlg, 1);

  return TRUE;
}

BOOL MenuEditor::DoMenuMouseLButtonUp(HWND hwndDlg, WPARAM wParam UNUSED, LPARAM lParam UNUSED)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);

  TVHITTESTINFO hitTest;
  TVINSERTSTRUCT tvInsert;
  TreeItemMap::iterator iter;

  hitTest.pt.x = LOWORD(lParam);
  hitTest.pt.y = HIWORD(lParam);

  HTREEITEM htItem = TreeView_HitTest(treeWnd, &hitTest);

  if (htItem != NULL)
    {
      if (dragging)
        {
          HTREEITEM parent, item, prev;
          iter = treeMap.find(selected);

          if (hitTest.hItem == rightRoot)
            {
              parent = rightRoot;
              prev = TVI_FIRST;
            }
          else if (hitTest.hItem == midRoot)
            {
              parent = midRoot;
              prev = TVI_FIRST;
            }
          else
            {
              parent = TreeView_GetParent(treeWnd, hitTest.hItem);
              prev = TreeView_GetPrevSibling(treeWnd, hitTest.hItem);
              if (!prev)
                prev = TVI_FIRST;
            }

          tvInsert.hParent = parent;
          tvInsert.hInsertAfter = prev;
          tvInsert.item.mask = TVIF_TEXT;
          if (iter->second.type == IT_SEPARATOR)
            tvInsert.item.pszText = (WCHAR*)TEXT("[Separator]");
          else if (iter->second.type == IT_DATE_TIME)
            tvInsert.item.pszText = (WCHAR*)TEXT("[DateTime]");
          else
            tvInsert.item.pszText = iter->second.name;

          item = TreeView_InsertItem(treeWnd, &tvInsert);

          treeMap.insert(TreeItem(item, iter->second));

          if (iter->second.type == IT_XML_MENU)
            MoveSubmenu(treeWnd, item, iter->first);

          DeleteItem(treeWnd, iter->first);

          ReleaseCapture();
          dragging = false;

          (void)TreeView_SelectItem(treeWnd, item);
        }

      return TRUE;
    }

  return FALSE;
}

void MenuEditor::MoveSubmenu(HWND treeWnd, HTREEITEM target, HTREEITEM source)
{
  TreeItemMap::iterator iter;
  TVINSERTSTRUCT tvInsert;
  HTREEITEM item, child = TreeView_GetChild(treeWnd, source);

  while (child != NULL)
    {
      iter = treeMap.find(child);

      tvInsert.hParent = target;
      tvInsert.hInsertAfter = TVI_LAST;
      tvInsert.item.mask = TVIF_TEXT;
      if (iter->second.type == IT_SEPARATOR)
        tvInsert.item.pszText = (WCHAR*)TEXT("[Separator]");
      else if (iter->second.type == IT_DATE_TIME)
        tvInsert.item.pszText = (WCHAR*)TEXT("[DateTime]");
      else
        tvInsert.item.pszText = iter->second.name;

      item = TreeView_InsertItem(treeWnd, &tvInsert);
      treeMap.insert(TreeItem(item, iter->second));

      if (iter->second.type == IT_XML_MENU)
        MoveSubmenu(treeWnd, item, child);

      child = TreeView_GetNextSibling(treeWnd, iter->first);

      DeleteItem(treeWnd, iter->first);
    }
}

bool MenuEditor::DoDelItem(HWND hwndDlg)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);

  SetDlgItemText(hwndDlg, IDC_ITEMNAME, TEXT("\0"));
  SetDlgItemText(hwndDlg, IDC_ITEMVALUE, TEXT("\0"));
  SendMessage(typeWnd, CB_SETCURSEL, (WPARAM)-1, 0);

  return DeleteItem(treeWnd, TreeView_GetSelection(treeWnd));
}

bool MenuEditor::DoEditItem(HWND hwndDlg)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);

  EnableWindow(treeWnd, false);
  EnableWindow(addWnd, false);
  EnableWindow(editWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);

  edit = true;

  return EnableFields(hwndDlg, (UINT)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));
}

bool MenuEditor::DoSelChange(HWND hwndDlg, HWND typeWnd)
{
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);

  //    SetDlgItemText(hwndDlg, IDC_ITEMNAME, TEXT("\0"));
  SetDlgItemText(hwndDlg, IDC_ITEMVALUE, TEXT("\0"));

  SetTooltip(browseWnd, GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0)));

  return EnableFields(hwndDlg, (UINT)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));
}

bool MenuEditor::ShowFields(HWND hwndDlg, UINT index)
{
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);

  switch (index)
    {
    case 0:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      break;
    case 2:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Command:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, false);
      ShowWindow(browseWnd, false);
      ShowWindow(specialFoldersWnd, false);
      ShowWindow(commandWnd, true);
      break;
    case 3:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Format:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      break;
    case 4:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Special Folder:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, false);
      ShowWindow(browseWnd, false);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, true);
      break;
    case 5:
    case 7:
    case 8:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      break;
    case 1:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      break;
    case 6:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Path:"));
      if (browseIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)browseIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      break;
    default:
      SetDlgItemText(hwndDlg, IDC_VALUETEXT, TEXT("Execute:"));
      if (fileIcon)
        SendMessage(browseWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)fileIcon);
      ShowWindow(valueWnd, true);
      ShowWindow(browseWnd, true);
      ShowWindow(commandWnd, false);
      ShowWindow(specialFoldersWnd, false);
      break;
    }

  return true;
}

bool MenuEditor::EnableFields(HWND hwndDlg, UINT index)
{
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND typeTextWnd = GetDlgItem(hwndDlg, IDC_TYPETEXT);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND valueTextWnd = GetDlgItem(hwndDlg, IDC_VALUETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND dirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND dirBrowseWnd = GetDlgItem(hwndDlg, IDC_DIRBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);

  EnableWindow(typeWnd, true);
  EnableWindow(typeTextWnd, true);

  switch (index)
    {
    case 0:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(nameWnd, false);
      EnableWindow(nameTextWnd, false);
      EnableWindow(valueWnd, false);
      EnableWindow(valueTextWnd, false);
      EnableWindow(browseWnd, false);
      break;
    case 2:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(commandWnd, true);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueTextWnd, true);
      break;
    case 3:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(valueWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(browseWnd, false);
      EnableWindow(nameWnd, false);
      EnableWindow(nameTextWnd, true);
      break;
    case 4:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(specialFoldersWnd, true);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueTextWnd, true);
      break;
    case 5:
    case 7:
    case 8:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(valueWnd, false);
      EnableWindow(valueTextWnd, false);
      EnableWindow(browseWnd, false);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      break;
    case 1:
      EnableWindow(dirWnd, true);
      EnableWindow(workingDirTextWnd, true);
      EnableWindow(dirBrowseWnd, true);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(browseWnd, true);
      break;
    case 6:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueWnd, true);
      EnableWindow(valueTextWnd, true);
      EnableWindow(browseWnd, true);
      break;
    default:
      EnableWindow(dirWnd, false);
      EnableWindow(workingDirTextWnd, false);
      EnableWindow(dirBrowseWnd, false);
      EnableWindow(nameWnd, true);
      EnableWindow(nameTextWnd, true);
      EnableWindow(valueWnd, false);
      EnableWindow(valueTextWnd, false);
      EnableWindow(browseWnd, false);
      break;
    }

  return ShowFields(hwndDlg, index);
}

bool MenuEditor::DoSaveItem(HWND hwndDlg)
{
  TreeItemMap::iterator iter;
  TVINSERTSTRUCT tvInsert;
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  TVITEM pItem;
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH];
  ZeroMemory(name, MAX_LINE_LENGTH);
  ZeroMemory(workingDir, MAX_LINE_LENGTH);
  ZeroMemory(tmp, MAX_LINE_LENGTH);
  BOOL ret;

  iter = treeMap.find(TreeView_GetSelection(treeWnd));

  if (iter == treeMap.end())
    return false;

  pItem.mask = TVIF_HANDLE | TVIF_TEXT;
  pItem.hItem = iter->first;

  GetDlgItemText(hwndDlg, IDC_ITEMNAME, name, MAX_LINE_LENGTH);
  GetDlgItemText(hwndDlg, IDC_ITEMVALUE, value, MAX_LINE_LENGTH);

  iter->second.type = GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));

  if (iter->second.type == IT_SEPARATOR)
    pItem.pszText = (WCHAR*)TEXT("[Separator]");
  else
    {
      if (iter->second.type == IT_INTERNAL_COMMAND)
        GetDlgItemText(hwndDlg, IDC_ITEMCOMMAND, value, MAX_LINE_LENGTH);
      else if (iter->second.type == IT_SPECIAL_FOLDER)
        {
          GetDlgItemText(hwndDlg, IDC_ITEMSPECIALFOLDERS, tmp, MAX_LINE_LENGTH);
          if (!ELSpecialFolderValue(tmp, value))
            wcscpy(value, TEXT(""));
        }
      else
        GetDlgItemText(hwndDlg, IDC_ITEMVALUE, value, MAX_LINE_LENGTH);

      if (iter->second.type == IT_DATE_TIME)
        pItem.pszText = (WCHAR*)TEXT("[DateTime]");

      GetDlgItemText(hwndDlg, IDC_WORKINGDIR, workingDir, MAX_LINE_LENGTH);

      if (wcslen(name) > 0)
        {
          wcscpy(iter->second.name, name);
          pItem.pszText = name;

          if ((iter->second.type == IT_XML_MENU) &&
              (TreeView_GetChild(treeWnd, iter->first) == NULL))
            {
              MENUTREEITEM menuItem;
              wcscpy(menuItem.name, TEXT("New Item"));
              menuItem.type = MAX_LINE_LENGTH;
              wcscpy(menuItem.value, TEXT(""));

              tvInsert.hParent = iter->first;
              tvInsert.hInsertAfter=TVI_FIRST;
              tvInsert.item.mask = TVIF_TEXT;
              tvInsert.item.pszText = menuItem.name;

              HTREEITEM child = TreeView_InsertItem(treeWnd, &tvInsert);

              treeMap.insert(TreeItem(child, menuItem));
            }
        }
      else if (IsWindowEnabled(nameWnd))
        {
          ELMessageBox(hwndDlg, (WCHAR*)TEXT("Name cannot be empty"), (WCHAR*)TEXT("emergeWorkspace"),
                       ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
          return false;
        }
    }

  wcscpy(iter->second.value, value);
  wcscpy(iter->second.workingDir, workingDir);

  ret = TreeView_SetItem(treeWnd, &pItem);

  saveCount++;
  deleteCount++;

  return DoUpdateFields(hwndDlg);
}

bool MenuEditor::DoAbortItem(HWND hwndDlg)
{
  TreeItemMap::iterator iter;
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);
  WCHAR value[MAX_LINE_LENGTH];

  iter = treeMap.find(TreeView_GetSelection(treeWnd));

  if (iter == treeMap.end())
    return false;

  if (edit)
    {
      SetDlgItemText(hwndDlg, IDC_ITEMNAME, iter->second.name);

      if (iter->second.type == IT_INTERNAL_COMMAND)
        SendMessage(commandWnd, CB_SETCURSEL,
                    SendMessage(commandWnd,
                                CB_FINDSTRINGEXACT,
                                (WPARAM)-1,
                                (LPARAM)iter->second.value),
                    0);
      else if (iter->second.type == IT_SPECIAL_FOLDER)
        {
          int folder = ELIsSpecialFolder(iter->second.value);
          if (ELGetSpecialFolder(folder, value))
            SendMessage(specialFoldersWnd, CB_SETCURSEL,
                        SendMessage(specialFoldersWnd,
                                    CB_FINDSTRINGEXACT,
                                    (WPARAM)-1,
                                    (LPARAM)value),
                        0);
        }
      else
        SetDlgItemText(hwndDlg, IDC_ITEMVALUE, iter->second.value);

      SetDlgItemText(hwndDlg, IDC_WORKINGDIR, iter->second.workingDir);

      SendMessage(typeWnd, CB_SETCURSEL,
                  (WPARAM)GetTypeValue(iter->second.type), 0);
    }
  else
    DeleteItem(treeWnd, iter->first);

  return DoUpdateFields(hwndDlg);
}

bool MenuEditor::DoUpdateFields(HWND hwndDlg)
{
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND typeTextWnd = GetDlgItem(hwndDlg, IDC_TYPETEXT);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND valueTextWnd = GetDlgItem(hwndDlg, IDC_VALUETEXT);
  HWND commandWnd = GetDlgItem(hwndDlg, IDC_ITEMCOMMAND);
  HWND browseWnd = GetDlgItem(hwndDlg, IDC_ITEMBROWSE);
  HWND dirWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIR);
  HWND workingDirTextWnd = GetDlgItem(hwndDlg, IDC_WORKINGDIRTEXT);
  HWND dirBrowseWnd = GetDlgItem(hwndDlg, IDC_DIRBROWSE);
  HWND specialFoldersWnd = GetDlgItem(hwndDlg, IDC_ITEMSPECIALFOLDERS);

  EnableWindow(treeWnd, true);
  EnableWindow(addWnd, true);
  EnableWindow(editWnd, true);
  EnableWindow(delWnd, true);
  EnableWindow(saveWnd, false);
  EnableWindow(abortWnd, false);
  EnableWindow(nameWnd, false);
  EnableWindow(nameTextWnd, false);
  EnableWindow(typeWnd, false);
  EnableWindow(typeTextWnd, false);
  EnableWindow(valueWnd, false);
  EnableWindow(valueTextWnd, false);
  EnableWindow(commandWnd, false);
  EnableWindow(browseWnd, false);
  EnableWindow(dirWnd, false);
  EnableWindow(workingDirTextWnd, false);
  EnableWindow(dirBrowseWnd, false);
  EnableWindow(specialFoldersWnd, false);

  edit = false;

  return true;
}

bool MenuEditor::DoAddItem(HWND hwndDlg)
{
  TVINSERTSTRUCT tvInsert;
  MENUTREEITEM menuItem;
  TreeItemMap::iterator iter;
  HTREEITEM item, parent, prev;
  BOOL ret;

  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND nameWnd = GetDlgItem(hwndDlg, IDC_ITEMNAME);
  HWND nameTextWnd = GetDlgItem(hwndDlg, IDC_NAMETEXT);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  HWND typeTextWnd = GetDlgItem(hwndDlg, IDC_TYPETEXT);
  HWND valueWnd = GetDlgItem(hwndDlg, IDC_ITEMVALUE);
  HWND valueTextWnd = GetDlgItem(hwndDlg, IDC_VALUETEXT);
  HWND saveWnd = GetDlgItem(hwndDlg, IDC_SAVEITEM);
  HWND abortWnd = GetDlgItem(hwndDlg, IDC_ABORTITEM);
  HWND addWnd = GetDlgItem(hwndDlg, IDC_ADDITEM);
  HWND editWnd = GetDlgItem(hwndDlg, IDC_EDITITEM);
  HWND delWnd = GetDlgItem(hwndDlg, IDC_DELITEM);

  EnableWindow(nameWnd, false);
  EnableWindow(nameTextWnd, false);
  EnableWindow(typeWnd, false);
  EnableWindow(typeTextWnd, false);
  EnableWindow(valueWnd, false);
  EnableWindow(valueTextWnd, false);
  EnableWindow(saveWnd, true);
  EnableWindow(abortWnd, true);
  EnableWindow(addWnd, false);
  EnableWindow(editWnd, false);
  EnableWindow(delWnd, false);
  EnableWindow(treeWnd, false);

  edit = false;

  iter = treeMap.find(selected);

  wcscpy(menuItem.name, TEXT("New Item"));
  menuItem.type = MAX_LINE_LENGTH;
  wcscpy(menuItem.value, TEXT(""));
  wcscpy(menuItem.workingDir, TEXT(""));

  if ((selected == rightRoot) || (selected == midRoot))
    {
      parent = selected;
      prev = TVI_FIRST;
    }
  else if (iter != treeMap.end())
    {
      parent = TreeView_GetParent(treeWnd, iter->first);
      prev = TreeView_GetPrevSibling(treeWnd, iter->first);

      if (!prev)
        prev = TVI_FIRST;
    }
  else
    return false;

  tvInsert.hParent = parent;
  tvInsert.hInsertAfter = prev;
  tvInsert.item.mask = TVIF_TEXT;
  tvInsert.item.pszText = menuItem.name;

  item = TreeView_InsertItem(treeWnd, &tvInsert);
  treeMap.insert(TreeItem(item, menuItem));
  ret = TreeView_SelectItem(treeWnd, item);

  return EnableFields(hwndDlg, (UINT)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));
}

bool MenuEditor::DeleteItem(HWND treeWnd, HTREEITEM item)
{
  TreeItemMap::iterator iter;

  if ((item != NULL) && (item != rightRoot) && (item != midRoot))
    {
      iter = treeMap.find(item);

      // Note: Order is important!
      if (iter != treeMap.end())
        treeMap.erase(iter);

      if (TreeView_DeleteItem(treeWnd, item))
        {
          deleteCount++;
          return true;
        }
    }

  return false;
}

bool MenuEditor::DoBrowseItem(HWND hwndDlg, bool workingDir)
{
  TreeItemMap::iterator iter;
  OPENFILENAME ofn;
  BROWSEINFO bi;
  HWND treeWnd = GetDlgItem(hwndDlg, IDC_MENUTREE);
  HWND typeWnd = GetDlgItem(hwndDlg, IDC_ITEMTYPE);
  WCHAR tmp[MAX_PATH], program[MAX_PATH], arguments[MAX_LINE_LENGTH];
  WCHAR initPath[MAX_PATH];
  UINT type = GetValueType((int)SendMessage(typeWnd, CB_GETCURSEL, 0, 0));
  std::wstring workingPath;

  iter = treeMap.find(TreeView_GetSelection(treeWnd));

  if (iter == treeMap.end())
    return false;

  ZeroMemory(tmp, MAX_PATH);

  if ((type == IT_FILE_MENU) || workingDir)
    {
      ZeroMemory(&bi, sizeof(BROWSEINFO));
      if (workingDir)
        GetDlgItemText(hwndDlg, IDC_WORKINGDIR, tmp, MAX_PATH);
      else
        GetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp, MAX_PATH);
      workingPath = tmp;
      workingPath = ELExpandVars(workingPath);
      wcscpy(tmp, workingPath.c_str());
      bi.pszDisplayName = tmp;
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

              ELUnExpandVars(tmp);
              if (workingDir)
                SetDlgItemText(hwndDlg, IDC_WORKINGDIR, tmp);
              else
                SetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp);

              return true;
            }
        }
    }
  else
    {
      ZeroMemory(&ofn, sizeof(ofn));

      ELGetCurrentPath(initPath);

      ofn.lpstrInitialDir = initPath;
      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hwndDlg;
      ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
      GetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp, MAX_PATH);
      workingPath = tmp;
      workingPath = ELExpandVars(workingPath);
      if (ELParseCommand(workingPath.c_str(), program, arguments))
        wcscpy(tmp, program);
      ofn.lpstrFile = tmp;
      ofn.nMaxFile = MAX_PATH;
      ofn.lpstrTitle = TEXT("Browse For File");
      ofn.lpstrDefExt = NULL;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS;

      if (GetOpenFileName(&ofn))
        {
          ELUnExpandVars(tmp);
          if (type == IT_SPECIAL_FOLDER)
            swprintf(tmp, TEXT("*%s"), PathFindFileName(tmp));

          SetDlgItemText(hwndDlg, IDC_ITEMVALUE, tmp);

          return true;
        }
    }

  return false;
}

void MenuEditor::SetTooltip(HWND browseWnd, UINT type)
{
  TOOLINFO ti;
  ZeroMemory(&ti, sizeof(TOOLINFO));

  ti.cbSize = TTTOOLINFOW_V2_SIZE;
  ti.hwnd = browseWnd;
  ti.uId = (ULONG_PTR)browseWnd;

  SendMessage(toolWnd, TTM_GETTOOLINFO, 0,(LPARAM) (LPTOOLINFO) &ti);

  switch (type)
    {
    case IT_EXECUTABLE:
      ti.lpszText = (WCHAR*)TEXT("Browse for a file");
      break;
    case IT_SPECIAL_FOLDER:
      ti.lpszText = (WCHAR*)TEXT("Browse for a special folder");
      break;
    case IT_FILE_MENU:
      ti.lpszText = (WCHAR*)TEXT("Browse for a directory");
    }

  SendMessage(toolWnd, TTM_SETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
}


