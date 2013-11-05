/*!
  @file main.h
  @brief header for emergeStyleEngine
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

#ifndef __EMERGESTYLEENGINEMAIN_H
#define __EMERGESTYLEENGINEMAIN_H

#define UNICODE 1

#undef _WIN32_IE
#define _WIN32_IE 0x0501

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#undef WINVER
#define WINVER 0x0501

#include <string>
#include "../emergeLib/emergeCoreLib.h"
#include "../emergeLib/emergeFileRegistryLib.h"
#include "../emergeLib/emergeUtilityLib.h"
#include "emergeStyleEngine.h"
//#include "BImage.h"
#include "StyleEditor.h"

// Globals
std::wstring style;
HBRUSH bgBrush = NULL;

// Globals
StyleEditor *pStyleEditor;

#endif
