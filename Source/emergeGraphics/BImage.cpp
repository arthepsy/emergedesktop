//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2011  The Emerge Desktop Development Team
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

#include "BImage.h"
#include "../emergeGraphics/emergeGraphics.h"
#include <malloc.h>

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

BYTE convertcolour(BYTE colour, bool dark)
{
  BYTE newColour;

  if (dark)
    {
      newColour = (colour >> 2) + (colour >> 1);
      if (newColour > colour) newColour = 0;
    }
  else
    {
      newColour = colour + (colour >> 1);
      if (newColour < colour) newColour = ~0;
    }

  return newColour;
}

//===========================================================================

// Fixes to make PipeCross and Rectangle work correctly / nivenh
template <typename T> inline T lmax(T a, T b)
{
  return ((a > b) ? a : b);
}
template <typename T> inline T lmin(T a, T b)
{
  return ((a < b) ? a : b);
}

//===========================================================================
// Below:   The original Blackbox (for the X Window System) math code,
//          modified to paint directly into the destination DIBSection! :D
//          The original code is issued under the BSD license,
//          for more info visit http://sourceforge.net/projects/blackboxwm/
//          Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh at debian.org>
//          Copyright (c) 1997 - 2000, 2002 Brad Hughes <bhughes at trolltech.com>
//===========================================================================

bool bevel(UINT width, UINT height, UINT bevelWidth, BYTE *pixels, bool sunken)
{
  if (width <= (2 * bevelWidth)  || height <= (2 * bevelWidth))
    return false;

  UINT i = 0;
  BYTE r, g, b, rr ,gg ,bb;
  UINT w, h, wh, count;

  wh = width * (height - 1) * 4;

  while (i < bevelWidth)
    {
      w = width - (2 * i);
      h = height - 1 - (2 * i);
      count = 0;

      while (--w)
        {
          r = pixels[count + (i * width * 4) + 2 + (i * 4)];
          rr = convertcolour(r, !sunken);
          g = pixels[count + (i * width * 4) + 1 + (i * 4)];
          gg = convertcolour(g, !sunken);
          b = pixels[count + (i * width * 4) + (i * 4)];
          bb = convertcolour(b, !sunken);

          pixels[count + (i * width * 4) + 3 + (i * 4)] = 0xff;
          pixels[count + (i * width * 4) + 2 + (i * 4)] = rr;
          pixels[count + (i * width * 4) + 1 + (i * 4)] = gg;
          pixels[count + (i * width * 4) + (i * 4)] = bb;

          r = pixels[count + 2 + wh - (i * width * 4) + (i * 4)];
          rr = convertcolour(r, sunken);
          g = pixels[count + 1 + wh - (i * width * 4) + (i * 4)];
          gg = convertcolour(g, sunken);
          b = pixels[count + wh - (i * width * 4) + (i * 4)];
          bb = convertcolour(b, sunken);

          pixels[count + 3 + wh - (i * width * 4) + (i * 4)] = 0xff;
          pixels[count + 2 + wh - (i * width * 4) + (i * 4)] = rr;
          pixels[count + 1 + wh - (i * width * 4) + (i * 4)] = gg;
          pixels[count + wh - (i * width * 4) + (i * 4)] = bb;

          count += 4;
        }

      r = pixels[count + (i * width * 4) + 2 + (i * 4)];
      rr = convertcolour(r, !sunken);
      g = pixels[count + (i * width * 4) + 1 + (i * 4)];
      gg = convertcolour(g, !sunken);
      b = pixels[count + (i * width * 4) + (i * 4)];
      bb = convertcolour(b, !sunken);

      pixels[count + (i * width * 4) + 3 + (i * 4)] = 0xff;
      pixels[count + (i * width * 4) + 2 + (i * 4)] = rr;
      pixels[count + (i * width * 4) + 1 + (i * 4)] = gg;
      pixels[count + (i * width * 4) + (i * 4)] = bb;

      count = (width * 4);

      while (--h)
        {
          r = pixels[count + 2 + (i * width * 4) + (i * 4)];
          rr = convertcolour(r, sunken);
          g = pixels[count + 1 + (i * width * 4) + (i * 4)];
          gg = convertcolour(g, sunken);
          b = pixels[count + (i * width * 4) + (i * 4)];
          bb = convertcolour(b, sunken);

          pixels[count + 3 + (i * width * 4) + (i * 4)] = 0xff;
          pixels[count + 2 + (i * width * 4) + (i * 4)] = rr;
          pixels[count + 1 + (i * width * 4) + (i * 4)] = gg;
          pixels[count + (i * width * 4) + (i * 4)] = bb;

          count += (width * 4) - 4;

          r = pixels[count + 2 + (i * width * 4) - (i * 4)];
          rr = convertcolour(r, !sunken);
          g = pixels[count + 1 + (i * width * 4) - (i * 4)];
          gg = convertcolour(g, !sunken);
          b = pixels[count + (i * width * 4) - (i * 4)];
          bb = convertcolour(b, !sunken);

          pixels[count + 3 + (i * width * 4) - (i * 4)] = 0xff;
          pixels[count + 2 + (i * width * 4) - (i * 4)] = rr;
          pixels[count + 1 + (i * width * 4) - (i * 4)] = gg;
          pixels[count + (i * width * 4) - (i * 4)] = bb;

          count += 4;
        }

      count += (width * 4) - 4;

      r = pixels[count + 2 + (i * width * 4) - (i * 4)];
      rr = convertcolour(r, sunken);
      g = pixels[count + 1 + (i * width * 4) - (i * 4)];
      gg = convertcolour(g, sunken);
      b = pixels[count + (i * width * 4) - (i * 4)];
      bb = convertcolour(b, sunken);

      pixels[count + 3 + (i * width * 4) - (i * 4)] = 0xff;
      pixels[count + 2 + (i * width * 4) - (i * 4)] = rr;
      pixels[count + 1 + (i * width * 4) - (i * 4)] = gg;
      pixels[count + (i * width * 4) - (i * 4)] = bb;

      i++;
    }

  return true;
}

//====================

bool dgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  float drx, dgx, dbx, dry, dgy, dby, yr = 0.0, yg = 0.0, yb = 0.0, xr, xg, xb;
  std::tr1::shared_ptr<UINT> xtable(new unsigned int[width * 3]);
  std::tr1::shared_ptr<UINT> ytable(new unsigned int[height * 3]);
  UINT w = width * 2, h = height * 2, *xt = xtable.get(), *yt = ytable.get(), x, y, count = 0;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  // diagonal gradient code was written by Mike Cole <mike@mydot.com>
  // modified for interlacing by Brad Hughes

  xr = (float) from_red;
  xg = (float) from_green;
  xb = (float) from_blue;

  dry = drx = (float) (to_red - from_red);
  dgy = dgx = (float) (to_green - from_green);
  dby = dbx = (float) (to_blue - from_blue);

  // Create X table
  drx /= w;
  dgx /= w;
  dbx /= w;

  for (x = 0; x < width; x++)
    {
      *(xt++) = (unsigned char) (xr);
      *(xt++) = (unsigned char) (xg);
      *(xt++) = (unsigned char) (xb);

      xr += drx;
      xg += dgx;
      xb += dbx;
    }

  // Create Y table
  dry /= h;
  dgy /= h;
  dby /= h;

  for (yt = (ytable.get() + (height * 3) - 1), y = 0; y < height; y++)
    {
      *(yt--) = ((unsigned char) yb);
      *(yt--) = ((unsigned char) yg);
      *(yt--) = ((unsigned char) yr);

      yr += dry;
      yg += dgy;
      yb += dby;
    }

  // Combine tables to create gradient

  if (!interlaced)
    {
      // normal dgradient
      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = *(xt++) + *(yt);
              pixels[count + 1] = *(xt++) + *(yt + 1);
              pixels[count] = *(xt++) + *(yt + 2);
              count += 4;
            }
        }
    }
  else
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = *(xt++) + *(yt);
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = *(xt++) + *(yt + 1);
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = *(xt++) + *(yt + 2);
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = *(xt++) + *(yt);
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = *(xt++) + *(yt + 1);
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = *(xt++) + *(yt + 2);
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
        }
    }

  return true;
}

//====================

bool hgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  float drx, dgx, dbx, xr, xg, xb;
  UINT x, y, original, count = 0;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  xr = (float) from_red;
  xg = (float) from_green;
  xb = (float) from_blue;

  drx = (float) (to_red - from_red);
  dgx = (float) (to_green - from_green);
  dbx = (float) (to_blue - from_blue);

  drx /= width;
  dgx /= width;
  dbx /= width;

  if (interlaced && height > 2)
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (x = 0; x < width; x++)
        {
          pixels[count + 3] = 0xff;

          channel = (unsigned char) xr;
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          pixels[count + 2] = channel2;

          channel = (unsigned char) xg;
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          pixels[count + 1] = channel2;

          channel = (unsigned char) xb;
          channel2 = (channel >> 1) + (channel >> 2);
          if (channel2 > channel) channel2 = 0;
          pixels[count] = channel2;

          pixels[count + 3 + (width * 4)] = 0xff;

          channel = (unsigned char) xr;
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          pixels[count + 2 + (width * 4)] = channel2;

          channel = (unsigned char) xg;
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          pixels[count + 1 + (width * 4)] = channel2;

          channel = (unsigned char) xb;
          channel2 = channel + (channel >> 3);
          if (channel2 < channel) channel2 = ~0;
          pixels[count + (width * 4)] = channel2;

          xr += drx;
          xg += dgx;
          xb += dbx;

          count += 4;
        }

      count += (width * 4);

      int offset;

      for (y = 2; y < height; y++)
        {
          if (y & 1) offset = (width * 4);
          else offset = 0;

          original = 0;

          for (x =0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = pixels[original + 2 + offset];
              pixels[count + 1] = pixels[original + 1 + offset];
              pixels[count] = pixels[original + offset];

              count += 4;
              original += 4;
            }
        }
    }
  else
    {
      // normal hgradient
      for (x = 0; x < width; x++)
        {
          pixels[count + 3] = 0xff;
          pixels[count + 2] = (unsigned char) (xr);
          pixels[count + 1] = (unsigned char) (xg);
          pixels[count] = (unsigned char) (xb);

          xr += drx;
          xg += dgx;
          xb += dbx;

          count += 4;
        }

      for (y = 1; y < height; y++)
        {
          original = 0;
          for (x =0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = pixels[original + 2];
              pixels[count + 1] = pixels[original + 1];
              pixels[count] = pixels[original];

              count += 4;
              original += 4;
            }
        }
    }

  return true;
}

//====================

bool vgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  float dry, dgy, dby, yr, yg, yb;
  UINT x, y, count = 0;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  yr = (float) to_red;
  yg = (float) to_green;
  yb = (float) to_blue;

  dry = (float) (from_red - to_red);
  dgy = (float) (from_green - to_green);
  dby = (float) (from_blue - to_blue);

  dry /= height;
  dgy /= height;
  dby /= height;

  if (interlaced)
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (y = 0; y < height; y++)
        {
          for (x =0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) yr;
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) yg;
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) yb;
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) yr;
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) yg;
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) yb;
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
          yr += dry;
          yg += dgy;
          yb += dby;
        }
    }
  else
    {
      // normal vgradient
      for (y =0; y < height ; y++)
        {
          BYTE pr=(BYTE)yr, pg=(BYTE)yg, pb=(BYTE)yb;
          for (x =0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = (unsigned char) pr;
              pixels[count + 1] = (unsigned char) pg;
              pixels[count] = (unsigned char) pb;

              count += 4;
            }
          yr += dry;
          yg += dgy;
          yb += dby;
        }
    }

  return true;
}

//====================

bool pgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  // pyramid gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Blackbox by Brad Hughes

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  std::tr1::shared_ptr<UINT> xtable(new unsigned int[width * 3]);
  std::tr1::shared_ptr<UINT> ytable(new unsigned int[height * 3]);
  float yr, yg, yb, drx, dgx, dbx, dry, dgy, dby, xr, xg, xb;
  int rsign, gsign, bsign;
  UINT tr, tg, tb, *xt = xtable.get(), *yt = ytable.get(), x, y, count = 0;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  tr = to_red;
  tg = to_green;
  tb = to_blue;

  dry = drx = (float) (to_red - from_red);
  dgy = dgx = (float) (to_green - from_green);
  dby = dbx = (float) (to_blue - from_blue);

  rsign = (drx < 0) ? -1 : 1;
  gsign = (dgx < 0) ? -1 : 1;
  bsign = (dbx < 0) ? -1 : 1;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++)
    {
      *(xt++) = (unsigned char) ((xr < 0) ? -xr : xr);
      *(xt++) = (unsigned char) ((xg < 0) ? -xg : xg);
      *(xt++) = (unsigned char) ((xb < 0) ? -xb : xb);

      xr -= drx;
      xg -= dgx;
      xb -= dbx;
    }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++)
    {
      *(yt++) = ((unsigned char) ((yr < 0) ? -yr : yr));
      *(yt++) = ((unsigned char) ((yg < 0) ? -yg : yg));
      *(yt++) = ((unsigned char) ((yb < 0) ? -yb : yb));

      yr -= dry;
      yg -= dgy;
      yb -= dby;
    }

  // Combine tables to create gradient

  if (!interlaced)
    {
      // normal pgradient
      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = (unsigned char) (tr - (rsign * (*(xt++) + *(yt))));
              pixels[count + 1] = (unsigned char) (tg - (gsign * (*(xt++) + *(yt + 1))));
              pixels[count] = (unsigned char) (tb - (bsign * (*(xt++) + *(yt + 2))));
              count += 4;
            }
        }
    }
  else
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) (tr - (rsign * (*(xt++) + *(yt))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) (tg - (gsign * (*(xt++) + *(yt + 1))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) (tb - (bsign * (*(xt++) + *(yt + 2))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) (tr - (rsign * (*(xt++) + *(yt))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) (tg - (gsign * (*(xt++) + *(yt + 1))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) (tb - (bsign * (*(xt++) + *(yt + 2))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
        }
    }

  return true;
}

//====================

bool rgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  std::tr1::shared_ptr<UINT> xtable(new unsigned int[width * 3]);
  std::tr1::shared_ptr<UINT> ytable(new unsigned int[height * 3]);
  UINT x, y, count = 0;
  float drx, dgx, dbx, dry, dgy, dby, xr, xg, xb, yr, yg, yb;
  int rsign, gsign, bsign;
  unsigned int tr, tg, tb, *xt = xtable.get(), *yt = ytable.get();

  // rectangle gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Blackbox by Brad Hughes

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  tr = to_red;
  tg = to_green;
  tb = to_blue;

  dry = drx = (float) (to_red - from_red);
  dgy = dgx = (float) (to_green - from_green);
  dby = dbx = (float) (to_blue - from_blue);

  rsign = (drx < 0) ? -2 : 2;
  gsign = (dgx < 0) ? -2 : 2;
  bsign = (dbx < 0) ? -2 : 2;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++)
    {
      *(xt++) = (unsigned char) ((xr < 0) ? -xr : xr);
      *(xt++) = (unsigned char) ((xg < 0) ? -xg : xg);
      *(xt++) = (unsigned char) ((xb < 0) ? -xb : xb);

      xr -= drx;
      xg -= dgx;
      xb -= dbx;
    }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++)
    {
      *(yt++) = ((unsigned char) ((yr < 0) ? -yr : yr));
      *(yt++) = ((unsigned char) ((yg < 0) ? -yg : yg));
      *(yt++) = ((unsigned char) ((yb < 0) ? -yb : yb));

      yr -= dry;
      yg -= dgy;
      yb -= dby;
    }

  // Combine tables to create gradient

  if (!interlaced)
    {
      // normal rgradient
      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = (unsigned char) (tr - (rsign * lmax(*(xt++), *(yt))));
              pixels[count + 1] = (unsigned char) (tg - (gsign * lmax(*(xt++), *(yt + 1))));
              pixels[count] = (unsigned char) (tb - (bsign * lmax(*(xt++), *(yt + 2))));
              count += 4;
            }
        }
    }
  else
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) (tr - (rsign * lmax(*(xt++), *(yt))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) (tg - (gsign * lmax(*(xt++), *(yt + 1))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) (tb - (bsign * lmax(*(xt++), *(yt + 2))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) (tr - (rsign * lmax(*(xt++), *(yt))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) (tg - (gsign * lmax(*(xt++), *(yt + 1))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) (tb - (bsign * lmax(*(xt++), *(yt + 2))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
        }
    }

  return true;
}

//====================

bool egradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  float drx, dgx, dbx, dry, dgy, dby, yr, yg, yb, xr, xg, xb;
  int rsign, gsign, bsign;
  std::tr1::shared_ptr<UINT> xtable(new unsigned int[width * 3]);
  std::tr1::shared_ptr<UINT> ytable(new unsigned int[height * 3]);
  unsigned int *xt = xtable.get(), *yt = ytable.get(), tr, tg, tb;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  // elliptic gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Blackbox by Brad Hughes

  tr = (unsigned long) to_red;
  tg = (unsigned long) to_green;
  tb = (unsigned long) to_blue;

  UINT x, y, count = 0;

  dry = drx = (float) (to_red - from_red);
  dgy = dgx = (float) (to_green - from_green);
  dby = dbx = (float) (to_blue - from_blue);

  rsign = (drx < 0) ? -1 : 1;
  gsign = (dgx < 0) ? -1 : 1;
  bsign = (dbx < 0) ? -1 : 1;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++)
    {
      *(xt++) = (unsigned long) (xr * xr);
      *(xt++) = (unsigned long) (xg * xg);
      *(xt++) = (unsigned long) (xb * xb);

      xr -= drx;
      xg -= dgx;
      xb -= dbx;
    }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++)
    {
      *(yt++) = (unsigned long) (yr * yr);
      *(yt++) = (unsigned long) (yg * yg);
      *(yt++) = (unsigned long) (yb * yb);

      yr -= dry;
      yg -= dgy;
      yb -= dby;
    }

  // Combine tables to create gradient

  if (!interlaced)
    {
      // normal egradient
      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = (unsigned char)
                                  (tr - (rsign * getSqrt(*(xt++) + *(yt))));
              pixels[count + 1] = (unsigned char)
                                  (tg - (gsign * getSqrt(*(xt++) + *(yt + 1))));
              pixels[count] = (unsigned char)
                              (tb - (bsign * getSqrt(*(xt++) + *(yt + 2))));
              count += 4;
            }
        }
    }
  else
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char)
                            (tr - (rsign * getSqrt(*(xt++) + *(yt))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char)
                            (tg - (gsign * getSqrt(*(xt++) + *(yt + 1))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char)
                            (tb - (bsign * getSqrt(*(xt++) + *(yt + 2))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char)
                            (tr - (rsign * getSqrt(*(xt++) + *(yt))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char)
                            (tg - (gsign * getSqrt(*(xt++) + *(yt + 1))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char)
                            (tb - (bsign * getSqrt(*(xt++) + *(yt + 2))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
        }
    }

  return true;
}

//====================

bool pcgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  float drx, dgx, dbx, dry, dgy, dby, xr, xg, xb, yr, yg, yb;
  int rsign, gsign, bsign;
  std::tr1::shared_ptr<UINT> xtable(new unsigned int[width * 3]);
  std::tr1::shared_ptr<UINT> ytable(new unsigned int[height * 3]);
  unsigned int *xt = xtable.get(), *yt = ytable.get(), tr, tg, tb;
  UINT x, y, count = 0;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  tr = to_red;
  tg = to_green;
  tb = to_blue;

  // pipe cross gradient - based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Blackbox by Brad Hughes

  dry = drx = (float) (to_red - from_red);
  dgy = dgx = (float) (to_green - from_green);
  dby = dbx = (float) (to_blue - from_blue);

  rsign = (drx < 0) ? -2 : 2;
  gsign = (dgx < 0) ? -2 : 2;
  bsign = (dbx < 0) ? -2 : 2;

  xr = yr = (drx / 2);
  xg = yg = (dgx / 2);
  xb = yb = (dbx / 2);

  // Create X table
  drx /= width;
  dgx /= width;
  dbx /= width;

  for (x = 0; x < width; x++)
    {
      *(xt++) = (unsigned char) ((xr < 0) ? -xr : xr);
      *(xt++) = (unsigned char) ((xg < 0) ? -xg : xg);
      *(xt++) = (unsigned char) ((xb < 0) ? -xb : xb);

      xr -= drx;
      xg -= dgx;
      xb -= dbx;
    }

  // Create Y table
  dry /= height;
  dgy /= height;
  dby /= height;

  for (y = 0; y < height; y++)
    {
      *(yt++) = ((unsigned char) ((yr < 0) ? -yr : yr));
      *(yt++) = ((unsigned char) ((yg < 0) ? -yg : yg));
      *(yt++) = ((unsigned char) ((yb < 0) ? -yb : yb));

      yr -= dry;
      yg -= dgy;
      yb -= dby;
    }

  // Combine tables to create gradient

  if (!interlaced)
    {
      // normal pcgradient
      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = (unsigned char) (tr - (rsign * lmin(*(xt++), *(yt))));
              pixels[count + 1] = (unsigned char) (tg - (gsign * lmin(*(xt++), *(yt + 1))));
              pixels[count] = (unsigned char) (tb - (bsign * lmin(*(xt++), *(yt + 2))));
              count += 4;
            }
        }
    }
  else
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) (tr - (rsign * lmin(*(xt++), *(yt))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) (tg - (bsign * lmin(*(xt++), *(yt + 1))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) (tb - (gsign * lmin(*(xt++), *(yt + 2))));
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = (unsigned char) (tr - (rsign * lmin(*(xt++), *(yt))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = (unsigned char) (tg - (gsign * lmin(*(xt++), *(yt + 1))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = (unsigned char) (tb - (bsign * lmin(*(xt++), *(yt + 2))));
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
        }
    }

  return true;
}

//====================

bool cdgradient(UINT width, UINT height, COLORREF gradientFrom, COLORREF gradientTo, BYTE *pixels, bool interlaced)
{
  if ((height <= 2) || (width <= 2))
    return false;

  BYTE to_red, to_green, to_blue, from_red, from_green, from_blue;
  float drx, dgx, dbx, dry, dgy, dby, yr = 0.0, yg = 0.0, yb = 0.0, xr, xg, xb;
  std::tr1::shared_ptr<UINT> xtable(new unsigned int[width * 3]);
  std::tr1::shared_ptr<UINT> ytable(new unsigned int[height * 3]);
  UINT w = width * 2, h = height * 2, *xt, *yt, x, y, count = 0;

  to_red = GetRValue(gradientTo);
  to_green = GetGValue(gradientTo);
  to_blue = GetBValue(gradientTo);

  from_red = GetRValue(gradientFrom);
  from_green = GetGValue(gradientFrom);
  from_blue = GetBValue(gradientFrom);

  // cross diagonal gradient -  based on original dgradient, written by
  // Mosfet (mosfet@kde.org)
  // adapted from kde sources for Blackbox by Brad Hughes

  xr = (float) from_red;
  xg = (float) from_green;
  xb = (float) from_blue;

  dry = drx = (float) (to_red - from_red);
  dgy = dgx = (float) (to_green - from_green);
  dby = dbx = (float) (to_blue - from_blue);

  // Create X table
  drx /= w;
  dgx /= w;
  dbx /= w;

  for (xt = (xtable.get() + (width * 3) - 1), x = 0; x < width; x++)
    {
      *(xt--) = (unsigned char) xb;
      *(xt--) = (unsigned char) xg;
      *(xt--) = (unsigned char) xr;

      xr += drx;
      xg += dgx;
      xb += dbx;
    }

  // Create Y table
  dry /= h;
  dgy /= h;
  dby /= h;

  for (yt = (ytable.get() + (height * 3) - 1), y = 0; y < height; y++)
    {
      *(yt--) = ((unsigned char) yb);
      *(yt--) = ((unsigned char) yg);
      *(yt--) = ((unsigned char) yr);

      yr += dry;
      yg += dgy;
      yb += dby;
    }

  // Combine tables to create gradient

  if (!interlaced)
    {
      // normal cdgradient
      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              pixels[count + 3] = 0xff;
              pixels[count + 2] = *(xt++) + *(yt);
              pixels[count + 1] = *(xt++) + *(yt + 1);
              pixels[count] = *(xt++) + *(yt + 2);
              count += 4;
            }
        }
    }
  else
    {
      // faked interlacing effect
      unsigned char channel, channel2;

      for (yt = ytable.get(), y = 0; y < height; y++, yt += 3)
        {
          for (xt = xtable.get(), x = 0; x < width; x++)
            {
              if (y & 1)
                {
                  pixels[count + 3] = 0xff;

                  channel = *(xt++) + *(yt);
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 2] = channel2;

                  channel = *(xt++) + *(yt + 1);
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count + 1] = channel2;

                  channel = *(xt++) + *(yt + 2);
                  channel2 = (channel >> 1) + (channel >> 2);
                  if (channel2 > channel) channel2 = 0;
                  pixels[count] = channel2;
                }
              else
                {
                  pixels[count + 3] = 0xff;

                  channel = *(xt++) + *(yt);
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 2] = channel2;

                  channel = *(xt++) + *(yt + 1);
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count + 1] = channel2;

                  channel = *(xt++) + *(yt + 2);
                  channel2 = channel + (channel >> 3);
                  if (channel2 < channel) channel2 = ~0;
                  pixels[count] = channel2;
                }
              count += 4;
            }
        }
    }

  return true;
}

//===========================================================================

unsigned long *sqrt_table = NULL;

unsigned long bsqrt(unsigned long x)
{
  if (x <= 0) return 0;
  if (x == 1) return 1;

  unsigned long r = x >> 1;
  unsigned long q;

  while (1)
    {
      q = x / r;
      if (q >= r) return r;
      r = (r + q) >> 1;
    }
}

unsigned long getSqrt(unsigned int x)
{
  if (!sqrt_table)
    {
      // build sqrt table for use with elliptic gradient
      sqrt_table = new unsigned long[(256 * 256 * 2) + 1];

      for (int i = 0; i < (256 * 256 * 2); i++)
        *(sqrt_table + i) = bsqrt(i);
    }
  return (*(sqrt_table + x));
}

