/*
   Copyright (c) 1990-2010 Info-ZIP.  All rights reserved.

   See the accompanying file LICENSE, version 2009-Jan-02 or later
   (the contents of which are also included in zip.h) for terms of use.
   If, for some reason, all these files are missing, the Info-ZIP license
   also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
   */
/*
   A very simplistic example of how to load the zip dll and make a call into it.
   Note that none of the command line options are implemented in this example.

*/

#ifndef WIN32
#  define WIN32
#endif
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

/*#include <stdlib.h>
#include <stdio.h>
#include <commdlg.h>
#include <dlgs.h>*/

#ifndef EXPENTRY
#define EXPENTRY WINAPI
#endif

#include "../zip/windll/structs.h"

/* Defines */
#ifndef MSWIN
#define MSWIN
#endif

typedef int (WINAPI * _DLL_ZIP)(ZCL, LPZPOPT);
typedef int (WINAPI * _ZIP_USER_FUNCTIONS)(LPZIPUSERFUNCTIONS);

/* Global variables */

extern LPZIPUSERFUNCTIONS lpZipUserFunctions;

extern HINSTANCE hZipDll;

extern int hFile;                 /* file handle             */

/* Global functions */

extern int WINAPI ZpArchive(ZCL C, LPZPOPT Opts);
extern int WINAPI ZpInit(LPZIPUSERFUNCTIONS lpZipUserFunc);

#include <assert.h>    /* required for all Windows applications */
#include <windowsx.h>
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#ifdef __BORLANDC__
#include <dir.h>
#else
#include <direct.h>
#endif
#include "zip.h"
#include "../zip/revision.h"
/* printf get redirected by a macro define in api.h !!! */
#ifdef printf
# undef printf
#endif

#ifdef WIN32
#include <commctrl.h>
#include <winver.h>
#else
#include <ver.h>
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#ifdef WIN32
#define ZIP_DLL_NAME "ZIP32Z64.DLL\0"
#else
#define ZIP_DLL_NAME "ZIP16.DLL\0"
#endif

#define DLL_WARNING "Cannot find %s."\
  " The Dll must be in the application directory, the path, "\
"the Windows directory or the Windows System directory."
#define DLL_VERSION_WARNING "%s has the wrong version number."\
  " Insure that you have the correct dll's installed, and that "\
"an older dll is not in your path or Windows System directory."

int hFile;              /* file handle */

ZCL ZpZCL;
LPZIPUSERFUNCTIONS lpZipUserFunctions;
HANDLE hZUF = (HANDLE)NULL;
HINSTANCE hUnzipDll;
HANDLE hFileList;
ZPOPT ZpOpt;
#ifdef WIN32
DWORD dwPlatformId = 0xFFFFFFFF;
#endif
HINSTANCE hZipDll;

/* Forward References */
_DLL_ZIP ZipArchive;
_ZIP_USER_FUNCTIONS ZipInit;

void FreeUpMemory(void);
int WINAPI DummyPassword(LPSTR, int, LPCSTR, LPCSTR);
int WINAPI DummyPrint(char far *, unsigned long);
int WINAPI WINAPI DummyComment(char far *);

#ifdef WIN32
BOOL IsNT(VOID);
#endif

/****************************************************************************

FUNCTION: Main(int argc, char **argv)

 ****************************************************************************/
#ifdef __BORLANDC__
#  ifdef WIN32
#pragma argsused
#  endif
#endif
int MakeZip(char *filename, char *root, char *path)
{
  /*char szFileList[1][MAX_PATH];
    char testPath[MAX_PATH];*/
  LPSTR szFileList;
  char **index, *sz;
  int retcode, i, cc;
  DWORD dwVerInfoSize;
  DWORD dwVerHnd;
  char szFullPath[PATH_MAX];
#ifdef WIN32
  char *ptr;
#else
  HFILE hfile;
  OFSTRUCT ofs;
#endif
  HANDLE  hMem;         /* handle to mem alloc'ed */

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
  if (retcode != 0)
    printf("Error in archiving\n");


  GlobalUnlock(hFileList);
  GlobalFree(hFileList);
  FreeUpMemory();
  FreeLibrary(hZipDll);
  return 1;
}

void FreeUpMemory(void)
{
  if (hZUF)
    {
      GlobalUnlock(hZUF);
      GlobalFree(hZUF);
    }
}

#ifdef WIN32
/* This simply determines if we are running on NT */
BOOL IsNT(VOID)
{
  if(dwPlatformId != 0xFFFFFFFF)
    return dwPlatformId;
  else
    /* note: GetVersionEx() doesn't exist on WinNT 3.1 */
    {
      if(GetVersion() < 0x80000000)
        {
          dwPlatformId = TRUE;
        }
      else
        {
          dwPlatformId = FALSE;
        }
    }
  return dwPlatformId;
}
#endif

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
