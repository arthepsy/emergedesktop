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

#include "CustomDropTarget.h"

CustomDropTarget::CustomDropTarget()
{
  refCount = 0;
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
  //OutputDebugStr((WCHAR*)TEXT("Target: Got QueryInterface"));

  if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
      AddRef();
      *ppvObject = this;
      //OutputDebugStr((WCHAR*)TEXT("Returning S_OK"));
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
  //OutputDebugStr((WCHAR*)TEXT("Got DragEnter"));

  if ((grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
    *pdwEffect = DROPEFFECT_LINK;
  else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
    *pdwEffect = DROPEFFECT_COPY;
  else
    {
      //OutputDebugStr((WCHAR*)TEXT("DragEnter: DROPEFFECTMOVE"));
      *pdwEffect = DROPEFFECT_MOVE;
    }

  return S_OK;
}

STDMETHODIMP CustomDropTarget::DragOver(DWORD grfKeyState, POINTL pt UNUSED, DWORD *pdwEffect)
{
  //OutputDebugStr((WCHAR*)TEXT("Got DragOver"));

  if ((grfKeyState & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
    *pdwEffect = DROPEFFECT_LINK;
  else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
    *pdwEffect = DROPEFFECT_COPY;
  else
    {
      //OutputDebugStr((WCHAR*)TEXT("DragOver: DROPEFFECTMOVE"));
      *pdwEffect = DROPEFFECT_MOVE;
    }

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
    {
      //OutputDebugStr((WCHAR*)TEXT("Drop: DROPEFFECTMOVE"));
      *pdwEffect = DROPEFFECT_MOVE;
    }

  return S_OK;
}

