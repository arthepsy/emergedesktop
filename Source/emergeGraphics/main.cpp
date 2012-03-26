// vim: tags+=../emergeLib/tags
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

#include "emergeGraphics.h"
#include "resource.h"
#include "BImage.h"
#include "../emergeLib/emergeLib.h"
#include <objbase.h>
#include <stdio.h>
#include <shlwapi.h>

#ifndef GIL_DEFAULTICON
#define GIL_DEFAULTICON 64
#endif

#define ICON_LOOKUP_TIMEOUT 1000

typedef UINT (WINAPI *fnPrivateExtractIcons)(LPCTSTR, int, int, int, HICON*, UINT*, UINT, UINT);
static fnPrivateExtractIcons MSPrivateExtractIcons = NULL;

typedef UINT (WINAPI *fnPickIcon)(HWND, WCHAR*, UINT, int*);
static fnPickIcon MSPickIcon = NULL;

typedef HRESULT (WINAPI *fnDwmIsCompositionEnabled)(BOOL *);
static fnDwmIsCompositionEnabled MSDwmIsCompositionEnabled = NULL;

typedef HRESULT (WINAPI *fnDwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND *);
static fnDwmEnableBlurBehindWindow MSDwmEnableBlurBehindWindow = NULL;

// Globals
HDC hdc = NULL;
HBITMAP hbitmap = NULL;
static HMODULE dwmapiDLL = NULL;
static HMODULE shell32DLL = NULL;
static HMODULE user32DLL = NULL;

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL UNUSED, DWORD fdwReason, LPVOID lpvReserved UNUSED)
{
  switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
      if (dwmapiDLL == NULL)
        dwmapiDLL = ELLoadSystemLibrary(TEXT("dwmapi.dll"));
      if (shell32DLL == NULL)
        shell32DLL = ELLoadSystemLibrary(TEXT("shell32.dll"));
      if (user32DLL == NULL)
        user32DLL = ELLoadSystemLibrary(TEXT("user32.dll"));
      break;
    case DLL_PROCESS_DETACH:
      if (dwmapiDLL != NULL)
        {
          FreeLibrary(dwmapiDLL);
          dwmapiDLL = NULL;
        }
      if (shell32DLL != NULL)
        {
          FreeLibrary(shell32DLL);
          shell32DLL = NULL;
        }
      if (user32DLL != NULL)
        {
          FreeLibrary(user32DLL);
          user32DLL = NULL;
        }
      break;
    }

  return TRUE;
}

BYTE EGGetMinAlpha(BYTE alphaBase, BYTE alphaDelta)
{
  BYTE alphaMin = alphaBase;
  float srcAlphaCheck, checkValue;

  if (alphaMin == 0)
    return alphaMin;

  // Base check on colour depth of display
  hdc = CreateCompatibleDC(NULL);
  /*int devCaps = GetDeviceCaps(hdc, BITSPIXEL);
  std::wstring debug = L"DeviceCaps: ";
  debug += towstring(devCaps);
  ELWriteDebug(debug);*/
  if (GetDeviceCaps(hdc, BITSPIXEL) == 32)
    checkValue = 0.5;
  else
    checkValue = 3.5;
  DeleteDC(hdc);

  srcAlphaCheck = (float)alphaMin * (float)(alphaDelta / 255.0);
  while ((srcAlphaCheck < checkValue) && (alphaMin < 255))
    {
      alphaMin++;
      srcAlphaCheck = (float)alphaMin * (float)(alphaDelta / 255.0);
    }

  return alphaMin;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:    EGConvertIcon
// Requires:    HICON sourceIcon - input Icon
//              HINSTANCE appInstance - applet Instance
// Returns:     HICON
// Purpose:     Converts the sourceIcon to a true 32-bit icon (if it isn't already)
//----  --------------------------------------------------------------------------------------------------------
HICON EGConvertIcon(HICON sourceIcon, BYTE foregroundAlpha)
{
  if (sourceIcon == NULL)
    return NULL;

  BYTE pixelAlpha = 0xff;
  double alphaFactor = 255.0 / (float)foregroundAlpha;
  double pixelFactor;
  UINT32 pixel;

  ICONINFO iconInfo;
  BITMAP bmp, mask;
  BITMAPINFO bmi;
  BYTE *bmpBits;
  VOID *targetBits;
  UINT32 x, y;
  HDC maskDC, bmpDC, targetDC;
  HICON targetIcon;
  HBITMAP hbitmap;
  bool foundPixel = false, hasAlpha = false;

  if (!GetIconInfo(sourceIcon, &iconInfo))
    return NULL;
  if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp) == 0)
    return NULL;
  if (GetObject(iconInfo.hbmMask, sizeof(BITMAP), &mask) == 0)
    {
      DeleteObject(iconInfo.hbmColor);
      return NULL;
    }

  bmpDC = CreateCompatibleDC(NULL);
  SelectObject(bmpDC, iconInfo.hbmColor);

  ZeroMemory(&bmi, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = bmp.bmWidth;
  bmi.bmiHeader.biHeight = bmp.bmHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  if (GetDIBits(bmpDC, iconInfo.hbmColor, 0, bmp.bmHeight, NULL, &bmi, DIB_RGB_COLORS) == 0)
    {
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      return NULL;
    }
  bmpBits = (BYTE*)GlobalAlloc(GMEM_FIXED, bmi.bmiHeader.biSizeImage);
  if (bmpBits == NULL)
    {
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      return NULL;
    }
  if (GetDIBits(bmpDC, iconInfo.hbmColor, 0, bmp.bmHeight, bmpBits, &bmi, DIB_RGB_COLORS) == 0)
    {
      GlobalFree((HGLOBAL)bmpBits);
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      return NULL;
    }

  bmi.bmiHeader.biSizeImage = bmp.bmWidth * bmp.bmHeight * 4;

  targetDC = CreateCompatibleDC(NULL);
  hbitmap = CreateDIBSection(targetDC, &bmi, DIB_RGB_COLORS, &targetBits, NULL, 0x0);
  if (hbitmap == NULL)
    {
      GlobalFree((HGLOBAL)bmpBits);
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      DeleteDC(targetDC);
      return NULL;
    }
  SelectObject(targetDC, hbitmap);

  maskDC = CreateCompatibleDC(NULL);
  SelectObject(maskDC, iconInfo.hbmMask);

  if (bmp.bmBitsPixel == 32)
    {
      for (y = 0; y < (UINT32)mask.bmHeight; y++)
        {
          for (x = 0; x < (UINT32)mask.bmWidth; x++)
            {
              BYTE alpha = ((UINT32*)bmpBits)[x + ((mask.bmHeight - 1) - y) * mask.bmWidth] >> 24;
              if (alpha != 0x00)
                {
                  hasAlpha = true;
                  break;
                }
            }
        }
    }

  for (y = 0; y < (UINT32)mask.bmHeight; y++)
    {
      for (x = 0; x < (UINT32)mask.bmWidth; x++)
        {
          if (!GetPixel(maskDC, x, (mask.bmHeight - 1) - y))
            {
              foundPixel = true;

              pixel = ((UINT32*)bmpBits)[x + y * mask.bmWidth] << 4;
              pixel = pixel >> 4;

              if (hasAlpha)
                {
                  pixelAlpha = ((UINT32*)bmpBits)[x + y * mask.bmWidth] >> 24;
                  pixelFactor = (double)pixelAlpha / alphaFactor;
                  pixelAlpha = (BYTE)pixelFactor;
                }
              else
                pixelAlpha = foregroundAlpha;

              ((UINT32*)targetBits)[x + y * mask.bmWidth] = pixel;
              ((UINT32*)targetBits)[x + y * mask.bmWidth] |= (pixelAlpha << 24);
            }
          else
            ((UINT32*)targetBits)[x + y * mask.bmWidth] = 0x00000000;
        }
    }

  DeleteObject(iconInfo.hbmColor);
  iconInfo.hbmColor = hbitmap;

  if (foundPixel)
    targetIcon = CreateIconIndirect(&iconInfo);
  else
    targetIcon = NULL;

  GlobalFree((HGLOBAL)bmpBits);
  DeleteObject(iconInfo.hbmMask);
  DeleteObject(hbitmap);
  DeleteDC(maskDC);
  DeleteDC(bmpDC);
  DeleteDC(targetDC);

  return targetIcon;
}

HBRUSH EGCreateBrush(BYTE alpha, COLORREF colour)
{
  HBITMAP bmp;
  HBRUSH brush;
  VOID *bits;
  HDC hdc;
  BITMAPINFO bmi;
  UINT32 pixel;

  pixel = EGGetPixel(alpha, colour);

  hdc = CreateCompatibleDC(NULL);

  // zero the memory for the bitmap info
  ZeroMemory(&bmi, sizeof(BITMAPINFO));

  // setup bitmap info
  // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on,
  // the blending will occur in the center of each of the three areas.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = 1;
  bmi.bmiHeader.biHeight = 1;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 4;

  // create our DIB section and select the bitmap into the dc
  bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0x0);
  ((UINT32 *)bits)[0] =
    pixel;

  brush = CreatePatternBrush(bmp);

  DeleteObject(bmp);
  DeleteDC(hdc);

  return brush;
}

HPEN EGCreatePen(DWORD style, DWORD width, BYTE alpha, COLORREF colour)
{
  HPEN pen;
  LOGBRUSH lb;
  HBITMAP bmp;
  VOID *bits;
  HDC hdc;
  BITMAPINFO bmi;
  UINT32 pixel;

  pixel = EGGetPixel(alpha, colour);

  hdc = CreateCompatibleDC(NULL);

  // zero the memory for the bitmap info
  ZeroMemory(&bmi, sizeof(BITMAPINFO));

  // setup bitmap info
  // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on,
  // the blending will occur in the center of each of the three areas.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = 1;
  bmi.bmiHeader.biHeight = 1;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 4;

  // create our DIB section and select the bitmap into the dc
  bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0x0);
  ((UINT32 *)bits)[0] =
    pixel;

  lb.lbStyle = BS_DIBPATTERN;
  lb.lbColor = DIB_RGB_COLORS;
  lb.lbHatch = (ULONG_PTR)bmp;

  pen = ExtCreatePen(style | PS_GEOMETRIC | PS_ENDCAP_ROUND | PS_JOIN_ROUND, width, &lb, 0, NULL);

  DeleteObject(bmp);
  DeleteDC(hdc);

  return pen;
}

bool EGFrameRect(HDC hdc, RECT *rect, BYTE bgAlpha, COLORREF borderColour, int borderWidth)
{
  RECT workingRect;
  HBRUSH borderBrush = EGCreateBrush(bgAlpha, borderColour);
  int ret = 0;

  if (CopyRect(&workingRect, rect) && (borderBrush != NULL))
    {
      for (int i = 0; i < borderWidth; i++)
        {
          ret = FrameRect(hdc, &workingRect, borderBrush);
          if (ret == 0)
            break;
          if (!InflateRect(&workingRect, -1, -1))
            {
              ret = 0;
              break;
            }
        }
    }

  DeleteObject(borderBrush);

  return (ret != 0);
}

bool EGGradientFillRect(HDC hdc, RECT *rect, BYTE alpha, COLORREF colourFrom, COLORREF colourTo, int bevelWidth, WCHAR *gradientMethod)
{
  bool interlaced = false, bevelled = false, sunken = false, paintedGradient = false;
  BLENDFUNCTION bf;
  BITMAPINFO bmi;
  VOID *gradientBits;
  HDC gradientDC = CreateCompatibleDC(hdc);
  WCHAR *lower;

  UINT x = rect->left;
  UINT y = rect->top;
  UINT width = rect->right - rect->left;
  UINT height = rect->bottom - rect->top;

  ZeroMemory(&bmi, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = width * height * 4;

  HBITMAP gradientBMP = CreateDIBSection(gradientDC, &bmi, DIB_RGB_COLORS, &gradientBits, NULL, 0x0);
  SelectObject(gradientDC, gradientBMP);

  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = alpha;
  bf.AlphaFormat = AC_SRC_ALPHA;

  lower = _wcslwr(_wcsdup(gradientMethod));
  if (wcsstr(lower, TEXT("interlaced")) != NULL)
    interlaced = true;

  if (wcsstr(lower, TEXT("raised")) != NULL)
    bevelled = true;
  else if (wcsstr(lower, TEXT("sunken")) != NULL)
    {
      bevelled = true;
      sunken = true;
    }

  if (wcsstr(lower, TEXT("horizontal")) != NULL)
    paintedGradient = hgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else if (wcsstr(lower, TEXT("crossdiagonal")) != NULL)
    paintedGradient = cdgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else if (wcsstr(lower, TEXT("diagonal")) != NULL)
    paintedGradient = dgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else if (wcsstr(lower, TEXT("pipecross")) != NULL)
    paintedGradient = pcgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else if (wcsstr(lower, TEXT("elliptic")) != NULL)
    paintedGradient = egradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else if (wcsstr(lower, TEXT("rectangle")) != NULL)
    paintedGradient = rgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else if (wcsstr(lower, TEXT("pyramid")) != NULL)
    paintedGradient = pgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  else
    paintedGradient = vgradient(width, height, colourFrom, colourTo, (BYTE*)gradientBits, interlaced);
  free(lower);

  if (paintedGradient)
    {
      if (bevelled)
        bevel(width, height, bevelWidth, (BYTE*)gradientBits, sunken);

      AlphaBlend(hdc, x, y, width, height, gradientDC, 0, 0, width, height, bf);
    }
  else
    EGFillRect(hdc, rect, bf.SourceConstantAlpha, colourFrom);

  DeleteObject(gradientBMP);
  DeleteDC(gradientDC);

  return true;
}

bool EGFillRect(HDC hdc, RECT *rect, BYTE alpha, COLORREF colour)
{
  BLENDFUNCTION bf;
  BOOL ret;

  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = 0xff;
  bf.AlphaFormat = AC_SRC_ALPHA;

  int width = rect->right - rect->left;
  int height = rect->bottom - rect->top;

  if ((width <= 0) || (height <= 0))
    return false;

  HDC fillDC = CreateCompatibleDC(hdc);

  HBITMAP fillBMP = EGCreateBitmap(alpha, colour, *rect);
  SelectObject(fillDC, fillBMP);

  ret = AlphaBlend(hdc, rect->left, rect->top, width, height, fillDC, 0, 0, width, height, bf);

  DeleteObject(fillBMP);
  DeleteDC(fillDC);

  return (ret == TRUE);
}

UINT32 EGGetPixel(BYTE alpha, COLORREF colour)
{
  UINT32 pixel;
  float fAlphaFactor;

  fAlphaFactor = (float)alpha / (float)0xff;

  pixel = (alpha << 24) |
          ((UCHAR)(GetRValue(colour) * fAlphaFactor) << 16) |
          ((UCHAR)(GetGValue(colour) * fAlphaFactor) << 8) |
          (UCHAR)(GetBValue(colour) * fAlphaFactor);

  return pixel;
}

HBITMAP EGCreateBitmap(BYTE alpha, COLORREF colour, RECT wndRect)
{
  HBITMAP bmp;
  HDC hdc;
  VOID *bits;
  BITMAPINFO bmi;
  UINT32 pixel;
  ULONG ulWindowWidth, ulWindowHeight, x, y;

  pixel = EGGetPixel(alpha, colour);

  hdc = CreateCompatibleDC(NULL);

  // calculate window width/height
  ulWindowWidth = wndRect.right - wndRect.left;
  ulWindowHeight = wndRect.bottom - wndRect.top;

  // zero the memory for the bitmap info
  ZeroMemory(&bmi, sizeof(BITMAPINFO));

  // setup bitmap info
  // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on,
  // the blending will occur in the center of each of the three areas.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = ulWindowWidth;
  bmi.bmiHeader.biHeight = ulWindowHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = ulWindowWidth * ulWindowHeight * 4;

  // create our DIB section and select the bitmap into the dc
  bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0x0);

  for (y = 0; y < ulWindowHeight; y++)
    for (x = 0; x < ulWindowWidth; x++)
      ((UINT32 *)bits)[x + y * ulWindowWidth] =
        pixel;

  DeleteDC(hdc);

  return bmp;
}

HICON EGGetClassIcon(std::wstring file, int iconSize)
{
  HICON icon = NULL;
  HKEY key = NULL, typeKey = NULL;
  WCHAR type[MAX_LINE_LENGTH], iconValue[MAX_LINE_LENGTH], *token;
  DWORD size = MAX_LINE_LENGTH;
  std::wstring iconPath;

  size_t extensionMark = file.rfind('.');
  if (extensionMark == std::wstring::npos)
    return icon;

  if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
                   file.substr(extensionMark, file.length() - extensionMark).c_str(),
                   0,
                   KEY_READ,
                   &key) == ERROR_SUCCESS)
    {
      if (RegQueryValueEx(key, NULL, NULL, NULL, (BYTE*)type, &size) == ERROR_SUCCESS)
        {
          wcscat(type, L"\\DefaultIcon");
          if (RegOpenKeyEx(HKEY_CLASSES_ROOT,
                           type,
                           0,
                           KEY_READ,
                           &typeKey) == ERROR_SUCCESS)
            {
              size = MAX_LINE_LENGTH;
              if (RegQueryValueEx(typeKey, NULL, NULL, NULL, (BYTE*)iconValue, &size) == ERROR_SUCCESS)
                {
                  int iconIndex = 0;
                  token = wcstok(iconValue, TEXT(","));
                  iconPath = token;
                  iconPath = ELExpandVars(iconPath);
                  token = wcstok(NULL, TEXT("\n"));
                  if (token != NULL)
                    iconIndex = _wtoi(token);
                  icon = EGExtractIcon(iconPath.c_str(), iconIndex, iconSize);
                }
              RegCloseKey(typeKey);
            }
        }
      RegCloseKey(key);
    }

  return icon;
}

HICON EGGetFileIcon(const WCHAR *file, UINT iconSize)
{
  HICON icon = NULL, smallIcon = NULL, largeIcon = NULL;

  IShellFolder *deskFolder = NULL;
  IShellFolder *appObject = NULL;
  LPITEMIDLIST pidlLocal = NULL;
  LPITEMIDLIST pidlRelative = NULL;
  IExtractIcon *extractIcon = NULL;
  WCHAR iconLocation[MAX_PATH], canonicalizedFile[MAX_PATH];
  int iconIndex = 0;
  UINT iconFlags = 0;
  HRESULT hr;
  LPVOID lpVoid;
  bool hasIndex = false;
  std::wstring suppliedFile;

  if (file == NULL)
    return icon;

  suppliedFile = file;
  if (!suppliedFile.empty())
    {
      if (suppliedFile.at(0) == '@')
        suppliedFile = suppliedFile.substr(1);
    }
  suppliedFile = ELExpandVars(suppliedFile);
  size_t comma = suppliedFile.find_last_of(',');
  if (comma != std::wstring::npos)
    {
      if (suppliedFile.substr(comma).find('.') == std::wstring::npos)
        {
          iconIndex = _wtoi(suppliedFile.substr(comma + 1).c_str());
          suppliedFile = suppliedFile.substr(0, comma);
          hasIndex = true;
        }
    }

  // Normalize the file path
  if (PathCanonicalize(canonicalizedFile, suppliedFile.c_str()))
    suppliedFile = canonicalizedFile;

  if (hasIndex)
    {
      icon = EGExtractIcon(suppliedFile.c_str(), iconIndex, iconSize);
      return icon;
    }

  hr = SHGetDesktopFolder(&deskFolder);
  if (SUCCEEDED(hr))
    {
      hr = deskFolder->ParseDisplayName(NULL, NULL, (WCHAR*)suppliedFile.c_str(), NULL, &pidlLocal, NULL);
      if (SUCCEEDED(hr))
        {
          pidlRelative = ILClone(ILFindLastID(pidlLocal));
          ILRemoveLastID(pidlLocal);

          hr = deskFolder->BindToObject(pidlLocal, NULL, IID_IShellFolder, &lpVoid);
          if (SUCCEEDED(hr))
            {
              appObject = reinterpret_cast <IShellFolder*> (lpVoid);
              ILFree(pidlLocal);

              hr = appObject->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlRelative,
                                            IID_IExtractIcon, NULL, &lpVoid);
              if (SUCCEEDED(hr))
                {
                  extractIcon = reinterpret_cast <IExtractIcon*> (lpVoid);

                  ILFree(pidlRelative);

                  hr = extractIcon->GetIconLocation(GIL_FORSHELL, iconLocation, MAX_PATH, &iconIndex, &iconFlags);
                  if (SUCCEEDED(hr))
                    {
                      if ((iconFlags & GIL_PERCLASS) == GIL_PERCLASS)
                        icon = EGGetClassIcon(suppliedFile, iconSize);
                      if (((iconFlags & GIL_NOTFILENAME) == GIL_NOTFILENAME) && (icon == NULL))
                        {
                          // For some reason, .cpl files have an iconLocation of "*" that seems to mess up extractIcon while
                          // for other iconLocations of "*" it's fine, so add this work around for now.
                          if ((suppliedFile.find(TEXT(".cpl")) != std::wstring::npos) && (wcscmp(iconLocation, TEXT("*")) == 0))
                            wcscpy(iconLocation, suppliedFile.c_str());

                          if (ELPathFileExists(iconLocation) || (wcscmp(iconLocation, TEXT("*")) == 0))
                            {
                              hr = extractIcon->Extract(iconLocation, iconIndex, &largeIcon, &smallIcon, MAKELONG(iconSize, iconSize));
                              if ((iconSize == 16) && smallIcon)
                                icon = CopyIcon(smallIcon);
                              else
                                icon = CopyIcon(largeIcon);

                              DestroyIcon(smallIcon);
                              DestroyIcon(largeIcon);
                            }
                        }
                    }
                  extractIcon->Release();
                }

              appObject->Release();
            }
        }

      deskFolder->Release();
    }

  if ((icon == NULL) && wcslen(iconLocation))
    icon = EGExtractIcon(iconLocation, iconIndex, iconSize);

  if (icon == NULL)
    icon = EGGetSystemIcon(ICON_DEFAULT, iconSize);

  return icon;
}

bool EGGetIconDialogue(HWND hwnd, WCHAR *iconPath, int iconIndex)
{
  WCHAR filename[MAX_PATH], shortPath[MAX_PATH];
  std::wstring tmpPath = iconPath;

  if (MSPickIcon == NULL)
    MSPickIcon = (fnPickIcon)GetProcAddress(shell32DLL, (LPCSTR)62);
  if (MSPickIcon == NULL)
    return false;

  ZeroMemory(shortPath, MAX_PATH);

  tmpPath = ELExpandVars(tmpPath);
  GetShortPathName(tmpPath.c_str(), shortPath, MAX_PATH);

  if (MSPickIcon(hwnd, shortPath, MAX_PATH, &iconIndex) != 0)
    {
      tmpPath = shortPath;
      tmpPath = ELExpandVars(tmpPath);
      GetLongPathName(tmpPath.c_str(), filename, MAX_PATH);
      swprintf(iconPath, TEXT("%ls,%d"), filename, iconIndex);
      return true;
    }

  return false;
}

VOID CALLBACK GetSmallIconCallBack(HWND hwnd, UINT uMsg UNUSED, ULONG_PTR dwData, LRESULT lResult)
{
  HICON icon = (HICON)lResult;

  if (!icon)
    icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);

  if (icon)
    PostMessage((HWND)dwData, TASK_ICON, (WPARAM)hwnd, (LPARAM)icon);
}

VOID CALLBACK GetIconCallBack(HWND hwnd, UINT uMsg UNUSED, ULONG_PTR dwData, LRESULT lResult)
{
  HICON icon = (HICON)lResult;

  if (!icon)
    icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);

  if (icon)
    PostMessage((HWND)dwData, TASK_ICON, (WPARAM)hwnd, (LPARAM)icon);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	EGGetWindowIcon
// Required:	HWND task - handle to the task's window
// Returns:	HICON
// Purpose:	Retrieve the icon for the task
//----  --------------------------------------------------------------------------------------------------------
HICON EGGetWindowIcon(HWND callerWnd, HWND hwnd, bool smallIcon, bool force)
{
  /* Set icon to NULL to return a NULL value in the case where an icon cannot
   * be found.  Leave it to the applet to create a default icon in this case.
   */
  HICON icon = NULL;

  if (force)
    {
      if (smallIcon)
        {
          SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL2, 0,
                             SMTO_ABORTIFHUNG, ICON_LOOKUP_TIMEOUT,
                             reinterpret_cast<ULONG_PTR*>(&icon));
          if (!icon)
            icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);
        }
      else
        {
          SendMessageTimeout(hwnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG,
                             ICON_LOOKUP_TIMEOUT,
                             reinterpret_cast<ULONG_PTR*>(&icon));
          if (!icon)
            icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
        }
    }
  else
    {
      if (smallIcon)
        SendMessageCallback(hwnd, WM_GETICON, ICON_SMALL2, 0,
                            GetSmallIconCallBack, (ULONG_PTR)callerWnd);
      else
        SendMessageCallback(hwnd, WM_GETICON, ICON_BIG, 0, GetIconCallBack,
                            (ULONG_PTR)callerWnd);
    }

  return icon;
}

HICON EGGetSpecialFolderIcon(int csidl, UINT iconSize)
{
  HICON icon = NULL;
  LPITEMIDLIST pidl = NULL;
  SHFILEINFO fileInfo;
  WCHAR iconLocation[MAX_LINE_LENGTH];

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, &pidl)))
    {
      if (SHGetFileInfo((LPCTSTR)pidl, 0, &fileInfo, sizeof(fileInfo), SHGFI_PIDL|SHGFI_ICONLOCATION) != 0)
        {
          swprintf(iconLocation, TEXT("%ls,%d"), fileInfo.szDisplayName, fileInfo.iIcon);
          icon = EGGetFileIcon(iconLocation, iconSize);
        }

      ILFree(pidl);
    }

  return icon;
}

HICON EGExtractIcon(const WCHAR *iconLocation, int iconIndex, int iconSize)
{
  HICON icon = NULL;
  UINT iconID;

  if (MSPrivateExtractIcons == NULL)
    MSPrivateExtractIcons = (fnPrivateExtractIcons)GetProcAddress(user32DLL, "PrivateExtractIconsW");
  if (MSPrivateExtractIcons != NULL)
    MSPrivateExtractIcons(iconLocation, iconIndex, iconSize, iconSize, &icon, &iconID, 1, 0);

  return icon;
}

HICON EGGetSystemIcon(UINT iconIndex, UINT iconSize)
{
  HICON icon = NULL;
  WCHAR source[MAX_PATH];
  int iconLocation = 0;

  if (ELVersionInfo() >= 6.1)
    {
      wcscpy(source, TEXT("%SystemRoot%\\system32\\imageres.dll"));
      switch (iconIndex)
        {
        case ICON_DEFAULT:
          iconLocation = 2;
          break;
        case ICON_RUN:
          iconLocation = 95;
          break;
        case ICON_QUESTION:
          iconLocation = 94;
          break;
        }
    }
  else
    {
      wcscpy(source, TEXT("%SystemRoot%\\system32\\shell32.dll"));
      switch (iconIndex)
        {
        case ICON_DEFAULT:
          iconLocation = 0;
          break;
        case ICON_RUN:
          iconLocation = 24;
          break;
        case ICON_QUESTION:
          iconLocation = 23;
          break;
        }
    }

  switch (iconIndex)
    {
    case ICON_EMERGE:
      wcscpy(source, TEXT("emergeIcons.dll"));
      iconLocation = 0;
      break;
    case ICON_QUIT:
      wcscpy(source, TEXT("emergeIcons.dll"));
      iconLocation = 19;
      break;
    case ICON_SHUTDOWN:
      wcscpy(source, TEXT("%SystemRoot%\\system32\\shell32.dll"));
      iconLocation = 27;
      break;
    case ICON_LOGOFF:
      wcscpy(source, TEXT("%SystemRoot%\\system32\\shell32.dll"));
      iconLocation = 44;
      break;
    case ICON_LOCK:
      wcscpy(source, TEXT("%SystemRoot%\\system32\\shell32.dll"));
      iconLocation = 47;
      break;
    }

  icon = EGExtractIcon(source, iconLocation, iconSize);

  return icon;
}

bool EGGetTextRect(WCHAR *text, HFONT font, RECT *rect, UINT flags)
{
  int ret = 0;
  HDC hdc = CreateCompatibleDC(NULL);

  DeleteObject(SelectObject(hdc, font));
  ret = DrawTextEx(hdc, text, wcslen(text), rect, DT_CALCRECT | flags, NULL);
  DeleteDC(hdc);

  return (ret != 0);
}

bool EGDrawAlphaText(BYTE alpha, CLIENTINFO clientInfo, FORMATINFO formatInfo, WCHAR *commandText)
{
  HDC maskdc, fillDC;
  HBITMAP hmask, fillBMP;
  BITMAPINFO bmi;
  BYTE *bmpBits, *maskBits;
  int x,y, verticalOffset;
  UINT drawFlags, fontSmoothing = 0;
  UCHAR alphaValue;
  UINT32 pixel;
  BLENDFUNCTION bf;
  BOOL ret;
  RECT maskRect, textRect;
  double alphaDelta = 0.0;

  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = alpha;
  bf.AlphaFormat = AC_SRC_ALPHA;

  int width = clientInfo.rt.right - clientInfo.rt.left;
  int height = clientInfo.rt.bottom - clientInfo.rt.top;

  ZeroMemory(&textRect, sizeof(RECT));
  textRect.right = clientInfo.rt.right;
  if (!EGGetTextRect(commandText, formatInfo.font, &textRect, formatInfo.flags))
    return false;
  int displayHeight = textRect.bottom;
  if (displayHeight == 0)
    return false;

  maskRect.left = 0;
  maskRect.top = 0;
  maskRect.right = width;
  maskRect.bottom =  displayHeight;

  if (formatInfo.verticalAlignment == EGDAT_VCENTER)
    {
      verticalOffset = clientInfo.rt.top;
      verticalOffset +=  height/ 2;
      verticalOffset -= displayHeight / 2;
    }
  else if (formatInfo.verticalAlignment == EGDAT_BOTTOM)
    verticalOffset = clientInfo.rt.bottom - displayHeight;
  else
    verticalOffset = clientInfo.rt.top;

  if ((width <= 0) || (height <= 0))
    return false;

  fillDC = CreateCompatibleDC(NULL);

  fillBMP = EGCreateBitmap(0x00, formatInfo.color, maskRect);
  SelectObject(fillDC, fillBMP);

  maskdc = CreateCompatibleDC(NULL);
  hmask = EGCreateBitmap(0x00, RGB(0, 0, 0), maskRect);
  SelectObject(maskdc, hmask);

  ZeroMemory(&bmi, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = displayHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  drawFlags = formatInfo.flags | DT_NOPREFIX;

  if (formatInfo.horizontalAlignment == EGDAT_HCENTER)
    drawFlags |= DT_CENTER;
  else if (formatInfo.horizontalAlignment == EGDAT_RIGHT)
    drawFlags |= DT_RIGHT;
  else
    drawFlags |= DT_LEFT;

  SetBkMode(maskdc, TRANSPARENT);
  SetTextColor(maskdc, RGB(255, 255, 255));
  SelectObject(maskdc, formatInfo.font);

  DrawTextEx(maskdc, commandText, (int)wcslen(commandText), &maskRect, drawFlags , NULL);

  if (GetDIBits(fillDC, fillBMP, 0, displayHeight, NULL, &bmi, DIB_RGB_COLORS) == 0)
    {
      DeleteObject(fillBMP);
      DeleteDC(fillDC);
      DeleteObject(hmask);
      DeleteDC(maskdc);
      return false;
    }
  bmpBits = (BYTE*)GlobalAlloc(GMEM_FIXED, bmi.bmiHeader.biSizeImage);
  maskBits = (BYTE*)GlobalAlloc(GMEM_FIXED, bmi.bmiHeader.biSizeImage);
  GetDIBits(fillDC, fillBMP, 0, displayHeight, bmpBits, &bmi, DIB_RGB_COLORS);
  GetDIBits(maskdc, hmask, 0, displayHeight, maskBits, &bmi, DIB_RGB_COLORS);

  SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &fontSmoothing, 0);

  if (fontSmoothing == FE_FONTSMOOTHINGCLEARTYPE)
    alphaDelta = ((double)clientInfo.bgAlpha / 255.0) * 20.0;
  else
    alphaDelta = ((double)clientInfo.bgAlpha / 255.0) * 100.0;

  for (y = 0; y < bmi.bmiHeader.biHeight; y++)
    for (x = 0; x < bmi.bmiHeader.biWidth; x++)
      {
        pixel = ((UINT32 *)maskBits)
                [x + ((bmi.bmiHeader.biHeight - (y + 1)) * bmi.bmiHeader.biWidth)];

        if (pixel != 0)
          {
            alphaValue = (GetRValue(pixel) + GetGValue(pixel) + GetBValue(pixel)) / 3;

            if ((alphaValue < 255) && (alphaValue > (BYTE)alphaDelta))
              alphaValue -= (BYTE)alphaDelta;

            ((UINT32 *)bmpBits)[x + ((bmi.bmiHeader.biHeight - (y + 1)) * bmi.bmiHeader.biWidth)] =
              EGGetPixel(alphaValue, formatInfo.color);
          }
      }

  SetDIBits(fillDC, fillBMP, 0, displayHeight, bmpBits, &bmi, DIB_RGB_COLORS);
  ret = AlphaBlend(clientInfo.hdc, clientInfo.rt.left, verticalOffset, width, displayHeight, fillDC, maskRect.left, maskRect.top, width, displayHeight, bf);

  // do cleanup
  GlobalFree((HGLOBAL)bmpBits);
  GlobalFree((HGLOBAL)maskBits);
  DeleteObject(fillBMP);
  DeleteDC(fillDC);
  DeleteObject(hmask);
  DeleteDC(maskdc);

  return (ret == TRUE);
}

HDC EGBeginPaint(HWND wnd)
{
  RECT clientrt;
  BITMAPINFO bmi;

  // get window dimensions
  GetClientRect(wnd, &clientrt);

  // create a DC for our bitmap -- the source DC for AlphaBlend
  hdc = CreateCompatibleDC(NULL);

  // zero the memory for the bitmap info
  ZeroMemory(&bmi, sizeof(BITMAPINFOHEADER));

  // set the bitmap width and height to 60% of the width and height of each of the
  // three horizontal areas. Later on, the blending will occur in the center of each
  // of the three areas.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = clientrt.right;
  bmi.bmiHeader.biHeight = clientrt.bottom;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 4;

  // create our DIB section and select the bitmap into the dc
  hbitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0x0);
  SelectObject(hdc, hbitmap);

  return hdc;
}

void EGEndPaint()
{
  if (hbitmap)
    DeleteObject(hbitmap);
  if (hdc)
    DeleteDC(hdc);
}

bool EGEqualLogFont(const LOGFONT& source, const LOGFONT& target)
{
  if (source.lfHeight != target.lfHeight)
    return false;

  if (source.lfWidth != target.lfWidth)
    return false;

  if (source.lfEscapement != target.lfEscapement)
    return false;

  if (source.lfOrientation != target.lfOrientation)
    return false;

  if (source.lfWeight != target.lfWeight)
    return false;

  if (source.lfItalic != target.lfItalic)
    return false;

  if (source.lfUnderline != target.lfUnderline)
    return false;

  if (source.lfStrikeOut != target.lfStrikeOut)
    return false;

  if (source.lfCharSet != target.lfCharSet)
    return false;

  if (source.lfOutPrecision != target.lfOutPrecision)
    return false;

  if (source.lfClipPrecision != target.lfClipPrecision)
    return false;

  if (source.lfQuality != target.lfQuality)
    return false;

  if (source.lfPitchAndFamily != target.lfPitchAndFamily)
    return false;

  if (_wcsicmp(source.lfFaceName, target.lfFaceName) != 0)
    return false;

  return true;
}

void EGFontToString(const LOGFONT& logFont, WCHAR *fontString)
{
  WCHAR tmp[MAX_LINE_LENGTH];
  HDC hdc = CreateCompatibleDC(NULL);
  wcscpy(tmp, logFont.lfFaceName);
  if (logFont.lfWeight == FW_BOLD)
    wcscat(tmp, TEXT("-Bold"));
  if (logFont.lfItalic)
    wcscat(tmp, TEXT("-Italic"));
  int fontHeight = MulDiv(logFont.lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
  swprintf(fontString, TEXT("%ls-%d"), tmp, fontHeight);
  DeleteDC(hdc);
}

void EGStringToFont(const WCHAR *fontString, LOGFONT& logFont)
{
  WCHAR *token;
  WCHAR *workingFontString = _wcsdup(fontString);
  HDC hdc = CreateCompatibleDC(NULL);

  ZeroMemory(&logFont, sizeof(LOGFONT));
  token = wcstok((WCHAR*)workingFontString, TEXT("-"));
  wcscpy(logFont.lfFaceName, token);
  token = wcstok(NULL, TEXT("-"));
  while (token)
    {
      if (_wcsicmp(token, TEXT("Bold")) == 0)
        logFont.lfWeight = FW_BOLD;

      if (_wcsicmp(token, TEXT("Italic")) == 0)
        logFont.lfItalic = TRUE;

      unsigned int i;
      for (i = 0; i < wcslen(token); i++)
        if (!iswdigit(token[i]))
          break;

      if (i == wcslen(token))
        logFont.lfHeight = MulDiv(_wtoi(token), GetDeviceCaps(hdc, LOGPIXELSY), 72);
      token = wcstok(NULL, TEXT("-"));
    }
  DeleteDC(hdc);
  free(workingFontString);

  logFont.lfCharSet = DEFAULT_CHARSET;
  logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
}

HBITMAP EGGetIconBitmap(HICON sourceIcon)
{
  if (sourceIcon == NULL)
    return NULL;

  BYTE pixelAlpha = 0xff;
  BYTE foregroundAlpha = 0xff;
  double alphaFactor = 255.0 / (float)foregroundAlpha;
  double pixelFactor;
  UINT32 pixel;

  ICONINFO iconInfo;
  BITMAP bmp, mask;
  BITMAPINFO bmi;
  BYTE *bmpBits;
  VOID *targetBits;
  UINT32 x, y;
  HDC maskDC, bmpDC, targetDC;
  HBITMAP hbitmap;
  bool hasAlpha = false;

  if (!GetIconInfo(sourceIcon, &iconInfo))
    return NULL;
  if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp) == 0)
    return NULL;
  if (GetObject(iconInfo.hbmMask, sizeof(BITMAP), &mask) == 0)
    {
      DeleteObject(iconInfo.hbmColor);
      return NULL;
    }

  bmpDC = CreateCompatibleDC(NULL);
  SelectObject(bmpDC, iconInfo.hbmColor);

  ZeroMemory(&bmi, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = bmp.bmWidth;
  bmi.bmiHeader.biHeight = bmp.bmHeight;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  if (GetDIBits(bmpDC, iconInfo.hbmColor, 0, bmp.bmHeight, NULL, &bmi, DIB_RGB_COLORS) == 0)
    {
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      return NULL;
    }
  bmpBits = (BYTE*)GlobalAlloc(GMEM_FIXED, bmi.bmiHeader.biSizeImage);
  if (bmpBits == NULL)
    {
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      return NULL;
    }
  if (GetDIBits(bmpDC, iconInfo.hbmColor, 0, bmp.bmHeight, bmpBits, &bmi, DIB_RGB_COLORS) == 0)
    {
      GlobalFree((HGLOBAL)bmpBits);
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      return NULL;
    }

  bmi.bmiHeader.biSizeImage = bmp.bmWidth * bmp.bmHeight * 4;

  targetDC = CreateCompatibleDC(NULL);
  hbitmap = CreateDIBSection(targetDC, &bmi, DIB_RGB_COLORS, &targetBits, NULL, 0x0);
  if (hbitmap == NULL)
    {
      GlobalFree((HGLOBAL)bmpBits);
      DeleteObject(iconInfo.hbmColor);
      DeleteObject(iconInfo.hbmMask);
      DeleteDC(bmpDC);
      DeleteDC(targetDC);
      return NULL;
    }
  SelectObject(targetDC, hbitmap);

  maskDC = CreateCompatibleDC(NULL);
  SelectObject(maskDC, iconInfo.hbmMask);

  if (bmp.bmBitsPixel == 32)
    {
      for (y = 0; y < (UINT32)mask.bmHeight; y++)
        {
          for (x = 0; x < (UINT32)mask.bmWidth; x++)
            {
              BYTE alpha = ((UINT32*)bmpBits)[x + ((mask.bmHeight - 1) - y) * mask.bmWidth] >> 24;
              if (alpha != 0x00)
                {
                  hasAlpha = true;
                  break;
                }
            }
        }
    }

  for (y = 0; y < (UINT32)mask.bmHeight; y++)
    {
      for (x = 0; x < (UINT32)mask.bmWidth; x++)
        {
          if (!GetPixel(maskDC, x, (mask.bmHeight - 1) - y))
            {
              pixel = ((UINT32*)bmpBits)[x + y * mask.bmWidth] << 4;
              pixel = pixel >> 4;

              if (hasAlpha)
                {
                  pixelAlpha = ((UINT32*)bmpBits)[x + y * mask.bmWidth] >> 24;
                  pixelFactor = (double)pixelAlpha / alphaFactor;
                  pixelAlpha = (BYTE)pixelFactor;
                }
              else
                pixelAlpha = foregroundAlpha;

              ((UINT32*)targetBits)[x + y * mask.bmWidth] = pixel;
              ((UINT32*)targetBits)[x + y * mask.bmWidth] |= (pixelAlpha << 24);
            }
          else
            ((UINT32*)targetBits)[x + y * mask.bmWidth] = 0x00000000;
        }
    }

  DeleteObject(iconInfo.hbmColor);
  GlobalFree((HGLOBAL)bmpBits);
  DeleteObject(iconInfo.hbmMask);
  DeleteDC(maskDC);
  DeleteDC(bmpDC);
  DeleteDC(targetDC);

  return hbitmap;
}

BOOL EGIsCompositionEnabled()
{
  BOOL check = FALSE;

  if (dwmapiDLL == NULL)
    return check;

  if (MSDwmIsCompositionEnabled == NULL)
    MSDwmIsCompositionEnabled = (fnDwmIsCompositionEnabled)GetProcAddress(dwmapiDLL, "DwmIsCompositionEnabled");
  if (MSDwmIsCompositionEnabled)
    {
      if (SUCCEEDED(MSDwmIsCompositionEnabled(&check)))
        return check;
    }

  return check;
}

HRESULT EGBlurWindow(HWND hwnd, bool enable)
{
  HRESULT hr = E_FAIL;
  RECT clientrt;
  DWM_BLURBEHIND bb;

  if (!GetClientRect(hwnd, &clientrt))
    return hr;

  if (dwmapiDLL == NULL)
    return hr;

  // If region is not set, there will be bleed over of the blur affect when
  // resizing the window.
  ZeroMemory(&bb, sizeof(DWM_BLURBEHIND));
  bb.dwFlags = DWM_BB_ENABLE;
  if (enable)
    {
      bb.dwFlags |= DWM_BB_BLURREGION;
      bb.fEnable = TRUE;
      bb.hRgnBlur = CreateRectRgn(clientrt.left, clientrt.top, clientrt.right,
                                  clientrt.bottom);
    }

  if (MSDwmEnableBlurBehindWindow == NULL)
    MSDwmEnableBlurBehindWindow = (fnDwmEnableBlurBehindWindow)GetProcAddress(dwmapiDLL, "DwmEnableBlurBehindWindow");
  if (MSDwmEnableBlurBehindWindow)
    hr = MSDwmEnableBlurBehindWindow(hwnd, &bb);

  if (bb.hRgnBlur != NULL)
    DeleteObject(bb.hRgnBlur);

  return hr;
}
