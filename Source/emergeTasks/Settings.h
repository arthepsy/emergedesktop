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

#ifndef __ET_SETTINGS_H
#define __ET_SETTINGS_H

#define UNICODE 1

#include "../emergeBaseClasses/BaseSettings.h"

class Settings : public BaseSettings
{
public:
  Settings();
  bool GetHiliteActive();
  bool SetHiliteActive(bool autoSize);
  bool GetSameMonitorOnly(); //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  bool SetSameMonitorOnly(bool sameMonitorOnly); //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  bool SetFlashCount(int flashCount);
  bool SetFlashInterval(int flashInterval);
  bool SetEnableFlash(bool enableFlash);
  bool SetEnableThumbnails(bool enableThumbnails);
  int GetFlashCount();
  int GetFlashInterval();
  bool GetEnableFlash();
  bool GetEnableThumbnails();
  int GetThumbnailAlpha();
  bool SetThumbnailAlpha(int thumbnailAlpha);
  int GetThumbnailScale();
  bool SetThumbnailScale(int thumbnailScale);

protected:
  virtual void DoReadSettings(IOHelper& helper);
  virtual void DoWriteSettings(IOHelper& helper);
  virtual void ResetDefaults();

private:
  bool hiliteActive, enableFlash, sameMonitorOnly; //ROBLARKY - 2012-08-11: Added for option to only show tasks on same monitor
  int flashInterval, flashCount;
  bool enableThumbnails;
  int thumbnailAlpha, thumbnailScale;
};

#endif
