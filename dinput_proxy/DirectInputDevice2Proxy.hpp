#ifndef DIRECT_INPUT_DEVICE2_PROXY_HPP
#define DIRECT_INPUT_DEVICE2_PROXY_HPP

#include "context.hpp"
#include "utils.hpp"

#include <dinput.h>

#include <map>
#include <mutex>

const unsigned int STK_LEFT{ 0x00000000 };
const unsigned int STK_RIGHT{ 0x0000FFFF };
const unsigned int STK_UP{ 0x00000000 };
const unsigned int STK_DOWN{ 0x0000FFFF };
const unsigned int STK_NEUTRAL{ 0x00007FFF };

const unsigned int TRG_LEFT{ 0x0000FF80 };
const unsigned int TRG_RIGHT{ 0x00000080 };
const unsigned int TRG_NEUTRAL{ 0x00007FFF };

const unsigned int DPD_UP{ 0x00000000 };
const unsigned int DPD_DOWN{ 0x00004650 };
const unsigned int DPD_LEFT{ 0x00006978 };
const unsigned int DPD_RIGHT{ 0x00002328 };
const unsigned int DPD_LEFT_UP{ 0x00007B0C };
const unsigned int DPD_LEFT_DOWN{ 0x000057E4 };
const unsigned int DPD_RIGHT_UP{ 0x00001194 };
const unsigned int DPD_RIGHT_DOWN{ 0x000034BC };
const unsigned int DPD_NEUTRAL{ 0xFFFFFFFF };

const unsigned char BTN_DOWN{ 0x80 };
const unsigned char BTN_UP{ 0x00 };

struct DirectInputDevice2Data
{
    // Left stick
    unsigned int stk_left_horizontal{ STK_NEUTRAL };
    unsigned int stk_left_vertical{ STK_NEUTRAL };

    // Triggers
    unsigned int triggers{ STK_NEUTRAL };

    // Right stick
    unsigned int stk_right_horizontal{ STK_NEUTRAL };
    unsigned int stk_right_vertical{ STK_NEUTRAL };

    unsigned int unknown_20_23{ 0x00000000 }; // default 0x00000000
    unsigned int unknown_24_27{ 0x00000000 }; // default 0x00000000
    unsigned int unknown_28_31{ 0x00000000 }; // default 0x00000000

    // D-Pad
    unsigned int dpad{ 0x00000000 };

    unsigned int unknown_36_39{ 0xFFFFFFFF }; // default 0xFFFFFFFF
    unsigned int unknown_40_43{ 0xFFFFFFFF }; // default 0xFFFFFFFF
    unsigned int unknown_44_47{ 0xFFFFFFFF }; // default 0xFFFFFFFF

    // Buttons
    unsigned char btn_A{ BTN_UP };
    unsigned char btn_B{ BTN_UP };
    unsigned char btn_X{ BTN_UP };
    unsigned char btn_Y{ BTN_UP };
    unsigned char btn_LB{ BTN_UP };
    unsigned char btn_RB{ BTN_UP };
    unsigned char btn_SELECT{ BTN_UP };
    unsigned char btn_START{ BTN_UP };
    unsigned char unknown_56{ 0x00 }; // default 0x00
    unsigned char unknown_57{ 0x00 }; // default 0x00
    unsigned char btn_stk_left{ BTN_UP };
    unsigned char btn_stk_right{ BTN_UP };

    unsigned int unknown_60_63{ 0x00000000 }; // default 0x00000000
    unsigned int unknown_64_67{ 0x00000000 }; // default 0x00000000
    unsigned int unknown_68_71{ 0x00000000 }; // default 0x00000000
    unsigned int unknown_72_75{ 0x00000000 }; // default 0x00000000
    unsigned int unknown_76_79{ 0x00000000 }; // default 0x00000000
};
static_assert(sizeof(DirectInputDevice2Data) == 80, "DeviceData: unexpected size");

struct DirectInputDevice2Proxy : IDirectInputDevice2
{
    Context& m_ctx;
    DirectInputDevice2Data m_devicedata;
 
    explicit DirectInputDevice2Proxy(Context& ctx)
        : m_ctx{ ctx }
    { }

    /*** IUnknown methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, LPVOID* ppvObj) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::QueryInterface()");
        return E_NOINTERFACE;
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE AddRef() {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::AddRef() -> 1");
        return 1;
    }

    virtual COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE Release() {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::Release() -> 0");
        return 0;
    }

    /*** IDirectInputDeviceA methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
        log("DirectInputDevice2Proxy::GetCapabilities() -> DI_OK");

        // Similar capabilities as an XBOX360 controller but simplified
        static const DIDEVCAPS deviceCapabilities = {
            sizeof(DIDEVCAPS),
            DIDC_ATTACHED | DIDC_EMULATED,
            (DIDEVTYPE_JOYSTICK << 8) | DIDEVTYPE_JOYSTICK,
            5,  // Axis
            10, // Buttons
            1,  // POV
        #if(DIRECTINPUT_VERSION >= 0x0500)
            0, 0, 0, 0, 0,
        #endif // DIRECTINPUT_VERSION >= 0x0500
        };

        std::memmove(lpDIDevCaps, &deviceCapabilities, sizeof(DIDEVCAPS));
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::EnumObjects() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetProperty(const GUID& rguidProp, LPDIPROPHEADER pdiph) {
        log("DirectInputDevice2Proxy::GetProperty() -> return DI_OK;");

        // pdiph->dwObj is the memory index of the axis that's queried (0x00, 0x04, 0x08,
        // 0x0C, 0x10). Return the same dead zone and saturation no matter the axis.
        
        // No idea how to properly deal with this guid prop crap
        const long prop = (long)(DWORD*)(&rguidProp);

        // The header is part of a larger structure, which is always DIPROPDWORD in our case.
        const LPDIPROPDWORD diPropWord = (LPDIPROPDWORD)pdiph;

        if (prop == 5) {
            // rguidProp == DIPROP_DEADZONE
            diPropWord->dwData = 0;
        } else if (prop == 6 ) {
            // rguidProp == DIPROP_SATURATION
            diPropWord->dwData = 10000;
        } else {
            // rguidProp == ???
            log("NOT IMPLEMENTED: GetProperty(prop=%08X)", prop);
            return DIERR_UNSUPPORTED;
        }
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetProperty(const GUID& riid, LPCDIPROPHEADER pdiph) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::SetProperty() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Acquire() {
        log("DirectInputDevice2Proxy::Acquire()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Unacquire() {
        log("DirectInputDevice2Proxy::Unacquire()");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
        // log("DirectInputDevice2Proxy::GetDeviceState(size=%d) -> %d", cbData, DI_OK);

        // Reset temporary button states.
        m_devicedata.btn_X = BTN_UP; // Upshift
        m_devicedata.btn_LB = BTN_UP; // Downshift
        m_devicedata.btn_RB = BTN_UP; // Camera
        m_devicedata.btn_START = BTN_UP; // Reset

        if (!m_ctx.drive_cmds.empty())
        {
            std::lock_guard<std::mutex> lock(m_ctx.mutex);

            while (true)
            {
                if (m_ctx.drive_cmds.empty())
                {
                    break;
                }

                const auto& cmd = m_ctx.drive_cmds.front();
                if (cmd.action == DriveCmd::Action::None)
                {
                    m_ctx.drive_cmds.pop_front();
                    break;
                }

                switch (cmd.action)
                {
                case DriveCmd::Action::Stear:
                    m_devicedata.stk_left_horizontal = cmd.value;
                    break;
                case DriveCmd::Action::Accelerate:
                    m_devicedata.btn_A = cmd.value == 0 ? BTN_UP : BTN_DOWN;
                    break;
                case DriveCmd::Action::Footbreak:
                    m_devicedata.btn_B = cmd.value == 0 ? BTN_UP : BTN_DOWN;
                    break;
                case DriveCmd::Action::Handbreak:
                    m_devicedata.btn_X = cmd.value == 0 ? BTN_UP : BTN_DOWN;
                    break;
                case DriveCmd::Action::Upshift:
                    m_devicedata.btn_Y = cmd.value == 0 ? BTN_UP : BTN_DOWN;
                    break;
                case DriveCmd::Action::Downshift:
                    m_devicedata.btn_LB = cmd.value == 0 ? BTN_UP : BTN_DOWN;
                    break;
                case DriveCmd::Action::Horn:
                    m_devicedata.btn_SELECT = cmd.value == 0 ? BTN_UP : BTN_DOWN;
                    break;
                case DriveCmd::Action::Camera:
                    m_devicedata.btn_RB = cmd.value;
                    break;
                case DriveCmd::Action::Reset:
                    m_devicedata.btn_START = cmd.value;
                    break;
                default:
                    break;
                }
                m_ctx.drive_cmds.pop_front();
            }
        }

        std::memmove(lpvData, &m_devicedata, sizeof(m_devicedata));
        return DI_OK;
    }
    
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::GetDeviceData() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::SetDataFormat() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::SetEventNotification() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD flags) {
        // flags = DISCL_EXCLUSIVE | DISCL_BACKGROUND
        log("DirectInputDevice2Proxy::SetCooperativeLevel(flags=%08X) -> DI_OK", flags);
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow) {
        log("DirectInputDevice2Proxy::GetObjectInfo() -> DI_OK");

        if (dwHow != DIPH_BYOFFSET) {
            return DIERR_INVALIDPARAM;
        }

        static const std::map<DWORD, DIDEVICEOBJECTINSTANCE> deviceObjectInstanceMap = {
            { 0x00, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_XAxis,  0x00, 0x0000 | DIDFT_ABSAXIS,   0x0100, "X Axis", } },
            { 0x04, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_YAxis,  0x04, 0x0100 | DIDFT_ABSAXIS,   0x0100, "Y Axis", } },
            { 0x08, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_ZAxis,  0x08, 0x0200 | DIDFT_ABSAXIS,   0x0100, "Z Axis", } },
            { 0x0C, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_XAxis,  0x0C, 0x0300 | DIDFT_ABSAXIS,   0x0100, "X Rotation", } },
            { 0x10, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_YAxis,  0x10, 0x0400 | DIDFT_ABSAXIS,   0x0100, "Y Rotation", } },
            { 0x20, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_POV,    0x20,          DIDFT_POV,       0x0000, "Hat Switch", } },
            { 0x30, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x30, 0x0000 | DIDFT_PSHBUTTON, 0x0000, "Button 0", } },
            { 0x31, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x31, 0x0100 | DIDFT_PSHBUTTON, 0x0100, "Button 1", } },
            { 0x32, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x32, 0x0200 | DIDFT_PSHBUTTON, 0x0200, "Button 2", } },
            { 0x33, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x33, 0x0300 | DIDFT_PSHBUTTON, 0x0300, "Button 3", } },
            { 0x34, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x34, 0x0400 | DIDFT_PSHBUTTON, 0x0400, "Button 4", } },
            { 0x35, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x35, 0x0500 | DIDFT_PSHBUTTON, 0x0500, "Button 5", } },
            { 0x36, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x36, 0x0600 | DIDFT_PSHBUTTON, 0x0600, "Button 6", } },
            { 0x37, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x37, 0x0700 | DIDFT_PSHBUTTON, 0x0700, "Button 7", } },
            { 0x38, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x38, 0x0800 | DIDFT_PSHBUTTON, 0x0800, "Button 8", } },
            { 0x39, { sizeof(DIDEVICEOBJECTINSTANCE), GUID_Button, 0x39, 0x0900 | DIDFT_PSHBUTTON, 0x0900, "Button 9", } },
        };

        const auto objInfoIter = deviceObjectInstanceMap.find(dwObj);
        if (objInfoIter != deviceObjectInstanceMap.end()) {
            std::memmove(pdidoi, &objInfoIter->second, sizeof(DIDEVICEOBJECTINSTANCE));
        }

        log("obj=%08X how=%08X size=%08X dwOfs=%08X type=%08X flags=%08X ", dwObj, dwHow, pdidoi->dwSize, pdidoi->dwOfs, pdidoi->dwType, pdidoi->dwFlags, pdidoi->tszName);
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCE pdidi) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::GetDeviceInfo() -> DI_OK");

//         pdidi->dwSize;
//         pdidi->guidInstance;
//         pdidi->guidProduct;
//         pdidi->dwDevType;
//         pdidi->tszInstanceName[MAX_PATH];
//         pdidi->tszProductName[MAX_PATH];
// #if(DIRECTINPUT_VERSION >= 0x0500)
//         pdidi->guidFFDriver;
//         pdidi->wUsagePage;
//         pdidi->wUsage;
// #endif // DIRECTINPUT_VERSION >= 0x0500

        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags) {
        log("DirectInputDevice2Proxy::RunControlPanel() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion, const GUID& riid) {
        log("DirectInputDevice2Proxy::Initialize() -> DI_OK");
        return DI_OK;
    }

    /*** IDirectInputDevice2A methods ***/
    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE CreateEffect(const GUID& riid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT *ppdeff, LPUNKNOWN punkOuter) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::CreateEffect() -> DIERR_DEVICEFULL");
        return DIERR_DEVICEFULL;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::EnumEffects() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFOA pdei, const GUID& riid) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::GetEffectInfo() -> DIERR_NOTINITIALIZED");
        return DIERR_NOTINITIALIZED;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::GetForceFeedbackState() -> DIERR_UNSUPPORTED");
        return DIERR_UNSUPPORTED;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::SendForceFeedbackCommand() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::EnumCreatedEffectObjects() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::Escape() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Poll() {
        //log("DirectInputDevice2Proxy::Poll() -> DI_OK");
        return DI_OK;
    }

    virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) {
        log("NOT IMPLEMENTED: DirectInputDevice2Proxy::SendDeviceData() -> DI_OK");
        return DI_OK;
    }
};

#endif // !DIRECT_INPUT_DEVICE2_PROXY_HPP
