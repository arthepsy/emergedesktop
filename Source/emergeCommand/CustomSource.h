/*!
  @file CustomSource.h
  @brief header for emergeCommand
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  @note The CustomSource class is based on CCustomAutoComplete by Klaus H. Probst
  which is copyright 2002.
  */

#ifndef __GUARD_1773d837_0871_4c82_b495_98801ddcfa86
#define __GUARD_1773d837_0871_4c82_b495_98801ddcfa86

#define UNICODE 1

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1 //suppress warnings about old versions of wcscpy, wcscat, etc.
#endif

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <stdio.h>
#include "Settings.h"

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
  void AddElement(WCHAR* element);
};

#endif

