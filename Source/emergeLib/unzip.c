//----  --------------------------------------------------------------------------------------------------------
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2010  The Emerge Desktop Development Team
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
//
// Based on uzexampl.c included as part of Info-zip's unzip60f release.
//
//----  --------------------------------------------------------------------------------------------------------

#ifndef WIN32   /* this code is currently only tested for 32-bit console */
#  define WIN32
#endif

#if defined(__WIN32__) && !defined(WIN32)
#  define WIN32
#endif

/* Tell Microsoft Visual C++ 2005 to leave us alone and
 * let us use standard C functions the way we're supposed to.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  ifndef _CRT_NONSTDC_NO_WARNINGS
#    define _CRT_NONSTDC_NO_WARNINGS
#  endif
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include "zip.h"
#include "../unzip/unzvers.h"
#include "../unzip/windll/structs.h"

#ifndef _MAX_PATH
#  define _MAX_PATH 260           /* max total file or directory name path */
#endif

LPUSERFUNCTIONS lpUserFunctions;
HANDLE hUF = (HANDLE)NULL;
LPDCL lpDCL = NULL;
HANDLE hDCL = (HANDLE)NULL;
HANDLE hZCL = (HANDLE)NULL;


/* Forward References */
int WINAPI DisplayBuf(LPSTR, unsigned long);
int WINAPI GetReplaceDlgRetVal(LPSTR);
int WINAPI password(LPSTR, int, LPCSTR, LPCSTR);
void WINAPI ReceiveDllMessage(z_uint8 ucsize, z_uint8 csiz,
                              unsigned cfactor, unsigned mo, unsigned dy, unsigned yr,
                              unsigned hh, unsigned mm, char c, LPCSTR filename,
                              LPCSTR methbuf, unsigned long crc, char fCrypt);

extern int WINAPI Wiz_SingleEntryUnzip(int, char **, int, char **, LPDCL, LPUSERFUNCTIONS);

static void FreeUpMemory(void);

int ExtractZip(char *zipFile, char *unzipPath)
{
  int retcode;

  hDCL = GlobalAlloc( GPTR, (DWORD)sizeof(DCL));
  if (!hDCL)
    {
      return 0;
    }
  lpDCL = (LPDCL)GlobalLock(hDCL);
  if (!lpDCL)
    {
      GlobalFree(hDCL);
      return 0;
    }

  hUF = GlobalAlloc( GPTR, (DWORD)sizeof(USERFUNCTIONS));
  if (!hUF)
    {
      GlobalUnlock(hDCL);
      GlobalFree(hDCL);
      return 0;
    }
  lpUserFunctions = (LPUSERFUNCTIONS)GlobalLock(hUF);

  if (!lpUserFunctions)
    {
      GlobalUnlock(hDCL);
      GlobalFree(hDCL);
      GlobalFree(hUF);
      return 0;
    }

  lpUserFunctions->password = password;
  lpUserFunctions->print = DisplayBuf;
  lpUserFunctions->sound = NULL;
  lpUserFunctions->replace = GetReplaceDlgRetVal;
  lpUserFunctions->SendApplicationMessage = ReceiveDllMessage;

  lpDCL->StructVersID = UZ_DCL_STRUCTVER; /* version of this structure */
  lpDCL->ncflag = 0;              /* write to stdout if true */
  lpDCL->fQuiet = 0;              /* we want all messages
                                   1 = fewer messages,
                                   2 = no messages */
  lpDCL->ntflag = 0;              /* test zip file if true */
  lpDCL->nvflag = 0;              /* give a verbose listing if true */
  lpDCL->nzflag = 0;              /* display zip file comment if true */
  lpDCL->ndflag = 1;              /* recreate directories != 0,
                                   skip "../" if < 2 */
  lpDCL->naflag = 0;              /* do not convert CR to CRLF */
  lpDCL->nfflag = 0;              /* do not freshen existing files only */
  lpDCL->noflag = 1;              /* over-write all files if true */
  lpDCL->nZIflag = 0;             /* no ZipInfo output mode */
  lpDCL->B_flag = 0;              /* do not backup existing files */
  lpDCL->C_flag = 0;              /* do not match case-insensitive */
  lpDCL->D_flag = 0;              /* restore all timestamps */
  lpDCL->U_flag = 0;              /* do not disable UTF-8 support */
  lpDCL->ExtractOnlyNewer = 0;    /* do not extract only newer */
  lpDCL->SpaceToUnderscore = 0;   /* do not convert space to '_' in filenames */
  lpDCL->PromptToOverwrite = 0;   /* "overwrite all" selected -> no query mode */
  lpDCL->lpszZipFN = zipFile;     /* the archive name */
  lpDCL->lpszExtractDir = unzipPath;   /* the directory to extract to.
                                        This is set to NULL if you are extracting
                                        to the current directory.
                                        */

  retcode = Wiz_SingleEntryUnzip(0,         /* Number of additional zip files */
                                 NULL,      /* array of additional zip file names */
                                 0,         /* Number of excludes */
                                 NULL,      /* array of execludes */
                                 lpDCL,
                                 lpUserFunctions);

  FreeUpMemory();
  return retcode;
}

int WINAPI GetReplaceDlgRetVal(LPSTR filename UNUSED)
{
  /* This is where you will decide if you want to replace, rename etc existing
     files.
   */
  return 1;
}

static void FreeUpMemory(void)
{
  if (hDCL)
    {
      GlobalUnlock(hDCL);
      GlobalFree(hDCL);
    }
  if (hUF)
    {
      GlobalUnlock(hUF);
      GlobalFree(hUF);
    }
}

/* This is a very stripped down version of what is done in Wiz. Essentially
   what this function is for is to do a listing of an archive contents. It
   is actually never called in this example, but a dummy procedure had to
   be put in, so this was used.
 */
void WINAPI ReceiveDllMessage(z_uint8 ucsize, z_uint8 csiz,
                              unsigned cfactor, unsigned mo, unsigned dy, unsigned yr,
                              unsigned hh, unsigned mm, char c, LPCSTR filename,
                              LPCSTR methbuf UNUSED, unsigned long crc UNUSED, char fCrypt UNUSED)
{
  char psLBEntry[_MAX_PATH];
  char LongHdrStats[] =
    "%7lu  %7lu %4s  %02u-%02u-%02u  %02u:%02u  %c%s";
  char CompFactorStr[] = "%c%d%%";
  char CompFactor100[] = "100%%";
  char szCompFactor[10];
  char sgn;

  if (csiz > ucsize)
    sgn = '-';
  else
    sgn = ' ';
  if (cfactor == 100)
    lstrcpy(szCompFactor, CompFactor100);
  else
    sprintf(szCompFactor, CompFactorStr, sgn, cfactor);
  wsprintf(psLBEntry, LongHdrStats,
           ucsize, csiz, szCompFactor, mo, dy, yr, hh, mm, c, filename);

  printf("%s\n", psLBEntry);
}

/* Password entry routine - see password.c in the wiz directory for how
   this is actually implemented in WiZ. If you have an encrypted file,
   this will probably give you great pain.
 */
int WINAPI password(LPSTR p UNUSED, int n UNUSED, LPCSTR m UNUSED, LPCSTR name UNUSED)
{
  return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */
int WINAPI DisplayBuf(LPSTR buf, unsigned long size)
{
  printf("%s", (char *)buf);
  return (int)(unsigned int) size;
}
