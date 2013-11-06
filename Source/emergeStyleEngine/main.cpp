// vim:tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

#include "main.h"

//----  --------------------------------------------------------------------------------------------------------
// Function:    ESEGetStyle
// Requires:    Nothing
// Returns:     std::wstring
// Purpose:     Retrieves the current style
//----  --------------------------------------------------------------------------------------------------------
std::wstring ESEGetStyle()
{
  return style;
}

void ESESetStyle(std::wstring styleFile)
{
  style = styleFile;
}

bool ESEEqualStyle(LPGUIINFO sourceInfo, LPGUIINFO targetInfo)
{
  if (sourceInfo->alphaActive != targetInfo->alphaActive)
  {
    return false;
  }

  if (sourceInfo->alphaInactive != targetInfo->alphaInactive)
  {
    return false;
  }

  if (sourceInfo->alphaBackground != targetInfo->alphaBackground)
  {
    return false;
  }

  if (sourceInfo->alphaMenu != targetInfo->alphaMenu)
  {
    return false;
  }

  if (sourceInfo->alphaForeground != targetInfo->alphaForeground)
  {
    return false;
  }

  if (sourceInfo->alphaFrame != targetInfo->alphaFrame)
  {
    return false;
  }

  if (sourceInfo->alphaSelected != targetInfo->alphaSelected)
  {
    return false;
  }

  if (sourceInfo->alphaBorder != targetInfo->alphaBorder)
  {
    return false;
  }

  if (sourceInfo->alphaText != targetInfo->alphaText)
  {
    return false;
  }

  if (sourceInfo->colorBackground != targetInfo->colorBackground)
  {
    return false;
  }

  if (sourceInfo->colorSelected != targetInfo->colorSelected)
  {
    return false;
  }

  if (sourceInfo->colorForeground != targetInfo->colorForeground)
  {
    return false;
  }

  if (sourceInfo->colorFrame != targetInfo->colorFrame)
  {
    return false;
  }

  if (sourceInfo->colorFont != targetInfo->colorFont)
  {
    return false;
  }

  if (sourceInfo->colorBorder != targetInfo->colorBorder)
  {
    return false;
  }

  if (sourceInfo->windowShadow != targetInfo->windowShadow)
  {
    return false;
  }

  if (sourceInfo->dragBorder != targetInfo->dragBorder)
  {
    return false;
  }

  if (sourceInfo->bevelWidth != targetInfo->bevelWidth)
  {
    return false;
  }

  if (sourceInfo->padding != targetInfo->padding)
  {
    return false;
  }

  if (sourceInfo->gradientFrom != targetInfo->gradientFrom)
  {
    return false;
  }

  if (sourceInfo->gradientTo != targetInfo->gradientTo)
  {
    return false;
  }

  if (ELToLower(sourceInfo->gradientMethod) != ELToLower(targetInfo->gradientMethod))
  {
    return false;
  }

  if (sourceInfo->windowBlur != targetInfo->windowBlur)
  {
    return false;
  }

  return true;
}

bool ESEWriteStyle(std::wstring styleFile, LPGUIINFO guiInfo, HWND hwnd)
{
  GUIINFO origGuiInfo;
  std::wstring workingStyle = styleFile;
  std::wstring theme = ELGetThemeName();
  workingStyle = ELExpandVars(workingStyle);

  ESEReadStyle(styleFile, &origGuiInfo);
  if (ESEEqualStyle(&origGuiInfo, guiInfo))
  {
    return true;
  }

  if (workingStyle.empty())
  {
    ELMessageBox(hwnd, TEXT("Style name cannot be empty"), TEXT("Style Editor"), ELMB_MODAL | ELMB_ICONERROR | ELMB_OK);
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

      if ((ELGetFileSpecialFlags(newThemePath) & SF_DIRECTORY) != SF_DIRECTORY)
      {
        ELCreateDirectory(newThemePath);
      }

      if (!ELFileOp(hwnd, false, FO_COPY, oldThemePath, newThemePath))
      {
        errorMessage = TEXT("Cannot create \'");
        errorMessage += newThemePath;
        errorMessage += TEXT("\'.");
        ELMessageBox(hwnd, errorMessage, TEXT("Style Editor"), ELMB_MODAL | ELMB_ICONERROR | ELMB_OK);
        return false;
      }

      workingStyle = ESEGetStyle();
      workingStyle = ELExpandVars(workingStyle);
    }
    else
    {
      errorMessage = TEXT("Failed to create modified theme.");
      ELMessageBox(hwnd, errorMessage, TEXT("Style Editor"), ELMB_MODAL | ELMB_ICONERROR | ELMB_OK);
      return false;
    }
  }

  ELWriteFileInt(workingStyle, TEXT("alpha.active:"), (int)guiInfo->alphaActive);
  ELWriteFileInt(workingStyle, TEXT("alpha.inactive:"), (int)guiInfo->alphaInactive);
  ELWriteFileInt(workingStyle, TEXT("alpha.background:"), (int)guiInfo->alphaBackground);
  ELWriteFileInt(workingStyle, TEXT("alpha.menu:"), (int)guiInfo->alphaMenu);
  ELWriteFileInt(workingStyle, TEXT("alpha.foreground:"), (int)guiInfo->alphaForeground);
  ELWriteFileInt(workingStyle, TEXT("alpha.frame:"), (int)guiInfo->alphaFrame);
  ELWriteFileInt(workingStyle, TEXT("alpha.selected:"), (int)guiInfo->alphaSelected);
  ELWriteFileInt(workingStyle, TEXT("alpha.border:"), (int)guiInfo->alphaBorder);
  ELWriteFileInt(workingStyle, TEXT("alpha.text:"), (int)guiInfo->alphaText);
  ELWriteFileColor(workingStyle, TEXT("color.background:"), guiInfo->colorBackground);
  ELWriteFileColor(workingStyle, TEXT("color.selected:"), guiInfo->colorSelected);
  ELWriteFileColor(workingStyle, TEXT("color.foreground:"), guiInfo->colorForeground);
  ELWriteFileColor(workingStyle, TEXT("color.frame:"), guiInfo->colorFrame);
  ELWriteFileColor(workingStyle, TEXT("color.font:"), guiInfo->colorFont);
  ELWriteFileColor(workingStyle, TEXT("color.border:"), guiInfo->colorBorder);
  ELWriteFileBool(workingStyle, TEXT("window.shadow:"), guiInfo->windowShadow);
  ELWriteFileInt(workingStyle, TEXT("window.dragborder:"), (int)guiInfo->dragBorder);
  ELWriteFileInt(workingStyle, TEXT("window.bevelWidth:"), (int)guiInfo->bevelWidth);
  ELWriteFileInt(workingStyle, TEXT("window.padding:"), (int)guiInfo->padding);
  ELWriteFileColor(workingStyle, TEXT("gradient.colorFrom:"), guiInfo->gradientFrom);
  ELWriteFileColor(workingStyle, TEXT("gradient.colorTo:"), guiInfo->gradientTo);
  ELWriteFileString(workingStyle, TEXT("gradient.method:"), guiInfo->gradientMethod);
  ELWriteFileBool(workingStyle, TEXT("window.blur:"), guiInfo->windowBlur);

  return true;
}

void ESEReadStyle(std::wstring styleFile, LPGUIINFO guiInfo)
{
  std::wstring workingStyle = styleFile;
  workingStyle = ELExpandVars(workingStyle);

  guiInfo->alphaActive = ELReadFileByte(workingStyle, TEXT("alpha.active:"), 100);
  guiInfo->alphaInactive = ELReadFileByte(workingStyle, TEXT("alpha.inactive:"), 100);
  guiInfo->alphaBackground = ELReadFileByte(workingStyle, TEXT("alpha.background:"), 100);
  guiInfo->alphaMenu = ELReadFileByte(workingStyle, TEXT("alpha.menu:"), 100);
  guiInfo->alphaForeground = ELReadFileByte(workingStyle, TEXT("alpha.foreground:"), 100);
  guiInfo->alphaFrame = ELReadFileByte(workingStyle, TEXT("alpha.frame:"), 100);
  guiInfo->alphaSelected = ELReadFileByte(workingStyle, TEXT("alpha.selected:"), 100);
  guiInfo->alphaBorder = ELReadFileByte(workingStyle, TEXT("alpha.border:"), 100);
  guiInfo->alphaText = ELReadFileByte(workingStyle, TEXT("alpha.text:"), 100);
  guiInfo->colorBackground = ELReadFileColor(workingStyle, TEXT("color.background:"), RGB(212, 208, 200));
  guiInfo->colorSelected = ELReadFileColor(workingStyle, TEXT("color.selected:"), RGB(10, 36, 106));
  guiInfo->colorForeground = ELReadFileColor(workingStyle, TEXT("color.foreground:"), RGB(255, 255, 255));
  guiInfo->colorFrame = ELReadFileColor(workingStyle, TEXT("color.frame:"), RGB(10, 36, 106));
  guiInfo->colorFont = ELReadFileColor(workingStyle, TEXT("color.font:"), RGB(255, 255, 255));
  guiInfo->colorBorder = ELReadFileColor(workingStyle, TEXT("color.border:"), RGB(0, 0, 0));
  guiInfo->windowShadow = ELReadFileBool(workingStyle, TEXT("window.shadow:"), false);
  guiInfo->dragBorder = ELReadFileInt(workingStyle, TEXT("window.dragborder:"), 0);
  guiInfo->bevelWidth = ELReadFileInt(workingStyle, TEXT("window.bevelWidth:"), 0);
  guiInfo->padding = ELReadFileInt(workingStyle, TEXT("window.padding:"), 4);
  guiInfo->gradientFrom = ELReadFileColor(workingStyle, TEXT("gradient.colorFrom:"), RGB(255, 255, 255));
  guiInfo->gradientTo = ELReadFileColor(workingStyle, TEXT("gradient.colorTo:"), RGB(0, 0, 0));
  guiInfo->gradientMethod = ELReadFileString(workingStyle, TEXT("gradient.method:"), TEXT("VerticalFlat"));
  guiInfo->windowBlur = ELReadFileBool(workingStyle, TEXT("window.blur:"), false);
}

void ESELoadStyle(std::wstring styleFile, LPGUIINFO guiInfo)
{
  int tmpAlpha;

  ESESetStyle(styleFile);
  ESEReadStyle(style, guiInfo);

  if (guiInfo->alphaMenu < 20)
  {
    guiInfo->alphaMenu = 20;
  }

  tmpAlpha = guiInfo->alphaMenu;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaMenu = tmpAlpha;

  if (guiInfo->alphaActive < 20)
  {
    guiInfo->alphaActive = 20;
  }

  tmpAlpha = guiInfo->alphaActive;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaActive = tmpAlpha;

  if (guiInfo->alphaInactive <= 0)
  {
    guiInfo->alphaInactive = 1;
  }

  tmpAlpha = guiInfo->alphaInactive;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaInactive = tmpAlpha;

  if (guiInfo->alphaBackground <= 0)
  {
    guiInfo->alphaBackground = 1;
  }

  tmpAlpha = guiInfo->alphaBackground;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaBackground = tmpAlpha;

  if (guiInfo->alphaForeground <= 0)
  {
    guiInfo->alphaForeground = 1;
  }

  tmpAlpha = guiInfo->alphaForeground;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaForeground = tmpAlpha;

  if (guiInfo->alphaSelected <= 0)
  {
    guiInfo->alphaSelected = 1;
  }

  tmpAlpha = guiInfo->alphaSelected;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaSelected = tmpAlpha;

  if (guiInfo->alphaFrame <= 0)
  {
    guiInfo->alphaFrame = 1;
  }

  tmpAlpha = guiInfo->alphaFrame;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaFrame = tmpAlpha;

  if (guiInfo->alphaBorder <= 0)
  {
    guiInfo->alphaBorder = 1;
  }

  tmpAlpha = guiInfo->alphaBorder;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaBorder = tmpAlpha;

  if (guiInfo->alphaText < 20)
  {
    guiInfo->alphaText = 20;
  }

  tmpAlpha = guiInfo->alphaText;
  tmpAlpha *= 255;
  tmpAlpha /= 100;
  guiInfo->alphaText = tmpAlpha;
}

void ESEPaintBackground(HDC hdc, RECT clientRect, LPGUIINFO guiInfo, bool active)
{
  std::wstring lower;
  BYTE alpha;
  COLORREF colourFrom = guiInfo->gradientFrom, colourTo = guiInfo->gradientTo;

  if (guiInfo->dragBorder > 0)
  {
    EGFrameRect(hdc, &clientRect, guiInfo->alphaBorder, guiInfo->colorBorder, guiInfo->dragBorder);
    InflateRect(&clientRect, 0 - guiInfo->dragBorder, 0 - guiInfo->dragBorder);
  }

  if (active)
  {
    alpha = EGGetMinAlpha(guiInfo->alphaBackground, guiInfo->alphaActive);
  }
  else
  {
    alpha = EGGetMinAlpha(guiInfo->alphaBackground, guiInfo->alphaInactive);
  }

  lower = ELToLower(guiInfo->gradientMethod);
  if (lower.find(TEXT("solid")) != std::wstring::npos)
  {
    colourFrom = guiInfo->colorBackground;
    colourTo = guiInfo->colorBackground;
  }

  EGGradientFillRect(hdc, &clientRect, alpha, colourFrom, colourTo, guiInfo->bevelWidth,
                     guiInfo->gradientMethod);
}
