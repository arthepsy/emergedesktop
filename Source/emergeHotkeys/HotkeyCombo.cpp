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

#include "HotkeyCombo.h"
#include <stdio.h>

HotkeyCombo::HotkeyCombo(WCHAR *keyCombo, WCHAR *action)
{
  wcscpy((*this).keyCombo, keyCombo);
  wcscpy((*this).action, action);

  modifiers = 0;

  ParseKeyCombo();

  ID = GlobalAddAtom(keyCombo);
}

HotkeyCombo::~HotkeyCombo()
{
  GlobalDeleteAtom(ID);
}

WCHAR *HotkeyCombo::GetHotkeyAction()
{
  return action;
}

UINT HotkeyCombo::GetHotkeyModifiers()
{
  return modifiers;
}

UINT HotkeyCombo::GetHotkeyID()
{
  return ID;
}

WCHAR HotkeyCombo::GetHotkeyKey()
{
  return key;
}

WCHAR *HotkeyCombo::GetHotkeyString()
{
  return keyCombo;
}

void HotkeyCombo::ParseKeyCombo()
{
  WCHAR *token;
  WCHAR tmp[MAX_LINE_LENGTH];

  wcscpy(tmp, keyCombo);

  token = wcstok(tmp, TEXT("+"));

  while (token != NULL)
    {
      if (_wcsicmp(token, TEXT("Win")) == 0)
        modifiers |= MOD_WIN;
      else if (_wcsicmp(token, TEXT("Ctrl")) == 0)
        modifiers |= MOD_CONTROL;
      else if (_wcsicmp(token, TEXT("Alt")) == 0)
        modifiers |= MOD_ALT;
      else if (_wcsicmp(token, TEXT("Shift")) == 0)
        modifiers |= MOD_SHIFT;
      else
        {
          if (_wcsicmp(token, TEXT("F1")) == 0)
            key = VK_F1;
          else if (_wcsicmp(token, TEXT("F2")) == 0)
            key = VK_F2;
          else if (_wcsicmp(token, TEXT("F3")) == 0)
            key = VK_F3;
          else if (_wcsicmp(token, TEXT("F4")) == 0)
            key = VK_F4;
          else if (_wcsicmp(token, TEXT("F5")) == 0)
            key = VK_F5;
          else if (_wcsicmp(token, TEXT("F6")) == 0)
            key = VK_F6;
          else if (_wcsicmp(token, TEXT("F7")) == 0)
            key = VK_F7;
          else if (_wcsicmp(token, TEXT("F8")) == 0)
            key = VK_F8;
          else if (_wcsicmp(token, TEXT("F9")) == 0)
            key = VK_F9;
          else if (_wcsicmp(token, TEXT("F10")) == 0)
            key = VK_F10;
          else if (_wcsicmp(token, TEXT("F11")) == 0)
            key = VK_F11;
          else if (_wcsicmp(token, TEXT("F12")) == 0)
            key = VK_F12;
          else if (_wcsicmp(token, TEXT("F13")) == 0)
            key = VK_F13;
          else if (_wcsicmp(token, TEXT("F14")) == 0)
            key = VK_F14;
          else if (_wcsicmp(token, TEXT("F15")) == 0)
            key = VK_F15;
          else if (_wcsicmp(token, TEXT("F16")) == 0)
            key = VK_F16;
          else if (_wcsicmp(token, TEXT("F17")) == 0)
            key = VK_F17;
          else if (_wcsicmp(token, TEXT("F18")) == 0)
            key = VK_F18;
          else if (_wcsicmp(token, TEXT("F19")) == 0)
            key = VK_F19;
          else if (_wcsicmp(token, TEXT("F20")) == 0)
            key = VK_F20;
          else if (_wcsicmp(token, TEXT("F21")) == 0)
            key = VK_F21;
          else if (_wcsicmp(token, TEXT("F22")) == 0)
            key = VK_F22;
          else if (_wcsicmp(token, TEXT("F23")) == 0)
            key = VK_F23;
          else if (_wcsicmp(token, TEXT("F24")) == 0)
            key = VK_F24;
          else if (_wcsicmp(token, TEXT("PrtScr")) == 0)
            key = VK_SNAPSHOT;
          else if (_wcsicmp(token, TEXT("Pause")) == 0)
            key = VK_PAUSE;
          else if (_wcsicmp(token, TEXT("Insert")) == 0)
            key = VK_INSERT;
          else if (_wcsicmp(token, TEXT("Delete")) == 0)
            key = VK_DELETE;
          else if (_wcsicmp(token, TEXT("Home")) == 0)
            key = VK_HOME;
          else if (_wcsicmp(token, TEXT("End")) == 0)
            key = VK_END;
          else if (_wcsicmp(token, TEXT("PageUp")) == 0)
            key = VK_PRIOR;
          else if (_wcsicmp(token, TEXT("PageDown")) == 0)
            key = VK_NEXT;
          else if (_wcsicmp(token, TEXT("Left")) == 0)
            key = VK_LEFT;
          else if (_wcsicmp(token, TEXT("Right")) == 0)
            key = VK_RIGHT;
          else if (_wcsicmp(token, TEXT("Up")) == 0)
            key = VK_UP;
          else if (_wcsicmp(token, TEXT("Down")) == 0)
            key = VK_DOWN;
          else if (_wcsicmp(token, TEXT("Tab")) == 0)
            key = VK_TAB;
          else if (_wcsicmp(token, TEXT("Backspace")) == 0)
            key = VK_BACK;
          else if (_wcsicmp(token, TEXT("Space")) == 0)
            key = VK_SPACE;
          else if (_wcsicmp(token, TEXT("Enter")) == 0)
            key = VK_RETURN;
          else if (_wcsicmp(token, TEXT("Num0")) == 0)
            key = VK_NUMPAD0;
          else if (_wcsicmp(token, TEXT("Num1")) == 0)
            key = VK_NUMPAD1;
          else if (_wcsicmp(token, TEXT("Num2")) == 0)
            key = VK_NUMPAD2;
          else if (_wcsicmp(token, TEXT("Num3")) == 0)
            key = VK_NUMPAD3;
          else if (_wcsicmp(token, TEXT("Num4")) == 0)
            key = VK_NUMPAD4;
          else if (_wcsicmp(token, TEXT("Num5")) == 0)
            key = VK_NUMPAD5;
          else if (_wcsicmp(token, TEXT("Num6")) == 0)
            key = VK_NUMPAD6;
          else if (_wcsicmp(token, TEXT("Num7")) == 0)
            key = VK_NUMPAD7;
          else if (_wcsicmp(token, TEXT("Num8")) == 0)
            key = VK_NUMPAD8;
          else if (_wcsicmp(token, TEXT("Num9")) == 0)
            key = VK_NUMPAD9;
          else if (_wcsicmp(token, TEXT("Multiply")) == 0)
            key = VK_MULTIPLY;
          else if (_wcsicmp(token, TEXT("Divide")) == 0)
            key = VK_DIVIDE;
          else if (_wcsicmp(token, TEXT("Add")) == 0)
            key = VK_ADD;
          else if (_wcsicmp(token, TEXT("Subtract")) == 0)
            key = VK_SUBTRACT;
          else if (_wcsicmp(token, TEXT("Decimal")) == 0)
            key = VK_DECIMAL;
          else if (_wcsicmp(token, TEXT("Escape")) == 0)
            key = VK_ESCAPE;
          else if (_wcsicmp(token, TEXT("BrowseBack")) == 0)
            key = VK_BROWSER_BACK;
          else if (_wcsicmp(token, TEXT("BrowseForward")) == 0)
            key = VK_BROWSER_FORWARD;
          else if (_wcsicmp(token, TEXT("BrowseRefresh")) == 0)
            key = VK_BROWSER_REFRESH;
          else if (_wcsicmp(token, TEXT("BrowseStop")) == 0)
            key = VK_BROWSER_STOP;
          else if (_wcsicmp(token, TEXT("BrowseSearch")) == 0)
            key = VK_BROWSER_SEARCH;
          else if (_wcsicmp(token, TEXT("BrowseFavorites")) == 0)
            key = VK_BROWSER_FAVORITES;
          else if (_wcsicmp(token, TEXT("BrowseHome")) == 0)
            key = VK_BROWSER_HOME;
          else if (_wcsicmp(token, TEXT("VolumeMute")) == 0)
            key = VK_VOLUME_MUTE;
          else if (_wcsicmp(token, TEXT("VolumeUp")) == 0)
            key = VK_VOLUME_UP;
          else if (_wcsicmp(token, TEXT("VolumeDown")) == 0)
            key = VK_VOLUME_DOWN;
          else if (_wcsicmp(token, TEXT("PlayerNext")) == 0)
            key = VK_MEDIA_NEXT_TRACK;
          else if (_wcsicmp(token, TEXT("PlayerPrevious")) == 0)
            key = VK_MEDIA_PREV_TRACK;
          else if (_wcsicmp(token, TEXT("PlayerStop")) == 0)
            key = VK_MEDIA_STOP;
          else if (_wcsicmp(token, TEXT("PlayerPause")) == 0)
            key = VK_MEDIA_PLAY_PAUSE;
          else if (_wcsicmp(token, TEXT("LaunchMail")) == 0)
            key = VK_LAUNCH_MAIL;
          else if (_wcsicmp(token, TEXT("LaunchPlayer")) == 0)
            key = VK_LAUNCH_MEDIA_SELECT;
          else if (_wcsicmp(token, TEXT("LaunchApp1")) == 0)
            key = VK_LAUNCH_APP1;
          else if (_wcsicmp(token, TEXT("LaunchApp2")) == 0)
            key = VK_LAUNCH_APP2;
          else if (_wcsicmp(token, TEXT("Sleep")) == 0)
            key = VK_SLEEP;
          else if (_wcsicmp(token, TEXT("LeftWinKey")) == 0)
            {
              if ((modifiers & MOD_WIN) != MOD_WIN)
                modifiers |= MOD_WIN;

              key = VK_LWIN;
            }
          else if (_wcsicmp(token, TEXT("RightWinKey")) == 0)
            {
              if ((modifiers & MOD_WIN) != MOD_WIN)
                modifiers |= MOD_WIN;

              key = VK_RWIN;
            }
          else
            key = _wcsupr(token)[0];

          break;
        }

      token = wcstok(NULL, TEXT("+"));
    }
}

