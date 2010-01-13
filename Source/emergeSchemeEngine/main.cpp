// vim:tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
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
//----  --------------------------------------------------------------------------------------------------------

#undef _WIN32_IE
#define _WIN32_IE 0x0501

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#undef WINVER
#define WINVER 0x0501

#include "emergeSchemeEngine.h"
//#include "BImage.h"
#include "../emergeLib/emergeLib.h"
#include "SchemeEditor.h"

// Globals
WCHAR scheme[MAX_PATH];
HBRUSH bgBrush = NULL;

// Globals
SchemeEditor *pSchemeEditor;

//----  --------------------------------------------------------------------------------------------------------
// Function:    ESEGetScheme
// Requires:    Nothing
// Returns:     WCHAR*
// Purpose:     Retrieves the current scheme
//----  --------------------------------------------------------------------------------------------------------
WCHAR *ESEGetScheme()
{
  return scheme;
}

void ESESetScheme(WCHAR *schemeFile)
{
  wcscpy(scheme, schemeFile);
}

bool ESEWriteScheme(WCHAR *schemeFile, LPGUIINFO guiInfo, HWND hwnd)
{
  std::wstring workingScheme = schemeFile;
  workingScheme = ELExpandVars(workingScheme);

  if (workingScheme.empty())
    {
      ELMessageBox(hwnd, L"Scheme name cannot be empty", L"Scheme Editor", ELMB_MODAL|ELMB_ICONERROR|ELMB_OK);
      return false;
    }

  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.active:"), (int)guiInfo->alphaActive);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.inactive:"), (int)guiInfo->alphaInactive);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.background:"), (int)guiInfo->alphaBackground);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.menu:"), (int)guiInfo->alphaMenu);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.foreground:"), (int)guiInfo->alphaForeground);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.frame:"), (int)guiInfo->alphaFrame);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.selected:"), (int)guiInfo->alphaSelected);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.border:"), (int)guiInfo->alphaBorder);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("alpha.text:"), (int)guiInfo->alphaText);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.background:"), guiInfo->colorBackground);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.selected:"), guiInfo->colorSelected);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.foreground:"), guiInfo->colorForeground);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.frame:"), guiInfo->colorFrame);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.font:"), guiInfo->colorFont);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.border:"), guiInfo->colorBorder);
  ELWriteFileBool(workingScheme.c_str(), (WCHAR*)TEXT("window.shadow:"), guiInfo->windowShadow);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("window.dragborder:"), (int)guiInfo->dragBorder);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("window.bevelWidth:"), (int)guiInfo->bevelWidth);
  ELWriteFileInt(workingScheme.c_str(), (WCHAR*)TEXT("window.padding:"), (int)guiInfo->padding);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("gradient.colorFrom:"), guiInfo->gradientFrom);
  ELWriteFileColor(workingScheme.c_str(), (WCHAR*)TEXT("gradient.colorTo:"), guiInfo->gradientTo);
  ELWriteFileString(workingScheme.c_str(), (WCHAR*)TEXT("gradient.method:"), guiInfo->gradientMethod);

  return true;
}

void ESEReadScheme(WCHAR *schemeFile, LPGUIINFO guiInfo)
{
  std::wstring workingScheme = schemeFile;
  workingScheme = ELExpandVars(workingScheme);

  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.active:"), &guiInfo->alphaActive, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.inactive:"), &guiInfo->alphaInactive, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.background:"), &guiInfo->alphaBackground, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.menu:"), &guiInfo->alphaMenu, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.foreground:"), &guiInfo->alphaForeground, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.frame:"), &guiInfo->alphaFrame, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.selected:"), &guiInfo->alphaSelected, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.border:"), &guiInfo->alphaBorder, 100);
  ELReadFileByte(workingScheme.c_str(), (WCHAR*)TEXT("alpha.text:"), &guiInfo->alphaText, 100);
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.background:"), &guiInfo->colorBackground,
                  GetSysColor(COLOR_BTNFACE));
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.selected:"), &guiInfo->colorSelected,
                  GetSysColor(COLOR_HIGHLIGHT));
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.foreground:"), &guiInfo->colorForeground,
                  GetSysColor(COLOR_WINDOW));
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.frame:"), &guiInfo->colorFrame,
                  GetSysColor(COLOR_ACTIVECAPTION));
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.font:"), &guiInfo->colorFont,
                  GetSysColor(COLOR_BTNTEXT));
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("color.border:"), &guiInfo->colorBorder, RGB(0, 0, 0));
  ELReadFileBool(workingScheme.c_str(), (WCHAR*)TEXT("window.shadow:"), &guiInfo->windowShadow, true);
  ELReadFileInt(workingScheme.c_str(), (WCHAR*)TEXT("window.dragborder:"), &guiInfo->dragBorder, 1);
  ELReadFileInt(workingScheme.c_str(), (WCHAR*)TEXT("window.bevelWidth:"), &guiInfo->bevelWidth, 1);
  ELReadFileInt(workingScheme.c_str(), (WCHAR*)TEXT("window.padding:"), &guiInfo->padding, 0);
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("gradient.colorFrom:"), &guiInfo->gradientFrom,
                  GetSysColor(COLOR_ACTIVECAPTION));
  ELReadFileColor(workingScheme.c_str(), (WCHAR*)TEXT("gradient.colorTo:"), &guiInfo->gradientTo,
                  GetSysColor(COLOR_GRADIENTACTIVECAPTION));
  ELReadFileString(workingScheme.c_str(), (WCHAR*)TEXT("gradient.method:"), guiInfo->gradientMethod, (WCHAR*)TEXT("Solid"));
}

void ESELoadScheme(WCHAR *schemeFile, LPGUIINFO guiInfo)
{
  int tmpAlpha;

  ESESetScheme(schemeFile);
  ESEReadScheme(scheme, guiInfo);

  if (guiInfo->alphaMenu < 20)
    guiInfo->alphaMenu = 20;

  tmpAlpha = guiInfo->alphaMenu;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaMenu = tmpAlpha;

  if (guiInfo->alphaActive < 20)
    guiInfo->alphaActive = 20;

  tmpAlpha = guiInfo->alphaActive;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaActive = tmpAlpha;

  if (guiInfo->alphaInactive <= 0)
    guiInfo->alphaInactive = 1;

  tmpAlpha = guiInfo->alphaInactive;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaInactive = tmpAlpha;

  if (guiInfo->alphaBackground <= 0)
    guiInfo->alphaBackground = 1;

  tmpAlpha = guiInfo->alphaBackground;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaBackground = tmpAlpha;

  if (guiInfo->alphaForeground <= 0)
    guiInfo->alphaForeground = 1;

  tmpAlpha = guiInfo->alphaForeground;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaForeground = tmpAlpha;

  if (guiInfo->alphaSelected <= 0)
    guiInfo->alphaSelected = 1;

  tmpAlpha = guiInfo->alphaSelected;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaSelected = tmpAlpha;

  if (guiInfo->alphaFrame <= 0)
    guiInfo->alphaFrame = 1;

  tmpAlpha = guiInfo->alphaFrame;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaFrame = tmpAlpha;

  if (guiInfo->alphaBorder <= 0)
    guiInfo->alphaBorder = 1;

  tmpAlpha = guiInfo->alphaBorder;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaBorder = tmpAlpha;

  if (guiInfo->alphaText < 20)
    guiInfo->alphaText = 20;

  tmpAlpha = guiInfo->alphaText;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaText = tmpAlpha;
}

HDC ESEPaintBackground(RECT clientRect, LPGUIINFO guiInfo, bool active)
{
  int width = clientRect.right - clientRect.left;
  int height = clientRect.bottom - clientRect.top;
  VOID *backgroundBits;
  WCHAR *lower;
  BITMAPINFO bmi;
  BYTE alpha;
  COLORREF colourFrom = guiInfo->gradientFrom, colourTo = guiInfo->gradientTo;

  HDC hdc = CreateCompatibleDC(NULL);
  ZeroMemory(&bmi, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = width * height * 4;

  HBITMAP backgroundBMP = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &backgroundBits, NULL, 0x0);
  SelectObject (hdc, backgroundBMP);

  if (guiInfo->dragBorder > 0)
    {
      EGFrameRect(hdc, &clientRect, guiInfo->alphaBorder, guiInfo->colorBorder, guiInfo->dragBorder);
      InflateRect(&clientRect, 0 - guiInfo->dragBorder, 0 - guiInfo->dragBorder);
    }

  if (active)
    alpha = EGGetMinAlpha(guiInfo->alphaBackground, guiInfo->alphaActive);
  else
    alpha = EGGetMinAlpha(guiInfo->alphaBackground, guiInfo->alphaInactive);

  lower = _wcslwr(_wcsdup(guiInfo->gradientMethod));
  if (wcsstr(lower, TEXT("solid")) != NULL)
    {
      colourFrom = guiInfo->colorBackground;
      colourTo = guiInfo->colorBackground;
    }
  free(lower);

  EGGradientFillRect(hdc, &clientRect, alpha, colourFrom, colourTo, guiInfo->bevelWidth,
                     guiInfo->gradientMethod);

  DeleteObject(backgroundBMP);

  return hdc;
}
