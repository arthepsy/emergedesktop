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

CustomDropTarget::CustomDropTarget(UINT type)
{
  refCount = 0;
  this->type = type;
}

CustomDropTarget::~CustomDropTarget()
{
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

STDMETHODIMP CustomDropTarget::DragEnter(IDataObject *pDataObj UNUSED, DWORD grfKeyState, POINTL pt UNUSED, DWORD *pdwEffect)
{
  if ((grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
    *pdwEffect = DROPEFFECT_LINK;
  else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
    *pdwEffect = DROPEFFECT_COPY;
  else
    *pdwEffect = DROPEFFECT_MOVE;

  if ((type == IT_FILE) || (type == IT_FILE_SUBMENU))
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}

STDMETHODIMP CustomDropTarget::DragOver(DWORD grfKeyState, POINTL pt UNUSED, DWORD *pdwEffect)
{
  if ((grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
    *pdwEffect = DROPEFFECT_LINK;
  else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
    *pdwEffect = DROPEFFECT_COPY;
  else
    *pdwEffect = DROPEFFECT_MOVE;

  if ((type == IT_FILE) || (type == IT_FILE_SUBMENU))
    *pdwEffect = DROPEFFECT_NONE;

  return S_OK;
}

STDMETHODIMP CustomDropTarget::DragLeave()
{
  return S_OK;
}

STDMETHODIMP CustomDropTarget::Drop(IDataObject *pDataObj UNUSED, DWORD grfKeyState, POINTL pt UNUSED, DWORD *pdwEffect)
{
  if ((grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
    *pdwEffect = DROPEFFECT_LINK;
  else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
    *pdwEffect = DROPEFFECT_COPY;
  else
    *pdwEffect = DROPEFFECT_MOVE;

  return S_OK;
}

