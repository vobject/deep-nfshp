#ifndef DIRECT_INPUT_MOUSE_PROXY_HPP
#define DIRECT_INPUT_MOUSE_PROXY_HPP

#include "context.hpp"
#include "utils.hpp"

#include <dinput.h>

#include <mutex>

struct DirectInputMouseProxy : IDirectInputDevice
{
    IDirectInputDeviceA* m_realMouse;
    Context& m_ctx;

    explicit DirectInputMouseProxy(IDirectInputDeviceA* realMouse, Context& ctx)
        : m_realMouse{ realMouse }
        , m_ctx{ ctx }
    { }

    /*** IUnknown methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, LPVOID* ppvObj) {
        log("DirectInputMouseProxy::QueryInterface(guid={%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX})", riid.Data1, riid.Data2, riid.Data3,
            riid.Data4[0], riid.Data4[1], riid.Data4[2], riid.Data4[3],
            riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);

        return m_realMouse->QueryInterface(riid, ppvObj);
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE AddRef() {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::AddRef()");
        return m_realMouse->AddRef();
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE Release() {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::Release()");
        return m_realMouse->Release();
    }

    /*** IDirectInputDeviceA methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::GetCapabilities()");
        return m_realMouse->GetCapabilities(lpDIDevCaps);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::EnumObjects()");
        return m_realMouse->EnumObjects(lpCallback, pvRef, dwFlags);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetProperty(const GUID& riid, LPDIPROPHEADER pdiph) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::GetProperty()");
        return m_realMouse->GetProperty(riid, pdiph);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetProperty(const GUID& riid, LPCDIPROPHEADER pdiph) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::SetProperty()");
        return m_realMouse->SetProperty(riid, pdiph);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Acquire() {
        const HRESULT hr = m_realMouse->Acquire();
        //log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::Acquire() == %d", hr);
        return hr;
        //return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Unacquire() {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::Unacquire()");
        return m_realMouse->Unacquire();
        //return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
        log("CUSTOM IMPLEMENTATION: DirectInputMouseProxy::GetDeviceState()");
        DIMOUSESTATE* mouseState = (DIMOUSESTATE*)lpvData;
        return m_realMouse->GetDeviceState(sizeof(DIMOUSESTATE), mouseState);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
        // struct DIDEVICEOBJECTDATA {
        /*DWORD dwOfs;
        DWORD dwData;
        DWORD dwTimeStamp;
        DWORD dwSequence;*/
        //log("CUSTOM IMPLEMENTATION ENTER: DirectInputMouseProxy::GetDeviceData(num=%d, flags=0x%x)", *pdwInOut, dwFlags);
        //POINT point;
        //if (::GetCursorPos(&point))
        //{
        //    log("CUSTOM IMPLEMENTATION GetCursorPos(x=%d, y=0x%d)", point.x, point.y);
        //}
        //return m_realMouse->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);

        if (*pdwInOut == INFINITE)
        {
            return DI_OK;
        }

        if (m_ctx.mouse_cmds.empty())
        {
            *pdwInOut = 0;
            return DI_OK;
        }

        *pdwInOut = 0;
        std::lock_guard<std::mutex> lock(m_ctx.mutex);

        if (!m_ctx.mouse_cmds.empty())
        {
            const auto& cmd = m_ctx.mouse_cmds.front();
            switch (cmd.action)
            {
            case MouseCmd::Action::Move:
                rgdod[0].dwOfs = 0;
                rgdod[0].dwData = cmd.x;
                rgdod[0].dwTimeStamp = ::GetTickCount();
                rgdod[0].dwSequence = 0;
                rgdod[1].dwOfs = 4;
                rgdod[1].dwData = cmd.y;
                rgdod[1].dwTimeStamp = ::GetTickCount();
                rgdod[1].dwSequence = 0;
                *pdwInOut = 2;
                break;
            case MouseCmd::Action::LBtnClick:
                rgdod[0].dwOfs = 12;
                rgdod[0].dwData = 0;
                rgdod[0].dwTimeStamp = ::GetTickCount();
                rgdod[0].dwSequence = 0;
                *pdwInOut = 1;
                break;
            default:
                *pdwInOut = 0;
                break;
            }
            m_ctx.mouse_cmds.pop_front();
        }
        return DI_OK;

        //if (m_inputData.mode == 1)
        //{
        //    //log("CUSTOM IMPLEMENTATION: DirectInputMouseProxy::GetDeviceData(pdwInOut=0x%p) == %d", pdwInOut, DI_OK);
        //    //log("CUSTOM IMPLEMENTATION: DirectInputMouseProxy::GetDeviceData(in_num=%d) == %d", *pdwInOut, DI_OK);
        //    rgdod[0].dwOfs = 0;
        //    rgdod[0].dwData = m_inputData.x;
        //    rgdod[0].dwTimeStamp = ::GetTickCount();
        //    rgdod[0].dwSequence = 0;
        //    rgdod[1].dwOfs = 4;
        //    rgdod[1].dwData = m_inputData.y;
        //    rgdod[1].dwTimeStamp = ::GetTickCount();
        //    rgdod[1].dwSequence = 0;
        //    //rgdod[0].dwOfs = 12;
        //    //rgdod[0].dwData = 0;
        //    //rgdod[0].dwTimeStamp = ::GetTickCount();
        //    //rgdod[0].dwSequence = 0;
        //    *pdwInOut = 2;
        //    m_inputData.mode = 0;
        //}
        //else if (m_inputData.mode == 2) {
        ////else if (m_inputData.lbtn_click) {
        ////    rgdod[0].dwOfs = 12;
        ////    rgdod[0].dwData = 0;
        ////    rgdod[0].dwTimeStamp = ::GetTickCount();
        ////    rgdod[0].dwSequence = 0;
        ////    *pdwInOut = 1;
        ////    m_inputData.lbtn_click = false;
        //    rgdod[0].dwOfs = 12;
        //    rgdod[0].dwData = 0;
        //    rgdod[0].dwTimeStamp = ::GetTickCount();
        //    rgdod[0].dwSequence = 0;
        //    *pdwInOut = 1;
        //    m_inputData.mode = 0;
        //}
        //else {
        //    *pdwInOut = 0;
        //}


        //HRESULT hr = m_realMouse->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
        //if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        //{
        //    *pdwInOut = 0;
        //    hr = DI_OK;
        //}
        ////log("CUSTOM IMPLEMENTATION: DirectInputMouseProxy::GetDeviceData() == %d", hr);
        //if (hr == DI_OK)
        //{
        //    //log("CUSTOM IMPLEMENTATION: DirectInputMouseProxy::GetDeviceData(outNum=%d) =>", *pdwInOut);
        //    for (size_t i = 0; i < *pdwInOut; i++)
        //    {
        //        //seq = rgdod[i].dwSequence;
        //        log("    DirectInputMouseProxy::GetDeviceData(#%d ofs=%d data=%d ts=%d seq=%d)", i, rgdod[i].dwOfs, rgdod[i].dwData, rgdod[i].dwTimeStamp, rgdod[i].dwSequence);

        //    }
        //    //if (*pdwInOut)
        //    //{
        //        //if (m_inputData.lbtn_click)
        //        //{
        //        //    rgdod[*pdwInOut - 1].dwOfs = 12;
        //        //    rgdod[*pdwInOut - 1].dwData = (m_inputData.lbtn_click == 1) ? 0x80 : 0;
        //        //    //rgdod[*pdwInOut].dwTimeStamp = rgdod[*pdwInOut - 1].dwTimeStamp;
        //        //    //rgdod[*pdwInOut].dwSequence = rgdod[*pdwInOut - 1].dwSequence;
        //        //    //pdwInOut += 1;
        //        //    m_inputData.lbtn_click = (m_inputData.lbtn_click == 1) ? 2 : 0;
        //        //    return DI_OK;
        //        //}
        //        if (m_inputData.lbtn_click)
        //        {
        //            rgdod[*pdwInOut].dwOfs = 12;
        //            rgdod[*pdwInOut].dwData = 0;
        //            rgdod[*pdwInOut].dwTimeStamp = ::GetTickCount(); // rgdod[*pdwInOut - 1].dwTimeStamp;
        //            rgdod[*pdwInOut].dwSequence = 0;// rgdod[*pdwInOut - 1].dwSequence;
        //            *pdwInOut += 1;
        //            m_inputData.lbtn_click = false;
        //            return DI_OK;
        //        }
        //    //}
        //}
        //return hr;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) {
        // lpdf->dwFlags == DIDF_RELAXIS
        // lpdf->dwDataSize == 16
        // lpdf->rgodf == [
        //     {offset=0  type=DIDFT_ANYINSTANCE | DIDFT_AXIS                flags=0 guid=GUID_XAxis }
        //     {offset=4  type=DIDFT_ANYINSTANCE | DIDFT_AXIS                flags=0 guid=GUID_YAxis }
        //     {offset=8  type=DIDFT_ANYINSTANCE | DIDFT_AXIS                flags=0 guid=GUID_ZAxis }
        //     {offset=12 type=DIDFT_ANYINSTANCE | DIDFT_BUTTON              flags=0 guid=NULL }
        //     {offset=13 type=DIDFT_ANYINSTANCE | DIDFT_BUTTON              flags=0 guid=NULL }
        //     {offset=14 type=DIDFT_ANYINSTANCE | DIDFT_BUTTON | 0x80000000 flags=0 guid=NULL }
        //     {offset=15 type=DIDFT_ANYINSTANCE | DIDFT_BUTTON | 0x80000000 flags=0 guid=NULL }
        // ]

        //const std::string flags_str = lpdf->dwFlags == DIDF_ABSAXIS ? "DIDF_ABSAXIS" : "DIDF_RELAXIS";
        //std::string obj_str = "{";
        //for (DWORD i = 0; i < lpdf->dwNumObjs; i++)
        //{
        //    if (lpdf->rgodf[i].pguid)
        //    {
        //    log("SetDataFormat(guid={%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX})", (lpdf->rgodf[i].pguid)->Data1, (lpdf->rgodf[i].pguid)->Data2, (lpdf->rgodf[i].pguid)->Data3,
        //        (lpdf->rgodf[i].pguid)->Data4[0], (lpdf->rgodf[i].pguid)->Data4[1], (lpdf->rgodf[i].pguid)->Data4[2], (lpdf->rgodf[i].pguid)->Data4[3],
        //        (lpdf->rgodf[i].pguid)->Data4[4], (lpdf->rgodf[i].pguid)->Data4[5], (lpdf->rgodf[i].pguid)->Data4[6], (lpdf->rgodf[i].pguid)->Data4[7]);
        //                }
        //    else {
        //        log("SetDataFormat(guid={null})");
        //    }
        //    obj_str += " offset=" + std::to_string(lpdf->rgodf[i].dwOfs);
        //    obj_str += " type=" + std::to_string(lpdf->rgodf[i].dwType);
        //    obj_str += " flags=" + std::to_string(lpdf->rgodf[i].dwFlags);
        //}
        //obj_str += " }";
        //log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::SetDataFormat(flags=%s dataSize=%d objs=%s)", flags_str.c_str(), lpdf->dwDataSize, obj_str.c_str());

        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::SetDataFormat()");
        return m_realMouse->SetDataFormat(lpdf);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::SetEventNotification()");
        return m_realMouse->SetEventNotification(hEvent);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD flags) {
        // flags == DISCL_EXCLUSIVE | DISCL_FOREGROUND
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::SetCooperativeLevel(hwnd=0x%08x flags=0x%08x)", hwnd, flags);
        return m_realMouse->SetCooperativeLevel(hwnd, flags);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::GetObjectInfo()");
        return m_realMouse->GetObjectInfo(pdidoi, dwObj, dwHow);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::GetDeviceInfo()");
        return m_realMouse->GetDeviceInfo(pdidi);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::RunControlPanel()");
        return m_realMouse->RunControlPanel(hwndOwner, dwFlags);
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, const GUID& riid) {
        log("DEFAULT IMPLEMENTATION: DirectInputMouseProxy::Initialize()");
        return m_realMouse->Initialize(hinst, dwVersion, riid);
    }
};

#endif // !DIRECT_INPUT_MOUSE_PROXY_HPP
