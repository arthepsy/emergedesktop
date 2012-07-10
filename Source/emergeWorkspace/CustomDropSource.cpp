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

#include "CustomDropSource.h"

HRESULT CreateDropSource(IDropSource **ppDropSource)
{
  if(ppDropSource == NULL)
    return E_INVALIDARG;

  *ppDropSource = new CustomDropSource();

  return (*ppDropSource) ? S_OK : E_OUTOFMEMORY;
}

CustomDropSource::CustomDropSource()
{
  refCount = 0;
}

CustomDropSource::~CustomDropSource()
{
}

STDMETHODIMP_(ULONG) CustomDropSource::AddRef()
{
  return ++refCount;
}

STDMETHODIMP_(ULONG) CustomDropSource::Release()
{
  ULONG tmp;
  tmp = --refCount;

  return tmp;
}

STDMETHODIMP CustomDropSource::QueryInterface(REFIID riid, void ** ppvObject)
{
  if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropSource))
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

STDMETHODIMP CustomDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
  // if the Escape key has been pressed since the last call, cancel the drop
  if(fEscapePressed == TRUE)
    return DRAGDROP_S_CANCEL;

  // if the LeftMouse button has been released, then do the drop!
  if((grfKeyState & MK_LBUTTON) == 0)
    return DRAGDROP_S_DROP;

  // continue with the drag-drop
  return S_OK;
}

STDMETHODIMP CustomDropSource::GiveFeedback(DWORD dwEffect UNUSED)
{
  return DRAGDROP_S_USEDEFAULTCURSORS;
}

