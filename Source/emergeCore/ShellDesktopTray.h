#ifndef __EC_SHELLDESKTOPTRAY_H
#define __EC_SHELLDESKTOPTRAY_H

#include "../emergeLib/emergeLib.h"

// {213E2DF9-9A14-4328-99B1-6961F9143CE9}
const GUID IID_IShellDesktopTray = {0x213E2DF9,0x9A14,0x4328,{0x99,0xB1,0x69,0x61,0xF9,0x14,0x3C,0xE9}};

class IShellDesktopTray
{
	public:
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(IShellDesktopTray * p, REFIID riid, LPVOID * ppvObj) PURE;
		virtual STDMETHODIMP_(ULONG) AddRef(IShellDesktopTray * p) PURE;
		virtual STDMETHODIMP_(ULONG) Release(IShellDesktopTray * p) PURE;

		virtual STDMETHODIMP_(ULONG) GetState() PURE;
		virtual STDMETHODIMP GetTrayWindow(HWND *o) PURE;
		virtual STDMETHODIMP RegisterDesktopWindow(HWND d) PURE;
		virtual STDMETHODIMP SetVar(int p1, ULONG p2) PURE;
};

class TShellDesktopTray : public IShellDesktopTray
{
	public:
		HRESULT STDMETHODCALLTYPE QueryInterface(IShellDesktopTray * p, REFIID riid, LPVOID * ppvObj);
		STDMETHODIMP_(ULONG) AddRef(IShellDesktopTray * p);
		STDMETHODIMP_(ULONG) Release(IShellDesktopTray * p);

		STDMETHODIMP_(ULONG) GetState();
		STDMETHODIMP GetTrayWindow(HWND *o);
		STDMETHODIMP RegisterDesktopWindow(HWND d);
		STDMETHODIMP SetVar(int p1, ULONG p2);
};

class TShellDesktopTrayFactory : public IClassFactory
{
	public:
		TShellDesktopTrayFactory();
		~TShellDesktopTrayFactory();

		// from IUnknown
		virtual STDMETHODIMP_(ULONG) AddRef();
		virtual STDMETHODIMP_(ULONG) Release();
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

		// from IClassFactory
		virtual STDMETHODIMP CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv);
		virtual STDMETHODIMP LockServer(BOOL fLock);
};

IShellDesktopTray *CreateInstance();

#endif
