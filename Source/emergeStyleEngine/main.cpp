// vim:tags+=../emergeLib/tags
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

#undef _WIN32_IE
#define _WIN32_IE 0x0501

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#undef WINVER
#define WINVER 0x0501

#include "emergeStyleEngine.h"
//#include "BImage.h"
#include "../emergeLib/emergeLib.h"
#include "StyleEditor.h"

// Globals
WCHAR style[MAX_PATH];
HBRUSH bgBrush = NULL;

// Globals
StyleEditor *pStyleEditor;

//----  --------------------------------------------------------------------------------------------------------
// Function:    ESEGetStyle
// Requires:    Nothing
// Returns:     WCHAR*
// Purpose:     Retrieves the current style
//----  --------------------------------------------------------------------------------------------------------
WCHAR *ESEGetStyle()
{
  return style;
}

void ESESetStyle(WCHAR *styleFile)
{
  wcscpy(style, styleFile);
}

bool ESEEqualStyle(LPGUIINFO sourceInfo, LPGUIINFO targetInfo)
{
  if (sourceInfo->alphaActive != targetInfo->alphaActive)
    return false;

  if (sourceInfo->alphaInactive != targetInfo->alphaInactive)
    return false;

  if (sourceInfo->alphaBackground != targetInfo->alphaBackground)
    return false;

  if (sourceInfo->alphaMenu != targetInfo->alphaMenu)
    return false;

  if (sourceInfo->alphaForeground != targetInfo->alphaForeground)
    return false;

  if (sourceInfo->alphaFrame != targetInfo->alphaFrame)
    return false;

  if (sourceInfo->alphaSelected != targetInfo->alphaSelected)
    return false;

  if (sourceInfo->alphaBorder != targetInfo->alphaBorder)
    return false;

  if (sourceInfo->alphaText != targetInfo->alphaText)
    return false;

  if (sourceInfo->colorBackground != targetInfo->colorBackground)
    return false;

  if (sourceInfo->colorSelected != targetInfo->colorSelected)
    return false;

  if (sourceInfo->colorForeground != targetInfo->colorForeground)
    return false;

  if (sourceInfo->colorFrame != targetInfo->colorFrame)
    return false;

  if (sourceInfo->colorFont != targetInfo->colorFont)
    return false;

  if (sourceInfo->colorBorder != targetInfo->colorBorder)
    return false;

  if (sourceInfo->windowShadow != targetInfo->windowShadow)
    return false;

  if (sourceInfo->dragBorder != targetInfo->dragBorder)
    return false;

  if (sourceInfo->bevelWidth != targetInfo->bevelWidth)
    return false;

  if (sourceInfo->padding != targetInfo->padding)
    return false;

  if (sourceInfo->gradientFrom != targetInfo->gradientFrom)
    return false;

  if (sourceInfo->gradientTo != targetInfo->gradientTo)
    return false;

  if (_wcsicmp(sourceInfo->gradientMethod, targetInfo->gradientMethod) != 0)
    return false;

  if (sourceInfo->windowBlur != targetInfo->windowBlur)
    return false;

  return true;
}

bool ESEWriteStyle(WCHAR *styleFile, LPGUIINFO guiInfo, HWND hwnd)
{
  GUIINFO origGuiInfo;
  std::wstring workingStyle = styleFile;
  std::wstring theme = ELGetThemeName();
  workingStyle = ELExpandVars(workingStyle);

  ESEReadStyle(styleFile, &origGuiInfo);
  if (ESEEqualStyle(&origGuiInfo, guiInfo))
    return true;

  if (workingStyle.empty())
    {
      ELMessageBox(hwnd, L"Style name cannot be empty", L"Style Editor", ELMB_MODAL|ELMB_ICONERROR|ELMB_OK);
      return false;
    }

  if (!ELIsModifiedTheme(theme))
    {
      std::wstring errorMessage;

      if (ELSetModifiedTheme(theme))
        {
          std::wstring oldThemePath = TEXT("%EmergeDir%\\themes\\") + theme;
          oldThemePath += TEXT("\\*");
          std::wstring newThemePath = TEXT("%ThemeDir%");

          if (!ELPathIsDirectory(newThemePath.c_str()))
            ELCreateDirectory(newThemePath);

          if (!ELFileOp(hwnd, FO_COPY, oldThemePath, newThemePath))
            {
              errorMessage = L"Cannot create \'";
              errorMessage += newThemePath;
              errorMessage += L"\'.";
              ELMessageBox(hwnd, errorMessage.c_str(), L"Style Editor", ELMB_MODAL|ELMB_ICONERROR|ELMB_OK);
              return false;
            }

          workingStyle = ESEGetStyle();
          workingStyle = ELExpandVars(workingStyle);
        }
      else
        {
          errorMessage = L"Failed to create modified theme.";
          ELMessageBox(hwnd, errorMessage.c_str(), L"Style Editor", ELMB_MODAL|ELMB_ICONERROR|ELMB_OK);
          return false;
        }
    }

  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.active:"), (int)guiInfo->alphaActive);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.inactive:"), (int)guiInfo->alphaInactive);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.background:"), (int)guiInfo->alphaBackground);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.menu:"), (int)guiInfo->alphaMenu);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.foreground:"), (int)guiInfo->alphaForeground);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.frame:"), (int)guiInfo->alphaFrame);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.selected:"), (int)guiInfo->alphaSelected);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.border:"), (int)guiInfo->alphaBorder);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("alpha.text:"), (int)guiInfo->alphaText);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.background:"), guiInfo->colorBackground);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.selected:"), guiInfo->colorSelected);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.foreground:"), guiInfo->colorForeground);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.frame:"), guiInfo->colorFrame);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.font:"), guiInfo->colorFont);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.border:"), guiInfo->colorBorder);
  ELWriteFileBool(workingStyle.c_str(), (WCHAR*)TEXT("window.shadow:"), guiInfo->windowShadow);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("window.dragborder:"), (int)guiInfo->dragBorder);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("window.bevelWidth:"), (int)guiInfo->bevelWidth);
  ELWriteFileInt(workingStyle.c_str(), (WCHAR*)TEXT("window.padding:"), (int)guiInfo->padding);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("gradient.colorFrom:"), guiInfo->gradientFrom);
  ELWriteFileColor(workingStyle.c_str(), (WCHAR*)TEXT("gradient.colorTo:"), guiInfo->gradientTo);
  ELWriteFileString(workingStyle.c_str(), (WCHAR*)TEXT("gradient.method:"), guiInfo->gradientMethod);
  ELWriteFileBool(workingStyle.c_str(), (WCHAR*)TEXT("window.blur:"), guiInfo->windowBlur);

  return true;
}

void ESEReadStyle(WCHAR *styleFile, LPGUIINFO guiInfo)
{
  std::wstring workingStyle = styleFile;
  workingStyle = ELExpandVars(workingStyle);

  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.active:"), &guiInfo->alphaActive, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.inactive:"), &guiInfo->alphaInactive, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.background:"), &guiInfo->alphaBackground, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.menu:"), &guiInfo->alphaMenu, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.foreground:"), &guiInfo->alphaForeground, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.frame:"), &guiInfo->alphaFrame, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.selected:"), &guiInfo->alphaSelected, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.border:"), &guiInfo->alphaBorder, 100);
  ELReadFileByte(workingStyle.c_str(), (WCHAR*)TEXT("alpha.text:"), &guiInfo->alphaText, 100);
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.background:"), &guiInfo->colorBackground,
                  RGB(212,208,200));
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.selected:"), &guiInfo->colorSelected,
                  RGB(10,36,106));
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.foreground:"), &guiInfo->colorForeground,
                  RGB(255,255,255));
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.frame:"), &guiInfo->colorFrame,
                  RGB(10,36,106));
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.font:"), &guiInfo->colorFont,
                  RGB(255,255,255));
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("color.border:"), &guiInfo->colorBorder, RGB(0, 0, 0));
  ELReadFileBool(workingStyle.c_str(), (WCHAR*)TEXT("window.shadow:"), &guiInfo->windowShadow, false);
  ELReadFileInt(workingStyle.c_str(), (WCHAR*)TEXT("window.dragborder:"), &guiInfo->dragBorder, 0);
  ELReadFileInt(workingStyle.c_str(), (WCHAR*)TEXT("window.bevelWidth:"), &guiInfo->bevelWidth, 0);
  ELReadFileInt(workingStyle.c_str(), (WCHAR*)TEXT("window.padding:"), &guiInfo->padding, 4);
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("gradient.colorFrom:"), &guiInfo->gradientFrom,
                  RGB(255,255,255));
  ELReadFileColor(workingStyle.c_str(), (WCHAR*)TEXT("gradient.colorTo:"), &guiInfo->gradientTo,
                  RGB(0,0,0));
  ELReadFileString(workingStyle.c_str(), (WCHAR*)TEXT("gradient.method:"), guiInfo->gradientMethod, (WCHAR*)TEXT("VerticalFlat"));
  ELReadFileBool(workingStyle.c_str(), (WCHAR*)TEXT("window.blur:"), &guiInfo->windowBlur, false);
}

void ESELoadStyle(WCHAR *styleFile, LPGUIINFO guiInfo)
{
  int tmpAlpha;

  ESESetStyle(styleFile);
  ESEReadStyle(style, guiInfo);

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
