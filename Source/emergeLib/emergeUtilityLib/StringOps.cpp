//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2013  The Emerge Desktop Development Team
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

#include "StringOps.h"

UINT ELStringReplace(WCHAR* original, const WCHAR* pattern, const WCHAR* replacement, bool ignoreCase)
{
  std::wstring workingOrig = original, workingRepl = replacement;
  std::wstring lowerOrig = original, lowerPat = pattern;
  size_t i = 0;
  UINT substitutions = 0;

  if (ignoreCase)
  {
    std::transform(lowerOrig.begin(), lowerOrig.end(), lowerOrig.begin(), (int(*)(int)) std::tolower);
    std::transform(lowerPat.begin(), lowerPat.end(), lowerPat.begin(), (int(*)(int)) std::tolower);
  }

  i = lowerOrig.find(lowerPat, i);
  while (i != std::wstring::npos)
  {
    workingOrig.replace(i, lowerPat.length(), workingRepl);
    lowerOrig.replace(i, lowerPat.length(), workingRepl);
    i += workingRepl.length();
    i = lowerOrig.find(lowerPat, i);
    substitutions++;
  }

  wcscpy(original, workingOrig.c_str());

  return substitutions;
}

std::wstring ELwstringReplace(std::wstring original, std::wstring pattern, std::wstring replacement, bool ignoreCase)
{
  std::wstring lowerOrig = original, lowerPat = pattern, returnValue = original;
  size_t i = 0;

  if (ignoreCase)
  {
    std::transform(lowerOrig.begin(), lowerOrig.end(), lowerOrig.begin(), (int(*)(int)) std::tolower);
    std::transform(lowerPat.begin(), lowerPat.end(), lowerPat.begin(), (int(*)(int)) std::tolower);
  }

  i = lowerOrig.find(lowerPat, i);
  while (i != std::wstring::npos)
  {
    returnValue.replace(i, pattern.length(), replacement);
    lowerOrig.replace(i, lowerPat.length(), replacement);
    i += replacement.length();
    i = lowerOrig.find(lowerPat, i);
  }

  return returnValue;
}

std::wstring ELToLower(std::wstring workingString)
{
  std::transform(workingString.begin(), workingString.end(), workingString.begin(), (int(*)(int)) std::tolower);
  return workingString;
}

std::string ELwstringTostring(std::wstring inString, UINT codePage)
{
  std::wstring wideString = inString;
  std::string returnString;

  size_t tmpStringLength = WideCharToMultiByte(codePage, 0, wideString.c_str(), wideString.length(), NULL, 0,
                           NULL, NULL);
  if (tmpStringLength != 0)
  {
    char* tmpString = new char[tmpStringLength + 1];
    size_t writtenBytes = WideCharToMultiByte(codePage, 0, wideString.c_str(), wideString.length(), tmpString,
                          tmpStringLength, NULL, NULL);
    if (writtenBytes != 0)
    {
      if (writtenBytes <= tmpStringLength)
      {
        tmpString[writtenBytes] = '\0';
      }
      returnString = tmpString;
    }
    delete [] tmpString;
  }
  return returnString;
}

std::wstring ELstringTowstring(std::string inString, UINT codePage)
{
  std::string narrowString = inString;
  std::wstring returnString;

  size_t tmpStringLength = MultiByteToWideChar(codePage, 0, narrowString.c_str(), narrowString.length(), NULL, 0);
  if (tmpStringLength != 0)
  {
    LPWSTR tmpString = new WCHAR[tmpStringLength + 1];
    size_t writtenBytes = MultiByteToWideChar(codePage, 0, narrowString.c_str(), narrowString.length(), tmpString,
                          tmpStringLength);
    if (writtenBytes != 0)
    {
      if (writtenBytes <= tmpStringLength)
      {
        tmpString[writtenBytes] = '\0';
      }
      returnString = tmpString;
    }
    delete [] tmpString;
  }

  return returnString;
}

std::wstring ELStripLeadingSpaces(std::wstring input)
{
  size_t i = 0;

  //< Search input for the first non-space character
  while (i < input.length())
  {
    if (input.at(i) != ' ')
    {
      break;
    }

    i++;
  }

  return input.substr(i, std::wstring::npos);
}
