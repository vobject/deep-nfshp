#ifndef XBOX360_INPUT_DEVICE_DATA_HPP
#define XBOX360_INPUT_DEVICE_DATA_HPP

#include <deque>
#include <mutex>

#include <windef.h>

struct DriveCmd
{
    enum class Action
    {
        None,
        Stear,
        Accelerate,
        Footbreak,
        Handbreak,
        Upshift,
        Downshift,
        Camera,
        Horn,
        Reset,
    };

    Action action{ Action::None };
    long value{ 0 };
};

struct MouseCmd
{
    enum class Action
    {
        None,
        Move,
        LBtnClick,
    };

    Action action{ Action::None };
    int x{ 0 };
    int y{ 0 };
};

struct Context
{
    HWND nfs3_hwnd{ 0 };
    std::deque<DriveCmd> drive_cmds;
    std::deque<MouseCmd> mouse_cmds;

    // Generic mutex for the whole struct.
    // Lock it before accessing members.
    std::mutex mutex;
};

//struct GamepadCommands {
//    unsigned int steering{ STK_NEUTRAL };
//    bool accelerate{ false };
//    bool footbreak{ false };
//    bool handbreak{ false };
//    bool upshift{ false };
//    bool downshift{ false };
//    bool camera{ false };
//    bool horn{ false };
//    bool reset{ false };
//};
//
//struct MouseCommands {
//    int mode = 0;
//    bool reset{ false };
//    int x{ 0 };
//    int y{ 0 };
//    bool lbtn_click{ false };
//    bool rbtn_click{ false };
//};

#endif // !XBOX360_INPUT_DEVICE_DATA_HPP
