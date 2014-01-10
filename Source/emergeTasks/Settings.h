/*!
  @file Settings.h
  @brief header for emergeTasks
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef __GUARD_92614c31_e189_4d3e_8d22_22766cd17452
#define __GUARD_92614c31_e189_4d3e_8d22_22766cd17452

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
