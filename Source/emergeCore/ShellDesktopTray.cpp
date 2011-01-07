#include "ShellDesktopTray.h"

IShellDesktopTray *CreateInstance()
{
  std::wstring debug = L"TShellDesktopTray::CreateInstance";
  ELWriteDebug(debug);

	return new TShellDesktopTray;
}

HRESULT TShellDesktopTray::QueryInterface(IShellDesktopTray *p UNUSED, REFIID riid, LPVOID *ppvObj)
{
  std::wstring debug = L"TShellDesktopTray::QueryInterface";
  ELWriteDebug(debug);

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

STDMETHODIMP_(ULONG) TShellDesktopTray::AddRef(IShellDesktopTray *p UNUSED)
{
  std::wstring debug = L"TShellDesktopTray::AddRef";
  ELWriteDebug(debug);

	return 2;
}

STDMETHODIMP_(ULONG) TShellDesktopTray::Release(IShellDesktopTray *p UNUSED)
{
  std::wstring debug = L"TShellDesktopTray::Release";
  ELWriteDebug(debug);

	return 1;
}

STDMETHODIMP_(ULONG) TShellDesktopTray::GetState()
{
  std::wstring debug = L"TShellDesktopTray::GetState";
  ELWriteDebug(debug);

	return 0;
}

STDMETHODIMP TShellDesktopTray::GetTrayWindow(HWND *o)
{
  std::wstring debug = L"TShellDesktopTray::GetTrayWindow";
  ELWriteDebug(debug);

	// Prevent Explorer from closing the tray window (and SharpCore) when shutting down
	*o = 0;
	//*o = FindWindow(L"Shell_TrayWnd", NULL);

	return S_OK;
}

STDMETHODIMP TShellDesktopTray::RegisterDesktopWindow(HWND d UNUSED)
{
  std::wstring debug = L"TShellDesktopTray::RegisterDesktopWindow";
  ELWriteDebug(debug);

	return S_OK;
}

STDMETHODIMP TShellDesktopTray::SetVar(int p1 UNUSED, ULONG p2 UNUSED)
{
  std::wstring debug = L"TShellDesktopTray::SetVar";
  ELWriteDebug(debug);

	return S_OK;
}


TShellDesktopTrayFactory::TShellDesktopTrayFactory()
{

}

TShellDesktopTrayFactory::~TShellDesktopTrayFactory()
{

}

STDMETHODIMP_(ULONG) TShellDesktopTrayFactory::AddRef()
{
  std::wstring debug = L"TShellDesktopTrayFactory::AddRef";
  ELWriteDebug(debug);

	return 2;
}

STDMETHODIMP_(ULONG) TShellDesktopTrayFactory::Release()
{
  std::wstring debug = L"TShellDesktopTrayFactory::Release";
  ELWriteDebug(debug);

	return 1;
}

HRESULT TShellDesktopTrayFactory::QueryInterface(REFIID riid, void** ppv)
{
  std::wstring debug = L"TShellDesktopTrayFactory::QueryInterface";
  ELWriteDebug(debug);

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

STDMETHODIMP TShellDesktopTrayFactory::CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv)
{
  std::wstring debug = L"TShellDesktopTrayFactory::CreateInstance";
  ELWriteDebug(debug);

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

STDMETHODIMP TShellDesktopTrayFactory::LockServer(BOOL fLock UNUSED)
{
  std::wstring debug = L"TShellDesktopTrayFactory::LockServer";
  ELWriteDebug(debug);

	return S_OK;
}
