//---
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
//---

#include <winver.h>
#include <winuser.h>
#include <commctrl.h>

#define MINIMUM_UPDATE_INTERVAL 500

#define IDC_STATIC1		    1001
#define IDC_STATIC2		    1002
#define IDC_STATIC3		    1003
#define IDC_STATIC4		    1004
#define IDC_STATIC5		    1005
#define IDC_STATIC6		    1006
#define IDC_STATIC7		    1007
#define IDC_STATIC8		    1008
#define IDC_STATIC9		    1009
#define IDC_STATIC10	    1010

#define IDD_CONFIG		    100
#define IDD_CONFIG_PAGE   101
#define IDD_DISPLAY_PAGE  102
#define IDD_POSITION_PAGE 103

#define IDC_AUTOSIZE            200
#define IDC_ACTIVETASK          201
#define IDC_SNAPMOVE            202
#define IDC_SNAPSIZE            203
#define IDC_CLICKTHROUGH        204
#define IDC_CLICKTHROUGHMETHOD  205
#define IDC_DYNAMICPOSITIONING  206
#define IDC_ANCHOR              207

#define IDC_FONTBUTTON           300
#define IDC_UPDATEINTERVAL       301
#define IDC_UPDATEINTERVALUPDOWN 302
#define IDC_MONITORCPU           303
#define IDC_MONITORCOMMIT        304
#define IDC_HISTORYMODE          305
#define IDC_SHOWNUMBERS          306
#define IDC_TEXTUP               307
#define IDC_TEXTDOWN             308
#define IDC_TEXTLEFT             309
#define IDC_TEXTRIGHT            310
#define IDC_MONITORPHY           311
#define IDC_MONITORPAGE          312

#define IDC_METHODCPU            350
#define IDC_FROMCOLOURCPU        351
#define IDC_TOCOLOURCPU          352
#define IDC_METHODMEM            353
#define IDC_FROMCOLOURMEM        354
#define IDC_TOCOLOURMEM          355

#define IDC_TOP			    400
#define IDC_NORMAL		    401
#define IDC_BOTTOM		    402

#define IDC_VERTICAL		500
#define IDC_HORIZONTAL	    501
#define IDC_LEFT		    502
#define IDC_RIGHT		    503
#define IDC_UP			    504
#define IDC_DOWN		    505
#define IDC_HCENTER     506
#define IDC_VCENTER     507

#define IDC_ICONSIZE          600
#define IDC_ICONSPACING       601
#define IDC_ICONSPACINGUPDOWN 602

#define IDI_DEFAULT		    1000
