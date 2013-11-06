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
//
// Note:  BImage is based of the BImage component (the gradient engine) of BB4Win,
//        http://bb4win.org
//
//-----

#ifndef __BIMAGE_H
#define __BIMAGE_H

#include <windows.h>

bool vgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool hgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool dgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool cdgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool pcgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool egradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool rgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool pgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE* pixels, bool interlaced);
bool bevel(UINT width, UINT height, UINT bevelWidth, BYTE* pixels, bool sunken);

BYTE convertcolour(BYTE colour, bool dark);
unsigned long bsqrt(unsigned long x);
unsigned long getSqrt(unsigned int x);

#endif

