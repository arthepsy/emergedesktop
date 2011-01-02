// vim: tags+=../emergeLib/tags
//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2009-2011  The Emerge Desktop Development Team
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

#include "TasksSettings.h"

TasksSettings::TasksSettings(HKEY key)
  :AppletSettings(key)
{
  appletName = TEXT("emergeTasks.xml");
}

void TasksSettings::ConvertSettings(IOHelper &keyHelper, IOHelper &xmlHelper)
{
  bool hiliteActive, enableFlash;
  int flashInterval, flashCount;

  AppletSettings::ConvertSettings(keyHelper, xmlHelper);

  // Read Tasks specific settings from the registry
  keyHelper.ReadBool(TEXT("HighLightActive"), hiliteActive, true);
  keyHelper.ReadInt(TEXT("FlashCount"), flashCount, 0);
  keyHelper.ReadBool(TEXT("EnableFlash"), enableFlash, true);
  keyHelper.ReadInt(TEXT("FlashInterval"), flashInterval, 1000);

  // Write Tasks specific settings to the XML file
  xmlHelper.WriteBool(TEXT("HighLightActive"), hiliteActive);
  xmlHelper.WriteInt(TEXT("FlashCount"), flashCount);
  xmlHelper.WriteBool(TEXT("EnableFlash"), enableFlash);
  xmlHelper.WriteInt(TEXT("FlashInterval"), flashInterval);
}
