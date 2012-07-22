// vim:tags+=../emergeLib/tags,../emergeAppletEngine/tags,../emergeGraphics/tags

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

#include "MenuBuilder.h"
#include "CustomDropSource.h"
#include "CustomDataObject.h"

WCHAR menuBuilderClass[ ] = TEXT("EmergeDesktopMenuBuilder");

MenuBuilder::MenuBuilder(HINSTANCE desktopInst)
{
  mainInst = desktopInst;
  menuWnd = NULL;
  rootMenu = NULL;
  taskMenu = NULL;
  MButtonDown = false;
  registered = false;
  winVersion = ELVersionInfo();
  SetRectEmpty(&explorerWorkArea);
  ShellMessage = 0;
  activeMenu = NULL;

  menuHook = SetWindowsHookEx(WH_CALLWNDPROC, HookCallWndProc, 0, GetCurrentThreadId());
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
  pItemEditor = std::tr1::shared_ptr<ItemEditor>(new ItemEditor(mainInst, menuWnd));

  SetWorkArea();

  // Format the time
  setlocale(LC_TIME, "");

  // Register to recieve the specified Emerge Desktop messages
  PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)menuWnd, (LPARAM)EMERGE_CORE);

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
        ELFileOp(menuWnd, false, FO_RENAME, oldXMLFile, xmlFile);
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
      // Register to recieve the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_REGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

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

    case WM_MEASUREITEM:
      return pMenuBuilder->DoMeasureItem((LPMEASUREITEMSTRUCT)lParam);

    case WM_DRAWITEM:
      return pMenuBuilder->DoDrawItem((LPDRAWITEMSTRUCT)lParam);

    case WM_CONTEXTMENU:
      return pMenuBuilder->DoContextMenu();

    case WM_MENUDRAG:
      return pMenuBuilder->DoMenuDrag(hwnd, (UINT)wParam, (HMENU)lParam);

    case WM_MENUGETOBJECT:
      return pMenuBuilder->DoMenuGetObject(hwnd, (MENUGETOBJECTINFO*)lParam);

    case WM_DISPLAYCHANGE:
      pMenuBuilder->SetWorkArea();
      break;

    case WM_DESTROY:
    case WM_NCDESTROY:
      // Unregister the specified Emerge Desktop messages
      PostMessage(ELGetCoreWindow(), EMERGE_UNREGISTER, (WPARAM)hwnd, (LPARAM)EMERGE_CORE);

      PostQuitMessage(0);
      break;

    default:
      return pMenuBuilder->DoDefault(hwnd, message, wParam, lParam);
    }

  return 0;
}

LRESULT MenuBuilder::DoCopyData(COPYDATASTRUCT *cds)
{
  if (cds->dwData == EMERGE_NEWITEM)
    {
      LPNEWMENUITEMDATA newMenuItemData = reinterpret_cast< LPNEWMENUITEMDATA >(cds->lpData);

      DropMenuItem(&newMenuItemData->menuItemData, newMenuItemData->newElement, newMenuItemData->menu, newMenuItemData->pt);

      return 1;
    }

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
            {
              Config config(mainInst, pSettings);
              if (config.Show() == IDOK)
                SetWorkArea();
            }
            break;

            case CORE_SHOWCONFIG:
            {
              if ((notifyInfo->InstanceName != NULL) && wcslen(notifyInfo->InstanceName))
                {
                  if (_wcsicmp(notifyInfo->InstanceName, TEXT("emergeWorkspace")) == 0)
                    {
                      Config config(mainInst, pSettings);
                      if (config.Show() == IDOK)
                        SetWorkArea();
                    }
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
  IID menuInterface = IID_IDropTarget;
  MENUITEMINFO menuItemInfo;
  IDropTarget *dropTarget = NULL;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;

  if (!GetMenuItemInfo(mgoInfo->hmenu, mgoInfo->uPos, TRUE, &menuItemInfo))
    return MNGO_NOINTERFACE;

  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(menuItemInfo.wID)->second;
  if (!menuItem)
    return MNGO_NOINTERFACE;

  dropTarget = menuItem->GetDropTarget();

  mgoInfo->riid = &menuInterface;
  mgoInfo->pvObj = dropTarget;

  return MNGO_NOERROR;
}

LRESULT MenuBuilder::DoMenuDrag(HWND hwnd UNUSED, UINT pos, HMENU menu)
{
  MenuMap::iterator subIter;
  MENUITEMINFO menuItemInfo;
  DWORD effect, dropEffects;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;

  if (!GetMenuItemInfo(menu, pos, TRUE, &menuItemInfo))
    return MND_CONTINUE;

  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(menuItemInfo.wID)->second;
  if (!menuItem)
    return MND_CONTINUE;

  if ((menuItem->GetType() == IT_TASK) ||
      (menuItem->GetType() == IT_HELP_ITEM) ||
      (menuItem->GetType() == IT_SETTING_ITEM))
    return MND_CONTINUE;

  FORMATETC fmtetc;
  STGMEDIUM stgmed;
  MENUITEMDATA menuItemData;

  UINT CF_EMERGE_MENUITEM = RegisterClipboardFormat(TEXT("CF_EMERGE_MENUITEM"));
  if (CF_EMERGE_MENUITEM == 0)
    ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Failed to register Emerge Desktop Menu Item clipboard format."), (WCHAR*)TEXT("emergeWorkspace"),
                 ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);

  ZeroMemory(&fmtetc, sizeof(FORMATETC));
  ZeroMemory(&stgmed, sizeof(STGMEDIUM));
  fmtetc.dwAspect = DVASPECT_CONTENT;
  fmtetc.lindex = -1;
  fmtetc.tymed = TYMED_HGLOBAL;
  stgmed.tymed = fmtetc.tymed;


  if ((menuItem->GetType() == IT_FILE) || (menuItem->GetType() == IT_FILE_SUBMENU))
    {
      fmtetc.cfFormat = CF_HDROP;
      stgmed.hGlobal = FileToHandle(menuItem->GetValue());
    }
  else
    {
      ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
      wcscpy(menuItemData.name, menuItem->GetName());
      menuItemData.type = menuItem->GetType();
      wcscpy(menuItemData.value, menuItem->GetValue());
      wcscpy(menuItemData.workingDir, menuItem->GetWorkingDir());
      menuItemData.element =  menuItem->GetElement();

      fmtetc.cfFormat = CF_EMERGE_MENUITEM;
      stgmed.hGlobal = MenuItemDataToHandle(&menuItemData);
    }

  dropEffects = DROPEFFECT_MOVE | DROPEFFECT_COPY;

  IDataObject *pDataObject;
  IDropSource *pDropSource;

  CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);
  CreateDropSource(&pDropSource);

  if (DoDragDrop(pDataObject, pDropSource, dropEffects, &effect) == DRAGDROP_S_DROP)
    {
      if (effect == DROPEFFECT_MOVE)
        {
          if ((menuItem->GetType() != IT_FILE) && (menuItem->GetType() != IT_FILE_SUBMENU))
            {
              // Remove the menu element
              TiXmlElement *menuElement = menuItem->GetElement();
              TiXmlDocument *configXML = ELGetXMLConfig(menuElement);
              if (ELRemoveXMLElement(menuElement))
                {
                  ELWriteXMLConfig(configXML);
                  if (menuItem->GetType() == IT_XML_MENU)
                    {
                      subIter = menuMap.find(menuItemInfo.hSubMenu);
                      if (subIter != menuMap.end())
                        ClearMenu(subIter);
                    }
                  DeleteMenu(menu, menuItem->GetID(), MF_BYCOMMAND);
                }
            }
        }
    }

  pDropSource->Release();
  pDataObject->Release();

  ReleaseStgMedium(&stgmed);

  return MND_CONTINUE;
}

HDROP MenuBuilder::FileToHandle(WCHAR *file)
{
  // allocate and lock a global memory buffer.
  HDROP ptr = (HDROP)GlobalAlloc(GMEM_ZEROINIT, sizeof(DROPFILES) + (sizeof(WCHAR) * (wcslen(file) + 2)));

  LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock(ptr);
  pDropFiles->pFiles = sizeof(DROPFILES);
  pDropFiles->fWide = TRUE;

  LPBYTE pData = (LPBYTE) pDropFiles + sizeof(DROPFILES);

  // copy the string into the buffer
  CopyMemory(pData, file, sizeof(WCHAR) * (wcslen(file) + 1));
  GlobalUnlock(ptr);

  return ptr;
}

HANDLE MenuBuilder::MenuItemDataToHandle(MENUITEMDATA *menuItemData)
{
  void *ptr = NULL;

  // allocate and lock a global memory buffer. Make it fixed
  // data so we don't have to use GlobalLock
  ptr = (void *)GlobalAlloc(GMEM_FIXED, sizeof(MENUITEMDATA));
  if (ptr != NULL)
    CopyMemory(ptr, menuItemData, sizeof(MENUITEMDATA));

  return reinterpret_cast< HANDLE >(ptr);
}

LRESULT MenuBuilder::DoContextMenu()
{
  MENUITEMINFO menuItemInfo;
  TiXmlElement *element;
  TiXmlDocument *configXML;
  WCHAR value[MAX_LINE_LENGTH];
  POINT pt;
  int index;

  GetCursorPos(&pt);
  index = MenuItemFromPoint(menuWnd, activeMenu, pt);
  if (index == -1)
    return 0;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID;
  if (!GetMenuItemInfo(activeMenu, index, TRUE, &menuItemInfo))
    return 0;

  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(menuItemInfo.wID)->second;
  if (!menuItem)
    return 0;

  wcscpy(value, menuItem->GetValue());
  element = menuItem->GetElement();

  switch (menuItem->GetType())
    {
      int res;
    case IT_HELP_ITEM:
    case IT_SETTING_ITEM:
      break;
    case IT_TASK:
    {
#ifdef _W64
      HWND task = (HWND)_wcstoi64(value, NULL, 10);
#else
      HWND task = (HWND)wcstol(value, NULL, 10);
#endif
      res = EAEDisplayMenu(menuWnd, task);
      if (res)
        {
          if (res == SC_CLOSE)
            DeleteMenu(activeMenu, index, MF_BYPOSITION);
          else
            {
              SendMessage(menuWnd, WM_CANCELMODE, 0, 0);
              ELStealFocus(task);
            }
          PostMessage(task, WM_SYSCOMMAND, (WPARAM)res, MAKELPARAM(pt.x, pt.y));
        }
    }
    break;
    case IT_FILE_SUBMENU:
    case IT_FILE:
      res = EAEDisplayFileMenu(value, menuWnd);
      if (res != 0)
        SendMessage(menuWnd, WM_CANCELMODE, 0, 0);
      break;
    default:
      res = DisplayRegContext(pt, menuItem->GetType());
      switch (res)
        {
        case DRM_DELETE:
          configXML = ELGetXMLConfig(element);
          if (ELRemoveXMLElement(element))
            {
              if (ELWriteXMLConfig(configXML))
                DeleteMenu(activeMenu, index, MF_BYPOSITION);
            }
          break;
        case DRM_EDIT:
          EditMenuItem(activeMenu, index);
          break;
        case DRM_ADD:
          AddMenuItem(activeMenu, index);
          break;
        case DRM_RUNAS:
          ElevatedExecute(menuItem);
          break;
        }
      break;
    }

  return 1;
}

void MenuBuilder::ElevatedExecute(std::tr1::shared_ptr<MenuItem> menuItem)
{
  ELExecute(menuItem->GetValue(), menuItem->GetWorkingDir(), SW_SHOW, (WCHAR*)TEXT("runas"));
}

bool MenuBuilder::DropMenuItem(MENUITEMDATA *menuItemData, TiXmlElement *newElement, HMENU menu, POINT pt)
{
  MenuMap::iterator iter = menuMap.find(menu);

  if (iter == menuMap.end())
    return false;

  MenuItem *menuItem = new MenuItem(menuItemData->name, menuItemData->type, menuItemData->value, menuItemData->workingDir, newElement, menu);

  MENUITEMINFO menuItemInfo;
  ZeroMemory(&menuItemInfo, sizeof(MENUITEMINFO));
  menuItemInfo.cbSize = sizeof(MENUITEMINFO);
  menuItemInfo.fMask = MIIM_ID | MIIM_STRING;
  menuItemInfo.dwTypeData = menuItemData->name;
  menuItemInfo.cch = MAX_LINE_LENGTH;
  menuItemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
  if (menuItemData->type == IT_SEPARATOR)
    {
      menuItemInfo.fMask |= MIIM_FTYPE;
      menuItemInfo.fType = MFT_SEPARATOR;
    }

  if ((winVersion < 6.0) || !pSettings->GetAeroMenus())
    {
      menuItemInfo.hbmpItem = HBMMENU_CALLBACK;
      menuItemInfo.fMask |= MIIM_BITMAP;
    }
  if (pSettings->GetMenuIcons())
    {
      menuItem->SetIcon();
      if ((winVersion >= 6.0) && pSettings->GetAeroMenus())
        {
          menuItemInfo.fMask |= MIIM_BITMAP;
          menuItemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
        }
    }
  menuItemMap.insert(MenuItemPair(menuItemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

  if (menuItemData->type >= IT_XML_MENU)
    {
      menuItemInfo.fMask |= MIIM_SUBMENU;
      menuItemInfo.hSubMenu = CreatePopupMenu();
      switch (menuItemData->type)
        {
        case IT_XML_MENU:
        {
          TiXmlElement *subSection = ELGetFirstXMLElementByName(newElement, (WCHAR*)TEXT("Submenu"), false);
          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(menuItemData->type,
                                                 NULL,
                                                 subSection));
          menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(menuItemInfo.hSubMenu, mli));
        }
        break;

        case IT_FILE_MENU:
        {
          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(menuItemData->type,
                                                 menuItemData->value,
                                                 NULL));
          menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(menuItemInfo.hSubMenu, mli));
        }
        break;

        default:
        {
          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(menuItemData->type,
                                                 NULL,
                                                 NULL));
          menuMap.insert(std::pair< HMENU, std::tr1::shared_ptr<MenuListItem> >(menuItemInfo.hSubMenu, mli));
        }
        break;
        }
    }

  // Insert the new element
  InsertMenuItem(menu, MenuItemFromPoint(NULL, menu, pt), TRUE, &menuItemInfo);

  return true;
}

bool MenuBuilder::AddMenuItem(HMENU menu, int index)
{
  int type;
  MENUITEMINFO menuItemInfo;
  TiXmlElement *element, *newElement, *subMenu, *subItem;;
  TiXmlDocument *configXML;
  MenuItem *menuItem;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_ID;

  if (!GetMenuItemInfo(menu, index, TRUE, &menuItemInfo))
    return false;

  element = menuItemMap.find(menuItemInfo.wID)->second->GetElement();

  if (!element)
    return false;

  menuItem = new MenuItem((WCHAR*)TEXT("New Item"), -1, NULL, NULL, NULL, menu);

  menuItemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
  menuItemMap.insert(MenuItemPair(menuItemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

  InsertMenuItem(menu, index, TRUE, &menuItemInfo);

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

bool MenuBuilder::EditMenuItem(HMENU menu, int index)
{
  WCHAR name[MAX_LINE_LENGTH], value[MAX_LINE_LENGTH], workingDir[MAX_LINE_LENGTH];
  UINT type;
  MENUITEMINFO menuItemInfo;
  TiXmlElement *element;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_ID;

  if (!GetMenuItemInfo(menu, index, TRUE, &menuItemInfo))
    return false;

  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(menuItemInfo.wID)->second;
  wcscpy(value, menuItem->GetValue());
  wcscpy(name, menuItem->GetName());
  wcscpy(workingDir, menuItem->GetWorkingDir());
  type = menuItem->GetType();
  element = menuItem->GetElement();

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
        {
          std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(lpDrawItem->itemID)->second;
          if (menuItem)
            icon = menuItem->GetIcon();
        }

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
  // reinterpret lParam as a CWPSTRUCT*
  CWPSTRUCT *pcwps = reinterpret_cast< CWPSTRUCT* >(lParam);
  CREATESTRUCT *cs;
  static MenuBuilder *pMenuBuilder = NULL;
  static bool buttonDown = false;

  if ( nCode == HC_ACTION )
    {
      WCHAR szClass[128];
      GetClassName(pcwps->hwnd, szClass, 127);
      if (_wcsicmp(szClass, TEXT("EmergeDesktopMenuBuilder")) == 0)
        switch (pcwps->message)
          {
          case WM_CREATE:
            cs = (CREATESTRUCT*)pcwps->lParam;
            pMenuBuilder = reinterpret_cast<MenuBuilder*>(cs->lpCreateParams);
            break;
          }
      if ((_wcsicmp(szClass, TEXT("#32768"))==0) && pMenuBuilder)
        switch (pcwps->message)
          {
          case WM_CREATE:
            SetWindowLongPtr(pcwps->hwnd,
                             GWL_EXSTYLE,
                             GetWindowLongPtr(pcwps->hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(pcwps->hwnd,
                                       0,
                                       (BYTE)((255 * pMenuBuilder->GetMenuAlpha()) / 100), LWA_ALPHA);
            break;
            // capture the MN_SELECTITEM message
          case MN_SELECTITEM:
            pMenuBuilder->SetActiveMenu((HMENU)SendMessage(pcwps->hwnd, MN_GETMENU, 0, 0));
            // Check to see if it's the up or down arrow and if so...
            if ((pcwps->wParam == MENU_DOWN) || (pcwps->wParam == MENU_UP))
              {
                // ...send a MN_BUTTONDOWN message to the menu window with that
                // arrow
                SendMessage(pcwps->hwnd, MN_BUTTONDOWN, pcwps->wParam, 0);
                buttonDown = true;
              }
            else if (buttonDown)
              {
                // ...or else send a MN_BUTTONUP message or WM_CONTEXTMENU is
                // not sent in the case of a WM_RBUTTONDOWN
                SendMessage(pcwps->hwnd, MN_BUTTONUP, pcwps->wParam, 0);
                buttonDown = false;
              }
            break;
          }
    }

  return CallNextHookEx((HHOOK)WH_CALLWNDPROC, nCode, wParam, lParam);
}

void MenuBuilder::SetActiveMenu(HMENU menu)
{
  activeMenu = menu;
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
  if ((type == IT_FILE_MENU) || (type == IT_FILE_SUBMENU))
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
  mii.fMask = MIIM_BITMAP | MIIM_SUBMENU | MIIM_ID;
  MenuMap::iterator subIter;

  while (GetMenuItemInfo(iter->first, 0, MF_BYPOSITION, &mii))
    {
      DeleteObject(mii.hbmpItem);
      menuItemMap.erase(mii.wID);
      if (mii.hSubMenu)
        {
          subIter = menuMap.find(mii.hSubMenu);
          if (subIter != menuMap.end())
            ClearMenu(subIter);
        }
      DeleteMenu(iter->first, 0, MF_BYPOSITION);
    }
}

void MenuBuilder::BuildXMLMenu(MenuMap::iterator iter)
{
  UINT i = 0;
  int type;
  bool found = false;
  WCHAR value[MAX_LINE_LENGTH], name[MAX_LINE_LENGTH], workingDir[MAX_PATH];
  MENUITEMINFO itemInfo;
  HMENU subMenu;
  TiXmlElement *tmp, *subSection, *child = ELGetFirstXMLElement(iter->second->GetSection());

  while (child)
    {
      found = true;
      ELReadXMLIntValue(child, (WCHAR*)TEXT("Type"), &type, 0);

      // Separator
      if (type == IT_SEPARATOR)
        {
          MenuItem *menuItem = new MenuItem(NULL, type, NULL, NULL, child, iter->first);

          itemInfo.fMask = MIIM_FTYPE|MIIM_ID;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = name;
          itemInfo.fType = MFT_SEPARATOR;
          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

          InsertMenuItem(iter->first, i, TRUE, &itemInfo);
        }

      // Executable
      if ((type == IT_EXECUTABLE) || (type == IT_FILE))
        {
          if (ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0")))
            {
              ELReadXMLStringValue(child, (WCHAR*)TEXT("WorkingDir"), workingDir, (WCHAR*)TEXT("\0"));
              ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
              NoPrefixString(name);
              MenuItem *menuItem = new MenuItem(name, type, value, workingDir, child, iter->first);

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

              itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
              menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

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
                  MenuItem *menuItem = new MenuItem(name, type, value, NULL, child, iter->first);

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

                  itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
                  menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

                  InsertMenuItem(iter->first, i, TRUE, &itemInfo);
                }
            }
        }

      if (type == IT_DATE_TIME)
        {
          if (ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0")))
            {
              MenuItem *menuItem = new MenuItem(NULL, type, value, NULL, child, iter->first);
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
              itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
              menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

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
              MenuItem *menuItem = new MenuItem(name, type, value, NULL, child, iter->first);

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

              itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
              menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

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
          MenuItem *menuItem = new MenuItem(name, type, NULL, NULL, child, iter->first);

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

          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);

          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(type, NULL, subSection));
          menuMap.insert(MenuPair(subMenu, mli));
        }

      // Path based submenu
      if (type == IT_FILE_MENU)
        {
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Value"), value, (WCHAR*)TEXT("\0"));
          subMenu = CreatePopupMenu();
          NoPrefixString(name);
          MenuItem *menuItem = new MenuItem(name, type, value, NULL, child, iter->first);

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

          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(type, value, NULL));
          menuMap.insert(MenuPair(subMenu, mli));
        }

      // Tasks(102) or Settings(103) submenu
      if ((type == IT_TASKS_MENU) || (type == IT_SETTINGS_MENU) || (type == IT_HELP_MENU))
        {
          ELReadXMLStringValue(child, (WCHAR*)TEXT("Name"), name, (WCHAR*)TEXT("\0"));
          subMenu = CreatePopupMenu();
          NoPrefixString(name);
          MenuItem *menuItem = new MenuItem(name, type, NULL, NULL, child, iter->first);

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

          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(type, NULL, NULL));
          menuMap.insert(MenuPair(subMenu, mli));
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

      if (ELIsKeyDown(VK_RBUTTON))
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
      else if (ELIsKeyDown(VK_MBUTTON))
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
  WCHAR searchPath[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH], *lwrEntry = NULL, *lwrValue = NULL;
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
              if (ELPathIsDirectory(shortcutInfo.Path))
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
          std::tr1::shared_ptr<MenuListItem> mli;

          if (!GetPos(iter, findData.cFileName, true, &folderPos, &itemID, &itemData))
            {
              MenuItem *menuItem = new MenuItem(NULL, IT_FILE_SUBMENU, tmp, NULL, NULL, iter->first);
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

              mli = std::tr1::shared_ptr<MenuListItem>(new MenuListItem(IT_FILE_SUBMENU, tmp, NULL));
              HMENU subMenu = CreatePopupMenu();

              wcscpy(tmp, findData.cFileName);

              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = tmp;
              itemInfo.hSubMenu = subMenu;

              NoPrefixString(tmp);
              menuItem->SetName(tmp);

              itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
              itemInfo.dwItemData = (ULONG_PTR)subMenu;

              menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));
              InsertMenuItem(iter->first, folderPos, TRUE, &itemInfo);

              menuMap.insert(MenuPair(subMenu, mli));
            }
          else
            {
              WCHAR path[MAX_LINE_LENGTH];
              iter2 = menuMap.find((HMENU)itemData);

              std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(itemID)->second;
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
          wcscpy(extension, PathFindExtension(tmp));
          bool isShortcut = (_wcsicmp(extension, TEXT(".lnk")) == 0) ||
                            (_wcsicmp(extension, TEXT(".lnk2")) == 0) ||
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
              std::tr1::shared_ptr<MenuItem> menuItem;

              menuItem = menuItemMap.find(itemID)->second;

              if (lwrEntry)
                free(lwrEntry);
              lwrEntry = _wcslwr(_wcsdup(entry));

              if (lwrValue)
                free(lwrValue);
              lwrValue = _wcslwr(_wcsdup(menuItem->GetValue()));

              if (ELParseShortcut(entry, &entryInfo))
                {
                  if (ELParseShortcut(menuItem->GetValue(), &targetInfo))
                    {
                      if (_wcsicmp(entryInfo.Path, targetInfo.Path) == 0)
                        continue;
                    }
                }
              else if ((wcsstr(lwrEntry, TEXT(".url")) != NULL) &&
                       (wcsstr(lwrValue, TEXT(".url")) != NULL))
                {
                  if (ELReadFileString(entry, (WCHAR*)TEXT("URL"), entryURL, (WCHAR*)TEXT("")) &&
                      ELReadFileString(menuItem->GetValue(), (WCHAR*)TEXT("URL"), targetURL, (WCHAR*)TEXT("")))
                    {
                      if (_wcsicmp(entryURL, targetURL) == 0)
                        continue;
                    }
                }
            }

          MenuItem *menuItem = new MenuItem(NULL, IT_FILE, entry, NULL, NULL, iter->first);

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

          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

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
  menuItem = new MenuItem(windowTitle, IT_TASK, tmp, NULL, NULL, iter->first);

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

  itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
  menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

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
  MENUINFO menuInfo;
  WCHAR menuName[MAX_LINE_LENGTH];
  LRESULT ret = 1;
  POINT mousePT;
  std::tr1::shared_ptr<MenuListItem> mli;
  MenuMap::iterator iter;
  std::tr1::shared_ptr<TiXmlDocument> configXML;
  TiXmlElement *section = NULL, *menu = NULL;
  std::wstring xmlPath = TEXT("%EmergeDir%\\files\\");
  xmlPath = ELExpandVars(xmlPath);
  if (!ELPathIsDirectory(xmlPath.c_str()))
    ELCreateDirectory(xmlPath);
  std::wstring xmlFile = xmlPath + TEXT("emergeWorkspace.xml");
  if (!PathFileExists(xmlFile.c_str()))
    {
      std::wstring oldXmlFile = xmlPath + TEXT("emergeDesktop.xml");
      if (PathFileExists(oldXmlFile.c_str()))
        ELFileOp(menuWnd, false, FO_RENAME, oldXmlFile, xmlFile);
    }

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
              rootMenu = CreatePopupMenu();
              mli = std::tr1::shared_ptr<MenuListItem>(new MenuListItem(IT_XML_MENU, NULL, menu));
              menuMap.insert(std::pair< HMENU, std::tr1::
                             shared_ptr<MenuListItem> >(rootMenu, mli));
              iter = menuMap.begin();

              // MNS_DRAGDROG must be set before the menu is visible.
              menuInfo.cbSize = sizeof(menuInfo);
              menuInfo.fMask = MIM_STYLE;
              if (GetMenuInfo(rootMenu, &menuInfo))
                {
                  menuInfo.fMask |= MIM_APPLYTOSUBMENUS;
                  menuInfo.dwStyle |= MNS_DRAGDROP;
                  SetMenuInfo(rootMenu, &menuInfo);
                }

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
  menuInfo.dwStyle = MNS_CHECKORBMP;
  SetMenuInfo(menu, &menuInfo);

  BuildMenu(iter);

  return 0;
}

LRESULT MenuBuilder::ExecuteMenuItem(UINT_PTR itemID)
{
  MenuMap::iterator iter;
  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(itemID)->second;
  WCHAR error[MAX_LINE_LENGTH];

  if (!menuItem)
    return 1;

  switch (menuItem->GetType())
    {
    case IT_EXECUTABLE:
    case IT_SPECIAL_FOLDER:
    case IT_INTERNAL_COMMAND:
      ExecuteXMLMenuItem(menuItem->GetType(),
                         menuItem->GetValue(),
                         menuItem->GetWorkingDir());
      break;
    case IT_FILE:
      if (!ELExecute(menuItem->GetValue()))
        {
          swprintf(error, TEXT("Failed to execute \"%ls\""), menuItem->GetValue());
          ELMessageBox(GetDesktopWindow(), error, (WCHAR*)TEXT("emergeWorkspace"), ELMB_ICONWARNING|ELMB_OK);
        }
      break;
    case IT_TASK:
#ifdef _W64
      ELSwitchToThisWindow((HWND)_wcstoi64(menuItem->GetValue(), NULL, 10));
#else
      ELSwitchToThisWindow((HWND)wcstol(menuItem->GetValue(), NULL, 10));
#endif
      break;
    case IT_SETTING_ITEM:
      ExecuteSettingsMenuItem(itemID);
      break;
    case IT_HELP_ITEM:
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
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("Configure Workspace"), BSM_CONFIGURE);
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("Configure Core"), BSM_CORE);
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("\0"), BSM_SEPARATOR);
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("Theme Manager"), BSM_SELECTTHEME);
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("\0"), BSM_SEPARATOR);
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("Change Desktop Shell"), BSM_SHELL);
  AddSpecialItem(iter, IT_SETTING_ITEM, (WCHAR*)TEXT("Quit Emerge Desktop"), BSM_QUIT);
}

void MenuBuilder::BuildHelpMenu(MenuMap::iterator iter)
{
  AddSpecialItem(iter, IT_HELP_ITEM, (WCHAR*)TEXT("Welcome"), BHM_WELCOME);
  AddSpecialItem(iter, IT_HELP_ITEM, (WCHAR*)TEXT("Offline Help"), BHM_OFFLINE);
  AddSpecialItem(iter, IT_HELP_ITEM, (WCHAR*)TEXT("Online Tutorial"), BHM_TUTORIAL);
  AddSpecialItem(iter, IT_HELP_ITEM, (WCHAR*)TEXT("Emerge Desktop Website"), BHM_WEBSITE);
  AddSpecialItem(iter, IT_HELP_ITEM, (WCHAR*)TEXT("\0"), BHM_SEPARATOR);
  AddSpecialItem(iter, IT_HELP_ITEM, (WCHAR*)TEXT("About"), BHM_ABOUT);
}

void MenuBuilder::AddSpecialItem(MenuMap::iterator iter, UINT type, WCHAR* text, UINT id)
{
  MENUITEMINFO itemInfo;
  UINT index = GetMenuItemCount(iter->first);
  WCHAR value[MAX_LINE_LENGTH];
  swprintf(value, TEXT("%d"), id);
  MenuItem *menuItem = new MenuItem(text, type, value, NULL, NULL, iter->first);

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
  itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
  itemInfo.dwTypeData = text;
  menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));
  InsertMenuItem(iter->first, index, TRUE, &itemInfo);
}

void MenuBuilder::ExecuteXMLMenuItem(UINT type, WCHAR *value, WCHAR *workingDir)
{
  bool ret = false;
  WCHAR error[MAX_LINE_LENGTH];
  swprintf(error, TEXT("Failed to execute \"%ls\""), value);

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

  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(index)->second;
  if (!menuItem)
    return;

  long value = wcstol(menuItem->GetValue(), NULL, 10);

  switch (value)
    {
      int res;
    case BSM_CONFIGURE:
      res = config.Show();
      if (res == IDOK)
        SetWorkArea();
      break;
    case BSM_CORE:
      ELExecuteInternal((WCHAR*)TEXT("LaunchEditor"));
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
  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(index)->second;
  if (!menuItem)
    return;

  long value = wcstol(menuItem->GetValue(), NULL, 10);

  switch (value)
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
    case BHM_WELCOME:
      ELExecuteInternal((WCHAR*)TEXT("Welcome"));
      break;
    }
}

BYTE MenuBuilder::GetMenuAlpha()
{
  return pSettings->GetMenuAlpha();
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
