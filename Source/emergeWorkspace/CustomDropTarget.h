/*!
@file CustomDropTarget.h
@brief header for emergeWorkspace
@author The Emerge Desktop Development Team

@attention This file is part of Emerge Desktop.
@attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

@attention Emerge Desktop is free software; you can redistribute it and/or
modify  it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

@attention Emerge Desktop is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

@attention You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __GUARD_150C3742_002B_4629_BBB9_637DA8C78EF9
#define __GUARD_150C3742_002B_4629_BBB9_637DA8C78EF9

#define UNICODE 1

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeItemDefinitions.h"
#include "../emergeLib/emergeUtilityLib.h"

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

  CustomDropTarget(MENUITEMDATA menuItemData, UINT_PTR dropID, HMENU dropMenu);
  virtual ~CustomDropTarget();

private:
  // internal helper function
  DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
  bool QueryDataObject(IDataObject *pDataObj);
  bool DataDrop(IDataObject *pDataObj, POINTL pt, DWORD dropEffect);
  bool MenuItemDrop(MENUITEMDATA *menuItemData, POINT menuItemPt);
  bool FileDrop(HDROP hdrop, DWORD dropEffect);

  // Private member variables
  bool allowDrop;
  UINT_PTR dropID;
  UINT refCount;
  UINT CF_EMERGE_MENUITEM;
  HMENU dropMenu;
  MENUITEMDATA dropItemData;
};

HRESULT CreateDropTarget(IDropTarget **ppDropTarget, MENUITEMDATA menuItemData, UINT_PTR dropID, HMENU dropMenu);

#endif
