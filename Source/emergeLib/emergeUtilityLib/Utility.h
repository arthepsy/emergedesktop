/*!
  @file Utility.h
  @brief internal header for emergeLib
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2012  The Emerge Desktop Development Team

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

#ifndef __GUARD_91aa91ce_7711_488d_b55f_31dd6ebab3fb
#define __GUARD_91aa91ce_7711_488d_b55f_31dd6ebab3fb

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#include <windows.h>
#include <time.h>
#include <fstream>
#include "../emergeCoreLib.h"
#include "../emergeFileRegistryLib.h"
#include "../emergeOSLib.h"
#include "../emergeUtilityLib.h"
#include "MsgBox.h"
#include "unzip.h"
#include "zip.h"

//Helper functions
void ZipAddDir(HZIP hz, std::wstring relativePath, std::wstring zipPath);

#endif
