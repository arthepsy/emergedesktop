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
//
// Note:  Many, many thanx go to fangr for providing the initial release
// of emergeCommand.
//
//----  --------------------------------------------------------------------------------------------------------

#include "Command.h"
#include "Applet.h"

WCHAR szCommandClass[] = TEXT("CommandWnd");

Command::Command(HWND mainWnd, HINSTANCE hInstance, std::tr1::shared_ptr<Settings> pSettings)
{
  pac = NULL;
  pom = NULL;
  fileSource = NULL;
  historySource = NULL;
  commandSource = NULL;
  hText = NULL;
  history = NULL;
  appletHidden = false;
  dragBorder = 0;
  pac2 = NULL;
  textFont = NULL;

  ZeroMemory(fontName, MAX_RUN_STRING);

  mainInst = hInstance;
  this->mainWnd = mainWnd;
  this->pSettings = pSettings;
  registered = true;
}

Command::~Command()
{
  if (pac)
  {
    pac->Release();
  }

  if (pom)
  {
    pom->Release();
  }

  if (fileSource)
  {
    fileSource->Release();
  }

  if (historySource)
  {
    historySource->Release();
  }

  if (commandSource)
  {
    commandSource->Release();
  }

  if (!registered)
    // Unregister the window class
  {
    UnregisterClass(szCommandClass, mainInst);
  }
}

bool Command::Init()
{
  WNDCLASSEX wincl;
  ZeroMemory(&wincl, sizeof(wincl));

  // Register the window class
  wincl.hInstance = mainInst;
  wincl.lpszClassName = szCommandClass;
  wincl.lpfnWndProc = CommandProc;
  wincl.cbSize = sizeof (WNDCLASSEX);
  wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.hbrBackground = GetSysColorBrush(COLOR_WINDOW);

  // Register the window class, and if it fails quit the program
  if (!RegisterClassEx (&wincl))
  {
    wincl.style = CS_DBLCLKS;
    if (!RegisterClassEx (&wincl))
    {
      registered = false;
      return false;
    }
  }

  // The class is registered, let's create the window
  commandWnd = CreateWindowEx (
                 WS_EX_TOOLWINDOW | WS_EX_LAYERED,
                 szCommandClass,
                 NULL,
                 WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                 0, 0,
                 0, 0,
                 NULL,
                 NULL,
                 mainInst,
                 this);

  // If the window failed to get created, unregister the class and quit the program
  if (!commandWnd)
  {
    return false;
  }

  return true;
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	EditProc
// Required:	HWND hText - The edit box this event is for
//				UINT message - The message to process
//				WPARAM wParam - dependant on message
//				LPARAM lParam - dependant on message
// Returns:		LRESULT
// Purpose:		Handles messages meant for the Edit Box
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Command::EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  //WCHAR error[MAX_LINE_LENGTH];
  WCHAR buf[MAX_LINE_LENGTH];
  static Command* pCommand = reinterpret_cast<Command*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

  if (message == WM_KEYDOWN)
  {
    // When the user presses return - execute the command
    if (wParam == VK_RETURN)
    {
      int length = GetWindowText(hwnd, buf, MAX_LINE_LENGTH);
      SetWindowText(hwnd, TEXT(""));
      //swprintf(error, TEXT("Failed to execute \"%ls\""), buf);
      if (length > 0)
      {
        buf[length] = '\0';
        if (ELExecuteFileOrCommand(buf))
        {
          pCommand->AddElement(buf);
          pCommand->ShowAppletWindow();
        }
        /*else
        {
          ELMessageBox(GetDesktopWindow(), error, TEXT("emergeCommand"), ELMB_ICONWARNING | ELMB_OK);
        }*/
      }

      return 0;
    }

    if (wParam == VK_ESCAPE)
    {
      SetWindowText(hwnd, TEXT(""));
      pCommand->ShowAppletWindow();
      return 0;
    }
  }

  // Do the default thing for everyone else
  return CallWindowProc(pCommand->GetWndProc(), hwnd, message, wParam, lParam);
}

void Command::AddElement(WCHAR* element)
{
  if (history)
  {
    history->AddElement(element);
  }
}

void Command::ShowAppletWindow()
{
  KillTimer(commandWnd, FOCUS_TIMER);
  SetWindowPos(mainWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE |
               SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
  SetWindowPos(commandWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE |
               SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
}

//----  --------------------------------------------------------------------------------------------------------
// Function:	CommandProc
// Required:	HWND hwnd - window handle that message was sent to
// 		UINT message - action to handle
// 		WPARAM wParam - dependant on message
// 		LPARAM lParam - dependant on message
// Returns:	LRESULT
// Purpose:	Handles messages sent from DispatchMessage
//----  --------------------------------------------------------------------------------------------------------
LRESULT CALLBACK Command::CommandProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  CREATESTRUCT* cs;
  static Command* pCommand = NULL;

  if (message == WM_CREATE)
  {
    cs = (CREATESTRUCT*)lParam;
    pCommand = reinterpret_cast<Command*>(cs->lpCreateParams);
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  if (pCommand == NULL)
  {
    return DefWindowProc(hwnd, message, wParam, lParam);
  }

  switch (message)
  {
    // Needed to handle changing the system colors.  It forces
    // a repaint of the window as well as the frame.
  case WM_SYSCOLORCHANGE:
    RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE);
    break;

    // Repaint the icons as the window size is changing
  case WM_WINDOWPOSCHANGING:
    {
      if (ELToLower(pCommand->GetZPosition()) == TEXT("bottom"))
      {
        ((WINDOWPOS*)lParam)->flags |= SWP_NOZORDER;
      }
    }
    break;

    // Don't close on alt-F4
  case WM_SYSCOMMAND:
    return pCommand->DoSysCommand(hwnd, message, wParam, lParam);

    // Send a quit message when the window is destroyed
  case WM_DESTROY:
  case WM_NCDESTROY:
    PostQuitMessage (0);
    break;

  case WM_SHOWWINDOW:
    {
      if (wParam)
      {
        pCommand->ShowTextWindow();
        SetTimer(hwnd, FOCUS_TIMER, 250, NULL);
      }
    }
    return 0;

  case WM_TIMER:
    if (GetForegroundWindow() != hwnd)
    {
      pCommand->ShowAppletWindow();
    }
    return 0;

    // If not handled just forward the message on
  default:
    return DefWindowProc (hwnd, message, wParam, lParam);
  }
  return 0;
}

void Command::SetHidden(bool appletHidden)
{
  this->appletHidden = appletHidden;
}

std::wstring Command::GetZPosition()
{
  return pSettings->GetZPosition();
}

LRESULT Command::DoSysCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (wParam)
  {
  case SC_CLOSE:
  case SC_MAXIMIZE:
  case SC_MINIMIZE:
    break;
  default:
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}

void Command::Show()
{
  if (!appletHidden)
  {
    ShowWindow(mainWnd, SW_SHOWNOACTIVATE);
  }
}

void Command::UpdateEdit(GUIINFO guiInfo, int width, int height)
{
  int top, bottom;
  int dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;
  LPVOID lpVoid;

  if (ELToLower(pSettings->GetCommandVerticalAlign()) == TEXT("center"))
  {
    top = (height / 2);
    top -= (pSettings->GetFont()->lfHeight / 2);
    if (top < dragBorder)
    {
      top = dragBorder;
    }
    bottom = height - (2 * dragBorder);
  }
  else if (ELToLower(pSettings->GetCommandVerticalAlign()) == TEXT("bottom"))
  {
    bottom = height - (2 * dragBorder);
    top = bottom - pSettings->GetFont()->lfHeight;
    if (top < dragBorder)
    {
      top = dragBorder;
    }
  }
  else
  {
    top = dragBorder;
    bottom = top + pSettings->GetFont()->lfHeight;
  }

  if (hText)
  {
    if (pac)
    {
      pac->Release();
      pac = NULL;
    }

    if (pom)
    {
      pom->Release();
      pom = NULL;
    }

    if (fileSource)
    {
      fileSource->Release();
      fileSource = NULL;
    }

    if (historySource)
    {
      historySource->Release();
      historySource = NULL;
    }

    if (commandSource)
    {
      commandSource->Release();
      commandSource = NULL;
    }

    DestroyWindow(hText);
  }

  if (!pac)
  {
    CoCreateInstance(CLSID_AutoComplete,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IAutoComplete,
                     &lpVoid);
    pac = reinterpret_cast <IAutoComplete*> (lpVoid);
  }

  if (!fileSource)
  {
    CoCreateInstance(CLSID_ACListISF,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IACList,
                     &lpVoid);
    fileSource = reinterpret_cast <IUnknown*> (lpVoid);
  }

  if (!historySource)
  {
    history = new CustomSource(pSettings);

    history->QueryInterface(IID_IEnumString, &lpVoid);
    historySource = reinterpret_cast <IEnumString*> (lpVoid);
  }

  if (!commandSource)
  {
    command = new CommandSource(mainInst);

    command->QueryInterface(IID_IEnumString, &lpVoid);
    commandSource = reinterpret_cast <IEnumString*> (lpVoid);
  }

  if (!pom)
  {
    CoCreateInstance(CLSID_ACLMulti,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IObjMgr,
                     &lpVoid);
    pom = reinterpret_cast <IObjMgr*> (lpVoid);
  }

  if (fileSource && pom)
  {
    pom->Append(fileSource);
  }

  if (historySource && pom)
  {
    pom->Append(historySource);
  }

  if (commandSource && pom)
  {
    pom->Append(commandSource);
  }

  if (ELToLower(pSettings->GetCommandTextAlign()) == TEXT("right"))
    hText = CreateWindowEx(
              WS_EX_TOOLWINDOW,
              TEXT("EDIT"),
              NULL,
              WS_CHILD | ES_AUTOHSCROLL | ES_RIGHT,
              dragBorder, top,
              (width - (2 * dragBorder)), bottom,
              commandWnd,
              NULL,
              mainInst,
              NULL);
  else if (ELToLower(pSettings->GetCommandTextAlign()) == TEXT("center"))
    hText = CreateWindowEx(
              WS_EX_TOOLWINDOW,
              TEXT("EDIT"),
              NULL,
              WS_CHILD | ES_AUTOHSCROLL | ES_CENTER,
              dragBorder, top,
              (width - (2 * dragBorder)), bottom,
              commandWnd,
              NULL,
              mainInst,
              NULL);
  else
    hText = CreateWindowEx(
              WS_EX_TOOLWINDOW,
              TEXT("EDIT"),
              NULL,
              WS_CHILD | ES_AUTOHSCROLL | ES_LEFT,
              dragBorder, top,
              (width - (2 * dragBorder)), bottom,
              commandWnd,
              NULL,
              mainInst,
              this);

  wpOld = (WNDPROC)GetWindowLongPtr(hText, GWLP_WNDPROC);
  SetWindowLongPtr(hText, GWLP_USERDATA, (LONG_PTR)this);
  SetWindowLongPtr(hText, GWLP_WNDPROC, (LONG_PTR)EditProc);

  if (textFont)
  {
    DeleteObject((HFONT)textFont);
  }

  textFont = CreateFontIndirect(pSettings->GetFont());
  SendMessage(hText, WM_SETFONT, (WPARAM)textFont, TRUE);
}

HWND Command::GetCommandWnd()
{
  return commandWnd;
}

WNDPROC Command::GetWndProc()
{
  return wpOld;
}

void Command::ShowTextWindow()
{
  LPVOID lpVoid;

  ShowWindow(hText, SW_SHOW);
  SetWindowText(hText, TEXT(""));
  ELStealFocus(commandWnd);
  SetFocus(hText);
  DWORD pacOptions = ACO_AUTOSUGGEST | ACO_UPDOWNKEYDROPSLIST | ACO_USETAB;
  if (pSettings->GetAutoComplete())
  {
    pacOptions |= ACO_AUTOAPPEND;
  }

  if (pac)
  {
    pac->Init(hText, pom, NULL, NULL);

    if (SUCCEEDED(pac->QueryInterface(IID_IAutoComplete2, &lpVoid)))
    {
      pac2 = reinterpret_cast <IAutoComplete2*> (lpVoid);
      pac2->SetOptions(pacOptions);
      pac2->Release();
    }
  }
}

void Command::UpdateGUI(GUIINFO guiInfo)
{
  int dragBorder = guiInfo.dragBorder + guiInfo.bevelWidth + guiInfo.padding;

  EAEUpdateGUI(commandWnd, guiInfo.windowShadow, pSettings->GetZPosition());

  SetWindowPos(commandWnd, NULL, pSettings->GetX(), pSettings->GetY(),
               pSettings->GetWidth() + (2 * dragBorder), pSettings->GetHeight() + (2 * dragBorder),
               SWP_NOZORDER);

  UpdateEdit(guiInfo, pSettings->GetWidth() + (2 * dragBorder), pSettings->GetHeight() + (2 * dragBorder));

  SetLayeredWindowAttributes(commandWnd, 0, guiInfo.alphaActive, LWA_ALPHA);
}
