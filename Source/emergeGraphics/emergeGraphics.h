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

#ifndef __EMERGEGRAPHICS_H
#define __EMERGEGRAPHICS_H

#ifdef EMERGEGRAPHICS_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#define UNICODE 1
#include <windows.h>

// Defines required for SetWindowLayeredAttributes
#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED   0x00080000
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY    0x00000001
#endif

#ifndef LWA_ALPHA
#define LWA_ALPHA       0x00000002
#endif

// Define required for creating the window drop shadow
#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW   0x00020000
#endif


#define ICON_DEFAULT        0
#define ICON_RUN            4
#define ICON_SHUTDOWN       5
#define ICON_LOGOFF         7
#define ICON_QUIT           9
#define ICON_LOCK           10
#define ICON_EMERGE         11

#define EGDAT_LEFT          1
#define EGDAT_HCENTER       2
#define EGDAT_RIGHT         3
#define EGDAT_TOP           4
#define EGDAT_VCENTER       5
#define EGDAT_BOTTOM        6

#ifndef FE_FONTSMOOTHINGSTANDARD
#define FE_FONTSMOOTHINGSTANDARD 1
#endif

#ifndef FE_FONTSMOOTHINGCLEARTYPE
#define FE_FONTSMOOTHINGCLEARTYPE 2
#endif

// Format Info
typedef struct _FORMATINFO
{
  UINT horizontalAlignment;
  HFONT font;
  COLORREF color;
  UINT lines;
  UINT fontHeight;
  UINT verticalAlignment;
}
FORMATINFO, *LPFORMATINFO;

// Client Info
typedef struct _CLIENTINFO
{
  HDC hdc;
  RECT rt;
  BYTE bgAlpha;
}
CLIENTINFO, *LPCLIENTINFO;

// GUI Info
typedef struct _GUIINFO
{
  BYTE alphaActive;
  BYTE alphaInactive;
  BYTE alphaBackground;
  BYTE alphaMenu;
  BYTE alphaForeground;
  BYTE alphaFrame;
  BYTE alphaSelected;
  BYTE alphaBorder;
  BYTE alphaText;
  COLORREF colorBackground;
  COLORREF colorForeground;
  COLORREF colorFrame;
  COLORREF colorSelected;
  COLORREF colorFont;
  COLORREF colorBorder;
  COLORREF gradientFrom;
  COLORREF gradientTo;
  bool windowShadow;
  int dragBorder;
  WCHAR gradientMethod[MAX_PATH];
  int bevelWidth;
  int padding;
}
GUIINFO, *LPGUIINFO;

// Declaration of functions to import
DLL_EXPORT HICON EGConvertIcon(HICON sourceIcon, RECT iconRect, HDC backgroundDC, BYTE foregroundAlpha);
DLL_EXPORT HBRUSH EGCreateBrush(BYTE alpha, COLORREF colour);
DLL_EXPORT HPEN EGCreatePen(DWORD style, DWORD width, BYTE alpha, COLORREF colour);
DLL_EXPORT HBITMAP EGCreateBitmap(BYTE alpha, COLORREF colour, RECT wndRect);
DLL_EXPORT HICON EGGetFileIcon(WCHAR *file, UINT iconSize);
DLL_EXPORT HICON EGGetWindowIcon(HWND hwnd, bool smallIcon, bool force);
DLL_EXPORT HICON EGGetSpecialFolderIcon(int csidl, UINT iconSize);
DLL_EXPORT HICON EGGetSystemIcon(UINT iconIndex, UINT iconSize);
DLL_EXPORT bool EGDrawAlphaText(BYTE alpha, CLIENTINFO clientInfo, FORMATINFO formatInfo, WCHAR *commandText);
DLL_EXPORT HDC EGBeginPaint(HWND wnd);
DLL_EXPORT void EGEndPaint();
DLL_EXPORT bool EGFillRect(HDC hdc, RECT *rect, BYTE alpha, COLORREF colour);
DLL_EXPORT bool EGGradientFillRect(HDC hdc, RECT *rect, BYTE alpha, COLORREF colourFrom, COLORREF colourTo, int bevelWidth, WCHAR *gradientMethod);
DLL_EXPORT bool EGFrameRect(HDC hdc, RECT *rect, BYTE bgAlpha, COLORREF borderColour, int borderWidth);
DLL_EXPORT UINT32 EGGetPixel(BYTE alpha, COLORREF colour);
DLL_EXPORT BYTE EGGetMinAlpha(BYTE alphaBase, BYTE alphaDelta);
DLL_EXPORT bool EGGetIconDialogue(HWND hwnd, WCHAR *iconPath, int iconIndex);
DLL_EXPORT void EGFontToString(const LOGFONT& font, WCHAR *str);
DLL_EXPORT void EGStringToFont(const WCHAR *str, LOGFONT& font);

#endif
