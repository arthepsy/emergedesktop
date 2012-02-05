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

#ifndef __ED_CUSTOMDATAOBJECT_H
#define __ED_CUSTOMDATAOBJECT_H

#include "../emergeLib/emergeLib.h"

/*static int
  FromDROPEFFECT(int effect)
  {
  int	    rc = 0;

  if (effect & DROPEFFECT_COPY)
  rc |= SW_DRAG_COPY;
  if (effect & DROPEFFECT_MOVE)
  rc |= SW_DRAG_MOVE;
  if (effect & DROPEFFECT_LINK)
  rc |= SW_DRAG_LINK;
  return rc;
  }*/

/*static DWORD
  ToDROPEFFECT(int effect)
  {
  DWORD	    rc = DROPEFFECT_NONE;

  if (effect & SW_DRAG_COPY)
  rc |= DROPEFFECT_COPY;
  if (effect & SW_DRAG_MOVE)
  rc |= DROPEFFECT_MOVE;
  if (effect & SW_DRAG_LINK)
  rc |= DROPEFFECT_LINK;
  return rc;
  }*/

class CustomDataObject : public IDataObject
{
private:
  ULONG refCount;
  std::tr1::shared_ptr<IDataAdviseHolder> advise;
  std::tr1::shared_ptr<FORMATETC> fetc;
  std::tr1::shared_ptr<STGMEDIUM> stgm;
  BOOL deleteMedium;

public:
  CustomDataObject();
  virtual ~CustomDataObject();
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();
  STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
  STDMETHODIMP DAdvise(FORMATETC FAR* pformatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD FAR* pdwConnection);
  STDMETHODIMP DUnadvise(DWORD dwConnection);
  STDMETHODIMP EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise);
  STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);
  STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pformatetc, LPFORMATETC pformatetcOut);
  STDMETHODIMP GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium);
  STDMETHODIMP GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium);
  STDMETHODIMP QueryGetData(LPFORMATETC pformatetc);
  STDMETHODIMP SetData(LPFORMATETC pformatetc, STGMEDIUM FAR * pmedium, BOOL fRelease);
};

#endif

