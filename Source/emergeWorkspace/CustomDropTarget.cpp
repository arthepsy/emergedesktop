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

#include "CustomDropTarget.h"

HRESULT CreateDropTarget(IDropTarget **ppDropTarget, UINT type, WCHAR *value, TiXmlElement *dropElement, HMENU dropMenu)
{
  if(ppDropTarget == NULL)
    return E_INVALIDARG;

  *ppDropTarget = new CustomDropTarget(type, value, dropElement, dropMenu);

  return (*ppDropTarget) ? S_OK : E_OUTOFMEMORY;
}

CustomDropTarget::CustomDropTarget(UINT type, WCHAR *value, TiXmlElement *dropElement, HMENU dropMenu)
{
  refCount = 0;
  allowDrop = false;
  this->type = type;
  this->dropElement = dropElement;
  this->dropMenu = dropMenu;
  this->value = _wcsdup(value);

  CF_EMERGE_MENUITEM = RegisterClipboardFormat(TEXT("CF_EMERGE_MENUITEM"));
  if (CF_EMERGE_MENUITEM == 0)
    ELMessageBox(GetDesktopWindow(), (WCHAR*)TEXT("Failed to register Emerge Desktop Menu Item clipboard format."), (WCHAR*)TEXT("emergeWorkspace"),
                 ELMB_OK|ELMB_ICONERROR|ELMB_MODAL);
}

CustomDropTarget::~CustomDropTarget()
{
  free(value);
}

bool CustomDropTarget::QueryDataObject(IDataObject *pDataObj)
{
  if (pDataObj == NULL)
    return false;

  FORMATETC fmtetc;

  ZeroMemory(&fmtetc, sizeof(FORMATETC));
  fmtetc.dwAspect = DVASPECT_CONTENT;
  if (type == IT_TASK)
    fmtetc.tymed = TYMED_NULL;
  else
    fmtetc.tymed = TYMED_HGLOBAL;
  fmtetc.lindex = -1;
  if ((type == IT_FILE) || (type == IT_FILE_SUBMENU))
    fmtetc.cfFormat = CF_HDROP;
  else
    fmtetc.cfFormat = CF_EMERGE_MENUITEM;

  // does the data object support CF_EMERGE_MENUITEM using a HGLOBAL?
  return pDataObj->QueryGetData(&fmtetc) == S_OK ? true : false;
}

DWORD CustomDropTarget::DropEffect(DWORD grfKeyState, POINTL pt UNUSED, DWORD dwAllowed)
{
  DWORD dwEffect = 0;

  // 1. check "pt" -> do we allow a drop at the specified coordinates?
  // 2. work out that the drop-effect should be based on grfKeyState
  if(grfKeyState & MK_CONTROL)
    dwEffect = dwAllowed & DROPEFFECT_COPY;
  else if(grfKeyState & MK_SHIFT)
    dwEffect = dwAllowed & DROPEFFECT_MOVE;

  // 3. no key-modifiers were specified (or drop effect not allowed), so
  //    base the effect on those allowed by the dropsource
  if(dwEffect == 0)
    {
      if(dwAllowed & DROPEFFECT_COPY)
        dwEffect = DROPEFFECT_COPY;
      if(dwAllowed & DROPEFFECT_MOVE)
        dwEffect = DROPEFFECT_MOVE;
    }

  return dwEffect;
}

bool CustomDropTarget::DataDrop(IDataObject *pDataObj, POINTL pt, DWORD dropEffect)
{
  FORMATETC fmtetc;
  STGMEDIUM stgmed;
  bool ret = false;

  ZeroMemory(&fmtetc, sizeof(FORMATETC));
  fmtetc.cfFormat = CF_EMERGE_MENUITEM;
  fmtetc.dwAspect = DVASPECT_CONTENT;
  fmtetc.lindex = -1;
  fmtetc.tymed = TYMED_HGLOBAL;

  if (SUCCEEDED(pDataObj->QueryGetData(&fmtetc)))
    {
      if (SUCCEEDED(pDataObj->GetData(&fmtetc, &stgmed)))
        {
          void *data = GlobalLock(stgmed.hGlobal);
          POINT menuItemPt;
          menuItemPt.x = pt.x;
          menuItemPt.y = pt.y;
          MENUITEMDATA *menuItemData = reinterpret_cast< MENUITEMDATA* >(data);

          MenuItemDrop(menuItemData, menuItemPt);

          ret = GlobalUnlock(stgmed.hGlobal);
        }
    }

  fmtetc.cfFormat = CF_HDROP;

  if (SUCCEEDED(pDataObj->QueryGetData(&fmtetc)))
    {
      if (SUCCEEDED(pDataObj->GetData(&fmtetc, &stgmed)))
        {
          HDROP hdrop = (HDROP)GlobalLock(stgmed.hGlobal);

          FileDrop(hdrop, dropEffect);

          ret = GlobalUnlock(stgmed.hGlobal);
        }
    }

  return ret;
}

bool CustomDropTarget::MenuItemDrop(MENUITEMDATA *menuItemData, POINT menuItemPt)
{
  TiXmlElement *newElement = ELCloneXMLElement(menuItemData->element, dropElement);
  if (newElement)
    {
      ELWriteXMLConfig(ELGetXMLConfig(newElement));

      NEWMENUITEMDATA newMenuItemData;
      COPYDATASTRUCT cds;

      ZeroMemory(&newMenuItemData, sizeof(NEWMENUITEMDATA));
      CopyMemory(&newMenuItemData.menuItemData, menuItemData, sizeof(NEWMENUITEMDATA));
      newMenuItemData.newElement = newElement;
      newMenuItemData.menu = dropMenu;
      newMenuItemData.pt = menuItemPt;

      cds.dwData = EMERGE_NEWITEM;
      cds.cbData = sizeof(NEWMENUITEMDATA);
      cds.lpData = &newMenuItemData;

      SendMessage(FindWindow(TEXT("EmergeDesktopMenuBuilder"), NULL),
                  WM_COPYDATA, 0, (LPARAM)&cds);

      return true;
    }

  return false;
}

bool CustomDropTarget::FileDrop(HDROP hdrop, DWORD dropEffect)
{
  bool ret = false;

  if (hdrop)
    {
      UINT count = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
      WCHAR filename[MAX_PATH];
      std::wstring workingValue = value;
      UINT fileOp = FO_COPY;
      if (dropEffect & DROPEFFECT_MOVE)
        fileOp = FO_MOVE;

      if (!ELPathIsDirectory(value))
        {
          size_t backslash = workingValue.rfind('\\');
          if (backslash != std::wstring::npos)
            workingValue = workingValue.substr(0, backslash);

          if (!ELPathIsDirectory(workingValue.c_str()))
            return false;
        }

      for (UINT i = 0; i < count; i++)
        {
          DragQueryFile(hdrop, i, filename, sizeof(filename) / sizeof(WCHAR));
          std::wstring workingFilename = filename;

          ret = ELFileOp(FindWindow(TEXT("EmergeDesktopMenuBuilder"), NULL),
                         true, fileOp, workingFilename, workingValue);
        }
    }

  return ret;
}

STDMETHODIMP_(ULONG) CustomDropTarget::AddRef()
{
  return ++refCount;
}

STDMETHODIMP_(ULONG) CustomDropTarget::Release()
{
  ULONG tmp;
  tmp = --refCount;

  return tmp;
}

STDMETHODIMP CustomDropTarget::QueryInterface(REFIID riid, void ** ppvObject)
{
  if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
      AddRef();
      *ppvObject = this;
      return S_OK;
    }
  else
    {
      *ppvObject = NULL;
      return E_NOINTERFACE;
    }
}

STDMETHODIMP CustomDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
  // does the dataobject contain data we want?
  allowDrop = QueryDataObject(pDataObj);

  if(allowDrop)
    // get the dropeffect based on keyboard state
    *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}

STDMETHODIMP CustomDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
  if(allowDrop)
    *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}

STDMETHODIMP CustomDropTarget::DragLeave()
{
  return S_OK;
}

STDMETHODIMP CustomDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
  DWORD dropEffect = DropEffect(grfKeyState, pt, *pdwEffect);

  if(allowDrop)
    {
      DataDrop(pDataObj, pt, dropEffect);
      *pdwEffect = dropEffect;
    }
  else
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}

