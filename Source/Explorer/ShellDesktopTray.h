/*!
  @file ShellDesktopTray.h
  @brief header for Explorer
  @author The Emerge Desktop Development Team

  @attention This file is part of Emerge Desktop.
  @attention Copyright (C) 2004-2013  The Emerge Desktop Development Team

  @attention Emerge Desktop is free software; you can redistribute it and/or
  modify  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  @attention Emerge Desktop is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  @attention You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef __GUARD_f1047019_bfbe_49fa_8992_cc23b61c3354
#define __GUARD_f1047019_bfbe_49fa_8992_cc23b61c3354

#define UNICODE 1

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#include <windows.h>

// {213E2DF9-9A14-4328-99B1-6961F9143CE9}
const IID IID_IShellDesktopTray = {0x213E2DF9, 0x9A14, 0x4328, {0x99, 0xB1, 0x69, 0x61, 0xF9, 0x14, 0x3C, 0xE9}};

#ifdef __cplusplus
extern "C"
{
#endif

#undef INTERFACE
#define INTERFACE IShellDesktopTray
DECLARE_INTERFACE_(IShellDesktopTray, IUnknown)
{
  STDMETHOD(QueryInterface)(THIS_ REFIID, LPVOID*) PURE;
  STDMETHOD_(ULONG, AddRef)(THIS) PURE;
  STDMETHOD_(ULONG, Release)(THIS) PURE;

  STDMETHOD_(ULONG, GetState)(THIS) PURE;
  STDMETHOD(GetTrayWindow)(THIS_ HWND*) PURE;
  STDMETHOD(RegisterDesktopWindow)(THIS_ HWND) PURE;
  STDMETHOD(SetVar)(THIS_ int, ULONG) PURE;
};
#undef INTERFACE
typedef IShellDesktopTray* LPSHELLDESKTOPTRAY;

#ifdef __cplusplus
}
#endif

class TShellDesktopTray : public IShellDesktopTray
{
private:
  ULONG refCount;
  HWND desktopWnd;

public:
  TShellDesktopTray();
  ~TShellDesktopTray();

  STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObj);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  STDMETHODIMP_(ULONG) STDMETHODCALLTYPE GetState();
  STDMETHODIMP GetTrayWindow(HWND* o);
  STDMETHODIMP RegisterDesktopWindow(HWND d);
  STDMETHODIMP SetVar(int p1, ULONG p2);
};

class TShellDesktopTrayFactory : public IClassFactory
{
private:
  ULONG refCount;

public:
  TShellDesktopTrayFactory();
  ~TShellDesktopTrayFactory();

  // from IUnknown
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();
  STDMETHODIMP QueryInterface(REFIID riid, void** ppv);

  // from IClassFactory
  STDMETHODIMP CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv);
  STDMETHODIMP LockServer(BOOL fLock);
};

#endif
