/*
  Copyright (c) 1990-2009 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-02 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
   unzvers.h (for UnZip) by Info-ZIP.
 */

#ifndef __unzvers_h     /* don't include more than once */
#define __unzvers_h

#ifndef BETA
#  define BETA          /* undefine BETA for public releases */
#endif

#ifdef BETA
#  define UZ_BETALEVEL      "f BETA"
#  define UZ_VERSION_DATE   "11 Jan 09"         /* internal beta version */
#else
#  define UZ_BETALEVEL      ""
#  define UZ_VERSION_DATE   "20 January 2009"   /* official release version */
#  define RELEASE
#endif

#define UZ_MAJORVER    6   /* UnZip */
#define UZ_MINORVER    0

#define ZI_MAJORVER    3   /* ZipInfo */
#define ZI_MINORVER    0

#define UZ_PATCHLEVEL  0

#define UZ_VER_STRING  "6.0"          /* keep in sync with Version numbers! */

#ifndef IZ_COMPANY_NAME               /* might be already defined... */
#  define IZ_COMPANY_NAME "Info-ZIP"
#endif

/* these are obsolete but remain for backward compatibility: */
#if (defined(OS2) || defined(__OS2__))
#  define D2_MAJORVER    UZ_MAJORVER    /* DLL for OS/2 */
#  define D2_MINORVER    UZ_MINORVER
#  define D2_PATCHLEVEL  UZ_PATCHLEVEL
#endif

#define DW_MAJORVER    UZ_MAJORVER    /* DLL for MS Windows */
#define DW_MINORVER    UZ_MINORVER
#define DW_PATCHLEVEL  UZ_PATCHLEVEL

#define WIN_VERSION_DATE  UZ_VERSION_DATE

#define UNZ_DLL_VERSION   UZ_VER_STRING

#endif /* !__unzvers_h */
