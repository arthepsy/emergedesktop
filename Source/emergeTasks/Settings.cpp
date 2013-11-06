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
#include <stdio.h>

Settings::Settings()
  : BaseSettings(true)
{
  hiliteActive = true;
  flashCount = 0;
  enableFlash = true;
  flashInterval = 1000;
  sameMonitorOnly = false; //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  enableThumbnails = true;
  thumbnailAlpha = 100;
  thumbnailScale = 20;
}

void Settings::DoReadSettings(IOHelper& helper)
{
  BaseSettings::DoReadSettings(helper);
  hiliteActive = helper.ReadBool(TEXT("HighLightActive"), true);
  sameMonitorOnly = helper.ReadBool(TEXT("SameMonitorOnly"), false); //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  flashCount = helper.ReadInt(TEXT("FlashCount"), 0);
  enableFlash = helper.ReadBool(TEXT("EnableFlash"), true);
  flashInterval = helper.ReadInt(TEXT("FlashInterval"), 1000);
  enableThumbnails = helper.ReadBool(TEXT("EnableThumbnails"), true);
  thumbnailAlpha = helper.ReadInt(TEXT("ThumbnailAlpha"), 100);
  thumbnailScale = helper.ReadInt(TEXT("ThumbnailScale"), 20);
}

void Settings::DoWriteSettings(IOHelper& helper)
{
  BaseSettings::DoWriteSettings(helper);
  helper.WriteBool(TEXT("HighLightActive"), hiliteActive);
  helper.WriteBool(TEXT("SameMonitorOnly"), sameMonitorOnly); //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  helper.WriteInt(TEXT("FlashCount"), flashCount);
  helper.WriteBool(TEXT("EnableFlash"), enableFlash);
  helper.WriteInt(TEXT("FlashInterval"), flashInterval);
  helper.WriteBool(TEXT("EnableThumbnails"), enableThumbnails);
  helper.WriteInt(TEXT("ThumbnailAlpha"), thumbnailAlpha);
  helper.WriteInt(TEXT("ThumbnailScale"), thumbnailScale);
}

void Settings::ResetDefaults()
{
  BaseSettings::ResetDefaults();
  hiliteActive = false;
  flashCount = 0;
  enableFlash = true;
  flashInterval = 1000;
  x = 285;
  y = -40;
  width = 271;
  height = 32;
  zPosition = TEXT("Top");
  horizontalDirection = TEXT("right");
  verticalDirection = TEXT("up");
  directionOrientation = TEXT("horizontal");
  autoSize = false;
  iconSize = 32;
  iconSpacing = 7;
  snapMove = true;
  snapSize = true;
  dynamicPositioning = true;
  clickThrough = 0;
  appletMonitor = 0;
  anchorPoint = TEXT("BottomLeft");
  sameMonitorOnly = false; //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  enableThumbnails = true;
  thumbnailAlpha = 100;
  thumbnailScale = 20;
}

bool Settings::GetHiliteActive()
{
  return hiliteActive;
}

bool Settings::SetHiliteActive(bool hiliteActive)
{
  if (this->hiliteActive != hiliteActive)
  {
    this->hiliteActive = hiliteActive;
    SetModified();
  }
  return true;
}

//ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
bool Settings::GetSameMonitorOnly()
{
  return sameMonitorOnly;
}

bool Settings::SetSameMonitorOnly(bool sameMonitorOnly)
{
  if (this->sameMonitorOnly != sameMonitorOnly)
  {
    this->sameMonitorOnly = sameMonitorOnly;
    SetModified();
  }
  return true;
}

int Settings::GetFlashCount()
{
  return flashCount;
}

int Settings::GetFlashInterval()
{
  return flashInterval;
}

bool Settings::GetEnableFlash()
{
  return enableFlash;
}

bool Settings::SetFlashCount(int flashCount)
{
  if (this->flashCount != flashCount)
  {
    this->flashCount = flashCount;
    SetModified();
  }
  return true;
}

bool Settings::SetFlashInterval(int flashInterval)
{
  if (this->flashInterval != flashInterval)
  {
    this->flashInterval = flashInterval;
    SetModified();
  }
  return true;
}

bool Settings::SetEnableFlash(bool enableFlash)
{
  if (this->enableFlash != enableFlash)
  {
    this->enableFlash = enableFlash;
    SetModified();
  }
  return true;
}

bool Settings::GetEnableThumbnails()
{
  return enableThumbnails;
}

bool Settings::SetEnableThumbnails(bool enableThumbnails)
{
  if (this->enableThumbnails != enableThumbnails)
  {
    this->enableThumbnails = enableThumbnails;
    SetModified();
  }
  return true;
}

int Settings::GetThumbnailAlpha()
{
  return thumbnailAlpha;
}

bool Settings::SetThumbnailAlpha(int thumbnailAlpha)
{
  if (this->thumbnailAlpha != thumbnailAlpha)
  {
    this->thumbnailAlpha = thumbnailAlpha;
    SetModified();
  }
  return true;
}

int Settings::GetThumbnailScale()
{
  return thumbnailScale;
}

bool Settings::SetThumbnailScale(int thumbnailScale)
{
  if (this->thumbnailScale != thumbnailScale)
  {
    this->thumbnailScale = thumbnailScale;
    SetModified();
  }
  return true;
}
