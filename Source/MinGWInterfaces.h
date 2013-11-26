#ifndef __MINGWINTERFACES_H
#define __MINGWINTERFACES_H

#include <initguid.h>
#include <propsys.h>

#define _MMDeviceEnumerator    1
#define _IMMDeviceEnumerator   2
#define _IAudioEndpointVolume  3

typedef enum
{
  eRender,
  eCapture,
  eAll,
  EDataFlow_enum_count
} EDataFlow;

typedef enum
{
  eConsole,
  eMultimedia,
  eCommunications,
  ERole_enum_count
} ERole;

#ifdef __cplusplus
extern "C"
{
#endif

#undef INTERFACE
#define INTERFACE IImageList
  DEFINE_GUID(IID_IImageList,0x46EB5926L,0x582E,0x4017,0x9F,0xDF,0xE8,0x99,0x8D,0xAA,0x09,0x50);
  DECLARE_INTERFACE_(IImageList, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
      STDMETHOD(Add)(THIS_ HBITMAP hbmImage, HBITMAP hbmMask, int *pi) PURE;
      STDMETHOD(ReplaceIcon)(THIS_ int i, HICON hicon, int *pi) PURE;
      STDMETHOD(SetOverlayImage)(THIS_ int iImage, int iOverlay) PURE;
      STDMETHOD(Replace)(THIS_ int i, HBITMAP hbmImage, HBITMAP hbmMask) PURE;
      STDMETHOD(AddMasked)(THIS_ HBITMAP hbmImage, COLORREF crMask, int *pi) PURE;
      STDMETHOD(Draw)(THIS_ IMAGELISTDRAWPARAMS *pimldp) PURE;
      STDMETHOD(Remove)(THIS_ int i) PURE;
      STDMETHOD(GetIcon)(THIS_ int i, UINT flags, HICON *picon) PURE;
      STDMETHOD(GetImageInfo)(THIS_ int i, IMAGEINFO *pImageInfo) PURE;
      STDMETHOD(Copy)(THIS_ int iDst, IUnknown *punkSrc, int iSrc, UINT uFlags) PURE;
      STDMETHOD(Merge)(THIS_ int i1, IUnknown *punk2, int i2, int dx, int dy, REFIID riid, PVOID *ppv) PURE;
      STDMETHOD(Clone)(THIS_ REFIID riid, PVOID *ppv) PURE;
      STDMETHOD(GetImageRect)(THIS_ int i, RECT *prc) PURE;
      STDMETHOD(GetIconSize)(THIS_ int *cx, int *cy) PURE;
      STDMETHOD(SetIconSize)(THIS_ int cx, int cy) PURE;
      STDMETHOD(GetImageCount)(THIS_ int *pi) PURE;
      STDMETHOD(SetImageCount)(THIS_ UINT uNewCount) PURE;
      STDMETHOD(SetBkColor)(THIS_ COLORREF clrBk, COLORREF *pclr) PURE;
      STDMETHOD(GetBkColor)(THIS_ COLORREF *pclr) PURE;
      STDMETHOD(BeginDrag)(THIS_ int iTrack, int dxHotspot, int dyHotspot) PURE;
      STDMETHOD(EndDrag)(THIS) PURE;
      STDMETHOD(DragEnter)(THIS_ HWND hwndLock, int x, int y) PURE;
      STDMETHOD(DragLeave)(THIS_ HWND hwndLock) PURE;
      STDMETHOD(DragMove)(THIS_ int x, int y) PURE;
      STDMETHOD(SetDragCursorImage)(THIS_ IUnknown *punk, int iDrag, int dxHotspot, int dyHotspot) PURE;
      STDMETHOD(DragShowNolock)(THIS_ BOOL fShow) PURE;
      STDMETHOD(GetDragImage)(THIS_ POINT *ppt, POINT *pptHotspot, REFIID riid, PVOID *ppv) PURE;
      STDMETHOD(GetItemFlags)(THIS_ int i, DWORD *dwFlags) PURE;
      STDMETHOD(GetOverlayImage)(THIS_ int iOverlay, int *piIndex) PURE;
    };
#undef INTERFACE
  typedef IImageList *LPIMAGELIST;

#if (__MINGW64_VERSION_MAJOR < 3) && !defined (__IPropertyStore_INTERFACE_DEFINED__)
#define INTERFACE IPropertyStore
  DECLARE_INTERFACE_(IPropertyStore, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
    };
#undef INTERFACE
  typedef IPropertyStore *LPPROPERTYSTORE;
#endif

#define INTERFACE IMMDevice
  DECLARE_INTERFACE_(IMMDevice, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
      STDMETHOD_(ULONG,Activate)(THIS_ REFIID,DWORD,PROPVARIANT*,void**) PURE;
      STDMETHOD_(ULONG,OpenPropertyStore)(THIS_ DWORD,IPropertyStore**) PURE;
      STDMETHOD_(ULONG,GetId)(THIS_ LPWSTR*) PURE;
      STDMETHOD_(ULONG,GetState)(THIS_ DWORD*) PURE;
    };
#undef INTERFACE
  typedef IMMDevice *LPMMDEVICE;

#define INTERFACE IMMDeviceCollection
  DECLARE_INTERFACE_(IMMDeviceCollection, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
    };
#undef INTERFACE
  typedef IMMDeviceCollection *LPMMDEVICECOLLECTION;

#define INTERFACE IMMNotificationClient
  DECLARE_INTERFACE_(IMMNotificationClient, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
    };
#undef INTERFACE
  typedef IMMNotificationClient *LPMMNOTIFICATIONCLIENT;

#define INTERFACE IMMDeviceEnumerator
  DEFINE_GUID(IID_IMMDeviceEnumerator,0xA95664D2,0x9614,0x4F35,0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6);
  DEFINE_GUID(CLSID_MMDeviceEnumerator,0xBCDE0395,0xE52F,0x467C,0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E);
  DECLARE_INTERFACE_(IMMDeviceEnumerator, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
      STDMETHOD_(ULONG,EnumAudioEndpoints)(THIS_ EDataFlow,DWORD,IMMDeviceCollection**) PURE;
      STDMETHOD_(ULONG,GetDefaultAudioEndpoint)(THIS_ EDataFlow,ERole,IMMDevice**) PURE;
      STDMETHOD_(ULONG,GetDevice)(THIS_ LPCWSTR,IMMDevice**) PURE;
      STDMETHOD_(ULONG,RegisterEndpointNotificationCallback)(THIS_ IMMNotificationClient*) PURE;
      STDMETHOD_(ULONG,UnregisterEndpointNotificationCallback)(THIS_ IMMNotificationClient*) PURE;
    };
#undef INTERFACE
  typedef IMMDeviceEnumerator *LPMMDEVICEENUMERATOR;

#define INTERFACE IAudioEndpointVolume
  DECLARE_INTERFACE_(IAudioEndpointVolumeCallback, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
    };
#undef INTERFACE
  typedef IAudioEndpointVolumeCallback *LPIAUDIOENDPOINTVOLUMECALLBACK;

#define INTERFACE IAudioEndpointVolume
  DEFINE_GUID(IID_IAudioEndpointVolume,0x5CDF2C82,0x841E,0x4546,0x97,0x22,0x0C,0xF7,0x40,0x78,0x22,0x9A);
  DECLARE_INTERFACE_(IAudioEndpointVolume, IUnknown)
    {
      STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
      STDMETHOD_(ULONG,AddRef)(THIS) PURE;
      STDMETHOD_(ULONG,Release)(THIS) PURE;
      STDMETHOD_(ULONG,RegisterControlChangeNotify)(THIS_ IAudioEndpointVolumeCallback*) PURE;
      STDMETHOD_(ULONG,UnregisterControlChangeNotify)(THIS_ IAudioEndpointVolumeCallback*) PURE;
      STDMETHOD_(ULONG,GetChannelCount)(THIS_ UINT*) PURE;
      STDMETHOD_(ULONG,SetMasterVolumeLevel)(THIS_ float,GUID*) PURE;
      STDMETHOD_(ULONG,SetMasterVolumeLevelScalar)(THIS_ float,GUID*) PURE;
      STDMETHOD_(ULONG,GetMasterVolumeLevel)(THIS_ float*) PURE;
      STDMETHOD_(ULONG,GetMasterVolumeLevelScalar)(THIS_ float*) PURE;
      STDMETHOD_(ULONG,SetChannelVolumeLevel)(THIS_ UINT,float,GUID*) PURE;
      STDMETHOD_(ULONG,SetChannelVolumeLevelScalar)(THIS_ UINT,float,GUID*) PURE;
      STDMETHOD_(ULONG,GetChannelVolumeLevel)(THIS_ UINT,float*) PURE;
      STDMETHOD_(ULONG,GetChannelVolumeLevelScalar)(THIS_ UINT,float*) PURE;
      STDMETHOD_(ULONG,SetMute)(THIS_ BOOL,GUID*) PURE;
      STDMETHOD_(ULONG,GetMute)(THIS_ BOOL*) PURE;
      STDMETHOD_(ULONG,GetVolumeStepInfo)(THIS_ UINT*,UINT*) PURE;
      STDMETHOD_(ULONG,VolumeStepUp)(THIS_ GUID*) PURE;
      STDMETHOD_(ULONG,VolumeStepDown)(THIS_ GUID*) PURE;
      STDMETHOD_(ULONG,QueryHardwareSupport)(THIS_ DWORD*) PURE;
      STDMETHOD_(ULONG,GetVolumeRange)(THIS_ float*,float*,float*) PURE;
    };
#undef INTERFACE
  typedef IAudioEndpointVolume *LPIAUDIOENDPOINTVOLUME;

#ifdef __cplusplus
}
#endif

GUID _uuidof(UINT iid)
{
  GUID IIDMMDeviceEnumerator = {0xA95664D2,0x9614,0x4F35,{0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6}};
  GUID IIDAudioEndpointVolume = {0x5CDF2C82,0x841E,0x4546,{0x97,0x22,0x0C,0xF7,0x40,0x78,0x22,0x9A}};
  GUID CLSIDMMDeviceEnumerator = {0xBCDE0395,0xE52F,0x467C,{0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E}};
  GUID GUIDNULL = {0x0,0x0,0x0,{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}};

  switch (iid)
    {
    case _MMDeviceEnumerator:
      return CLSIDMMDeviceEnumerator;
    case _IMMDeviceEnumerator:
      return IIDMMDeviceEnumerator;
    case _IAudioEndpointVolume:
      return IIDAudioEndpointVolume;
    }

  return GUIDNULL;
}

#endif
