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

#include "Utility.h"

void ELWriteDebug(std::wstring debugText)
{
  std::wfstream out;
  std::string filename;
  std::wstring wideFilename;

  wideFilename = ELGetUserDataPath() + TEXT("\\Debug.txt");
  filename = ELwstringTostring(wideFilename);

  out.open(filename.c_str(), std::wfstream::out | std::wfstream::app);
  out << ELGetProcessIDApp(GetCurrentProcessId(), false) << TEXT(": ") << debugText << std::endl;
  out.close();
}

/*!
  @fn ELMid(int left, int right)
  @brief Returns the mid point between two sides.
  @param left value of one side
  @param right value of other side
  */

int ELMid(int left, int right)
{
  int middle;

  if (right > 0)
  {
    middle = (left - right) / 2;
  }
  else
  {
    middle = (left + right) / 2;
  }

  return middle;
}

/** \brief Augments wcsftime to support '%u' and '%V' as defined by glibc.
 *
 * \note ELwcsftime is based on the pseudo code found here: http://www.personal.ecu.edu/mccartyr/ISOwdALG.txt
 * \note This function is left with WCHAR parameters rather than std::wstring to better match the original wcsftime function
 *
 * \param strDest - destination string
 * \param maxsize - the size of destination string
 * \param format - format string
 * \param timptr - pointer to the current time in a 'tm' struct
 * \return the new size of strDest
 *
 */
std::wstring ELwcsftime(std::wstring format, const struct tm* timeptr)
{
  int year = timeptr->tm_year + 1900;
  int previousYear = year - 1;
  int month = timeptr->tm_mon + 1;
  int day = timeptr->tm_mday;
  int yy, c, g, jan1Weekday;
  int monthDay[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int dayOfYear, weekday, h, yearNumber, weekNumber, i;
  bool isLeapYear = false, previousIsLeapYear = false;
  WCHAR stringDay[2], paddedStringWeek[3], stringWeek[3];
  char tmpFormat[MAX_LINE_LENGTH], tmpDest[MAX_LINE_LENGTH];
  WCHAR strDest[MAX_LINE_LENGTH], formatBuffer[MAX_LINE_LENGTH];
  BOOL defaultUsed;

  /**< Determine if the current year is a leap year */
  if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
  {
    isLeapYear = true;
  }

  /**< Determine if the previous year is a leap year */
  if (((previousYear % 4 == 0) && (previousYear % 100 != 0)) || (previousYear % 400 == 0))
  {
    previousIsLeapYear = true;
  }

  /**< Determine the dayOfYear for the current date */
  dayOfYear = day + monthDay[month - 1];
  if (isLeapYear && (month > 2))
  {
    dayOfYear++;
  }

  /**< Determine jan1Weekday for the current year (Monday = 1, Sunday = 7) */
  yy = previousYear % 100;
  c = previousYear - yy;
  g = yy + (yy / 4);
  jan1Weekday = 1 + (((((c / 100) % 4) * 5) + g) % 7);

  /**< Determine the current weekday (Monday = 1, Sunday = 7) */
  h = dayOfYear + (jan1Weekday - 1);
  weekday = 1 + ((h - 1) % 7);

  /**< Determine the current date falls in previous year, week numbers 52 or 53 */
  if ((dayOfYear <= (8 - jan1Weekday)) && (jan1Weekday > 4))
  {
    yearNumber = previousYear;
    if ((jan1Weekday == 5) || ((jan1Weekday == 6) && previousIsLeapYear))
    {
      weekNumber = 53;
    }
    else
    {
      weekNumber = 52;
    }
  }
  else
  {
    yearNumber = year;
  }

  /**< Determine if the current date falls in the previous year, week number 1 */
  if (yearNumber == year)
  {
    if (isLeapYear)
    {
      i = 366;
    }
    else
    {
      i = 365;
    }

    if ((i - dayOfYear) < (4 - weekday))
    {
      yearNumber = year + 1;
      weekNumber = 1;
    }
  }

  /**< Determine if the current date falls in the current year, week numbers 1 through 53 */
  if (yearNumber == year)
  {
    i = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    weekNumber = i / 7;
    if (jan1Weekday > 4)
    {
      weekNumber--;
    }
  }

  /**< Convert weekday and weekNumber into strings */
  wsprintf(stringDay, TEXT("%d"), weekday);
  wsprintf(stringWeek, TEXT("%d"), weekNumber);
  wsprintf(paddedStringWeek, TEXT("%02d"), weekNumber);

  wcscpy(formatBuffer, format.c_str());
  /**< Replace and occurances of '%u' and '%V' appropriately */
  ELStringReplace(formatBuffer, (WCHAR*)TEXT("%u"), stringDay, false);
  ELStringReplace(formatBuffer, (WCHAR*)TEXT("%V"), paddedStringWeek, false);
  ELStringReplace(formatBuffer, (WCHAR*)TEXT("%#V"), stringWeek, false);

  /**< Convert to UTF-8 to pass to strftime due to issues with wcsftime */
  WideCharToMultiByte(CP_ACP, 0, formatBuffer, wcslen(formatBuffer) + 1,
                      tmpFormat, MAX_LINE_LENGTH, NULL, &defaultUsed);

  strftime(tmpDest, MAX_LINE_LENGTH, tmpFormat, timeptr);

  /**< Convert back to unicode */
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, tmpDest, (int)strlen(tmpDest) + 1,
                      strDest, MAX_LINE_LENGTH);

  return strDest;
}

int ELMessageBox(HWND hwnd, std::wstring messageText, std::wstring messageTitle, DWORD msgFlags)
{
  int ret;

  MsgBox* msgBox = new MsgBox(ELGetEmergeLibrary(TEXT("emergeLib.dll")), hwnd, messageText, messageTitle,
                              (msgFlags & 0x0000ff), (msgFlags & 0x00ff00));
  ret = msgBox->Show((msgFlags & ELMB_MODAL) == ELMB_MODAL);

  return ret;
}

int ELMakeZip(std::wstring zipFile, std::wstring zipRoot, std::wstring zipPath)
{
  std::wstring relativePath;

  zipFile = ELExpandVars(zipFile);
  zipRoot = ELExpandVars(zipRoot);
  zipPath = ELExpandVars(zipPath);

  relativePath = zipPath.substr(zipPath.rfind('\\') + 1);

  HZIP hz = CreateZip(zipFile.c_str(), 0);
  if (hz)
  {
    ZipAddFolder(hz, relativePath.c_str());
    ZipAddDir(hz, relativePath, zipPath);

    CloseZip(hz);
  }

  return 0;
}

int ELExtractZip(std::wstring zipFile, std::wstring unzipPath)
{
  std::wstring tmpPath = unzipPath + TEXT("\\"), themeName;
  tmpPath = ELExpandVars(tmpPath);

  HZIP hz = OpenZip(zipFile.c_str(), 0);

  if (hz)
  {
    ZIPENTRY ze;

    unzipPath = ELExpandVars(unzipPath);
    SetUnzipBaseDir(hz, unzipPath.c_str());

    GetZipItem(hz, -1, &ze); // -1 gives overall information about the zipfile
    int numitems = ze.index;

    for (int zi = 0; zi < numitems; zi++)
    {
      GetZipItem(hz, zi, &ze); // fetch individual details

      if (zi == 0)
      {
        tmpPath += ze.name;

        if (ze.attr == FILE_ATTRIBUTE_DIRECTORY)
        {
          if ((ELGetFileSpecialFlags(tmpPath) & SF_DIRECTORY) == SF_DIRECTORY)
          {
            themeName = ze.name;
            themeName = themeName.substr(0, themeName.rfind('/'));
            WCHAR message[MAX_LINE_LENGTH];
            swprintf(message, TEXT("Do you want to overwrite the '%ls' theme?"), themeName.c_str());
            if (ELMessageBox(NULL, message, TEXT("Warning"),
                             ELMB_YESNO | ELMB_ICONQUESTION) == IDNO)
            {
              return 2;
            }
          }
        }
      }

      UnzipItem(hz, zi, ze.name); // e.g. the item's name.
    }

    CloseZip(hz);

    return 0;
  }

  return 1;
}

void ZipAddDir(HZIP hz, std::wstring relativePath, std::wstring zipPath)
{
  WIN32_FIND_DATA findData;
  HANDLE fileHandle;
  std::wstring tmpPath, searchPath, tmpRelativePath;

  searchPath = zipPath + TEXT("\\*");

  fileHandle = FindFirstFile(searchPath.c_str(), &findData);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    return;
  }

  do
  {
    // Skip hidden files
    if (wcscmp(findData.cFileName, TEXT(".")) == 0 ||
        wcscmp(findData.cFileName, TEXT("..")) == 0 ||
        (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
    {
      continue;
    }

    tmpPath = zipPath + TEXT("\\");
    tmpPath += findData.cFileName;

    if (!relativePath.empty())
    {
      tmpRelativePath = relativePath + TEXT("\\");
    }
    tmpRelativePath += findData.cFileName;

    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      ZipAddFolder(hz, tmpRelativePath.c_str());
      ZipAddDir(hz, tmpRelativePath, tmpPath);
    }
    else
    {
      ZipAdd(hz, tmpRelativePath.c_str(), tmpPath.c_str());
    }
  }
  while (FindNextFile(fileHandle, &findData));

  FindClose(fileHandle);
}
