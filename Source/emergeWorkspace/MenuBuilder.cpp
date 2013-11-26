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
  SetRectEmpty(&explorerWorkArea);
  ShellMessage = 0;
  activeMenu = NULL;
  topGap = false;
  bottomGap = false;

  menuHook = SetWindowsHookEx(WH_CALLWNDPROC, HookCallWndProc, 0, GetCurrentThreadId());
}

bool MenuBuilder::Initialize()
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(WNDCLASSEX));

  if (FAILED(OleInitialize(NULL)))
    {
      ELMessageBox(GetDesktopWindow(), TEXT("COM initialization failed"), TEXT("emergeWorkspace"),
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
                   TEXT("Failed to create desktop window"),
                   TEXT("emergeWorkspace"),
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

      DropMenuItem(&newMenuItemData->menuItemData,
                   &newMenuItemData->dropItemData,
                   newMenuItemData->menu,
                   newMenuItemData->pt);

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

  topGap = false;
  bottomGap = false;

  if (mgoInfo->dwFlags & MNGOF_BOTTOMGAP)
    bottomGap = true;
  else if (mgoInfo->dwFlags & MNGOF_TOPGAP)
    topGap = true;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID | MIIM_STATE;

  if (!GetMenuItemInfo(mgoInfo->hmenu, mgoInfo->uPos, TRUE, &menuItemInfo))
    return MNGO_NOINTERFACE;

  if ((menuItemInfo.fState & MFS_GRAYED) && (menuItemInfo.fType != MFT_SEPARATOR))
    return MNGO_NOINTERFACE;

  std::tr1::shared_ptr<MenuItem> menuItem = menuItemMap.find(menuItemInfo.wID)->second;
  if (!menuItem)
    return MNGO_NOINTERFACE;

  // Don't allow drop directly on settings, help or task menu.  Allow for top
  // gap and bottom gap.
  if (!bottomGap && !topGap &&
      ((menuItem->GetType() == IT_SETTINGS_MENU) ||
       (menuItem->GetType() == IT_HELP_MENU) ||
       (menuItem->GetType() == IT_TASKS_MENU)))
    return MNGO_NOINTERFACE;

  // Don't allow for drop anywhere on setting, help or task item.
  if ((menuItem->GetType() == IT_SETTING_ITEM) ||
      (menuItem->GetType() == IT_HELP_ITEM) ||
      (menuItem->GetType() == IT_TASK))
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
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID | MIIM_STATE;

  if (!GetMenuItemInfo(menu, pos, TRUE, &menuItemInfo))
    return MND_CONTINUE;

  if ((menuItemInfo.fState & MFS_GRAYED) && (menuItemInfo.fType != MFT_SEPARATOR))
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
  DRAGITEMDATA dragItemData;

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
      CopyMemory(&dragItemData.menuItemData, menuItem->GetMenuItemData(), sizeof(MENUITEMDATA));
      dragItemData.itemID = menuItem->GetID();
      fmtetc.cfFormat = CF_EMERGE_MENUITEM;
      stgmed.hGlobal = DragItemDataToHandle(&dragItemData);
    }

  dropEffects = DROPEFFECT_MOVE | DROPEFFECT_COPY;

  IDataObject *pDataObject;
  IDropSource *pDropSource;

  CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);
  CreateDropSource(&pDropSource);

  if (DoDragDrop(pDataObject, pDropSource, dropEffects, &effect) ==
      DRAGDROP_S_DROP)
    {
      if (effect == DROPEFFECT_MOVE)
        {
          if ((menuItem->GetType() != IT_FILE) &&
              (menuItem->GetType() != IT_FILE_SUBMENU))
            {
              // Remove the menu element
              TiXmlElement *menuElement = menuItem->GetElement();
              if (menuElement)
                {
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
                      EnableMenuItem(menu, menuItem->GetID(),
                                     MF_BYCOMMAND | MF_GRAYED);
                    }
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

HANDLE MenuBuilder::DragItemDataToHandle(DRAGITEMDATA *dragItemData)
{
  void *ptr = NULL;

  // allocate and lock a global memory buffer. Make it fixed
  // data so we don't have to use GlobalLock
  ptr = (void *)GlobalAlloc(GMEM_FIXED, sizeof(DRAGITEMDATA));
  if (ptr != NULL)
    CopyMemory(ptr, dragItemData, sizeof(DRAGITEMDATA));

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
  MenuMap::iterator subIter;

  GetCursorPos(&pt);
  index = MenuItemFromPoint(menuWnd, activeMenu, pt);
  if (index == -1)
    return 0;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_FTYPE | MIIM_SUBMENU | MIIM_ID | MIIM_STATE;
  if (!GetMenuItemInfo(activeMenu, index, TRUE, &menuItemInfo))
    return 0;

  if ((menuItemInfo.fState & MFS_GRAYED) && (menuItemInfo.fType != MFT_SEPARATOR))
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
            {
              if (DeleteMenu(menuItem->GetMenu(), menuItem->GetID(), MF_BYCOMMAND))
                menuItemMap.erase(menuItemMap.find(menuItem->GetID()));
            }
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
              ELWriteXMLConfig(configXML);
              if (menuItem->GetType() == IT_XML_MENU)
                {
                  subIter = menuMap.find(menuItemInfo.hSubMenu);
                  if (subIter != menuMap.end())
                    ClearMenu(subIter);
                }
              if (DeleteMenu(menuItem->GetMenu(), menuItem->GetID(), MF_BYCOMMAND))
                menuItemMap.erase(menuItemMap.find(menuItem->GetID()));
            }
          break;
        case DRM_EDIT:
          EditMenuItem(menuItem);
          break;
        case DRM_ADD:
          AddMenuItem(menuItem);
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
  ELExecuteFileOrCommand(menuItem->GetValue(), TEXT(""), menuItem->GetWorkingDir(), SW_SHOW, TEXT("runas"));
}

bool MenuBuilder::DropMenuItem(MENUITEMDATA *menuItemData, MENUITEMDATA *dropItemData, HMENU menu, POINT pt)
{
  UINT pos = MenuItemFromPoint(NULL, menu, pt);

  if ((dropItemData->type == IT_XML_MENU) && !topGap && !bottomGap)
    {
      menu = dropItemData->subMenu;
      TiXmlElement *subSection = ELGetFirstXMLElementByName(dropItemData->element, TEXT("Submenu"), false);
      ELSetFirstXMLElement(subSection, menuItemData->element);
    }
  else
    {
      menuItemData->element = ELSetSiblingXMLElement(dropItemData->element, menuItemData->element, bottomGap);
      if (bottomGap)
        ++pos;
    }

  ELWriteXMLConfig(ELGetXMLConfig(dropItemData->element));

  MenuItem *menuItem = new MenuItem(*menuItemData, menu);

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

  if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
    {
      menuItemInfo.hbmpItem = HBMMENU_CALLBACK;
      menuItemInfo.fMask |= MIIM_BITMAP;
    }
  if (pSettings->GetMenuIcons())
    {
      menuItem->SetIcon();
      if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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
          TiXmlElement *subSection = ELGetFirstXMLElementByName(menuItemData->element, (WCHAR*)TEXT("Submenu"), false);
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
  InsertMenuItem(menu, pos, TRUE, &menuItemInfo);

  return true;
}

bool MenuBuilder::AddMenuItem(std::tr1::shared_ptr<MenuItem> menuItem)
{
  int type;
  MENUITEMINFO menuItemInfo;
  TiXmlElement *newElement, *subMenu, *subItem;;
  TiXmlDocument *configXML;
  MenuItem *newMenuItem;

  menuItemInfo.cbSize = sizeof(menuItemInfo);
  menuItemInfo.fMask = MIIM_ID;

  MENUITEMDATA menuItemData;
  ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
  menuItemData.type = -1;
  wcscpy(menuItemData.name, TEXT("New Item"));

  newMenuItem = new MenuItem(menuItemData, menuItem->GetMenu());

  menuItemInfo.wID = reinterpret_cast< UINT_PTR >(newMenuItem);
  menuItemMap.insert(MenuItemPair(menuItemInfo.wID, std::tr1::shared_ptr<MenuItem>(newMenuItem)));

  InsertMenuItem(menuItem->GetMenu(), menuItem->GetID(), FALSE, &menuItemInfo);

  newElement = ELSetSiblingXMLElementByName(menuItem->GetElement(), (WCHAR*)TEXT("item"));
  if (!newElement)
    return false;

  configXML = ELGetXMLConfig(newElement);

  if (pItemEditor->Show(newElement,
                        newMenuItem->GetName(),
                        newMenuItem->GetValue(),
                        newMenuItem->GetType(),
                        newMenuItem->GetWorkingDir()) == IDCANCEL)
    {
      ELRemoveXMLElement(newElement);
      ELWriteXMLConfig(configXML);
    }
  else
    {
      type = ELReadXMLIntValue(newElement, TEXT("Type"), 0);
      if (type == IT_XML_MENU)
      {
        subMenu = ELGetFirstXMLElementByName(newElement, TEXT("Submenu"), true);
        if (subMenu)
        {
          subItem = ELGetFirstXMLElementByName(subMenu, TEXT("item"), true);
          if (subItem)
          {
            ELWriteXMLIntValue(subItem, TEXT("Type"), 0);
            ELWriteXMLConfig(configXML);
          }
        }
      }
    }

  return true;
}

bool MenuBuilder::EditMenuItem(std::tr1::shared_ptr<MenuItem> menuItem)
{
  pItemEditor->Show(menuItem->GetElement(), menuItem->GetName(), menuItem->GetValue(), menuItem->GetType(), menuItem->GetWorkingDir());

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
  std::wstring value, name, workingDir;
  MENUITEMINFO itemInfo;
  MENUITEMDATA menuItemData;
  HMENU subMenu;
  TiXmlElement *tmp, *subSection, *child = NULL;

  if (iter->second->GetSection())
    child = ELGetFirstXMLElement(iter->second->GetSection());

  while (child)
    {
      found = true;
      type = ELReadXMLIntValue(child, TEXT("Type"), 0);

      // Separator
      if (type == IT_SEPARATOR)
        {
          ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
          menuItemData.element = child;
          MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

          itemInfo.fMask = MIIM_FTYPE|MIIM_ID;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = (WCHAR*)name.c_str();
          itemInfo.fType = MFT_SEPARATOR;
          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

          InsertMenuItem(iter->first, i, TRUE, &itemInfo);
        }

      // Executable
      if ((type == IT_EXECUTABLE) || (type == IT_FILE))
        {
          value = ELReadXMLStringValue(child, TEXT("Value"), TEXT(""));
          if (!value.empty())
            {
              workingDir = ELReadXMLStringValue(child, TEXT("WorkingDir"), TEXT(""));
              name = ELReadXMLStringValue(child, TEXT("Name"), TEXT(""));
              name = NoPrefixString(name);

              ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
              wcscpy(menuItemData.name, name.c_str());
              menuItemData.type = type;
              wcscpy(menuItemData.value, value.c_str());
              wcscpy(menuItemData.workingDir, workingDir.c_str());
              menuItemData.element = child;
              MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

              itemInfo.fMask = MIIM_STRING | MIIM_ID;
              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = (WCHAR*)name.c_str();
              if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
                {
                  itemInfo.hbmpItem = HBMMENU_CALLBACK;
                  itemInfo.fMask |= MIIM_BITMAP;
                }

              if (pSettings->GetMenuIcons())
                {
                  menuItem->SetIcon();
                  if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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
          value = ELReadXMLStringValue(child, TEXT("Value"), TEXT(""));
          if (!value.empty())
            {
              name = ELReadXMLStringValue(child, TEXT("Name"), TEXT(""));
              if (ELToLower(name) == ELToLower(TEXT("DateTime")))
                type = 3;
              else
                {
                  name = NoPrefixString(name);

                  ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
                  wcscpy(menuItemData.name, name.c_str());
                  menuItemData.type = type;
                  wcscpy(menuItemData.value, value.c_str());
                  menuItemData.element = child;
                  MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

                  itemInfo.fMask = MIIM_STRING | MIIM_ID;
                  itemInfo.cbSize = sizeof(MENUITEMINFO);
                  itemInfo.dwTypeData = (WCHAR*)name.c_str();
                  if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
                    {
                      itemInfo.hbmpItem = HBMMENU_CALLBACK;
                      itemInfo.fMask |= MIIM_BITMAP;
                    }

                  if (pSettings->GetMenuIcons())
                    {
                      menuItem->SetIcon();
                      if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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
          value = ELReadXMLStringValue(child, TEXT("Value"), TEXT(""));
          if (!value.empty())
            {
              ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
              menuItemData.type = type;
              wcscpy(menuItemData.value, value.c_str());
              menuItemData.element = child;
              MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

              std::wstring datetimeString;
              time_t tVal;
              struct tm *stVal;

              // Grab the current time
              _tzset();
              time(&tVal);
              stVal = localtime(&tVal);

              datetimeString = ELwcsftime(value, stVal);

              itemInfo.fMask = MIIM_STRING|MIIM_ID;
              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = (WCHAR*)datetimeString.c_str();
              itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
              menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

              InsertMenuItem(iter->first, i, TRUE, &itemInfo);
            }
        }

      // Special Folder
      if (type == IT_SPECIAL_FOLDER)
        {
          value = ELReadXMLStringValue(child, TEXT("Value"), TEXT(""));
          if (!value.empty())
            {
              name = ELReadXMLStringValue(child, TEXT("Name"), TEXT(""));
              name = NoPrefixString(name);

              ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
              wcscpy(menuItemData.name, name.c_str());
              menuItemData.type = type;
              wcscpy(menuItemData.value, value.c_str());
              menuItemData.element = child;
              MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

              itemInfo.fMask = MIIM_STRING | MIIM_ID;
              itemInfo.cbSize = sizeof(MENUITEMINFO);
              itemInfo.dwTypeData = (WCHAR*)name.c_str();
              if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
                {
                  itemInfo.hbmpItem = HBMMENU_CALLBACK;
                  itemInfo.fMask |= MIIM_BITMAP;
                }

              if (pSettings->GetMenuIcons())
                {
                  menuItem->SetIcon();
                  if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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
          name = ELReadXMLStringValue(child, TEXT("Name"), TEXT(""));
          subSection = ELGetFirstXMLElementByName(child, TEXT("Submenu"), false);
          subMenu = CreatePopupMenu();
          name = NoPrefixString(name);

          ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
          wcscpy(menuItemData.name, name.c_str());
          menuItemData.type = type;
          menuItemData.element = child;
          menuItemData.subMenu = subMenu;
          MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

          itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = (WCHAR*)name.c_str();
          itemInfo.hSubMenu = subMenu;
          if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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
          name = ELReadXMLStringValue(child, TEXT("Name"), TEXT(""));
          value = ELReadXMLStringValue(child, TEXT("Value"), TEXT(""));
          subMenu = CreatePopupMenu();
          name = NoPrefixString(name);

          ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
          wcscpy(menuItemData.name, name.c_str());
          menuItemData.type = type;
          wcscpy(menuItemData.value, value.c_str());
          menuItemData.element = child;
          MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

          itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = (WCHAR*)name.c_str();
          itemInfo.hSubMenu = subMenu;
          if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
                {
                  itemInfo.fMask |= MIIM_BITMAP;
                  itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                }
            }

          itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
          menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));
          InsertMenuItem(iter->first, i, TRUE, &itemInfo);

          std::tr1::shared_ptr<MenuListItem> mli(new MenuListItem(type, (WCHAR*)value.c_str(), NULL));
          menuMap.insert(MenuPair(subMenu, mli));
        }

      // Tasks(102) or Settings(103) submenu
      if ((type == IT_TASKS_MENU) || (type == IT_SETTINGS_MENU) || (type == IT_HELP_MENU))
        {
          name = ELReadXMLStringValue(child, TEXT("Name"), TEXT(""));
          subMenu = CreatePopupMenu();
          name = NoPrefixString(name);

          ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
          wcscpy(menuItemData.name, name.c_str());
          menuItemData.type = type;
          menuItemData.element = child;
          MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

          itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU;
          itemInfo.cbSize = sizeof(MENUITEMINFO);
          itemInfo.dwTypeData = (WCHAR*)name.c_str();
          itemInfo.hSubMenu = subMenu;
          if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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
  std::wstring specialFolder, value;
  TiXmlElement *section, *xmlItem;

  section = iter->second->GetSection();
  if (section)
    {
      int type;

      if (ELIsKeyDown(VK_RBUTTON))
        {
          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_SPECIAL_FOLDER;
          if (xmlItem)
            {
              specialFolder = ELGetSpecialFolderPathFromID(CSIDL_DRIVES);
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), specialFolder);
              value = ELGetSpecialFolderNameFromPath(specialFolder);
              if (!value.empty())
                ELWriteXMLStringValue(xmlItem, TEXT("Value"), value);
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              specialFolder = ELGetSpecialFolderPathFromID(CSIDL_BITBUCKET);
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), specialFolder);
              value = ELGetSpecialFolderNameFromPath(specialFolder);
              if (!value.empty())
                ELWriteXMLStringValue(xmlItem, TEXT("Value"), value);
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_FILE_MENU;
          if (xmlItem)
            {
              specialFolder = ELGetSpecialFolderPathFromID(CSIDL_STARTMENU);
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), specialFolder);
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("%StartMenu%|%CommonStartMenu%"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Quick Launch"));
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Value"),
                                    TEXT("%AppData%\\Microsoft\\Internet Explorer\\Quick Launch"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              specialFolder = ELGetSpecialFolderPathFromID(CSIDL_DESKTOP);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), specialFolder);
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("%Desktop%|%CommonDesktop%"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_SETTINGS_MENU;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Settings"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT(""));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_HELP_MENU;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Help"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("\0"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_EXECUTABLE;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Display Properties"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("desk.cpl"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_INTERNAL_COMMAND;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Run"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("Run"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Logoff"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("Logoff"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Shutdown"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("Shutdown"));
            }

          ELWriteXMLConfig(ELGetXMLConfig(xmlItem));
        }
      else if (ELIsKeyDown(VK_MBUTTON))
        {
          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_TASKS_MENU;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Tasks"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("\0"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_SEPARATOR;
          if (xmlItem)
            ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          type = IT_INTERNAL_COMMAND;
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Show Applets"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("Show"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Hide Applets"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("Hide"));
            }

          xmlItem = ELSetFirstXMLElementByName(section, TEXT("item"));
          if (xmlItem)
            {
              ELWriteXMLIntValue(xmlItem, TEXT("Type"), type);
              ELWriteXMLStringValue(xmlItem, TEXT("Name"), TEXT("Show Desktop"));
              ELWriteXMLStringValue(xmlItem, TEXT("Value"), TEXT("ShowDesktop"));
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
  MENUITEMDATA menuItemData;

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
              if ((ELGetFileSpecialFlags(shortcutInfo.Path) & SF_DIRECTORY) == SF_DIRECTORY)
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
              ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
              menuItemData.type = IT_FILE_SUBMENU;
              wcscpy(menuItemData.value, tmp);
              MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

              MENUITEMINFO itemInfo;

              itemInfo.fMask = MIIM_STRING | MIIM_ID | MIIM_SUBMENU | MIIM_DATA;
              if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
                {
                  itemInfo.hbmpItem = HBMMENU_CALLBACK;
                  itemInfo.fMask |= MIIM_BITMAP;
                }

              if (pSettings->GetMenuIcons())
                {
                  menuItem->SetIcon();
                  if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
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

              wcscpy(tmp, NoPrefixString(tmp).c_str());
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
              std::wstring targetURL, entryURL;
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
                  entryURL = ELReadFileString(entry, TEXT("URL"), TEXT(""));
                  targetURL = ELReadFileString(menuItem->GetValue(), TEXT("URL"), TEXT(""));
                  if ((!entryURL.empty()) && (!targetURL.empty()))
                    {
                      if (ELToLower(entryURL) == ELToLower(targetURL))
                      {
                        continue;
                      }
                    }
                }
            }

          ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
          menuItemData.type = IT_FILE;
          wcscpy(menuItemData.value, entry);
          MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

          itemInfo.fMask = MIIM_STRING | MIIM_ID;
          if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
            {
              itemInfo.hbmpItem = HBMMENU_CALLBACK;
              itemInfo.fMask |= MIIM_BITMAP;
            }

          if (pSettings->GetMenuIcons())
            {
              menuItem->SetIcon();
              if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
                {
                  itemInfo.fMask |= MIIM_BITMAP;
                  itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
                }
            }

          wcscpy(tmp, NoPrefixString(tmp).c_str());
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

  if (ELIsValidTaskWindow(hwnd))
    pMenuBuilder->AddTaskItem(hwnd);

  return true;
}

void MenuBuilder::AddTaskItem(HWND task)
{
  WCHAR windowTitle[MAX_LINE_LENGTH], tmp[MAX_LINE_LENGTH];
  MENUITEMINFO itemInfo;
  MENUITEMDATA menuItemData;
  MenuItem *menuItem;
  MenuMap::iterator iter;

  GetWindowText(task, windowTitle, MAX_LINE_LENGTH);
  wcscpy(windowTitle, NoPrefixString(windowTitle).c_str());

  iter = menuMap.find(taskMenu);
  if (iter == menuMap.end())
    return;

  swprintf(tmp, TEXT("%d"), (ULONG_PTR)task);
  ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
  wcscpy(menuItemData.name, windowTitle);
  menuItemData.type = IT_TASK;
  wcscpy(menuItemData.value, tmp);
  menuItem = new MenuItem(menuItemData, iter->first);

  itemInfo.fMask = MIIM_STRING | MIIM_ID;
  if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
    {
      itemInfo.hbmpItem = HBMMENU_CALLBACK;
      itemInfo.fMask |= MIIM_BITMAP;
    }

  if (pSettings->GetMenuIcons())
    {
      menuItem->SetIcon();
      if ((IsWindowsVistaOrGreater()) && pSettings->GetAeroMenus())
        {
          itemInfo.fMask |= MIIM_BITMAP;
          itemInfo.hbmpItem = EGGetIconBitmap(menuItem->GetIcon());
        }
    }

  itemInfo.cbSize = sizeof(MENUITEMINFO);
  itemInfo.dwTypeData = windowTitle;

  itemInfo.wID = reinterpret_cast< UINT_PTR >(menuItem);
  menuItemMap.insert(MenuItemPair(itemInfo.wID, std::tr1::shared_ptr<MenuItem>(menuItem)));

  InsertMenuItem(iter->first, GetMenuItemCount(iter->first), TRUE, &itemInfo);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	NoPrefixString
// Requires:	WCHAR *source - source string
// Returns:	bool
// Purpose:	duplicated ampersands to remove menu prefixes
//----  --------------------------------------------------------------------------------------------------------
std::wstring MenuBuilder::NoPrefixString(std::wstring source)
{
  WCHAR tmp[MAX_LINE_LENGTH], *token = NULL;
  ZeroMemory(tmp, MAX_LINE_LENGTH);

  token = wcstok((WCHAR*)source.c_str(), TEXT("&"));

  while (token != NULL)
    {
      wcscat(tmp, token);
      token = wcstok(NULL, TEXT("&"));
      if (token != NULL)
        wcscat(tmp, TEXT("&&"));
    }

  if (wcslen(tmp) > 0)
    return tmp;

  return TEXT("");
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
  if ((ELGetFileSpecialFlags(xmlPath) & SF_DIRECTORY) != SF_DIRECTORY)
  {
    ELCreateDirectory(xmlPath);
  }
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
      section = ELGetXMLSection(configXML.get(), TEXT("Menus"), true);

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
      if (!ELExecuteFileOrCommand(menuItem->GetValue()))
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
  MENUITEMDATA menuItemData;

  ZeroMemory(&menuItemData, sizeof(MENUITEMDATA));
  wcscpy(menuItemData.name, text);
  menuItemData.type = type;
  swprintf(menuItemData.value, TEXT("%d"), id);
  MenuItem *menuItem = new MenuItem(menuItemData, iter->first);

  itemInfo.fMask = MIIM_STRING | MIIM_ID;
  if (id == BSM_SEPARATOR)
    {
      itemInfo.fMask = MIIM_FTYPE | MIIM_ID;
      itemInfo.fType = MFT_SEPARATOR;
    }

  if ((!IsWindowsVistaOrGreater()) || !pSettings->GetAeroMenus())
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
      ret = ELExecuteFileOrCommand(value, workingDir);
      break;
    case IT_INTERNAL_COMMAND:
      ret = ELExecuteFileOrCommand(value);
      break;
    case IT_DATE_TIME:
      ret = ELExecuteFileOrCommand(TEXT("timedate.cpl"));
      break;
    case IT_SPECIAL_FOLDER:
      ret = ELExecuteFileOrCommand(value);
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
      ELExecuteFileOrCommand(TEXT("LaunchEditor"));
      break;
    case BSM_SHELL:
      ELExecuteFileOrCommand(TEXT("ShellChanger"));
      break;
    case BSM_SELECTTHEME:
      ELExecuteFileOrCommand(TEXT("ThemeManager"));
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
      ELExecuteFileOrCommand(TEXT("About"));
      break;
    case BHM_OFFLINE:
      ELExecuteFileOrCommand(TEXT("%AppletDir%\\Documentation\\Emerge Desktop.chm"));
      break;
    case BHM_WEBSITE:
      ELExecuteFileOrCommand(TEXT("Homepage"));
      break;
    case BHM_TUTORIAL:
      ELExecuteFileOrCommand(TEXT("Tutorial"));
      break;
    case BHM_WELCOME:
      ELExecuteFileOrCommand(TEXT("Welcome"));
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
