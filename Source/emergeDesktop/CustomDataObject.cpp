//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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

CustomDataObject::CustomDataObject(HMENU menu)
{
  this->menu = menu;
  refCount = 0;
}

CustomDataObject::~CustomDataObject()
{
  if (stgm != NULL)
    {
      if (deleteMedium)
        ReleaseStgMedium(stgm.get());
    }
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

STDMETHODIMP CustomDataObject::DAdvise(FORMATETC FAR* pformatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
  IDataAdviseHolder *myadvise = advise.get();
  if (!myadvise)
    CreateDataAdviseHolder(&myadvise);

  return myadvise->Advise(this, pformatetc, advf, pAdvSink, pdwConnection);
}

STDMETHODIMP CustomDataObject::DUnadvise(DWORD dwConnection)
{
  return advise->Unadvise(dwConnection);
}

STDMETHODIMP CustomDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{
  return advise->EnumAdvise(ppenumAdvise);
}

STDMETHODIMP CustomDataObject::EnumFormatEtc(DWORD dwDirection UNUSED, LPENUMFORMATETC FAR* ppenumFormatEtc)
{
  *ppenumFormatEtc = NULL;

  return E_NOTIMPL;
}

STDMETHODIMP CustomDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc UNUSED, LPFORMATETC pformatetcOut)
{
  pformatetcOut->ptd = NULL;

  return E_NOTIMPL;
}

STDMETHODIMP CustomDataObject::GetData(LPFORMATETC pformatetcIn UNUSED, LPSTGMEDIUM pmedium UNUSED)
{
  return DV_E_FORMATETC;
}

STDMETHODIMP CustomDataObject::GetDataHere(LPFORMATETC pformatetc UNUSED, LPSTGMEDIUM pmedium UNUSED)
{
  return DV_E_FORMATETC;
}

STDMETHODIMP CustomDataObject::QueryGetData(LPFORMATETC pformatetc UNUSED)
{
  return E_NOTIMPL;
}

STDMETHODIMP CustomDataObject::SetData(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease UNUSED)
{
  if(pformatetc == NULL || pmedium == NULL)
    return E_INVALIDARG;

  fetc=std::tr1::shared_ptr<FORMATETC>(new FORMATETC);
  stgm=std::tr1::shared_ptr<STGMEDIUM>(new STGMEDIUM);

  if(fetc == NULL || stgm == NULL)
    return E_OUTOFMEMORY;

  *fetc = *pformatetc;
  *stgm = *pmedium;
  deleteMedium = fRelease;

  return S_OK;
}

