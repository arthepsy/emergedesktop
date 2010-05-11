//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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
//---
//
// Note: The CustomSource class is based on CCustomAutoComplete by Klaus H. Probst
// which is copyright 2002.
//
//-----

#ifndef __ECM_CUSTOMSOURCE_H
#define __ECM_CUSTOMSOURCE_H

#include <stdio.h>
#include "Settings.h"

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

//#define MAX_HISTORY 9

class CustomSource : public IEnumString
{
private:
  std::tr1::shared_ptr<Settings> pSettings;
  ULONG refCount;
  ULONG currentElement;

public:
  CustomSource(std::tr1::shared_ptr<Settings> pSettings);
  virtual ~CustomSource();
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();
  STDMETHODIMP Next(ULONG, LPOLESTR*, ULONG*);
  STDMETHODIMP Skip(ULONG jump);
  STDMETHODIMP Reset();
  STDMETHODIMP Clone(IEnumString**);
  void AddElement(WCHAR *element);
};

#endif

