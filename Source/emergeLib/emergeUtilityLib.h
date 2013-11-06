/*!
  @file emergeUtilityLib.h
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

#ifndef __GUARD_f689f691_555c_4e90_924d_372089b43057
#define __GUARD_f689f691_555c_4e90_924d_372089b43057

#define UNICODE 1

#ifdef EMERGELIB_EXPORTS
#undef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllexport)
#else
#ifndef DLL_EXPORT
#define DLL_EXPORT  __declspec(dllimport)
#endif
#endif

#ifndef MAX
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#include <windows.h>
#include <iomanip>
#include <sstream>
#include <string>

typedef enum _ELMBFLAGS
{
  ELMB_ICONINFORMATION = 0x00,
  ELMB_ICONWARNING,
  ELMB_ICONQUESTION,
  ELMB_ICONERROR,
  ELMB_OK = 0x0100,
  ELMB_YESNO = 0x0200,
  ELMB_MODAL = 0x010000
} ELMBFLAGS;

template <class T>
inline std::wstring towstring(const T& t)
{
  std::wostringstream ostr;
  ostr << t;
  return ostr.str();
}
template <class T>
inline std::wstring towstring(const T& t, int precision)
{
  std::wostringstream ostr;
  ostr << std::fixed << std::setprecision(precision) << t;
  return ostr.str();
}
template <class T>
inline T fromwstring(const std::wstring& s)
{
  T t;
  std::wistringstream istr(s);
  istr >> t;
  return t;
}

//Utility.h
DLL_EXPORT void ELWriteDebug(std::wstring debugText);
DLL_EXPORT int ELMid(int left, int right);
DLL_EXPORT std::wstring ELwcsftime(std::wstring format, const struct tm* timeptr);
DLL_EXPORT int ELMessageBox(HWND hwnd, std::wstring messageText, std::wstring messageTitle, DWORD msgFlags);
DLL_EXPORT int ELMakeZip(std::wstring zipFile, std::wstring zipRoot, std::wstring zipPath);
DLL_EXPORT int ELExtractZip(std::wstring zipFile, std::wstring unzipPath);

//StringOps.h
DLL_EXPORT UINT ELStringReplace(WCHAR* original, const WCHAR* pattern, const WCHAR* replacement, bool ignoreCase);
DLL_EXPORT std::wstring ELwstringReplace(std::wstring original, std::wstring pattern, std::wstring replacement, bool ignoreCase);
DLL_EXPORT std::wstring ELToLower(std::wstring workingString);
DLL_EXPORT std::wstring ELStripLeadingSpaces(std::wstring input);
DLL_EXPORT std::string ELwstringTostring(std::wstring wideString, UINT codePage = CP_UTF8);
DLL_EXPORT std::wstring ELstringTowstring(std::string narrowString, UINT codePage = CP_UTF8);

#endif
