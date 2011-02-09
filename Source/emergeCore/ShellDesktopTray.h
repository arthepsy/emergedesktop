#include "../emergeLib/emergeLib.h"

// {213E2DF9-9A14-4328-99B1-6961F9143CE9}
const IID IID_IShellDesktopTray = {0x213E2DF9,0x9A14,0x4328,{0x99,0xB1,0x69,0x61,0xF9,0x14,0x3C,0xE9}};

//class __declspec(novtable) IShellDesktopTray
class IShellDesktopTray
{
	public:
		virtual HRESULT QueryInterface(IShellDesktopTray * p, REFIID riid, LPVOID * ppvObj)=0;
		virtual ULONG AddRef(IShellDesktopTray * p)=0;
		virtual ULONG Release(IShellDesktopTray * p)=0;

		virtual ULONG STDMETHODCALLTYPE GetState()=0;
		virtual HRESULT STDMETHODCALLTYPE GetTrayWindow(HWND *o)=0;
		virtual HRESULT STDMETHODCALLTYPE RegisterDesktopWindow(HWND d)=0;
		virtual HRESULT STDMETHODCALLTYPE SetVar(int p1, ULONG p2)=0;
};

class TShellDesktopTray : public IShellDesktopTray
{
	public:
		TShellDesktopTray();
		~TShellDesktopTray();

		HRESULT QueryInterface(IShellDesktopTray * p, REFIID riid, LPVOID * ppvObj);
		ULONG AddRef(IShellDesktopTray * p);
		ULONG Release(IShellDesktopTray * p);

		ULONG STDMETHODCALLTYPE GetState();
		HRESULT STDMETHODCALLTYPE GetTrayWindow(HWND *o);
		HRESULT STDMETHODCALLTYPE RegisterDesktopWindow(HWND d);
		HRESULT STDMETHODCALLTYPE SetVar(int p1, ULONG p2);

  private:
    ULONG refCount;
};

class TShellDesktopTrayFactory : public IClassFactory {
	public:
		TShellDesktopTrayFactory();
		~TShellDesktopTrayFactory();

		// from IUnknown
		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

		// from IClassFactory
		virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv);
		virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);

  private:
    ULONG refCount;
};

IShellDesktopTray *CreateInstance();
