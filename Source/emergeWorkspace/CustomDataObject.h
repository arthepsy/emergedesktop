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
#include <shlobj.h>

HRESULT CreateDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject);
HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc);

class CustomDataObject : public IDataObject
{
private:
  ULONG refCount;
  UINT numFormats;
  UINT CF_EMERGE_MENUITEM;
  LPFORMATETC pFormatEtc;
  LPSTGMEDIUM pStgMedium;

public:
  // IUnknown members
  STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // IDataObject members
  STDMETHODIMP GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
  STDMETHODIMP GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
  STDMETHODIMP QueryGetData(LPFORMATETC pformatetc);
  STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
  STDMETHODIMP SetData(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease);
  STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
  STDMETHODIMP DAdvise(FORMATETC FAR* pformatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
  STDMETHODIMP DUnadvise(DWORD dwConnection);
  STDMETHODIMP EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise);

  CustomDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, UINT count);
  virtual ~CustomDataObject();
  int LookupFormatEtc(FORMATETC *pFormatEtc);
  HGLOBAL DupGlobalMem(HGLOBAL hMem);
};
