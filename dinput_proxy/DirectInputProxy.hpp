#ifndef DIRECT_INPUT_PROXY_HPP
#define DIRECT_INPUT_PROXY_HPP

#include "DirectInputDeviceProxy.hpp"
#include "DirectInputMouseProxy.hpp"
#include "context.hpp"
#include "utils.hpp"

#include <dinput.h>

struct DirectInputProxy : IDirectInput
{
    IDirectInput* const m_realDirectInput;
    Context& m_ctx;

    explicit DirectInputProxy(IDirectInput* realDirectInput, Context& ctx)
        : m_realDirectInput{ realDirectInput }
        , m_ctx{ ctx }
    {}

    /*** IUnknown methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, LPVOID* ppvObj) {
        log("DirectInputProxy::QueryInterface()");
        return m_realDirectInput->QueryInterface(riid, ppvObj);
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE AddRef() {
        log("DirectInputProxy::AddRef()");
        return m_realDirectInput->AddRef();
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE Release() {
        log("DirectInputProxy::Release()");
        return m_realDirectInput->Release();
    }

    /*** IDirectInput methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE CreateDevice(const GUID& riid, LPDIRECTINPUTDEVICEA* lplpDirectInputDevice, LPUNKNOWN pUnkOuter) {
        log("DirectInputProxy::CreateDevice(guid={%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX})", riid.Data1, riid.Data2, riid.Data3,
            riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
            riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

        if (IsEqualGUID(riid, GUID_DirectInputDeviceProxy))
        {
            *lplpDirectInputDevice = new DirectInputDeviceProxy(m_ctx);
            return DI_OK;
        }

        if (IsEqualGUID(riid, GUID_SysMouse))
        {
            const HRESULT hr = m_realDirectInput->CreateDevice(riid, lplpDirectInputDevice, pUnkOuter);
            if (hr != DI_OK)
            {
                log("Failed to create mouse device");
                return hr;
            }
            log("Mouse device proxy created!!");
            *lplpDirectInputDevice = new DirectInputMouseProxy(*lplpDirectInputDevice, m_ctx);
            return DI_OK;
        }

        return m_realDirectInput->CreateDevice(riid, lplpDirectInputDevice, pUnkOuter);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) {
        log("DirectInputProxy::EnumDevices(devType=%08X flags=%08X)", dwDevType, dwFlags);

        if (dwDevType == DIDEVTYPE_JOYSTICK)
        {
            const BOOL cbResult = (*lpCallback)(&DirectInputDeviceProxy_DeviceInstance, pvRef);
            if (cbResult != TRUE) {
                log("Error: device enum callback returned %d", cbResult);
            }
        }

        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceStatus(const GUID& riid) {
        log("DirectInputProxy::GetDeviceStatus()");
        return m_realDirectInput->GetDeviceStatus(riid);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        log("DirectInputProxy::RunControlPanel()");
        return m_realDirectInput->RunControlPanel(hwndOwner, dwFlags);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion) {
        log("DirectInputProxy::Initialize()");
        return m_realDirectInput->Initialize(hinst, dwVersion);
    }
};

#endif // !DIRECT_INPUT_PROXY_HPP
