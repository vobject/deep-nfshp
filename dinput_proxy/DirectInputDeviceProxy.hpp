#ifndef DIRECT_INPUT_DEVICE_PROXY_HPP
#define DIRECT_INPUT_DEVICE_PROXY_HPP

#include "DirectInputDevice2Proxy.hpp"
#include "context.hpp"
#include "utils.hpp"

#include <dinput.h>

static const GUID GUID_DirectInputDeviceProxy{ 0x10ADAB1E, 0xD15A, 0xB1ED, {0xFA, 0x11, 0x5A, 0xFE, 0xD1, 0x5E, 0xA5, 0xE5} };
static const DIDEVICEINSTANCE DirectInputDeviceProxy_DeviceInstance = {
    sizeof(DIDEVICEINSTANCE),
    GUID_DirectInputDeviceProxy,
    GUID_DirectInputDeviceProxy,
    (DIDEVTYPE_JOYSTICK << 8) | DIDEVTYPE_JOYSTICK,
    "DirectInputDeviceProxy_InstanceName",
    "DirectInputDeviceProxy_ProductName",
#if(DIRECTINPUT_VERSION >= 0x0500)
    GUID_DirectInputDeviceProxy,
    0,
    0,
#endif // DIRECTINPUT_VERSION >= 0x0500
};

struct DirectInputDeviceProxy : IDirectInputDevice
{
    Context& m_ctx;

    explicit DirectInputDeviceProxy(Context& ctx)
        : m_ctx{ ctx }
    { }

    /*** IUnknown methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, LPVOID* ppvObj) {
        log("DirectInputDeviceProxy::QueryInterface(guid={%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX})", riid.Data1, riid.Data2, riid.Data3,
            riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
            riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
        
        if (ppvObj == nullptr) {
            return E_POINTER;
        }
        if (IsEqualGUID(riid, IID_IDirectInputDevice2A) == FALSE) {
            return E_NOINTERFACE;
        }

        *ppvObj = new DirectInputDevice2Proxy(m_ctx);
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE AddRef() {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::AddRef()");
        return 1;
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE Release() {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::Release()");
        return 0;
    }

    /*** IDirectInputDeviceA methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::GetCapabilities()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::EnumObjects()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetProperty(const GUID& riid, LPDIPROPHEADER pdiph) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::GetProperty()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetProperty(const GUID& riid, LPCDIPROPHEADER pdiph) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::SetProperty()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Acquire() {
        log("DirectInputDeviceProxy::Acquire()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Unacquire() {
        log("DirectInputDeviceProxy::Unacquire()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::GetDeviceState()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
        //log("NOT IMPLEMENTED: DirectInputDeviceProxy::GetDeviceData()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::SetDataFormat()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::SetEventNotification()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD flags) {
        log("DirectInputDeviceProxy::SetCooperativeLevel(flags=%08X)", flags);
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::GetObjectInfo()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi) {
        log("NOT IMPLEMENTED: DirectInputDeviceProxy::GetDeviceInfo()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        log("DirectInputDeviceProxy::RunControlPanel()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, const GUID& riid) {
        log("DirectInputDeviceProxy::Initialize()");
        return DI_OK;
    }
};

#endif // !DIRECT_INPUT_DEVICE_PROXY_HPP
