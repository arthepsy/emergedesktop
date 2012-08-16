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

#ifndef DWM_BB_ENABLE
#define DWM_BB_ENABLE 0x00000001
#endif

#ifndef DWM_BB_BLURREGION
#define DWM_BB_BLURREGION 0x00000002
#endif

#ifndef DWM_BB_TRANSITIONONMAXIMIZED
#define DWM_BB_TRANSITIONONMAXIMIZED 0x00000004
#endif

#ifndef DWM_TNP_RECTDESTINATION
#define DWM_TNP_RECTDESTINATION 0x00000001
#endif

#ifndef DWM_TNP_RECTSOURCE
#define DWM_TNP_RECTSOURCE 0x00000002
#endif

#ifndef DWM_TNP_OPACITY
#define DWM_TNP_OPACITY 0x00000004
#endif

#ifndef DWM_TNP_VISIBLE
#define DWM_TNP_VISIBLE 0x00000008
#endif

#ifndef DWM_TNP_SOURCECLIENTAREAONLY
#define DWM_TNP_SOURCECLIENTAREAONLY 0x00000010
#endif

#define ICON_DEFAULT        0
#define ICON_RUN            4
#define ICON_SHUTDOWN       5
#define ICON_LOGOFF         7
#define ICON_QUIT           9
#define ICON_LOCK           10
#define ICON_EMERGE         11
#define ICON_QUESTION       12

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

typedef HANDLE HTHUMBNAIL;
typedef HTHUMBNAIL* PHTHUMBNAIL;


typedef struct _MARGINS
{
  int cxLeftWidth;
  int cxRightWidth;
  int cyTopHeight;
  int cyBottomHeight;
}
MARGINS;

typedef struct _DWM_BLURBEHIND
{
  DWORD dwFlags;
  BOOL  fEnable;
  HRGN  hRgnBlur;
  BOOL  fTransitionOnMaximized;
}
DWM_BLURBEHIND, *PDWM_BLURBEHIND;

typedef struct _DWM_THUMBNAIL_PROPERTIES
{
  DWORD dwFlags;
  RECT  rcDestination;
  RECT  rcSource;
  BYTE  opacity;
  BOOL  fVisible;
  BOOL  fSourceClientAreaOnly;
}
DWM_THUMBNAIL_PROPERTIES, *PDWM_THUMBNAIL_PROPERTIES;

// Format Info
typedef struct _FORMATINFO
{
  UINT horizontalAlignment;
  HFONT font;
  COLORREF color;
  UINT verticalAlignment;
  UINT flags;
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
  bool windowBlur;
}
GUIINFO, *LPGUIINFO;

static const UINT TASK_ICON = RegisterWindowMessage(TEXT("TaskIcon"));

// Declaration of functions to import
DLL_EXPORT HICON EGConvertIcon(HICON sourceIcon, BYTE foregroundAlpha);
DLL_EXPORT HBRUSH EGCreateBrush(BYTE alpha, COLORREF colour);
DLL_EXPORT HPEN EGCreatePen(DWORD style, DWORD width, BYTE alpha, COLORREF colour);
DLL_EXPORT HBITMAP EGCreateBitmap(BYTE alpha, COLORREF colour, RECT wndRect);
DLL_EXPORT HICON EGGetFileIcon(const WCHAR *file, UINT iconSize);
DLL_EXPORT HICON EGGetWindowIcon(HWND callerWnd, HWND hwnd, bool smallIcon, bool force);
DLL_EXPORT HICON EGGetSpecialFolderIcon(int csidl, UINT iconSize);
DLL_EXPORT HICON EGGetSystemIcon(UINT iconIndex, UINT iconSize);
DLL_EXPORT HICON EGExtractIcon(const WCHAR *iconLocation, int iconIndex, int iconSize);
DLL_EXPORT bool EGDrawAlphaText(BYTE alpha, CLIENTINFO clientInfo, FORMATINFO formatInfo, WCHAR *commandText);
DLL_EXPORT bool EGFillRect(HDC hdc, RECT *rect, BYTE alpha, COLORREF colour);
DLL_EXPORT bool EGGradientFillRect(HDC hdc, RECT *rect, BYTE alpha, COLORREF colourFrom, COLORREF colourTo, int bevelWidth, WCHAR *gradientMethod);
DLL_EXPORT bool EGFrameRect(HDC hdc, RECT *rect, BYTE bgAlpha, COLORREF borderColour, int borderWidth);
DLL_EXPORT UINT32 EGGetPixel(BYTE alpha, COLORREF colour);
DLL_EXPORT BYTE EGGetMinAlpha(BYTE alphaBase, BYTE alphaDelta);
DLL_EXPORT bool EGGetIconDialogue(HWND hwnd, WCHAR *iconPath, int iconIndex);
DLL_EXPORT void EGFontToString(const LOGFONT& font, WCHAR *str);
DLL_EXPORT void EGStringToFont(const WCHAR *str, LOGFONT& font);
DLL_EXPORT bool EGEqualLogFont(const LOGFONT& source, const LOGFONT& target);
DLL_EXPORT bool EGGetTextRect(WCHAR *text, HFONT font, RECT *rect, UINT flags);
DLL_EXPORT HBITMAP EGGetIconBitmap(HICON sourceIcon);
DLL_EXPORT BOOL EGIsCompositionEnabled();
DLL_EXPORT HRESULT EGBlurWindow(HWND hwnd, bool enable);
DLL_EXPORT HRESULT EGDwmRegisterThumbnail(HWND hwndDestination, HWND hwndSource, PHTHUMBNAIL phThumbnailId);
DLL_EXPORT HRESULT EGDwmUpdateThumbnailProperties(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES *ptnProperties);
DLL_EXPORT HRESULT EGDwmUnregisterThumbnail(HTHUMBNAIL hThumbnailId);
DLL_EXPORT HRESULT EGDwmQueryThumbnailSourceSize(HTHUMBNAIL hThumbnailId, PSIZE pSize);
#endif
