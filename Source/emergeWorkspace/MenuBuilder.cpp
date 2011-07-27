// vim:tags+=../emergeLib/tags,../emergeAppletEngine/tags,../emergeGraphics/tags

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

#include "MenuBuilder.h"

WCHAR menuBuilderClass[ ] = TEXT("EmergeDesktopMenuBuilder");
BYTE globalMenuAlpha;

MenuBuilder::MenuBuilder(HINSTANCE desktopInst)
{
  mainInst = desktopInst;
  MButtonDown = false;
  registered = false;
  winVersion = ELVersionInfo();
  SetRectEmpty(&explorerWorkArea);
}

bool MenuBuilder::Initialize()
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  if (FAILED(OleInitialize(NULL)))
    {
      ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("COM initialization failed"), (WCHAR*)TEXT("emergeWorkspace"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return registered;
    }

  // Register the window class
  wincl.hInstance = mainInst;
  wincl.lpszClassName = menuBuilderClass;
  wincl.lpfnWndProc = MenuProcedure;
  wincl.style = CS_DBLCLKS;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);

  // Register the window class, and if it fails quit the program
  if (!RegisterClassEx (&wincl))
    return false;

  // The class is registered, let's create the window
  menuWnd = CreateWindowEx(WS_EX_TOOLWINDOW, menuBuilderClass, NULL, WS_POPUP,
                           0, 0, 0, 0, NULL, NULL, mainInst, reinterpret_cast<LPVOID>(this));

  // If the window failed to get created, unregister the class and quit the program
  if (!menuWnd)
    {
      ELMessageBox(GetDesktopWindow(),
                   (WCHAR*)TEXT("Failed to create desktop window"),
                   (WCHAR*)TEXT("emergeWorkspace"),
                   ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
      return false;
    }

  ELStealFocus(menuWnd);
  SetWindowPos(menuWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

  // Disable menu animation, as it overrides the alpha blending
  SystemParametersInfo(SPI_SETMENUANIMATION, 0, (PVOID)false, SPIF_SENDCHANGE);

  pSettings = std::tr1::shared_ptr<Settings>(new Settings());
  RenameConfigFile();
  pSettings->Init(menuWnd, (WCHAR*)TEXT("emergeWorkspace"), 0);
  pSettings->ReadSettings();
  pMenuEditor = std::tr1::shared_ptr<MenuEditor>(new MenuEditor(mainInst));
  pItemEditor = std::tr1::shared_ptr<ItemEditor>(new ItemEditor(mainInst, menuWnd));

  SetWorkArea();

  // Format the time
  setlocale(LC_TIME, "");

  // Hook the menus (for transparency)
  menuHook = NULL;
  UpdateMenuHook();

  // Register to recieve the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)menuWnd, (LPARAM)EMERGE_CORE);

  //  LPVOID lpVoid;
  //  customDropTarget = new CustomDropTarget(menuWnd);
  //  customDropTarget->QueryInterface(IID_IDropTarget, &lpVoid);
  //  dropTarget = reinterpret_cast <IDropTarget*> (lpVoid);

  //  if (RegisterDragDrop(menuWnd, dropTarget) != S_OK)
  //    return false;

  return true;
}

void MenuBuilder::RenameConfigFile()
{
  std::wstring oldXMLFile = TEXT("%ThemeDir%\\emergeDesktop.xml");
  oldXMLFile = ELExpandVars(oldXMLFile);
  std::wstring xmlFile = TEXT("%ThemeDir%\\emergeWorkspace.xml");
  xmlFile = ELExpandVars(xmlFile);
  if (!PathFileExists(xmlFile.c_str()))
    {
      if (PathFileExists(oldXMLFile.c_str()))
        ELFileOp(menuWnd, FO_RENAME, oldXMLFile, xmlFile);
    }
}

MenuBuilder::~MenuBuilder()
{
  if (!IsRectEmpty(&explorerWorkArea))
    SystemParametersInfo(SPI_SETWORKAREA, 0, &explorerWorkArea, SPIF_SENDCHANGE);

  if (registered)
    {
      // Unregister the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)menuWnd, (LPARAM)EMERGE_CORE);

      //      dropTarget->Release();
      //      RevokeDragDrop(menuWnd);

      // Clear the menu hook
      if (menuHook)
        UnhookWindowsHookEx(menuHook);

      // Unregister the window class
      UnregisterClass(menuBuilderClass, mainInst);

      OleUninitialize();
    }
}

LRESULT CALLBACK MenuBuilder::MenuProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT *cs;
  static MenuBuilder *pMenuBuilder = NULL;

  if (message == WM_CREATE)
    {
      cs = (CREATESTRUCT*)lParam;
      pMenuBuilder = reinterpret_cast<MenuBuilder*>(cs->lpCreateParams);
      return DefWindowProc(hwnd, message, wParam, lParam);
    }

  if (pMenuBuilder == NULL)
    return DefWindowProc(hwnd, message, wParam, lParam);

  switch (message)
    {
    case WM_COPYDATA:
      return pMenuBuilder->DoCopyData((COPYDATASTRUCT *)lParam);

    case WM_SYSCOMMAND:
      switch (wParam)
        {
        case SC_CLOSE:
        case SC_MAXIMIZE:
        case SC_MINIMIZE:
          break;
        default:
          return DefWindowProc(hwnd, message, wParam, lParam);
        }
      break;

    case WM_INITMENUPOPUP:
      return pMenuBuilder->DoInitMenu((HMENU)wParam);

    case WM_MENUSELECT:
      return pMenuBuilder->DoMenuSelect((HMENU)lParam, HIWORD(wParam), LOWORD(wParam));

    case WM_MEASUREITEM:
      return pMenuBuilder->DoMeasureItem((LPMEASUREITEMSTRUCT)lParam);

    case WM_DRAWITEM:
      return pMenuBuilder->DoDrawItem((LPDRAWITEMSTRUCT)lParam);

    case WM_CONTEXTMENU:
      POINT pt;
      GetCursorPos(&pt);
      return pMenuBuilder->DoContextMenu(pt);
      break;

    case WM_MENUDRAG:
      return pMenuBuilder->DoMenuDrag(hwnd, (HMENU)lParam);

    case WM_MENUGETOBJECT:
      return pMenuBuilder->DoMenuGetObject(hwnd, (MENUGETOBJECTINFO*)lParam);

    case WM_DISPLAYCHANGE:
      pMenuBuilder->SetWorkArea();
      break;

    case WM_DESTROY:
    case WM_NCDESTROY:
      PostQuitMessage(0);
      break;

    default:
      return pMenuBuilder->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

LRESULT MenuBuilder::DoCopyData(COPYDATASTRUCT *cds)
{

  if (cds->dwData == EMERGE_MESSAGE)
    {
      std::wstring theme = reinterpret_cast<WCHAR*>(cds->lpData);
      SetEnvironmentVariable(TEXT("ThemeDir"), theme.c_str());
      return 1;
    }

  if ((cds->dwData == EMERGE_NOTIFY) && (cds->cbData == sizeof(NOTIFYINFO)))
    {
      LPNOTIFYINFO notifyInfo = reinterpret_cast<LPNOTIFYINFO>(cds->lpData);

      if ((notifyInfo->Type & EMERGE_CORE) == EMERGE_CORE)
        {
          switch (notifyInfo->Message)
            {
            case CORE_SETTINGS:
            case CORE_SHOWCONFIG:
            {
              Config config(mainInst, pSettings);
              if (config.Show() == IDOK)
                {
                  UpdateMenuHook();
                  SetWorkArea();
                }
            }
            break;

            case CORE_RIGHTMENU:
              return DoButtonDown(WM_RBUTTONDOWN);

            case CORE_MIDMENU:
              return DoButtonDown(WM_MBUTTONDOWN);

            case CORE_LEFTMENU:
              return DoButtonDown(WM_LBUTTONDOWN);

            case CORE_WRITESETTINGS:
              pSettings->WriteSettings();
              break;

            case CORE_RECONFIGURE:
              RenameConfigFile();
              pSettings->ReadSettings();
              UpdateMenuHook();
              SetWorkArea();
              break;
            }

          return 1;
        }
    }

  return 0;
}

LRESULT MenuBuilder::DoDefault(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT MenuBuilder::DoMenuGetObject(HWND hwnd UNUSED, MENUGETOBJECTINFO *mgoInfo)
{
  MenuMap::iterator iter, subIter;
  IID menuInterface = IID_IDropTarget;
  MENUITEMINFO menuItemInfo;
  HMENU menu;
  IDropTarget *dropTarget;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;

  /*if (mgoInfo->dwFlags == MNGOF_BOTTOMGAP)
    OutputDebugStr((WCHAR*)TEXT("Bottom Gap"));

    if (mgoInfo->dwFlags == MNGOF_TOPGAP)
    OutputDebugStr((WCHAR*)TEXT("Top Gap"));*/

  iter = menuMap.find(mgoInfo->hmenu);
  if (iter == menuMap.end())
    return MNGO_NOINTERFACE;

  if (!GetMenuItemInfo(iter->first, mgoInfo->uPos, TRUE, &menuItemInfo))
    return MNGO_NOINTERFACE;

  menu = menuItemInfo.hSubMenu;
  if (menu)
    {
      subIter = menuMap.find(menu);
      if (subIter == menuMap.end())
        return MNGO_NOINTERFACE;

      dropTarget = subIter->second->GetDropTarget();
    }
  else
    {
      UINT itemID = menuItemInfo.wID;
      itemID--;
      dropTarget = iter->second->GetMenuItem(itemID)->GetDropTarget();
    }

  mgoInfo->riid = &menuInterface;
  mgoInfo->pvObj = dropTarget;

  return MNGO_NOERROR;
}

LRESULT MenuBuilder::DoMenuDrag(HWND hwnd, HMENU menu)
{
  DWORD /*effect,*/ dropEffects;
  LPVOID lpVoid;
  IDropSource *dropSource;
  IDataObject *dataObject;
  std::tr1::shared_ptr<CustomDropSource> customDropSource(new CustomDropSource(hwnd));
  std::tr1::shared_ptr<CustomDataObject> customDataObject(new CustomDataObject(menu));

  dropEffects = DROPEFFECT_COPY;

  customDropSource->QueryInterface(IID_IDropSource, &lpVoid);
  dropSource = reinterpret_cast <IDropSource*> (lpVoid);
  customDataObject->QueryInterface(IID_IDataObject, &lpVoid);
  dataObject = reinterpret_cast <IDataObject*> (lpVoid);

  /*if (DoDragDrop(dataObject, dropSource, dropEffects, &effect) == DRAGDROP_S_DROP)
    OutputDebugString((WCHAR*)TEXT("Drop successfull"));*/

  dropSource->Release();
  dataObject->Release();
  customDropSource->Release();
  customDataObject->Release();

  return MND_CONTINUE;
}

MenuMap::iterator MenuBuilder::GetMenuIterID(POINT pt, int *index)
{
  MenuMap::reverse_iterator revIter;
  MenuMap::iterator iter;

  // Go through the map backwards to make sure that the newest menu is matched first
  for (revIter = menuMap.rbegin(); revIter != menuMap.rend(); revIter++)
    {
      *(index) = MenuItemFromPoint(menuWnd, revIter->first, pt);
      if (*(index) != -1)
        break;
    }

  revIter++;

  iter = revIter.base();

  return iter;
}

LRESULT MenuBuilder::DoContextMenu(POINT pt)
{
  MENUITEMINFO menuItemInfo;
  MenuMap::iterator iter, subIter;
  HMENU menu = NULL;
  int index;
  WCHAR value[MAX_LINE_LENGTH];
  UINT itemID = 0;
  TiXmlElement *element;
  TiXmlDocument *configXML;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;

  if (menuMap.empty())
    return 0;

  iter = GetMenuIterID(pt, &index);
  if (index == -1)
    return 0;

  if (!GetMenuItemInfo(iter->first, index, TRUE, &menuItemInfo))
    return 0;

  menu = menuItemInfo.hSubMenu;
  itemID = menuItemInfo.wID;
  itemID--;

  if (menu)
    {
      subIter = menuMap.find(menu);
      if (subIter == menuMap.end())
        return 0;

      wcscpy(value, subIter->second->GetValue());
    }
  else
    wcscpy(value, iter->second->GetMenuItem(itemID)->GetValue());

  element = iter->second->GetMenuItem(itemID)->GetElement();

  switch (iter->second->GetType())
    {
      int res;
    case IT_XML_MENU:
      res = DisplayRegContext(pt, iter->second->GetMenuItem(itemID)->GetType());
      switch (res)
        {
        case DRM_DELETE:
          configXML = ELGetXMLConfig(element);
          if (ELRemoveXMLElement(element))
            {
              if (ELWriteXMLConfig(configXML))
                DeleteMenu(iter->first, index, MF_BYPOSITION);
            }
          break;
        case DRM_EDIT:
          EditMenuItem(iter, index);
          break;
        case DRM_ADD:
          AddMenuItem(iter, index);
          break;
        case DRM_RUNAS:
          ElevatedExecute(iter->second->GetMenuItem(itemID));
          break;
        }
      break;
    case IT_FILE_MENU:
      res = EAEDisplayFileMenu(value, menuWnd);
      if (res != 0)
        SendMessage(menuWnd, WM_CANCELMODE, 0, 0);
      break;
    case IT_TASKS_MENU:
    {
      HWND task = (HWND)_wtoi(value);
      res = EAEDisplayMenu(menuWnd, task);
      switch (res)
        {
        case SC_CLOSE:
          DeleteMenu(iter->first, index, MF_BYPOSITION);
          break;
        case SC_SIZE:
        case SC_MOVE:
        case SC_MAXIMIZE:
        case SC_RESTORE:
          ELSwitchToThisWindow(task);
          SendMessage(menuWnd, WM_CANCELMODE, 0, 0);
          break;
        }
      if (res)
        PostMessage(task, WM_SYSCOMMAND, (WPARAM)res, MAKELPARAM(pt.x, pt.y));
    }
    break;
    /*case IT_SETTINGS_MENU:
      ExecuteSettingsMenuItem(itemID);
      break;*/
    /*case IT_HELP_MENU:
      ExecuteSettingsMenuItem(itemID);
      break;*/
    }

  return 1;
}

void MenuBuilder::ElevatedExecute(MenuItem *menuItem)
{
  ELExecute(menuItem->GetValue(), menuItem->GetWorkingDir(), SW_SHOW, (WCHAR*)TEXT("runas"));
}

bool MenuBuilder::AddMenuItem(MenuMap::iterator iter, int index)
{
  UINT itemID;
  int type;
  MENUITEMINFO menuItemInfo;
  TiXmlElement *element, *newElement, *subMenu, *subItem;;
  TiXmlDocument *configXML;
  MenuItem *menuItem;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_ID;

  if (!GetMenuItemInfo(iter->first, index, TRUE, &menuItemInfo))
    return false;

  itemID = menuItemInfo.wID;
  itemID--;
  element = iter->second->GetMenuItem(itemID)->GetElement();

  if (!element)
    return false;

  menuItem = new MenuItem((WCHAR*)TEXT("New Item"), -1, NULL, NULL, NULL);

  menuItemInfo.wID = GetMenuItemCount(iter->first) + 1;

  iter->second->AddMenuItem(menuItem);

  InsertMenuItem(iter->first, index, TRUE, &menuItemInfo);

  newElement = ELSetSibilingXMLElement(element, (WCHAR*)TEXT("item"));
  if (!newElement)
    return false;

  configXML = ELGetXMLConfig(newElement);

  if (pItemEditor->Show(newElement,
                        menuItem->GetName(),
                        menuItem->GetValue(),
                        menuItem->GetType(),
                        menuItem->GetWorkingDir()) == IDCANCEL)
    {
      ELRemoveXMLElement(newElement);
      ELWriteXMLConfig(configXML);
    }
  else
    {
      if (ELReadXMLIntValue(newElement,  (WCHAR*)TEXT("Type"), &type, 0))
        {
          if (type == IT_XML_MENU)
            {
              subMenu = ELGetFirstXMLElementByName(newElement, (WCHAR*)TEXT("Submenu"), true);
              if (subMenu)
                {
                  subItem = ELGetFirstXMLElementByName(subMenu, (WCHAR*)TEXT("item"), true);
                  if (subItem)
                    {
                      ELWriteXMLIntValue(subItem, (WCHAR*)TEXT("Type"), 0);
                      ELWriteXMLConfig(configXML);
                    }
                }
            }
        }
    }

  return true;
}

bool MenuBuilder::EditMenuItem(MenuMap::iterator iter, int index)
{
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  UINT type, itemID;
  MENUITEMINFO menuItemInfo;
  TiXmlElement *element;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_ID;

  if (!GetMenuItemInfo(iter->first, index, TRUE, &menuItemInfo))
    return false;

  itemID = menuItemInfo.wID;
  itemID--;
  wcscpy(value, iter->second->GetMenuItem(itemID)->GetValue());
  wcscpy(name, iter->second->GetMenuItem(itemID)->GetName());
  wcscpy(workingDir, iter->second->GetMenuItem(itemID)->GetWorkingDir());
  type = iter->second->GetMenuItem(itemID)->GetType();
  element = iter->second->GetMenuItem(itemID)->GetElement();

  pItemEditor->Show(element, name, value, type, workingDir);

  return true;
}

int MenuBuilder::DisplayRegContext(POINT pt, int type)
{
  HMENU menu;

  menu = CreatePopupMenu();
  if (type == IT_EXECUTABLE)
    {
      AppendMenu(menu, MF_STRING, DRM_RUNAS, TEXT("Run Elevated"));
      AppendMenu(menu, MF_SEPARATOR, 0, NULL);
    }
  AppendMenu(menu, MF_STRING, DRM_ADD, TEXT("Insert Menu Item"));
  AppendMenu(menu, MF_STRING, DRM_EDIT, TEXT("Edit Menu Item"));
  AppendMenu(menu, MF_STRING, DRM_DELETE, TEXT("Delete Menu Item"));

  return TrackPopupMenuEx(menu, TPM_RETURNCMD|TPM_RECURSE, pt.x, pt.y, menuWnd, NULL);
}

LRESULT MenuBuilder::DoMenuSelect(HMENU menu, WORD itemType, WORD item)
{
  if (menu != NULL)
    {
      SelectedMenu = menu;
      SelectedItem = item;
      SelectedItemType = itemType;
    }

  return 1;
}

BOOL MenuBuilder::DoMeasureItem(LPMEASUREITEMSTRUCT lpMeasureItem)
{
  if (lpMeasureItem->CtlType == ODT_MENU)
    {
      lpMeasureItem->itemWidth = 0;
      lpMeasureItem->itemHeight = 16;

      if (pSettings->GetMenuIcons())
        lpMeasureItem->itemWidth = 16;

      return TRUE;
    }

  return FALSE;
}

BOOL MenuBuilder::DoDrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
  HICON icon = NULL;
  MenuMap::iterator iter;

  if ((lpDrawItem->CtlType == ODT_MENU) && pSettings->GetMenuIcons())
    {
      iter = menuMap.find((HMENU)lpDrawItem->hwndItem);
      if (iter != menuMap.end())
        icon = iter->second->GetMenuItem(lpDrawItem->itemID - 1)->GetIcon();

      if (icon != NULL)
        DrawIconEx(lpDrawItem->hDC, lpDrawItem->rcItem.left, lpDrawItem->rcItem.top, icon, 16, 16, 0, NULL, DI_NORMAL);

      return TRUE;
    }

  return FALSE;
}

HWND MenuBuilder::GetWnd()
{
  return menuWnd;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	HookCallWndProc
// Requires:	int nCode - action
// 		wParam - not used
// 		lParam - message data
// Returns:	LRESULT
// Purpose:	Hooks the menu windows of a given application
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK MenuBuilder::HookCallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  CWPSTRUCT cwps;

  if ( nCode == HC_ACTION )
    {
      CopyMemory(&cwps, (LPVOID)lParam, sizeof(CWPSTRUCT));

      switch (cwps.message)
        {
        case WM_CREATE:
        {
          WCHAR szClass[128];
          GetClassName(cwps.hwnd, szClass, 127);
          if (_wcsicmp(szClass, TEXT("#32768"))==0)
            {
              SetWindowLongPtr(cwps.hwnd,
                               GWL_EXSTYLE,
                               GetWindowLongPtr(cwps.hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
              SetLayeredWindowAttributes(cwps.hwnd,
                                         0,
                                         (BYTE)((255 * globalMenuAlpha) / 100), LWA_ALPHA);
            }
        }
        break;
        }
    }

  return CallNextHookEx((HHOOK)WH_CALLWNDPROC, nCode, wParam, lParam);
}

void MenuBuilder::BuildMenu(MenuMap::iterator iter)
{
  UINT type = iter->second->GetType();

  // Clear existing menu items
  ClearMenu(iter);

  // Registry based menu
  if (type == IT_XML_MENU)
    {
      BuildXMLMenu(iter);
      return;
    }

  // Path based menu
  if (type == IT_FILE_MENU)
    {
      BuildFileMenu(iter);
      return;
    }

  // Tasks based menu
  if (type == IT_TASKS_MENU)
    {
      taskMenu = iter->first;
      EnumWindows(BuildTasksMenu, reinterpret_cast<LPARAM>(this));
      return;
    }

  // Settings menu
  if (type == IT_SETTINGS_MENU)
    {
      BuildSettingsMenu(iter);
      return;
    }

  // Settings menu
  if (type == IT_HELP_MENU)
    {
      BuildHelpMenu(iter);
      return;
    }
}

void MenuBuilder::ClearAllMenus()
{
  while (!menuMap.empty())
    {
      ClearMenu(menuMap.begin());
      DestroyMenu(menuMap.begin()->first);
      menuMap.erase(menuMap.begin());
    }
}

void MenuBuilder::ClearMenu(MenuMap::iterator iter)
{
  MENUITEMINFO mii;
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_BITMAP;

  while (GetMenuItemInfo(iter->first, 0, MF_BYPOSITION, &mii))
    {
      DeleteObject(mii.hbmpItem);
      DeleteMenu(iter->first, 0, MF_BYPOSITION);
    }
  while (iter->second->GetMenuItemCount() > 0)
    iter->second->DeleteMenuItem(0);
}

void MenuBuilder::BuildXMLMenu(MenuMap::iterator iter)
{
  UINT i = 0;
  int type;
  bool found = false;
  WCHAR value[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH], workingDir[MAX_PATH];
  MENUITEMINFO itemInfo;
  MenuMap::iterator iter2;
  HMENU subMenu;
  TiXmlElement *tmp, *subSection, *child = ELGetFirstXMLElement(iter->second->GetSection());

  while (child)
    {
      found = true;
      ELReadXMLIntValue(child, (WCHAR*)TEXT("Type"), &type, 0);

      // Separator
      if (type == IT_SEPARATOR)
        {
          MenuItem *menuItem = new MenuItem(NULL, type, NULL, NULL, child);

          itemInfo.fMask = MIIM_FTYPE|MIIM_ID;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = name;
          itemInfo.fType = MFT_SEPARATOR;
          itemInfo.wID = GetMenuItemCount(iter->first) + 1;

          iter->second->AddMenuItem(menuItem);

          InsertMenuItem(iter->first, i, TRUE, &itemInfo);
        }

      // Executable
      if (type == IT_EXECUTABLE)
        {
          if (ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0")))
            {
              ELReadXMLStringValue(child, (WCHAR*)TEXT("WorkingDir"), workingDir, (WCHAR*)TEXT("\0"));
              ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
              NoPrefixString(name);
              MenuItem *menuItem = new MenuItem(name, type, value, workingDir, child);

              itemInfo.fMask = MIIM_STRING | MIIM_ID;
              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = name;
              if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
                {
                  itemInfo.hbmpItem = HBMMENU_CALLBACK;
                  itemInfo.fMask |= MIIM_BITMAP;
                }

              if (pSettings->GetMenuIcons())
                {
                  menuItem->SetIcon();
                  if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                    {
                      itemInfo.fMask |= MIIM_BITMAP;
                      itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                    }
                }

              itemInfo.wID = GetMenuItemCount(iter->first) + 1;

              iter->second->AddMenuItem(menuItem);

              InsertMenuItem(iter->first, i, TRUE, &itemInfo);
            }
        }

      // System Command
      if (type == IT_INTERNAL_COMMAND)
        {
          if (ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0")))
            {
              ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
              if (_wcsicmp(name, TEXT("DateTime")) == 0)
                type = 3;
              else
                {
                  NoPrefixString(name);
                  MenuItem *menuItem = new MenuItem(name, type, value, NULL, child);

                  itemInfo.fMask = MIIM_STRING | MIIM_ID;
                  itemInfo.cbSize = sizeof(MENUITEMINFO);
                  itemInfo.dwTypeData = name;
                  if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
                    {
                      itemInfo.hbmpItem = HBMMENU_CALLBACK;
                      itemInfo.fMask |= MIIM_BITMAP;
                    }

                  if (pSettings->GetMenuIcons())
                    {
                      menuItem->SetIcon();
                      if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                        {
                          itemInfo.fMask |= MIIM_BITMAP;
                          itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                        }
                    }

                  itemInfo.wID = GetMenuItemCount(iter->first) + 1;

                  iter->second->AddMenuItem(menuItem);

                  InsertMenuItem(iter->first, i, TRUE, &itemInfo);
                }
            }
        }

      if (type == IT_DATE_TIME)
        {
          if (ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0")))
            {
              MenuItem *menuItem = new MenuItem(NULL, type, value, NULL, child);
              WCHAR datetimeString[MAX_LINE_LENGTH];
              time_t tVal;
              struct tm *stVal;

              // Grab the current time
              _tzset();
              time(&tVal);
              stVal = localtime(&tVal);

              ELwcsftime(datetimeString, MAX_LINE_LENGTH, value, stVal);

              itemInfo.fMask = MIIM_STRING|MIIM_ID;
              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = datetimeString;
              itemInfo.wID = GetMenuItemCount(iter->first) + 1;

              iter->second->AddMenuItem(menuItem);

              InsertMenuItem(iter->first, i, TRUE, &itemInfo);
            }
        }

      // Special Folder
      if (type == IT_SPECIAL_FOLDER)
        {
          if (ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0")))
            {
              ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
              NoPrefixString(name);
              MenuItem *menuItem = new MenuItem(name, type, value, NULL, child);

              itemInfo.fMask = MIIM_STRING | MIIM_ID;
              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = name;
              if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
                {
                  itemInfo.hbmpItem = HBMMENU_CALLBACK;
                  itemInfo.fMask |= MIIM_BITMAP;
                }

              if (pSettings->GetMenuIcons())
                {
                  menuItem->SetIcon();
                  if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                    {
                      itemInfo.fMask |= MIIM_BITMAP;
                      itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                    }
                }

              itemInfo.wID = GetMenuItemCount(iter->first) + 1;

              iter->second->AddMenuItem(menuItem);

              InsertMenuItem(iter->first, i, TRUE, &itemInfo);
            }
        }

      // Registry based submenu
      if (type == IT_XML_MENU)
        {
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
          subSection = ELGetFirstXMLElementByName(child, (WCHAR*)TEXT("Submenu"), false);
          subMenu = CreatePopupMenu();
          NoPrefixString(name);
          MenuItem *menuItem = new MenuItem(name, type, NULL, NULL, child);

          itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = name;
          itemInfo.hSubMenu = subMenu;
          if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                {
                  itemInfo.fMask |= MIIM_BITMAP;
                  itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                }
            }

          itemInfo.wID = GetMenuItemCount(iter->first) + 1;

          iter->second->AddMenuItem(menuItem);
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          iter2 = menuMap.find(iter->first);
          if (iter2 != menuMap.end())
            {
              std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(name, type, NULL, subSection));
              menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(subMenu, mli));
            }
        }

      // Path based submenu
      if (type == IT_FILE_MENU)
        {
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0"));
          subMenu = CreatePopupMenu();
          NoPrefixString(name);
          MenuItem *menuItem = new MenuItem(name, type, value, NULL, child);

          itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = name;
          itemInfo.hSubMenu = subMenu;
          if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                {
                  itemInfo.fMask |= MIIM_BITMAP;
                  itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                }
            }

          itemInfo.wID = GetMenuItemCount(iter->first) + 1;
          iter->second->AddMenuItem(menuItem);
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          iter2 = menuMap.find(iter->first);
          if (iter2 != menuMap.end())
            {
              std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(name, type, value, NULL));
              menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(subMenu, mli));
            }
        }

      // Tasks(102) or Settings(103) submenu
      if ((type == IT_TASKS_MENU) || (type == IT_SETTINGS_MENU) || (type == IT_HELP_MENU))
        {
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
          subMenu = CreatePopupMenu();
          NoPrefixString(name);
          MenuItem *menuItem = new MenuItem(name, type, NULL, NULL, child);

          itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = name;
          itemInfo.hSubMenu = subMenu;
          if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                {
                  itemInfo.fMask |= MIIM_BITMAP;
                  itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                }
            }

          itemInfo.wID = GetMenuItemCount(iter->first) + 1;
          iter->second->AddMenuItem(menuItem);
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          iter2 = menuMap.find(iter->first);
          if (iter2 != menuMap.end())
            {
              std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(name, type, NULL, NULL));
              menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(subMenu, mli));
            }
        }

      tmp = child;
      child = ELGetSiblingXMLElement(tmp);

      i++;
    }

  if (!found && (iter->first == rootMenu))
    BuildDefaultMenu(iter);
}

void MenuBuilder::BuildDefaultMenu(MenuMap::iterator iter)
{
  WCHAR specialFolder[MAX_PATH], value[MAX_PATH];
  TiXmlElement *section, *xmlItem;

  section = iter->second->GetSection();
  if (section)
    {
      int type;

      std::wstring menuRoot = iter->second->GetName();
      if (menuRoot.compare(L"RightMenu") == 0)
        {
          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_SPECIAL_FOLDER;
          if (xmlItem)
            {
              ELGetSpecialFolder(CSIDL_DRIVES, specialFolder);
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), specialFolder);
              if (ELSpecialFolderValue(specialFolder, value))
                ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), value);
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELGetSpecialFolder(CSIDL_BITBUCKET, specialFolder);
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), specialFolder);
              if (ELSpecialFolderValue(specialFolder, value))
                ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), value);
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_FILE_MENU;
          if (xmlItem)
            {
              ELGetSpecialFolder(CSIDL_STARTMENU, specialFolder);
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), specialFolder);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("%StartMenu%|%CommonStartMenu%"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Quick Launch"));
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"),
                                    (WCHAR*)TEXT("%AppData%\\Microsoft\\Internet Explorer\\Quick Launch"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELGetSpecialFolder(CSIDL_DESKTOP, specialFolder);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), specialFolder);
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("%Desktop%|%CommonDesktop%"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_SETTINGS_MENU;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Settings"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("\0"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_HELP_MENU;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Help"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("\0"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_EXECUTABLE;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Display Properties"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("desk.cpl"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_INTERNAL_COMMAND;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Run"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("Run"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Logoff"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("Logoff"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Shutdown"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("Shutdown"));
            }

          ELWriteXMLConfig(ELGetXMLConfig(xmlItem));
        }
      else if (menuRoot.compare(L"MidMenu") == 0)
        {
          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_TASKS_MENU;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Tasks"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("\0"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          type = IT_INTERNAL_COMMAND;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Show Applets"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("Show"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Hide Applets"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("Hide"));
            }

          xmlItem = ELSetFirstXMLElement(section, (WCHAR*)TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, (WCHAR*)TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Name"), (WCHAR*)TEXT("Show Desktop"));
              ELWriteXMLStringValue(xmlItem, (WCHAR*)TEXT("Value"), (WCHAR*)TEXT("ShowDesktop"));
            }

          ELWriteXMLConfig(ELGetXMLConfig(xmlItem));
        }
    }

  BuildXMLMenu(iter);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	BuildFileMenu
// Requires:	UINT location - position of menu in menu vector
// 		LPTSTR inputString - path to evaluate
// Returns:	Nothing
// Purpose:	Enumerates the path adding entries to the specified menu
//----  --------------------------------------------------------------------------------------------------------
void MenuBuilder::BuildFileMenuFromString(MenuMap::iterator iter, WCHAR *parsedValue)
{
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  WCHAR searchPath[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH], *lwrEntry, *lwrValue;
  MenuMap::iterator iter2;
  std::wstring workingValue = parsedValue;

  workingValue = ELExpandVars(workingValue);
  wcscpy(searchPath, workingValue.c_str());
  wcscat(searchPath, TEXT("\\*"));
  fileHandle = FindFirstFile(searchPath, &findData);
  if (fileHandle == INVALID_HANDLE_VALUE)
    return;

  do
    {
      // Skip hidden files
      if (wcscmp(findData.cFileName, TEXT(".")) == 0 ||
          wcscmp(findData.cFileName, TEXT("..")) == 0 ||
          (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        continue;

      wcscpy(tmp, workingValue.c_str());
      if (tmp[wcslen(tmp) - 1] != '\\')
        wcscat(tmp, TEXT("\\"));
      wcscat(tmp, findData.cFileName);

      if (wcsstr(tmp, TEXT("target.lnk")) != NULL)
        {
          SHORTCUTINFO shortcutInfo;
          shortcutInfo.flags = SI_PATH;

          if (ELParseShortcut(tmp, &shortcutInfo))
            {
              if (PathIsDirectory(shortcutInfo.Path))
                {
                  iter->second->SetValue(shortcutInfo.Path);
                  BuildFileMenu(iter);
                  break;
                }
            }
        }

      // Process a subdirectory
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          UINT folderPos, itemID;
          ULONG_PTR itemData;
          MenuItem *menuItem;
          std::tr1::shared_ptr<MenuListItem> mli;

          if (!GetPos(iter, findData.cFileName, true, &folderPos, &itemID, &itemData))
            {
              menuItem = new MenuItem(NULL, 101, tmp, NULL, NULL);
              MENUITEMINFO itemInfo;

              itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
              if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
                {
                  itemInfo.hbmpItem = HBMMENU_CALLBACK;
                  itemInfo.fMask |= MIIM_BITMAP;
                }

              if (pSettings->GetMenuIcons())
                {
                  menuItem->SetIcon();
                  if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                    {
                      itemInfo.fMask |= MIIM_BITMAP;
                      itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                    }
                }

              mli = std::tr1::shared_ptr<MenuListItem>(new MenuListItem(NULL, 101, tmp, NULL));
              HMENU subMenu = CreatePopupMenu();

              wcscpy(tmp, findData.cFileName);

              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = tmp;
              itemInfo.hSubMenu = subMenu;

              NoPrefixString(tmp);
              menuItem->SetName(tmp);
              mli->SetName(tmp);

              itemInfo.wID = GetMenuItemCount(iter->first) + 1;
              itemInfo.dwItemData = (ULONG_PTR)subMenu;

              iter->second->AddMenuItem(menuItem);
              InsertMenuItem(iter->first, folderPos, TRUE, &itemInfo);

              iter2 = menuMap.find(iter->first);
              if (iter2 != menuMap.end())
                menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(subMenu, mli));
            }
          else
            {
              WCHAR path[MAX_LINE_LENGTH];
              iter2 = menuMap.find((HMENU)itemData);

              menuItem = iter->second->GetMenuItem(itemID - 1);
              mli = iter2->second;

              wcscpy(path, menuItem->GetValue());
              wcscat(path, TEXT("|"));
              wcscat(path, tmp);

              menuItem->SetValue(path);
              mli->SetValue(path);
            }
        }
      // Executable
      else
        {
          MENUITEMINFO itemInfo;
          UINT filePos, itemID;
          ULONG_PTR itemData;
          WCHAR entry[MAX_LINE_LENGTH], extension[MAX_PATH];
          MenuItem *menuItem;
          wcscpy(extension, PathFindExtension(tmp));
          bool isShortcut = (_wcsicmp(extension, TEXT(".lnk")) == 0) ||
                            (_wcsicmp(extension, TEXT(".pif")) == 0) ||
                            (_wcsicmp(extension, TEXT(".scf")) == 0) ||
                            (_wcsicmp(extension, TEXT(".pnagent")) == 0) ||
                            (_wcsicmp(extension, TEXT(".url")) == 0);

          wcscpy(entry, tmp);
          wcscpy(tmp, findData.cFileName);
          if (isShortcut)
            PathRemoveExtension(tmp);

          if (GetPos(iter, tmp, false, &filePos, &itemID, &itemData))
            {
              SHORTCUTINFO entryInfo, targetInfo;
              entryInfo.flags = SI_PATH;
              targetInfo.flags = SI_PATH;
              WCHAR targetURL[MAX_LINE_LENGTH], entryURL[MAX_LINE_LENGTH];

              menuItem = iter->second->GetMenuItem(itemID - 1);
              lwrEntry = _wcslwr(_wcsdup(entry));
              lwrValue = _wcslwr(_wcsdup(menuItem->GetValue()));

              if (ELParseShortcut(entry, &entryInfo))
                {
                  if (ELParseShortcut(menuItem->GetValue(), &targetInfo))
                    {
                      if (_wcsicmp(entryInfo.Path, targetInfo.Path) == 0)
                        {
                          free(lwrEntry);
                          free(lwrValue);
                          continue;
                        }
                    }
                }
              else if ((wcsstr(lwrEntry, TEXT(".url")) != NULL) &&
                       (wcsstr(lwrValue, TEXT(".url")) != NULL))
                {
                  if (ELReadFileString(entry, (WCHAR*)TEXT("URL"), entryURL, (WCHAR*)TEXT("")) &&
                      ELReadFileString(menuItem->GetValue(), (WCHAR*)TEXT("URL"), targetURL, (WCHAR*)TEXT("")))
                    {
                      if (_wcsicmp(entryURL, targetURL) == 0)
                        {
                          free(lwrEntry);
                          free(lwrValue);
                          continue;
                        }
                    }
                }

              free(lwrEntry);
              free(lwrValue);
            }

          menuItem = new MenuItem(NULL, 1, entry, NULL, NULL);

          itemInfo.fMask = MIIM_STRING | MIIM_ID;
          if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
                {
                  itemInfo.fMask |= MIIM_BITMAP;
                  itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                }
            }

          NoPrefixString(tmp);
          menuItem->SetName(tmp);

          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = tmp;

          itemInfo.wID = GetMenuItemCount(iter->first) + 1;

          iter->second->AddMenuItem(menuItem);

          InsertMenuItem(iter->first, filePos, TRUE, &itemInfo);
        }
    }
  while (FindNextFile(fileHandle, &findData));

  FindClose(fileHandle);
}

void MenuBuilder::BuildFileMenu(MenuMap::iterator iter)
{
  WCHAR tmp[MAX_PATH], inputString[MAX_LINE_LENGTH];
  WCHAR delimiter = '|';
  UINT i = 0, j = 0;

  wcscpy(inputString, iter->second->GetValue());

  while (i < wcslen(inputString))
    {
      if (inputString[i] == delimiter)
        {
          tmp[j] = '\0';
          BuildFileMenuFromString(iter, tmp);

          j = 0;
        }
      else
        {
          tmp[j] = inputString[i];
          j++;
        }

      i++;
    }

  tmp[j] = '\0';
  BuildFileMenuFromString(iter, tmp);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	BuildTasksMenu
// Requires:	HWND hwnd - window handle
// 		LPARAM lParam - location of th menu in the menu vector
// Returns:	BOOL
// Purpose:	Enumerates all the valid windows putting an entry in the specified menu
//----  --------------------------------------------------------------------------------------------------------
BOOL CALLBACK MenuBuilder::BuildTasksMenu(HWND hwnd, LPARAM lParam)
{
  MenuBuilder *pMenuBuilder = reinterpret_cast<MenuBuilder*>(lParam);

  if (ELCheckWindow(hwnd))
    pMenuBuilder->AddTaskItem(hwnd);

  return true;
}

void MenuBuilder::AddTaskItem(HWND task)
{
  WCHAR windowTitle[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH];
  MENUITEMINFO itemInfo;
  MenuItem *menuItem;
  MenuMap::iterator iter;

  GetWindowText(task, windowTitle, MAX_LINE_LENGTH);
  NoPrefixString(windowTitle);

  iter = menuMap.find(taskMenu);
  if (iter == menuMap.end())
    return;

  swprintf(tmp, TEXT("%d"), (ULONG_PTR)task);
  menuItem = new MenuItem(windowTitle, 0, tmp, NULL, NULL);

  itemInfo.fMask = MIIM_STRING | MIIM_ID;
  if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
    {
      itemInfo.hbmpItem = HBMMENU_CALLBACK;
      itemInfo.fMask |= MIIM_BITMAP;
    }

  if (pSettings->GetMenuIcons())
    {
      menuItem->SetIcon();
      if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
        {
          itemInfo.fMask |= MIIM_BITMAP;
          itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
        }
    }

  itemInfo.cbSize = sizeof(MENUITEMINFO);
  itemInfo.dwTypeData = windowTitle;

  itemInfo.wID = GetMenuItemCount(iter->first) + 1;

  iter->second->AddMenuItem(menuItem);

  InsertMenuItem(iter->first, itemInfo.wID, TRUE, &itemInfo);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	NoPrefixString
// Requires:	WCHAR *source - source string
// Returns:	bool
// Purpose:	duplicated ampersands to remove menu prefixes
//----  --------------------------------------------------------------------------------------------------------
bool MenuBuilder::NoPrefixString(WCHAR *source)
{
  bool result = false;
  WCHAR tmp[MAX_LINE_LENGTH], *token = NULL;
  ZeroMemory(tmp, MAX_LINE_LENGTH);

  token = wcstok(source, TEXT("&"));

  while (token != NULL)
    {
      result = true;
      wcscat(tmp, token);
      token = wcstok(NULL, TEXT("&"));
      if (token != NULL)
        wcscat(tmp, TEXT("&&"));
    }

  if (wcslen(tmp) != 0)
    wcscpy(source, tmp);

  return result;
}

LRESULT MenuBuilder::DoButtonDown(UINT button)
{
  WCHAR menuName[MAX_LINE_LENGTH];
  LRESULT ret = 1;
  POINT mousePT;
  std::tr1::shared_ptr<MenuListItem> mli;
  MenuMap::iterator iter;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section = NULL, *menu = NULL;
  std::wstring xmlPath = TEXT("%EmergeDir%\\files\\");
  xmlPath = ELExpandVars(xmlPath);
  if (!PathIsDirectory(xmlPath.c_str()))
    ELCreateDirectory(xmlPath);
  std::wstring xmlFile = xmlPath + TEXT("emergeWorkspace.xml");
  if (!PathFileExists(xmlFile.c_str()))
    {
      std::wstring oldXmlFile = xmlPath + TEXT("emergeDesktop.xml");
      if (PathFileExists(oldXmlFile.c_str()))
        ELFileOp(menuWnd, FO_RENAME, oldXmlFile, xmlFile);
    }

  if (pMenuEditor->GetVisible())
    return 0;

  GetCursorPos(&mousePT);

  ELStealFocus(menuWnd);
  SetWindowPos(menuWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
  SendMessage(menuWnd, WM_CANCELMODE, 0, 0);

  if (button == WM_LBUTTONDOWN)
    return 0;

  configXML = ELOpenXMLConfig(xmlFile, true);
  if (configXML)
    {
      section = ELGetXMLSection(configXML.get(), (WCHAR*)TEXT("Menus"), true);

      if (section)
        {

          switch (button)
            {
            case WM_MBUTTONDOWN:
              wcscpy(menuName, TEXT("MidMenu"));
              break;
            case WM_RBUTTONDOWN:
              wcscpy(menuName, TEXT("RightMenu"));
              break;
            }

          menu = ELGetFirstXMLElementByName(section, menuName, true);
          if (menu)
            {
              ClearAllMenus();
              mli = std::tr1::shared_ptr<MenuListItem>(new MenuListItem(menuName, 100, NULL, menu));
              rootMenu = CreatePopupMenu();
              menuMap.insert(std::pair< HMENU, std::tr1::
                             shared_ptr<MenuListItem> >(rootMenu, mli));
              iter = menuMap.begin();

              UINT itemID = TrackPopupMenuEx(rootMenu, TPM_RETURNCMD|TPM_RECURSE, mousePT.x, mousePT.y, menuWnd, NULL);
              if (itemID != 0)
                ExecuteMenuItem(itemID);

              ClearAllMenus();
              rootMenu = NULL;

              ret = 0;
            }
        }
    }

  return ret;
}

LRESULT MenuBuilder::DoInitMenu(HMENU menu)
{
  MenuMap::iterator iter;
  POINT mousePT;
  MONITORINFO monitorInfo;
  MENUINFO menuInfo;
  HMONITOR monitor;
  UINT monitorHeight;

  iter = menuMap.find(menu);
  if (iter == menuMap.end())
    return 1;

  GetCursorPos(&mousePT);

  monitor = MonitorFromPoint(mousePT, MONITOR_DEFAULTTONEAREST);
  monitorInfo.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(monitor, &monitorInfo);
  monitorHeight = abs(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top);

  menuInfo.cbSize = sizeof(MENUINFO);
  menuInfo.fMask = MIM_MAXHEIGHT | MIM_STYLE;
  menuInfo.cyMax = monitorHeight;
  // Temporarily disable Drag-and-Drop until it does something useful
  //menuInfo.dwStyle |= MNS_DRAGDROP;
  menuInfo.dwStyle = MNS_CHECKORBMP;
  SetMenuInfo(menu, &menuInfo);

  BuildMenu(iter);

  return 0;
}

LRESULT MenuBuilder::ExecuteMenuItem(UINT itemID)
{
  MenuMap::iterator iter;
  MenuItem *menuItem;
  itemID--;
  WCHAR error[MAX_LINE_LENGTH];

  iter = menuMap.find(SelectedMenu);
  if (iter == menuMap.end())
    return 1;

  menuItem = iter->second->GetMenuItem(itemID);
  swprintf(error, TEXT("Failed to execute \"%s\""), menuItem->GetValue());

  switch (iter->second->GetType())
    {
    case IT_XML_MENU:
      ExecuteXMLMenuItem(menuItem->GetType(),
                         menuItem->GetValue(),
                         menuItem->GetWorkingDir());
      break;
    case IT_FILE_MENU:
      if (!ELExecute(menuItem->GetValue()))
        ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeWorkspace"), ELMB_ICONWARNING|ELMB_OK);
      break;
    case IT_TASKS_MENU:
      ELSwitchToThisWindow((HWND)_wtoi(menuItem->GetValue()));
      break;
    case IT_SETTINGS_MENU:
      ExecuteSettingsMenuItem(itemID);
      break;
    case IT_HELP_MENU:
      ExecuteHelpMenuItem(itemID);
      break;
    }

  return 0;
}

bool MenuBuilder::GetPos(MenuMap::iterator iter, WCHAR *input, bool directory, UINT *pos, UINT *itemID, ULONG_PTR *itemData)
{
  UINT index = 0, menuListSize;
  std::vector<MenuListItem*>::iterator menuListIter;
  WCHAR normalizedInput[MAX_LINE_LENGTH], normalizedValue[MAX_LINE_LENGTH], buffer[MAX_LINE_LENGTH];
  MENUITEMINFO mii;

  (*pos) = 0;

  ZeroMemory(&mii, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);

  ZeroMemory(normalizedInput, MAX_LINE_LENGTH);
  LCMapString(LOCALE_USER_DEFAULT, LCMAP_UPPERCASE,
              input, (int)wcslen(input),
              normalizedInput, MAX_LINE_LENGTH);

  menuListSize = GetMenuItemCount(iter->first);

  while (index < menuListSize)
    {
      ZeroMemory(normalizedValue, MAX_LINE_LENGTH);
      mii.fMask = MIIM_STRING | MIIM_SUBMENU | MIIM_ID | MIIM_DATA;
      mii.dwTypeData = buffer;
      mii.cch = MAX_LINE_LENGTH;
      GetMenuItemInfo(iter->first,
                      index,
                      TRUE,
                      &mii);
      LCMapString(LOCALE_USER_DEFAULT, LCMAP_UPPERCASE,
                  mii.dwTypeData, mii.cch,
                  normalizedValue, MAX_LINE_LENGTH);

      (*itemData) = mii.dwItemData;
      (*itemID) = mii.wID;

      if (mii.hSubMenu != NULL)
        {
          if (directory)
            {
              if (CompareString(LOCALE_USER_DEFAULT,
                                NORM_IGNORECASE|NORM_IGNOREKANATYPE|NORM_IGNOREWIDTH,
                                normalizedInput, (int)wcslen(normalizedInput),
                                normalizedValue, (int)wcslen(normalizedValue))
                  == CSTR_EQUAL)
                return true;

              if (CompareString(LOCALE_USER_DEFAULT,
                                NORM_IGNORECASE|NORM_IGNOREKANATYPE|NORM_IGNOREWIDTH,
                                normalizedInput, (int)wcslen(normalizedInput),
                                normalizedValue, (int)wcslen(normalizedValue))
                  == CSTR_LESS_THAN)
                break;

              (*pos)++;
            }
          else
            {
              (*pos)++;
              index++;
              continue;
            }
        }
      else
        {
          if (directory)
            {
              index++;
              continue;
            }
          else
            {
              if (CompareString(LOCALE_USER_DEFAULT,
                                NORM_IGNORECASE|NORM_IGNOREKANATYPE|NORM_IGNOREWIDTH,
                                normalizedInput, (int)wcslen(normalizedInput),
                                normalizedValue, (int)wcslen(normalizedValue))
                  == CSTR_EQUAL)
                return true;

              if (CompareString(LOCALE_USER_DEFAULT,
                                NORM_IGNORECASE|NORM_IGNOREKANATYPE|NORM_IGNOREWIDTH,
                                normalizedInput, (int)wcslen(normalizedInput),
                                normalizedValue, (int)wcslen(normalizedValue))
                  == CSTR_LESS_THAN)
                break;

              (*pos)++;
            }
        }

      index++;
    }

  return false;
}

void MenuBuilder::BuildSettingsMenu(MenuMap::iterator iter)
{
  AddSettingsItem(iter, (WCHAR*)TEXT("Configure Workspace"), BSM_CONFIGURE);
  AddSettingsItem(iter, (WCHAR*)TEXT("Configure Core"), BSM_CORE);
  AddSettingsItem(iter, (WCHAR*)TEXT("\0"), BSM_SEPARATOR);
  AddSettingsItem(iter, (WCHAR*)TEXT("Theme Manager"), BSM_SELECTTHEME);
  AddSettingsItem(iter, (WCHAR*)TEXT("\0"), BSM_SEPARATOR);
  AddSettingsItem(iter, (WCHAR*)TEXT("Change Desktop Shell"), BSM_SHELL);
  AddSettingsItem(iter, (WCHAR*)TEXT("Quit Emerge Desktop"), BSM_QUIT);
}

void MenuBuilder::BuildHelpMenu(MenuMap::iterator iter)
{
  AddSettingsItem(iter, (WCHAR*)TEXT("Offline Help"), BHM_OFFLINE);
  AddSettingsItem(iter, (WCHAR*)TEXT("Online Tutorial"), BHM_TUTORIAL);
  AddSettingsItem(iter, (WCHAR*)TEXT("Emerge Desktop Website"), BHM_WEBSITE);
  AddSettingsItem(iter, (WCHAR*)TEXT("\0"), BHM_SEPARATOR);
  AddSettingsItem(iter, (WCHAR*)TEXT("About"), BHM_ABOUT);
}

void MenuBuilder::AddSettingsItem(MenuMap::iterator iter, WCHAR* text, UINT id)
{
  MENUITEMINFO itemInfo;
  MenuItem *menuItem = new MenuItem(text, 0, text, NULL, NULL);
  UINT index = GetMenuItemCount(iter->first);

  itemInfo.fMask = MIIM_STRING | MIIM_ID;
  if (id == BSM_SEPARATOR)
    {
      itemInfo.fMask = MIIM_FTYPE | MIIM_ID;
      itemInfo.fType = MFT_SEPARATOR;
    }

  if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
    {
      itemInfo.hbmpItem = HBMMENU_CALLBACK;
      itemInfo.fMask |= MIIM_BITMAP;
    }

  itemInfo.cbSize = sizeof(MENUITEMINFO);
  itemInfo.wID = id;
  itemInfo.dwTypeData = text;

  iter->second->AddMenuItem(menuItem);
  InsertMenuItem(iter->first, index, TRUE, &itemInfo);
}

void MenuBuilder::ExecuteXMLMenuItem(UINT type, WCHAR *value, WCHAR *workingDir)
{
  bool ret = false;
  WCHAR error[MAX_LINE_LENGTH];
  swprintf(error, TEXT("Failed to execute \"%s\""), value);

  switch (type)
    {
    case IT_EXECUTABLE:
      ret = ELExecute(value, workingDir);
      break;
    case IT_INTERNAL_COMMAND:
      ret = ELExecuteInternal(value);
      break;
    case IT_DATE_TIME:
      ret = ELExecute((WCHAR*)TEXT("timedate.cpl"));
      break;
    case IT_SPECIAL_FOLDER:
      ret = ELExecuteSpecialFolder(value);
      break;
    }

  if (!ret)
    ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeWorkspace"), ELMB_ICONWARNING|ELMB_OK);
}

void MenuBuilder::ExecuteSettingsMenuItem(UINT index)
{
  std::wstring aliasFile;
  Config config(mainInst, pSettings);

  switch (++index)
    {
      int res;
    case BSM_CONFIGURE:
      res = config.Show();
      if (res == IDOK)
        {
          SetWorkArea();
          UpdateMenuHook();
        }
      break;
    case BSM_CORE:
      ELExecuteInternal((WCHAR*)TEXT("CoreSettings"));
      break;
    case BSM_SHELL:
      ELExecuteInternal((WCHAR*)TEXT("ShellChanger"));
      break;
    case BSM_SELECTTHEME:
      ELExecuteInternal((WCHAR*)TEXT("ThemeManager"));
      break;
    case BSM_QUIT:
      ELQuit(true);
      break;
    }
}

void MenuBuilder::ExecuteHelpMenuItem(UINT index)
{
  switch (++index)
    {
    case BHM_ABOUT:
      ELExecuteInternal((WCHAR*)TEXT("About"));
      break;
    case BHM_OFFLINE:
      ELExecute((WCHAR*)TEXT("%AppletDir%\\Documentation\\Emerge Desktop.chm"));
      break;
    case BHM_WEBSITE:
      ELExecuteInternal((WCHAR*)TEXT("Homepage"));
      break;
    case BHM_TUTORIAL:
      ELExecuteInternal((WCHAR*)TEXT("Tutorial"));
      break;
    }
}

void MenuBuilder::UpdateMenuHook()
{
  globalMenuAlpha = pSettings->GetMenuAlpha();

  // Clear the menu hook
  if (menuHook)
    UnhookWindowsHookEx(menuHook);

  // Hook the menus (for transparency)
  menuHook = SetWindowsHookEx(WH_CALLWNDPROC, HookCallWndProc, 0, GetWindowThreadProcessId(menuWnd, 0));
}

BOOL CALLBACK MenuBuilder::SetMonitorArea(HMONITOR hMonitor, HDC hdcMonitor UNUSED, LPRECT lprcMonitor UNUSED, LPARAM dwData)
{
  Settings *pSettings = reinterpret_cast<Settings*>(dwData);
  MONITORINFO monInfo;
  monInfo.cbSize = sizeof(monInfo);

  GetMonitorInfo(hMonitor, &monInfo);

  if ((monInfo.dwFlags==1 && pSettings->GetBorderPrimary()) || !pSettings->GetBorderPrimary())
    pSettings->GetDesktopRect(&monInfo.rcMonitor);

  SystemParametersInfo(SPI_SETWORKAREA, 0, &monInfo.rcMonitor, SPIF_SENDCHANGE);

  return TRUE;
}

void MenuBuilder::SetWorkArea()
{
  HDC hdc = GetDC(NULL);
  SystemParametersInfo(SPI_GETWORKAREA, 0, &explorerWorkArea, 0);

  if (GetSystemMetrics(SM_CMONITORS) < 2)
    {
      RECT areaRect;
      areaRect.top = 0;
      areaRect.left = 0;
      areaRect.bottom = GetSystemMetrics(SM_CYSCREEN);
      areaRect.right = GetSystemMetrics(SM_CXSCREEN);
      pSettings->GetDesktopRect(&areaRect);
      SystemParametersInfo(SPI_SETWORKAREA, 0, &areaRect, SPIF_SENDCHANGE);
    }
  else
    EnumDisplayMonitors(hdc, NULL, SetMonitorArea, reinterpret_cast<LPARAM>(pSettings.get()));
  ReleaseDC(NULL, hdc);
}
