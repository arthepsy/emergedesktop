/*!
  @file emergeStyleEngine.h
  @brief export header for emergeStyleEngine
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
  */

#ifndef __GUARD_77398f4b_6e40_4041_b72f_26e357f268d3
#define __GUARD_77398f4b_6e40_4041_b72f_26e357f268d3

#define UNICODE 1

#ifdef EMERGESTYLEENGINE_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

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
