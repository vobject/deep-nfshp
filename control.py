import win32ui
import win32gui
import win32con
import win32com.client

import enum
import logging
import time

import requests

import transitions


# Based on https://gist.github.com/chriskiehl/2906125
VK_CODE = {
    'backspace':0x08,
    'tab':0x09,
    'clear':0x0C,
    'enter':0x0D,
    'shift':0x10,
    'ctrl':0x11,
    'alt':0x12,
    'pause':0x13,
    'caps_lock':0x14,
    'esc':0x1B,
    'spacebar':0x20,
    'page_up':0x21,
    'page_down':0x22,
    'end':0x23,
    'home':0x24,
    'left_arrow':0x25,
    'up_arrow':0x26,
    'right_arrow':0x27,
    'down_arrow':0x28,
    'select':0x29,
    'print':0x2A,
    'execute':0x2B,
    'print_screen':0x2C,
    'ins':0x2D,
    'del':0x2E,
    'help':0x2F,
    '0':0x30,
    '1':0x31,
    '2':0x32,
    '3':0x33,
    '4':0x34,
    '5':0x35,
    '6':0x36,
    '7':0x37,
    '8':0x38,
    '9':0x39,
    'a':0x41,
    'b':0x42,
    'c':0x43,
    'd':0x44,
    'e':0x45,
    'f':0x46,
    'g':0x47,
    'h':0x48,
    'i':0x49,
    'j':0x4A,
    'k':0x4B,
    'l':0x4C,
    'm':0x4D,
    'n':0x4E,
    'o':0x4F,
    'p':0x50,
    'q':0x51,
    'r':0x52,
    's':0x53,
    't':0x54,
    'u':0x55,
    'v':0x56,
    'w':0x57,
    'x':0x58,
    'y':0x59,
    'z':0x5A,
    'numpad_0':0x60,
    'numpad_1':0x61,
    'numpad_2':0x62,
    'numpad_3':0x63,
    'numpad_4':0x64,
    'numpad_5':0x65,
    'numpad_6':0x66,
    'numpad_7':0x67,
    'numpad_8':0x68,
    'numpad_9':0x69,
    'multiply_key':0x6A,
    'add_key':0x6B,
    'separator_key':0x6C,
    'subtract_key':0x6D,
    'decimal_key':0x6E,
    'divide_key':0x6F,
    'F1':0x70,
    'F2':0x71,
    'F3':0x72,
    'F4':0x73,
    'F5':0x74,
    'F6':0x75,
    'F7':0x76,
    'F8':0x77,
    'F9':0x78,
    'F10':0x79,
    'F11':0x7A,
    'F12':0x7B,
    'F13':0x7C,
    'F14':0x7D,
    'F15':0x7E,
    'F16':0x7F,
    'F17':0x80,
    'F18':0x81,
    'F19':0x82,
    'F20':0x83,
    'F21':0x84,
    'F22':0x85,
    'F23':0x86,
    'F24':0x87,
    'num_lock':0x90,
    'scroll_lock':0x91,
    'left_shift':0xA0,
    'right_shift ':0xA1,
    'left_control':0xA2,
    'right_control':0xA3,
    'left_menu':0xA4,
    'right_menu':0xA5,
    'browser_back':0xA6,
    'browser_forward':0xA7,
    'browser_refresh':0xA8,
    'browser_stop':0xA9,
    'browser_search':0xAA,
    'browser_favorites':0xAB,
    'browser_start_and_home':0xAC,
    'volume_mute':0xAD,
    'volume_Down':0xAE,
    'volume_up':0xAF,
    'next_track':0xB0,
    'previous_track':0xB1,
    'stop_media':0xB2,
    'play/pause_media':0xB3,
    'start_mail':0xB4,
    'select_media':0xB5,
    'start_application_1':0xB6,
    'start_application_2':0xB7,
    'attn_key':0xF6,
    'crsel_key':0xF7,
    'exsel_key':0xF8,
    'play_key':0xFA,
    'zoom_key':0xFB,
    'clear_key':0xFE,
    '+':0xBB,
    ',':0xBC,
    '-':0xBD,
    '.':0xBE,
    '/':0xBF,
    '`':0xC0,
    ';':0xBA,
    '[':0xDB,
    '\\':0xDC,
    ']':0xDD,
    "'":0xDE,
}


def drive_cmd(action: str, value: int):
    logging.warning(f"drive_cmd(action={action}, value={value})")
    try:
        requests.post("http://localhost:8080/drive", data=f"{action}={value}", timeout=0.001)
    except:
        pass


def move_mouse(x: int, y: int):
    logging.warning(f"move_mouse(x={x}, y={y})")
    try:
        requests.post("http://localhost:8080/mouse", data={"pos": f"{x},{y}"}, timeout=0.001)
    except:
        pass


def set_cursor(x: int, y: int, wait_time: float=.25):
    move_mouse(-8192, -8192)
    time.sleep(wait_time)
    move_mouse(x, y)
    time.sleep(wait_time)


def press_key(wnd, key: int, wait_time: float=.25):
    virt_key = VK_CODE[key]
    logging.warning(f"press_key: wnd={wnd} key={key} virt_key={virt_key}")
    wnd.PostMessage(win32con.WM_KEYDOWN, virt_key, 0x10001),
    time.sleep(wait_time)
    wnd.PostMessage(win32con.WM_KEYUP, virt_key, 0xc0010001),


MENU_TRANSITION_OPS = {
    "main_to_setup": lambda wnd: (
        set_cursor(60, 195),
        press_key(wnd, "enter"),
        set_cursor(310, 195),
        press_key(wnd, "enter"),
        time.sleep(1.),
    ),
    "main_to_race": lambda wnd: (
        set_cursor(60, 195),
        press_key(wnd, "enter"),
        set_cursor(310, 195),
        press_key(wnd, "enter"),
        MENU_TRANSITION_OPS["setup_to_race"](wnd),
    ),
    "main_to_exit": lambda wnd: (
        set_cursor(60, 195),
        press_key(wnd, "esc"),
        set_cursor(290, 255),
        press_key(wnd, "enter"),
        time.sleep(1.),
    ),

    "setup_to_main": lambda wnd: (
        press_key(wnd, "esc"),
        time.sleep(1.),
    ),
    "setup_to_race": lambda wnd: (
        # press_key(wnd, "up_arrow"),
        # press_key(wnd, "up_arrow"),
        # press_key(wnd, "up_arrow"),
        set_cursor(350, 450),
        press_key(wnd, "enter"),
        time.sleep(4.),

        win32com.client.Dispatch("WScript.Shell").AppActivate("Need For Speed III: Hot Pursuit"),
    ),
    "setup_to_exit": lambda wnd: (
        MENU_TRANSITION_OPS["setup_to_main"](wnd),
        MENU_TRANSITION_OPS["main_to_exit"](wnd),
    ),

    "race_to_setup": lambda wnd: (
        press_key(wnd, "esc"),
        set_cursor(900, 720),
        press_key(wnd, "enter"),
        set_cursor(500, 340),
        press_key(wnd, "enter"),
        set_cursor(480, 410),
        press_key(wnd, "enter"),

        # press_key(wnd, "tab", .5),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "enter"),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "enter"),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "enter"),
        time.sleep(2.),
    ),
    "race_to_main": lambda wnd: (
        MENU_TRANSITION_OPS["race_to_setup"](wnd),
        MENU_TRANSITION_OPS["setup_to_main"](wnd),
    ),
    "race_to_exit": lambda wnd: (
        press_key(wnd, "esc"),
        set_cursor(900, 720),
        press_key(wnd, "enter"),
        set_cursor(500, 430),
        press_key(wnd, "enter"),
        set_cursor(450, 410),
        press_key(wnd, "enter"),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "enter"),
        # press_key(wnd, "up_arrow"),
        # press_key(wnd, "enter"),
        # press_key(wnd, "tab", .5),
        # press_key(wnd, "enter"),
    ),
}


class Nfs3MenuControl:
    MENU_SCREENS = [
        "main",
        "setup",
        "race",
        "exit",
    ]

    MENU_TRANSITIONS = [
        { "trigger": "setupmenu", "source": "main", "dest": "setup", "before": "switchto_menu", },
        { "trigger": "racenow", "source": "main", "dest": "race", "before": "switchto_menu", },
        { "trigger": "exit", "source": "main", "dest": "exit", "before": "switchto_menu", },

        { "trigger": "mainmenu", "source": "setup", "dest": "main", "before": "switchto_menu", },
        { "trigger": "racenow", "source": "setup", "dest": "race", "before": "switchto_menu", },
        { "trigger": "exit", "source": "setup", "dest": "exit", "before": "switchto_menu", },

        { "trigger": "setupmenu", "source": "race", "dest": "setup", "before": "switchto_menu", },
        { "trigger": "mainmenu", "source": "race", "dest": "main", "before": "switchto_menu", },
        { "trigger": "exit", "source": "race", "dest": "exit", "before": "switchto_menu", },
    ]

    def __init__(self, hwnd, wnd):
        self.hwnd = hwnd
        self.wnd = wnd

        self.machine = transitions.Machine(
            model=self,
            states=Nfs3MenuControl.MENU_SCREENS,
            transitions=Nfs3MenuControl.MENU_TRANSITIONS,
            send_event=True,
            initial="main"
        )

    def switchto_menu(self, event: transitions.EventData):
        operation = f"{event.transition.source}_to_{event.transition.dest}"
        print("switchto_menu", operation)
        MENU_TRANSITION_OPS[operation](self.wnd)


class Nfs3DriveControl:
    def stear(self, direction):
        drive_cmd("dir", direction)
    def acc(self, active):
        drive_cmd("acc", 1 if active else 0)
    def fb(self, active):
        drive_cmd("fb", 1 if active else 0)
    def hb(self, active):
        drive_cmd("hb", 1 if active else 0)
    def shift(self, value):
        drive_cmd("shift", value)
    def horn(self, active):
        drive_cmd("horn", 1 if active else 0)
    def cam(self, active):
        drive_cmd("cam", 1 if active else 0)
    def reset(self, active):
        drive_cmd("reset", 1 if active else 0)


class Nfs3:
    WINDOW_CLASS = "NFS3CLASS"
    WINDOW_TEXT = "Need For Speed III: Hot Pursuit"

    def __init__(self):
        self.shell = win32com.client.Dispatch("WScript.Shell")
        self.hwnd = None
        self.wnd = None
        self.menu = None
        self.drive = None

    def launch_or_find(self, exe: str, dll_hook: str=None, withdll: str=None, find_by_class: bool=True):
        if self._find_window(find_by_class):
            return

        self._launch_process(exe, dll_hook, withdll)
        self._find_window(find_by_class)

    def press(self, keys, wait_time: float=.25):
        for key in [keys] if isinstance(keys, str) else keys:
            press_key(self.wnd, VK_CODE[key], wait_time)

    def _launch_process(self, exe: str=None, dll_hook: str=None, withdll: str=None):
        cmd = f"{withdll} /d:{dll_hook} {exe}" if (dll_hook and withdll) else exe
        self.shell.Run(cmd)

        # Wait for process to launch.
        time.sleep(1.)

    def _find_window(self, by_class: bool=True):
        hwnd = win32gui.FindWindowEx(0, 0, Nfs3.WINDOW_CLASS, None) if by_class else win32gui.FindWindowEx(0, 0, None, Nfs3.WINDOW_TEXT)
        if not hwnd:
            # TODO: print GetLastError()
            return False
        try:
            wnd = win32ui.CreateWindowFromHandle(hwnd)
        except win32ui.error:
            return False
        self.hwnd = hwnd
        self.wnd = wnd
        # TODO: find out which menu the process is currently in
        self.menu = Nfs3MenuControl(self.hwnd, self.wnd)
        self.drive = Nfs3DriveControl()
        self.shell.AppActivate(Nfs3.WINDOW_TEXT),
        return True
