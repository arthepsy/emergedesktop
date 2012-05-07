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

#include "CommandSource.h"

CommandSource::CommandSource(HINSTANCE hInst)
{
  currentElement = 0;
  refCount = 0;
  this->hInst = hInst;

  commandWnd = CreateWindowEx(
              WS_EX_TOOLWINDOW,
              TEXT("COMBOBOX"),
              NULL,
              CBS_DROPDOWNLIST,
              0, 0,
              0, 0,
              NULL,
              NULL,
              hInst,
              NULL);

  if (commandWnd)
    ELPopulateInternalCommandList(commandWnd);
  else
    ELWriteDebug(L"Command List failed");

  ShowWindow(commandWnd, SW_HIDE);
}

CommandSource::~CommandSource()
{
  DestroyWindow(commandWnd);
}

STDMETHODIMP_(ULONG) CommandSource::AddRef()
{
  return ++refCount;
}

STDMETHODIMP_(ULONG) CommandSource::Release()
{
  ULONG tmp;
  tmp = --refCount;

  return tmp;
}

STDMETHODIMP CommandSource::Clone(IEnumString **target)
{
  *target = NULL;

  CommandSource *copy = new CommandSource(hInst);

  *target = copy;

  return NOERROR;
}

STDMETHODIMP CommandSource::Skip(ULONG jump)
{
  currentElement += jump;

  if (currentElement > (ULONG)SendMessage(commandWnd, CB_GETCOUNT, 0, 0))
    currentElement = 0;

  return NOERROR;
}

STDMETHODIMP CommandSource::Next(ULONG current, LPOLESTR *nextString, ULONG *next)
{
  UINT i, size = SendMessage(commandWnd, CB_GETCOUNT, 0, 0);
  int wideSize;
  WCHAR tmp[MAX_PATH];

  *next = 0;

  for (i = 0; i < current; i++)
    {
      if (currentElement == size)
        break;

      SendMessage(commandWnd, CB_SETCURSEL, currentElement, 0);
      GetWindowText(commandWnd, tmp, MAX_PATH);

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

STDMETHODIMP CommandSource::Reset()
{
  currentElement = 0;
  return NOERROR;
}

STDMETHODIMP CommandSource::QueryInterface(REFIID riid, void **ppv)
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
