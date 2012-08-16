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

HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc);

class CustomEnumFormatEtc : public IEnumFORMATETC
{
public:
	// IUnknown members
	STDMETHODIMP  QueryInterface (REFIID iid, void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef (void);
	STDMETHODIMP_(ULONG) Release (void);

	// IEnumFormatEtc members
	STDMETHODIMP Next(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
	STDMETHODIMP Skip(ULONG celt);
	STDMETHODIMP Reset(void);
	STDMETHODIMP Clone(IEnumFORMATETC ** ppEnumFormatEtc);

	CustomEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
	virtual ~CustomEnumFormatEtc();
  void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source);

private:

	LONG refCount;		// Reference count for this COM interface
	ULONG	index;			// current enumerator index
	ULONG	numFormats;		// number of FORMATETC members
	FORMATETC *pFormatEtc;		// array of FORMATETC objects
};
