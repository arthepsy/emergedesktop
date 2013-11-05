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

#ifndef __EMERGESTYLEENGINE_H
#define __EMERGESTYLEENGINE_H

#ifdef EMERGESTYLEENGINE_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#define UNICODE 1
#include <windows.h>
#include <string>
#include "../emergeGraphics/emergeGraphics.h"

// Declaration of functions to import
DLL_EXPORT std::wstring ESEGetStyle();
DLL_EXPORT void ESESetStyle(std::wstring styleFile);
DLL_EXPORT bool ESEWriteStyle(std::wstring styleFile, LPGUIINFO guiInfo, HWND hwnd);
DLL_EXPORT void ESEReadStyle(std::wstring styleFile, LPGUIINFO guiInfo);
DLL_EXPORT void ESELoadStyle(std::wstring styleFile, LPGUIINFO guiInfo);
DLL_EXPORT bool ESEEqualStyle(LPGUIINFO sourceInfo, LPGUIINFO targetInfo);
DLL_EXPORT void ESEPaintBackground(HDC hdc, RECT clientRect, LPGUIINFO guiInfo, bool active);

#endif
