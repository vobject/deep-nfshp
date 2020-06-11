#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dinput.h>

#include "detours.h"
#include "EmbeddableWebServer.h"

#include "DirectInputProxy.hpp"
#include "context.hpp"
#include "utils.hpp"

#include <mutex>

static Context context;

static Server server;
static THREAD_RETURN_TYPE STDCALL_ON_WIN32 acceptConnectionsThread(void* params)
{
    serverInit(&server);
    server.tag = params;

    acceptConnectionsUntilStoppedFromEverywhereIPv4(&server, 8080);
    return 0;
}

struct Response* createResponseForRequest(const struct Request* request, struct Connection* connection)
{
    DirectInputProxy* const diProxy = static_cast<DirectInputProxy*>(connection->server->tag);

    if (request->path == strstr(request->path, "/mouse"))
    {
        serverMutexLock(connection->server);

        struct Response* response = responseAlloc(200, "OK", "text/html; charset=UTF-8", 0);

        char* mouse_pos = strdupDecodePOSTParam("pos=", request, nullptr);
        char* mouse_action = strdupDecodePOSTParam("action=", request, nullptr);

        if (mouse_pos)
        {
            log("mouse_pos=%s", mouse_pos);

            int x;
            int y;
            if (2 == sscanf(mouse_pos, "%d,%d", &x, &y))
            {
                std::lock_guard<std::mutex> lock(context.mutex);
                context.mouse_cmds.push_back({ MouseCmd::Action::Move, x, y});
                //diProxy->m_mouseInputData.x = x;
                //diProxy->m_mouseInputData.y = y;
            }
            //diProxy->m_mouseInputData.mode = 1;
        }

        if (mouse_action)
        {
            log("mouse_action=%s", mouse_action);

        //    if (0 == strcmp(mouse_click, "1")) {
        //        diProxy->m_mouseInputData.lbtn_click = true;
        //    }
        //    if (0 == strcmp(mouse_click, "2")) {
        //        diProxy->m_mouseInputData.rbtn_click = true;
        //    }
        //    //log("test=1");
        //    //if (::IsWindow(hwnd_out))
        //    //{
        //    //    ::Sleep(2000);
        //    //    auto press_key = [](HWND wnd, UINT_PTR virt_key, int wait_time=250) {
        //    //        ::PostMessage(wnd, WM_KEYDOWN, virt_key, 0x10001);
        //    //        ::Sleep(wait_time);
        //    //        ::PostMessage(wnd, WM_KEYUP, virt_key, 0xc0010001);
        //    //    };

        //    //    auto press_mouse = [](HWND wnd, short x, short y, int wait_time = 250) {
        //    //        LPARAM lparam = (y << 16) | x;
        //    //        log("PRESSING x=%d y=%d lparam=0x%08x", x, y, lparam);
        //    //        ::PostMessage(wnd, WM_LBUTTONDOWN, 1, lparam);
        //    //        ::Sleep(wait_time);
        //    //        ::PostMessage(wnd, WM_LBUTTONUP, 0, lparam);
        //    //    };
        //    //    
        //    //    //press_key(hwnd_out, VK_RETURN);
        //    //    //press_key(hwnd_out, VK_RETURN);
        //    //    press_mouse(hwnd_out, 60, 195);
        //    //    press_mouse(hwnd_out, 100, 300);
        //    //    ::Sleep(1000);
        //    //}
        }

        free(mouse_pos);
        free(mouse_action);

        serverMutexUnlock(connection->server);
        return response;
    }

    if (request->path == strstr(request->path, "/drive"))
    {
        serverMutexLock(connection->server);

        struct Response* response = responseAlloc(200, "OK", "text/html; charset=UTF-8", 0);

        char* dir = strdupDecodePOSTParam("dir=", request, nullptr);
        char* acc = strdupDecodePOSTParam("acc=", request, nullptr);
        char* footbreak = strdupDecodePOSTParam("fb=", request, nullptr);
        char* handbreak = strdupDecodePOSTParam("hb=", request, nullptr);
        char* shift = strdupDecodePOSTParam("shift=", request, nullptr);
        char* cam = strdupDecodePOSTParam("cam=", request, nullptr);
        char* horn = strdupDecodePOSTParam("horn=", request, nullptr);
        char* reset = strdupDecodePOSTParam("reset=", request, nullptr);

        if (dir) {
            const int angle = std::atoi(dir);
            if (angle >= STK_LEFT && angle <= STK_RIGHT) {
                log("steering=%u", angle);
                std::lock_guard<std::mutex> lock(context.mutex);
                context.drive_cmds.push_back({ DriveCmd::Action::Stear, angle });
            }
        }
        if (acc) {
            const int action = std::atoi(acc);
            log("acc=%d", action);
            std::lock_guard<std::mutex> lock(context.mutex);
            context.drive_cmds.push_back({ DriveCmd::Action::Accelerate, (action == 0) ? 0 : 1 });
        }
        if (footbreak) {
            const int action = std::atoi(footbreak);
            log("footbreak=%d", action);
            std::lock_guard<std::mutex> lock(context.mutex);
            context.drive_cmds.push_back({ DriveCmd::Action::Footbreak, (action == 0) ? 0 : 1 });
        }
        if (handbreak) {
            const int action = std::atoi(handbreak);
            log("handbreak=%d", action);
            std::lock_guard<std::mutex> lock(context.mutex);
            context.drive_cmds.push_back({ DriveCmd::Action::Handbreak, (action == 0) ? 0 : 1 });
        }
        if (horn) {
            const int action = std::atoi(horn);
            log("horn=%d", action);
            std::lock_guard<std::mutex> lock(context.mutex);
            context.drive_cmds.push_back({ DriveCmd::Action::Horn, (action == 0) ? 0 : 1 });
        }
        if (shift) {
            const int action = std::atoi(shift);
            log("shift=%d", action);
            std::lock_guard<std::mutex> lock(context.mutex);
            if (action == 0) {
                context.drive_cmds.push_back({ DriveCmd::Action::Downshift, 1 });
            } else if (action == 1) {
                context.drive_cmds.push_back({ DriveCmd::Action::Upshift, 1 });
            }
        }
        if (cam && 0 == strcmp(cam, "1")) {
            log("cam=1");
            std::lock_guard<std::mutex> lock(context.mutex);
            context.drive_cmds.push_back({ DriveCmd::Action::Camera, 1 });
        }
        if (reset && 0 == strcmp(reset, "1")) {
            log("reset=1");
            std::lock_guard<std::mutex> lock(context.mutex);
            context.drive_cmds.push_back({ DriveCmd::Action::Reset, 1 });
        }

        free(dir);
        free(acc);
        free(footbreak);
        free(handbreak);
        free(horn);
        free(shift);
        free(cam);
        free(reset);

        serverMutexUnlock(connection->server);
        return response;
    }

    return responseAllocHTMLWithFormat("Default response\n");
}

HWND(WINAPI* RealCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) = CreateWindowExA;
HWND ProxyCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    const HWND hwnd = RealCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    log("ProxyCreateWindowExA(class=%s window=%s) == %d", lpClassName, lpWindowName, hwnd);
    if (::IsWindow(hwnd) == TRUE && strcmp(lpClassName, "NFS3CLASS") == 0)
    {
        log("ProxyCreateWindowExA() found window 0x%p", hwnd);
        std::lock_guard<std::mutex> lock(context.mutex);
        context.nfs3_hwnd = hwnd;
    }
    return hwnd;
}


BOOL(WINAPI* RealGetMessage)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) = GetMessage;
BOOL WINAPI ProxyGetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    //log("ProxyGetMessage(hwnd=0x%08x msg_hwnd=0x%08x msg=0x%04x wParam=0x%02x lParam=0x%02x time=%d)",
    //    hWnd, lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam, lpMsg->time);
    const BOOL ret = RealGetMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    log("ProxyGetMessage(hwnd=0x%08x msg_hwnd=0x%08x msg=0x%04x wParam=0x%02x lParam=0x%02x time=%d) == %d",
        hWnd, lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam, lpMsg->time, ret);
    return ret;
}

HRESULT(WINAPI * RealDirectInputCreateA)(HINSTANCE, DWORD, LPDIRECTINPUTA*, LPUNKNOWN) = DirectInputCreateA;
HRESULT WINAPI ProxyDirectInputCreateA(HINSTANCE hinst,  DWORD dwVersion, LPDIRECTINPUTA *ppDI, LPUNKNOWN punkOuter)
{
    log("DirectInputCreate(version=%04X)", dwVersion);

    IDirectInput* diReal;
    const HRESULT hResult = RealDirectInputCreateA(hinst, dwVersion, &diReal, punkOuter);
    if (hResult != DI_OK) {
        log("DirectInputCreate() failed");
    }

    DirectInputProxy* diProxy = new DirectInputProxy(diReal, context);

    pthread_t threadHandle;
    pthread_create(&threadHandle, nullptr, &acceptConnectionsThread, diProxy);
    
    *ppDI = diProxy;
    return hResult;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

    if (DLL_PROCESS_ATTACH == dwReason)
    {
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)RealDirectInputCreateA, ProxyDirectInputCreateA);
        DetourAttach(&(PVOID&)RealGetMessage, ProxyGetMessage);
        DetourAttach(&(PVOID&)RealCreateWindowExA, ProxyCreateWindowExA);
        const LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            log("Could not install detour. Error=%d", result);
        }
    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)RealCreateWindowExA, ProxyCreateWindowExA);
        DetourDetach(&(PVOID&)RealGetMessage, ProxyGetMessage);
        DetourDetach(&(PVOID&)RealDirectInputCreateA, ProxyDirectInputCreateA);
        const LONG result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            log("Failed to uninstall detour. Error=%d", result);
        }
    }
    return TRUE;
}

__declspec(dllexport) void dummy_export()
{

}
