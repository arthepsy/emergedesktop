//---
//
//  This file is part of Emerge Desktop.
//  Copyright (C) 2004-2007  The Emerge Desktop Development Team
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
//---

#include <winver.h>
#include <winuser.h>
#include <commctrl.h>

#define IDC_STATIC1             1001
#define IDC_STATIC2             1002
#define IDC_STATIC3             1003
#define IDC_STATIC4             1004
#define IDC_STATIC5             1005
#define IDC_STATIC6             1006
#define IDC_STATIC7             1007
#define IDC_STATIC8             1008
#define IDC_STATIC9             1009
#define IDC_STATIC10            1010
#define IDC_STATIC11            1011
#define IDC_STATIC12            1012
#define IDC_STATIC13            1013
#define IDC_STATIC14            1014
#define IDC_STATIC15            1015
#define IDC_STATIC16            1016
#define IDC_STATIC17            1017
#define IDC_STATIC18            1018
#define IDC_STATIC19            1019
#define IDC_STATIC20            1020
#define IDC_STATIC21            1021
#define IDC_STATIC22            1022
#define IDC_STATIC23            1023
#define IDC_STATIC24            1024
#define IDC_STATIC25            1025
#define IDC_STATIC26            1026
#define IDC_STATIC27            1027

#define IDD_STYLEEDITOR        100
#define IDC_PREVIEW             101
#define IDC_SAVEAS              102
#define IDC_LOAD                103
#define IDC_STYLENAME          104
#define IDC_DEFAULTS            105

#define IDC_ACTIVESLIDER        200
#define IDC_ACTIVEALPHA         201
#define IDC_INACTIVESLIDER      202
#define IDC_INACTIVEALPHA       203
#define IDC_BACKGROUNDSLIDER    204
#define IDC_BACKGROUNDALPHA     205
#define IDC_MENUSLIDER          206
#define IDC_MENUALPHA           207
#define IDC_FOREGROUNDSLIDER    208
#define IDC_FOREGROUNDALPHA     209
#define IDC_FRAMESLIDER         210
#define IDC_FRAMEALPHA          211
#define IDC_SELECTEDSLIDER      212
#define IDC_SELECTEDALPHA       213
#define IDC_BORDERSLIDER        214
#define IDC_BORDERALPHA         215
#define IDC_TEXTSLIDER          216
#define IDC_TEXTALPHA           217

#define IDC_BACKGROUNDCOLOUR    300
#define IDC_FOREGROUNDCOLOUR    301
#define IDC_SELECTEDCOLOUR      302
#define IDC_FRAMECOLOUR         303
#define IDC_FONTCOLOUR          304
#define IDC_BORDERCOLOUR        305

#define IDC_FONT                400
#define IDC_SHADOW              402
#define IDC_BEVEL               403
#define IDC_BORDER              404
#define IDC_PADDING             405
#define IDC_PADDINGUPDOWN       406
#define IDC_BEVELUPDOWN         407
#define IDC_BORDERUPDOWN        408

#define IDC_METHOD              500
#define IDC_INTERLACED          501
#define IDC_FLAT                502
#define IDC_RAISED              503
#define IDC_SUNKEN              504
#define IDC_FROMCOLOUR          505
#define IDC_TOCOLOUR            506
#define IDC_SWITCH              507

#define IDC_PANELTREE           600
