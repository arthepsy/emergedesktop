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

#include "CustomDataObject.h"
#include "CustomEnumFormat.h"

HRESULT CreateDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject)
{
  if(ppDataObject == NULL)
    return E_INVALIDARG;

  *ppDataObject = new CustomDataObject(fmtetc, stgmeds, count);

  return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
}

CustomDataObject::CustomDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, UINT count)
{
  refCount = 0;
  numFormats  = count;

  pFormatEtc = new FORMATETC[count];
  pStgMedium = new STGMEDIUM[count];

  for(UINT i = 0; i < count; i++)
    {
      pFormatEtc[i] = fmtetc[i];
      pStgMedium[i] = stgmed[i];
    }
}

CustomDataObject::~CustomDataObject()
{
}

int CustomDataObject::LookupFormatEtc(FORMATETC *pFormatEtcIn)
{
  // check each of our formats in turn to see if one matches
  for(UINT i = 0; i < numFormats; i++)
    {
      if((pFormatEtc[i].tymed    &  pFormatEtcIn->tymed)   &&
          pFormatEtc[i].cfFormat == pFormatEtcIn->cfFormat &&
          pFormatEtc[i].dwAspect == pFormatEtcIn->dwAspect)
        {
          // return index of stored format
          return i;
        }
    }

  // error, format not found
  return -1;
}

HGLOBAL CustomDataObject::DupGlobalMem(HGLOBAL hMem)
{
  DWORD len = GlobalSize(hMem);
  PVOID source = GlobalLock(hMem);
  PVOID dest = GlobalAlloc(GMEM_FIXED, len);

  memcpy(dest, source, len);
  GlobalUnlock(hMem);
  return dest;
}

STDMETHODIMP_(ULONG) CustomDataObject::AddRef()
{
  return ++refCount;
}

STDMETHODIMP_(ULONG) CustomDataObject::Release()
{
  ULONG tmp;
  tmp = --refCount;

  return tmp;
}

STDMETHODIMP CustomDataObject::QueryInterface(REFIID riid, void ** ppvObject)
{
  if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDataObject))
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

STDMETHODIMP CustomDataObject::DAdvise(FORMATETC FAR* pformatetc UNUSED, DWORD advf UNUSED, LPADVISESINK pAdvSink UNUSED, DWORD FAR* pdwConnection UNUSED)
{
  return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CustomDataObject::DUnadvise(DWORD dwConnection UNUSED)
{
  return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CustomDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise UNUSED)
{
  return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP CustomDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppEnumFormatEtc)
{
  // only the get direction is supported for OLE
  if(dwDirection == DATADIR_GET)
    return CreateEnumFormatEtc(numFormats, pFormatEtc, ppEnumFormatEtc);
  else
    return E_NOTIMPL;
}

STDMETHODIMP CustomDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc UNUSED, LPFORMATETC pformatetcOut)
{
  pformatetcOut->ptd = NULL;
  return E_NOTIMPL;
}

STDMETHODIMP CustomDataObject::GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium)
{
  int idx;

  // try to match the specified FORMATETC with one of our supported formats
  if((idx = LookupFormatEtc(pformatetcIn)) == -1)
    return DV_E_FORMATETC;

  // found a match - transfer data into supplied storage medium
  pmedium->tymed = pFormatEtc[idx].tymed;
  pmedium->pUnkForRelease  = 0;

  // copy the data into the caller's storage medium
  switch(pFormatEtc[idx].tymed)
    {
    case TYMED_HGLOBAL:
      pmedium->hGlobal = DupGlobalMem(pStgMedium[idx].hGlobal);
      break;

    default:
      return DV_E_FORMATETC;
    }
  return S_OK;
}

STDMETHODIMP CustomDataObject::GetDataHere(LPFORMATETC pformatetc UNUSED, LPSTGMEDIUM pmedium UNUSED)
{
  return DV_E_FORMATETC;
}

STDMETHODIMP CustomDataObject::QueryGetData(LPFORMATETC pformatetc)
{
  return (LookupFormatEtc(pformatetc) == -1) ? DV_E_FORMATETC : S_OK;
}

STDMETHODIMP CustomDataObject::SetData(LPFORMATETC pformatetc UNUSED, STGMEDIUM FAR * pmedium UNUSED, BOOL fRelease UNUSED)
{
  return E_NOTIMPL;
}
