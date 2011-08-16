//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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

#ifndef __ED_CUSTOMDROPSOURCE_H
#define __ED_CUSTOMDROPSOURCE_H

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

class CustomDropSource : public IDropSource
{
private:
  ULONG refCount;

public:
  CustomDropSource();
  virtual ~CustomDropSource();
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();
  STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
  STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
  STDMETHODIMP GiveFeedback(DWORD dwEffect);
};

#endif

