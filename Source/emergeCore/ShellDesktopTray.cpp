#include "ShellDesktopTray.h"

HRESULT TShellDesktopTray::QueryInterface(IShellDesktopTray * p UNUSED, REFIID riid, LPVOID * ppvObj)
{
	if(!ppvObj)
		return E_POINTER;

	if(riid == IID_IUnknown || riid == IID_IShellDesktopTray)
		*ppvObj = this;
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}

	AddRef(this);
	return S_OK;
}

ULONG TShellDesktopTray::AddRef(IShellDesktopTray * p UNUSED)
{
	return 2;
}

ULONG TShellDesktopTray::Release(IShellDesktopTray * p UNUSED)
{
	return 1;
}

ULONG TShellDesktopTray::GetState()
{
	return 2;
}

HRESULT TShellDesktopTray::GetTrayWindow(HWND *o)
{
	// Prevent Explorer from closing the tray window (and SharpCore) when shutting down
	*o = 0;
	//*o = FindWindow(TEXT("EmergeDesktopCore"), NULL);


	return S_OK;
}

HRESULT TShellDesktopTray::RegisterDesktopWindow(HWND d UNUSED)
{
	return S_OK;
}

HRESULT TShellDesktopTray::SetVar(int p1 UNUSED, ULONG p2 UNUSED)
{
	return S_OK;
}


TShellDesktopTrayFactory::TShellDesktopTrayFactory()
{

}

TShellDesktopTrayFactory::~TShellDesktopTrayFactory()
{

}

ULONG TShellDesktopTrayFactory::AddRef()
{
	return 2;
}

ULONG TShellDesktopTrayFactory::Release()
{
	return 1;
}

HRESULT TShellDesktopTrayFactory::QueryInterface(REFIID riid, void** ppv)
{
	if(!ppv)
		return E_POINTER;

	if(riid == IID_IUnknown || riid == IID_IClassFactory)
		*ppv = this;
	else
	{
		*ppv = 0;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

HRESULT TShellDesktopTrayFactory::CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv)
{
	if(!ppv)
		return E_POINTER;

	if(pOuter)
		return CLASS_E_NOAGGREGATION;

	TShellDesktopTray* pShellDesktopTray = new TShellDesktopTray;

	HRESULT hr = pShellDesktopTray->QueryInterface(NULL, riid, ppv);
	if(hr)
		delete pShellDesktopTray;

	return hr;
}

HRESULT TShellDesktopTrayFactory::LockServer(BOOL fLock UNUSED)
{
	return S_OK;
}
