/*!
  @file FileRegistryIO.h
  @brief export header for emergeLib
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

#ifndef __FILEREGISTRYIO_H
#define __FILEREGISTRYIO_H

#define UNICODE 1

#define MAX_LINE_LENGTH 4096

#ifdef __GNUC__
#include <tr1/memory>
#include <tr1/shared_ptr.h>
#else
#include <memory>
#endif

#include <windows.h>
#include <shlwapi.h>
#include <string>
#include "emergeFileRegistryLib.h"
#include "emergeLib.h"
#include "emergeUtilityLib.h"
#include "../tinyxml/tinyxml.h"

//Helper functions
std::tr1::shared_ptr<TiXmlDocument> OpenXMLConfig(std::string filename, bool create);
std::wstring ReadValue(std::wstring fileName, std::wstring keyword);
bool WriteValue(std::wstring fileName, std::wstring keyword, std::wstring value);

#endif
