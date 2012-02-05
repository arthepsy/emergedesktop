// vim:tags+=../emergeLib/tags
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

#include "Settings.h"

Settings::Settings()
  :BaseSettings(false)
{
  ResetDefaults();
}

void Settings::DoReadSettings(IOHelper& helper)
{
  helper.ReadInt(TEXT("TopBorder"), topBorder, 0);
  helper.ReadInt(TEXT("BottomBorder"), bottomBorder, 0);
  helper.ReadInt(TEXT("LeftBorder"), leftBorder, 0);
  helper.ReadInt(TEXT("RightBorder"), rightBorder, 0);
  helper.ReadInt(TEXT("MenuAlpha"), menuAlpha, 100);
  helper.ReadBool(TEXT("MenuIcons"), menuIcons, true);
  helper.ReadBool(TEXT("BorderPrimary"), borderPrimary, true);
  helper.ReadBool(TEXT("AeroMenus"), aeroMenus, false);
}

void Settings::DoInitialize()
{
  BaseSettings::DoInitialize();
}

void Settings::ResetDefaults()
{
  topBorder = 0;
  bottomBorder = 44;
  leftBorder = 0;
  rightBorder = 0;
  menuAlpha = 100;
  menuIcons = true;
  borderPrimary = true;
  aeroMenus = false;
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  helper.WriteInt(TEXT("TopBorder"), topBorder);
  helper.WriteInt(TEXT("BottomBorder"), bottomBorder);
  helper.WriteInt(TEXT("LeftBorder"), leftBorder);
  helper.WriteInt(TEXT("RightBorder"), rightBorder);
  helper.WriteInt(TEXT("MenuAlpha"), menuAlpha);
  helper.WriteBool(TEXT("MenuIcons"), menuIcons);
  helper.WriteBool(TEXT("BorderPrimary"), borderPrimary);
  helper.WriteBool(TEXT("AeroMenus"), aeroMenus);
}

void Settings::GetDesktopRect(RECT *rect)
{
  (*rect).top += topBorder;
  (*rect).left += leftBorder;
  (*rect).bottom -= bottomBorder;
  (*rect).right -= rightBorder;
}

int Settings::GetMenuAlpha()
{
  return menuAlpha;
}

bool Settings::GetMenuIcons()
{
  return menuIcons;
}

bool Settings::GetAeroMenus()
{
  return aeroMenus;
}

void Settings::SetMenuIcons(bool value)
{
  if (menuIcons != value)
    {
      menuIcons = value;
      SetModified();
    }
}

void Settings::SetAeroMenus(bool value)
{
  if (aeroMenus != value)
    {
      aeroMenus = value;
      SetModified();
    }
}

void Settings::SetTopBorder(int value)
{
  if (topBorder != value)
    {
      topBorder = value;
      SetModified();
    }
}

int Settings::GetTopBorder()
{
  return topBorder;
}

bool Settings::GetBorderPrimary()
{
  return borderPrimary;
}

void Settings::SetLeftBorder(int value)
{
  if (leftBorder != value)
    {
      leftBorder = value;
      SetModified();
    }
}

void Settings::SetBorderPrimary(bool value)
{
  if (borderPrimary != value)
    {
      borderPrimary = value;
      SetModified();
    }
}

int Settings::GetLeftBorder()
{
  return leftBorder;
}

void Settings::SetRightBorder(int value)
{
  if (rightBorder != value)
    {
      rightBorder = value;
      SetModified();
    }
}

int Settings::GetRightBorder()
{
  return rightBorder;
}

void Settings::SetBottomBorder(int value)
{
  if (bottomBorder != value)
    {
      bottomBorder = value;
      SetModified();
    }
}

int Settings::GetBottomBorder()
{
  return bottomBorder;
}

void Settings::SetMenuAlpha(int value)
{
  if (menuAlpha != value)
    {
      menuAlpha = value;
      SetModified();
    }
}
