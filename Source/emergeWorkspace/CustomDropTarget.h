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

#pragma once

#include "../emergeLib/emergeLib.h"

typedef struct _NEWMENUITEMDATA
{
  MENUITEMDATA menuItemData;
  MENUITEMDATA dropItemData;
  HMENU menu;
  POINT pt;
}
NEWMENUITEMDATA, *LPNEWMENUITEMDATA;

typedef struct _DRAGITEMDATA
{
  MENUITEMDATA menuItemData;
  UINT itemID;
}
DRAGITEMDATA, *LPDRAGITEMDATA;


static const UINT EMERGE_NEWITEM = RegisterWindowMessage(TEXT("EmergeNewItem"));

class CustomDropTarget : public IDropTarget
{
public:
  // IUnknown members
  STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IDropTarget members
  STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
  STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
  STDMETHODIMP DragLeave();
  STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

  CustomDropTarget(MENUITEMDATA menuItemData, UINT dropID, HMENU dropMenu);
  virtual ~CustomDropTarget();

private:
  // internal helper function
  DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
  bool QueryDataObject(IDataObject *pDataObj);
  bool DataDrop(IDataObject *pDataObj, POINTL pt, DWORD dropEffect);
  bool MenuItemDrop(MENUITEMDATA *menuItemData, POINT menuItemPt);
  bool FileDrop(HDROP hdrop, DWORD dropEffect);

  // Private member variables
  bool   allowDrop;
  UINT dropID;
  UINT refCount;
  UINT CF_EMERGE_MENUITEM;
  HMENU dropMenu;
  MENUITEMDATA dropItemData;
};

HRESULT CreateDropTarget(IDropTarget **ppDropTarget, MENUITEMDATA menuItemData, UINT dropID, HMENU dropMenu);
