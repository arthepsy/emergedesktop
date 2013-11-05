//---
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
//---

#include "CustomSource.h"

CustomSource::CustomSource(std::tr1::shared_ptr<Settings> pSettings)
{
  currentElement = 0;
  refCount = 0;
  pSettings->BuildHistoryList();
  this->pSettings = pSettings;
}

CustomSource::~CustomSource()
{}

STDMETHODIMP_(ULONG) CustomSource::AddRef()
{
  return ++refCount;
}

STDMETHODIMP_(ULONG) CustomSource::Release()
{
  ULONG tmp;
  tmp = --refCount;

  return tmp;
}

STDMETHODIMP CustomSource::Clone(IEnumString **target)
{
  *target = NULL;

  CustomSource *copy = new CustomSource(pSettings);

  *target = copy;

  return NOERROR;
}

STDMETHODIMP CustomSource::Skip(ULONG jump)
{
  currentElement += jump;

  if (currentElement > MAX_HISTORY)
    currentElement = 0;

  return NOERROR;
}

STDMETHODIMP CustomSource::Next(ULONG current, LPOLESTR *nextString, ULONG *next)
{
  UINT i, size = pSettings->GetHistoryListSize();
  int wideSize;
  WCHAR tmp[MAX_LINE_LENGTH];

  *next = 0;

  for (i = 0; i < current; i++)
    {
      if (currentElement == size)
        break;

      wcscpy(tmp, pSettings->GetHistoryListItem(currentElement).c_str());

      wideSize = sizeof(WCHAR) * (int)(wcslen(tmp) + 1);
      nextString[i] = (LPWSTR)CoTaskMemAlloc((ULONG)wideSize);
      wcscpy(nextString[i], tmp);

      *next = i;

      currentElement++;
    }

  if (i == current)
    return NOERROR;

  return S_FALSE;
}

STDMETHODIMP CustomSource::Reset()
{
  currentElement = 0;
  return NOERROR;
}

STDMETHODIMP CustomSource::QueryInterface(REFIID riid, void **ppv)
{
  *ppv=NULL;

  if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumString))
    *ppv=(LPVOID)this;

  if (NULL!=*ppv)
    {
      ((LPUNKNOWN)*ppv)->AddRef();
      return NOERROR;
    }

  return E_NOINTERFACE;
}

void CustomSource::AddElement(WCHAR *element)
{
  pSettings->AddHistoryItem(element);
}
