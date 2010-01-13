//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2009  The Emerge Desktop Development Team
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

#include "Button.h"
#define STEP 10
#define Inside(r, x, y) (r.left <= x && r.right >= x && r.top <= y && r.bottom >= y)


Button::Button(Applet& app, Settings* sett, HFONT& font, GUIINFO& ginfo, HWND hw, bool iterated):
  applet(app), mainFont(font), guiInfo(ginfo)
{
  pSettings = sett;
  hwnd = hw;
  clicked = false;
  iteration = 0;
  this->iterated = iterated;
}
void Button::SetTitle(const std::wstring& text)
{
  if (title != text)
    {
      title = text;
      iteration = 0;
    }
}

void Button::NextIteration()
{
  ++iteration;
  // protect from overflow
  if (iteration > 30000)
    iteration = 0;
}

void Button::SetBounds(const RECT& rect)
{
  bounds = rect;
}

void Button::Paint(HDC hdc)
{
  int width = bounds.right - bounds.left;
  if (width > 0)
    {
      if (clicked)
        EGFillRect(hdc, &bounds, guiInfo.alphaSelected, guiInfo.colorSelected);

      // clip region
      HRGN hrgn = CreateRectRgn(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1);
      SelectClipRgn(hdc, hrgn);

      // calc needed place for text
      SelectObject(hdc, mainFont);
      RECT r;
      r.left = 0;
      r.top = 0;
      DrawTextEx(hdc, (WCHAR*)title.c_str(), title.size(), &r, DT_CENTER | DT_VCENTER | DT_CALCRECT, NULL);
      int requiredTextWidth = r.right;
      OffsetRect(&r, bounds.left + 1, bounds.top);
      // calc offset of iterated text
      if (iterated && requiredTextWidth > width)
        {
          int totalSteps = requiredTextWidth / STEP + 1;
          int visibleSteps = width / STEP;
          if (iteration > totalSteps)
            iteration = 0;
          if (iteration > visibleSteps)
            OffsetRect(&r, -(iteration - visibleSteps) * STEP, 0);
        }
      else
        {
          // keep the text in the middle
          r.right = bounds.right;
        }
      r.bottom = bounds.bottom;

      // drawAlphaText
      CLIENTINFO clientInfo;
      FORMATINFO formatInfo;
      clientInfo.hdc = hdc;
      clientInfo.bgAlpha = guiInfo.alphaBackground;
      CopyRect(&clientInfo.rt, &r);
      formatInfo.font = mainFont;
      formatInfo.color = guiInfo.colorFont;
      formatInfo.lines = 1;
      formatInfo.fontHeight = pSettings->GetFont()->lfHeight;
      if (iterated)
        {
          if (_wcsicmp(pSettings->GetHorizontalDirection(), TEXT("center")) == 0)
            formatInfo.horizontalAlignment = EGDAT_HCENTER;
          else if (_wcsicmp(pSettings->GetHorizontalDirection(), TEXT("right")) == 0)
            formatInfo.horizontalAlignment = EGDAT_RIGHT;
          else
            formatInfo.horizontalAlignment = EGDAT_LEFT;
          if (_wcsicmp(pSettings->GetVerticalDirection(), TEXT("center")) == 0)
            formatInfo.verticalAlignment = EGDAT_VCENTER;
          else if (_wcsicmp(pSettings->GetVerticalDirection(), TEXT("down")) == 0)
            formatInfo.verticalAlignment = EGDAT_BOTTOM;
          else
            formatInfo.verticalAlignment = EGDAT_TOP;
        }
      else
        {
          formatInfo.horizontalAlignment = EGDAT_HCENTER;
          formatInfo.verticalAlignment = EGDAT_VCENTER;
        }
      EGDrawAlphaText(guiInfo.alphaText, clientInfo, formatInfo, (WCHAR*)title.c_str());

      // restore clip
      SelectClipRgn(hdc, NULL);
      DeleteObject(hrgn);

      EGFrameRect(hdc, &bounds, guiInfo.alphaFrame, guiInfo.colorFrame, 1);
    }
}

bool Button::LButtonDown(int x, int y)
{
  if (Inside(bounds, x, y))
    {
      clicked = true;
      SetCapture(hwnd);
      return true;
    }
  return false;
}

bool Button::LButtonUp(int x, int y)
{
  if (clicked)
    {
      ReleaseCapture();
      clicked = false;
      if (Inside(bounds, x, y))
        return true;
    }
  return false;
}
