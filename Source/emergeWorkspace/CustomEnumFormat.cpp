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

#include "CustomEnumFormat.h"

HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc)
{
  if(nNumFormats == 0 || pFormatEtc == 0 || ppEnumFormatEtc == 0)
    return E_INVALIDARG;

  *ppEnumFormatEtc = new CustomEnumFormatEtc(pFormatEtc, nNumFormats);

  return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}

CustomEnumFormatEtc::CustomEnumFormatEtc(FORMATETC *pFormatEtcIn, int nNumFormats)
{
  refCount = 0;
  index = 0;
  numFormats = nNumFormats;
  pFormatEtc  = new FORMATETC[nNumFormats];

  // copy the FORMATETC structures
  for(int i = 0; i < nNumFormats; i++)
    DeepCopyFormatEtc(&pFormatEtc[i], &pFormatEtcIn[i]);
}

CustomEnumFormatEtc::~CustomEnumFormatEtc()
{
  if(pFormatEtc)
    {
      for(ULONG i = 0; i < numFormats; i++)
        {
          if(pFormatEtc[i].ptd)
            CoTaskMemFree(pFormatEtc[i].ptd);
        }

      delete[] pFormatEtc;
    }
}

void CustomEnumFormatEtc::DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
{
  // copy the source FORMATETC into dest
  *dest = *source;

  if(source->ptd)
    {
      // allocate memory for the DVTARGETDEVICE if necessary
      dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

      // copy the contents of the source DVTARGETDEVICE into dest->ptd
      *(dest->ptd) = *(source->ptd);
    }
}

STDMETHODIMP_(ULONG) CustomEnumFormatEtc::AddRef(void)
{
  // increment object reference count
  return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) CustomEnumFormatEtc::Release(void)
{
  // decrement object reference count
  LONG count = InterlockedDecrement(&refCount);

  if(count == 0)
    {
      delete this;
      return 0;
    }
  else
    return count;
}

STDMETHODIMP CustomEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject)
{
  // check to see what interface has been requested
  if(iid == IID_IEnumFORMATETC || iid == IID_IUnknown)
    {
      AddRef();
      *ppvObject = this;
      return S_OK;
    }
  else
    {
      *ppvObject = 0;
      return E_NOINTERFACE;
    }
}

STDMETHODIMP CustomEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtcIn, ULONG * pceltFetched)
{
  ULONG copied  = 0;

  // validate arguments
  if(celt == 0 || pFormatEtc == 0)
    return E_INVALIDARG;

  // copy FORMATETC structures into caller's buffer
  while(index < numFormats && copied < celt)
    {
      DeepCopyFormatEtc(&pFormatEtcIn[copied], &pFormatEtc[index]);
      copied++;
      index++;
    }

  // store result
  if(pceltFetched != 0)
    *pceltFetched = copied;

  // did we copy all that was requested?
  return (copied == celt) ? S_OK : S_FALSE;
}

STDMETHODIMP CustomEnumFormatEtc::Skip(ULONG celt)
{
  index += celt;
  return (index <= numFormats) ? S_OK : S_FALSE;
}

STDMETHODIMP CustomEnumFormatEtc::Reset(void)
{
  index = 0;
  return S_OK;
}

STDMETHODIMP CustomEnumFormatEtc::Clone(IEnumFORMATETC ** ppEnumFormatEtc)
{
  HRESULT hResult;

  // make a duplicate enumerator
  hResult = CreateEnumFormatEtc(numFormats, pFormatEtc, ppEnumFormatEtc);

  if(hResult == S_OK)
    // manually set the index state
    ((CustomEnumFormatEtc*) *ppEnumFormatEtc)->index = index;

  return hResult;
}
