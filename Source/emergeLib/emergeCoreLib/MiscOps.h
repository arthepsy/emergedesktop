/*!
  @file MiscOps.h
  @brief internal header for emergeCoreLib
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

#ifndef __GUARD_d800fce9_edaf_4519_a4a6_ccb867530405
#define __GUARD_d800fce9_edaf_4519_a4a6_ccb867530405

#define UNICODE 1

#ifndef SND_SYSTEM
#define SND_SYSTEM      0x200000
#endif

#include <windows.h>
#include "../emergeCoreLib.h"
#include "../emergeFileRegistryLib.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "../emergeWindowLib.h"

#if defined (__MINGW32__)
#include <commctrl.h> //needed for MinGWInterfaces.h to not break
#include "../../MinGWInterfaces.h"
#else
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>
#define _MMDeviceEnumerator MMDeviceEnumerator
#define _IMMDeviceEnumerator IMMDeviceEnumerator
#define _IAudioEndpointVolume IAudioEndpointVolume
#endif

//Helper functions
BOOL CALLBACK ThemeEnum(HWND hwnd, LPARAM lParam);
bool VistaVolumeControl(VOLUMEFLAGS command);
bool VolumeControl(VOLUMEFLAGS command);

#endif
