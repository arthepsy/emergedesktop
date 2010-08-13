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
// Based on example.c included as part of Info-zip's zip31c release.
//
//----  --------------------------------------------------------------------------------------------------------

#define API

/* Tell Microsoft Visual C++ 2005 to leave us alone and
 * let us use standard C functions the way we're supposed to.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  ifndef _CRT_SECURE_NO_DEPRECATE
#    define _CRT_SECURE_NO_DEPRECATE
#  endif
#  ifndef _CRT_NONSTDC_NO_DEPRECATE
#    define _CRT_NONSTDC_NO_DEPRECATE
#  endif
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include "../zip/windll/structs.h"
#include <windows.h>
#include "zip.h"
#include "../zip/revision.h"
/* printf get redirected by a macro define in api.h !!! */
#ifdef printf
#undef printf
#endif

/* Global variables */

extern LPZIPUSERFUNCTIONS lpZipUserFunctions;
ZCL ZpZCL;
LPZIPUSERFUNCTIONS lpZipUserFunctions;
HANDLE hZUF = (HANDLE)NULL;
HANDLE hFileList;
ZPOPT ZpOpt;

/* Global functions */

extern int WINAPI ZpArchive(ZCL C, LPZPOPT Opts);
extern int WINAPI ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc);

/* Forward References */

void FreeUpMemory(void);
int WINAPI DummyPassword(LPSTR, int, LPCSTR, LPCSTR);
int WINAPI DummyPrint(char far *, unsigned long);
int WINAPI WINAPI DummyComment(char far *);

/****************************************************************************

FUNCTION: Main(int argc, char **argv)

 ****************************************************************************/
int MakeZip(char *filename, char *root, char *path)
{
  /*char szFileList[1][MAX_PATH];
    char testPath[MAX_PATH];*/
  LPSTR szFileList = NULL;
  char **index, *sz;
  int retcode, i, cc;
  char szFullPath[PATH_MAX];

  hZUF = GlobalAlloc( GPTR, (DWORD)sizeof(ZIPUSERFUNCTIONS));
  if (!hZUF)
    {
      return 0;
    }
  lpZipUserFunctions = (LPZIPUSERFUNCTIONS)GlobalLock(hZUF);

  if (!lpZipUserFunctions)
    {
      GlobalFree(hZUF);
      return 0;
    }

  lpZipUserFunctions->print = DummyPrint;
  lpZipUserFunctions->password = DummyPassword;
  lpZipUserFunctions->comment = DummyComment;

  if (!ZpInit(lpZipUserFunctions))
    {
      printf("Application functions not set up properly. Terminating...");
      FreeUpMemory();
      return 0;
    }

  /* Here is where the action starts */
  memset(&ZpOpt, 0, sizeof(ZpOpt));
  ZpOpt.ExcludeBeforeDate = NULL;    /* set to valid Zip date, or NULL */
  ZpOpt.IncludeBeforeDate = NULL;    /* set to valid Zip date, or NULL */
  ZpOpt.szRootDir = root;            /* set to root dir (will cd to), or NULL */
  ZpOpt.szTempDir = NULL;            /* set to dir for temp files, or NULL */
  ZpOpt.fUnicode = 4;                /* Unicode flag */
  ZpOpt.fEncrypt = FALSE;            /* Encrytion flag */
  ZpOpt.fSystem = FALSE;             /* true to include system/hidden files */
  ZpOpt.fVolume = FALSE;             /* true if storing volume label */
  ZpOpt.fExtra = FALSE;              /* true if including extra attributes */
  ZpOpt.fNoDirEntries = FALSE;       /* true if ignoring directory entries */
  ZpOpt.fVerbose = FALSE;            /* true if full messages wanted */
  ZpOpt.fQuiet = TRUE;               /* true if minimum messages wanted */
  ZpOpt.fCRLF_LF = FALSE;            /* true if translate CR/LF to LF */
  ZpOpt.fLF_CRLF = FALSE;            /* true if translate LF to CR/LF */
  ZpOpt.fJunkDir = FALSE;            /* true if junking directory names */
  ZpOpt.fGrow = FALSE;               /* true if allow appending to zip file */
  ZpOpt.fForce = FALSE;              /* true if making entries using DOS names */
  ZpOpt.fMove = FALSE;               /* true if deleting files added or updated */
  ZpOpt.fDeleteEntries = FALSE;      /* true if deleting files from archive */
  ZpOpt.fUpdate = FALSE;             /* true if updating zip file--overwrite only
                                        if newer */
  ZpOpt.fFreshen = FALSE;            /* true if freshening zip file--overwrite only */
  ZpOpt.fJunkSFX = FALSE;            /* true if junking sfx prefix*/
  ZpOpt.fLatestTime = FALSE;         /* true if setting zip file time to time of
                                        latest file in archive */
  ZpOpt.fComment = FALSE;            /* true if putting comment in zip file */
  ZpOpt.fOffsets = FALSE;            /* true if updating archive offsets for sfx
                                        files */
  ZpOpt.fPrivilege = 0;
  ZpOpt.fEncryption = 0;
  ZpOpt.szSplitSize = NULL;

  ZpOpt.szIncludeList = NULL;
  ZpOpt.IncludeListCount = 0;
  ZpOpt.IncludeList = NULL;
  ZpOpt.szExcludeList = NULL;
  ZpOpt.ExcludeListCount = 0;
  ZpOpt.ExcludeList = NULL;

  ZpOpt.fRecurse = 1;           /* subdir recursing mode: 1 = "-r", 2 = "-R" */
  ZpOpt.fRepair = 0;            /* archive repair mode: 1 = "-F", 2 = "-FF" */
  ZpOpt.fLevel = '6';           /* Default deflate compression level */
  ZpOpt.szCompMethod = NULL;
  for (i = 0; i < 8; i++) {
    ZpOpt.fluff[i] = 0;
  }
  getcwd(szFullPath, PATH_MAX); /* Set directory to current directory */

  ZpZCL.argc = 1;
  /*ZpZCL.argc = argc - 2;*/        /* number of files to archive - adjust for the
                                       actual number of file names to be added */
  ZpZCL.lpszZipFN = filename;    /* archive to be created/updated */

  /* Copy over the appropriate portions of argv, basically stripping out argv[0]
     (name of the executable) and argv[1] (name of the archive file)
     */
  hFileList = GlobalAlloc( GPTR, 0x10000L);
  if ( hFileList )
    {
      szFileList = (char far *)GlobalLock(hFileList);
    }
  index = (char **)szFileList;
  cc = (sizeof(char *) * ZpZCL.argc);
  sz = szFileList + cc;

  cc = lstrlen(path);
  lstrcpy(sz, path);
  index[0] = sz;
  sz += (cc + 1);

  ZpZCL.FNV = (char **)szFileList;  /* list of files to archive */

  /*lstrcpy(szFileList[0], path);
    ZpZCL.FNV = (char **)szFileList;*/

  /* Go zip 'em up */
  retcode = ZpArchive(ZpZCL, &ZpOpt);

  GlobalUnlock(hFileList);
  GlobalFree(hFileList);
  FreeUpMemory();
  return retcode;
}

void FreeUpMemory(void)
{
  if (hZUF)
    {
      GlobalUnlock(hZUF);
      GlobalFree(hZUF);
    }
}

/* Password entry routine - see password.c in the wiz directory for how
   this is actually implemented in Wiz. If you have an encrypted file,
   this will probably give you great pain. Note that none of the
   parameters are being used here, and this will give you warnings.
   */
int WINAPI DummyPassword(LPSTR p UNUSED, int n UNUSED, LPCSTR m UNUSED, LPCSTR name UNUSED)
{
  return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */
int WINAPI DummyPrint(char far *buf, unsigned long size)
{
  printf("%s", buf);
  return (unsigned int) size;
}


/* Dummy "comment" routine. See comment.c in the wiz directory for how
   this is actually implemented in Wiz. This will probably cause you
   great pain if you ever actually make a call into it.
   */
int WINAPI DummyComment(char far *szBuf)
{
  szBuf[0] = '\0';
  return TRUE;
}
