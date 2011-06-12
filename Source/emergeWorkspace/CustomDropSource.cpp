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

#include "CustomDropSource.h"

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
  if (fEscapePressed)
    return DRAGDROP_S_CANCEL;
  else if (grfKeyState & MK_LBUTTON)
    return S_OK;
  else
    return DRAGDROP_S_DROP;
}

STDMETHODIMP CustomDropSource::GiveFeedback(DWORD dwEffect UNUSED)
{
  /*OutputDebugStr((WCHAR*)TEXT("GiveFeedback"));*/

  return DRAGDROP_S_USEDEFAULTCURSORS;
}

