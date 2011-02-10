#include "ShellDesktopTray.h"

TShellDesktopTray::TShellDesktopTray()
{
  refCount = 0;
  desktopWnd = NULL;
}

TShellDesktopTray::~TShellDesktopTray()
{

}

HRESULT TShellDesktopTray::QueryInterface(REFIID riid, LPVOID * ppvObj)
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

  AddRef();
  return S_OK;
}

ULONG TShellDesktopTray::AddRef()
{
  ++refCount;

  return refCount;
}

ULONG TShellDesktopTray::Release()
{
  --refCount;

  return refCount;
}

ULONG TShellDesktopTray::GetState()
{
  return 2;
}

HRESULT TShellDesktopTray::GetTrayWindow(HWND *o)
{
  // Prevent Explorer from closing the tray window (and SharpCore) when shutting down
  *o = NULL;
  //*o = FindWindow(TEXT("EmergeDesktopCore"), NULL);


  return S_OK;
}

HRESULT TShellDesktopTray::RegisterDesktopWindow(HWND d)
{
  desktopWnd = d;

  return S_OK;
}

HRESULT TShellDesktopTray::SetVar(int p1 UNUSED, ULONG p2 UNUSED)
{
  return S_OK;
}


TShellDesktopTrayFactory::TShellDesktopTrayFactory()
{
  refCount = 0;
}

TShellDesktopTrayFactory::~TShellDesktopTrayFactory()
{

}

ULONG TShellDesktopTrayFactory::AddRef()
{
  ++refCount;

  return refCount;
}

ULONG TShellDesktopTrayFactory::Release()
{
  if (refCount > 0)
    --refCount;

  return refCount;
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

  HRESULT hr = pShellDesktopTray->QueryInterface(riid, ppv);
  if(hr)
    delete pShellDesktopTray;

  return hr;
}

HRESULT TShellDesktopTrayFactory::LockServer(BOOL fLock UNUSED)
{
  return S_OK;
}
