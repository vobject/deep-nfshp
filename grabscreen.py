"""
based on work by Frannecklp
changes by dandrews
more changes by vobject based on https://github.com/speeli/pygta5/blob/master/grabscreen.py
"""

import win32ui
import win32gui
import win32con
import numpy as np


def grab_screen(wnd_title, dst_size=None, src_offset=(0,0,0,0), wnd_handle=None):
    """
    Grabs screens from windows applications.
    Caches the windows api objects and reuses them as long as you call
    screen_grab with the same arguements
    Arguments:
    :parameter regions: tuple of (top, left, height, width). None grabs whole screen.
    :parameter window_title: string to search title bars for. Defaults to 'Game'
    """

    if ((grab_screen.screen is None) or
        (grab_screen._src_offset != src_offset) or
        (grab_screen._dst_size != dst_size) or
        (grab_screen._wnd_title != wnd_title)):
        # Cleanup old object and rebuild with new region.
        if grab_screen.screen is not None:
            grab_screen.screen.cleanup()
        grab_screen.screen = windows_screen_grab(wnd_title, dst_size, src_offset, wnd_handle)

    bits = grab_screen.screen.get_screen_bits()
    rgb = grab_screen.screen.get_rgb_from_bits(bits)

    grab_screen._dst_size = dst_size
    grab_screen._src_offset = src_offset
    grab_screen._wnd_title
    return rgb
grab_screen._wnd_title = ""
grab_screen._dst_size = None
grab_screen._src_offset = (0,0,0,0)
grab_screen.screen = None


class windows_screen_grab:
    _hwnd = 0

    def enumHandler(self, hwnd, lParam):
        """
        Callback to find correct window handle.
        """
        if win32gui.IsWindowVisible(hwnd):
            title = win32gui.GetWindowText(hwnd)
            if self._search_str == title:
                self._hwnd = hwnd

    def __init__(self, wnd_title: str, dst_size, src_offset, wnd_handle = None):
        """
        :parameter wnd_title: substring to search for in titles bars. required.
        :parameter regions: tuple of (top, left, height, width). None grabs whole screen.
        """
        self._search_str = wnd_title
        if wnd_handle:
            self._hwnd = wnd_handle
        else:
            # Look for window title, case sensitive.
            win32gui.EnumWindows(self.enumHandler, None)
            if self._hwnd == 0:
                message = "wnd_title '{}' not found.".format(wnd_title)
                raise ValueError(message)
        self._dst_size = dst_size
        self._src_offset = src_offset

        hwnd = self._hwnd
        l,t,r,b = win32gui.GetWindowRect(hwnd)
        h=b-t
        w=r-l

        if not self._dst_size:
            self._dst_size = (w, h)

        w_handle_DC = win32gui.GetWindowDC(hwnd)
        windowDC = win32ui.CreateDCFromHandle(w_handle_DC)
        memDC = windowDC.CreateCompatibleDC()

        dataBitMap = win32ui.CreateBitmap()
        dataBitMap.CreateCompatibleBitmap(windowDC , self._dst_size[0], self._dst_size[1])

        memDC.SelectObject(dataBitMap)

        self._w_handle_DC = w_handle_DC
        self._dataBitMap = dataBitMap
        self._memDC = memDC
        self._windowDC = windowDC
        self._src_size = (w - src_offset[0] - src_offset[2], h - src_offset[1] - src_offset[3])
        self._top  = t + src_offset[0]
        self._left = l + src_offset[1]


    def get_screen_bits(self):
        """
        Get the raw screen bits.
        returns: a numpy array of the bits in the format (r, g, b, a, r, g, b, a, ...)
        """
        self._memDC.StretchBlt((0,0), self._dst_size, self._windowDC, (self._src_offset[0],self._src_offset[1]), self._src_size, win32con.SRCCOPY)
        bits = np.fromstring(self._dataBitMap.GetBitmapBits(True), np.uint8)
        return bits


    def get_rgb_from_bits(self, bits):
        """
        Reshape to rgb and strip the alpha channel.
        :parameter bits: numpy array in the format (r, g, b, a, r, g, b, a, ...)
        """
        bits.shape = (self._dst_size[1], self._dst_size[0],4)
        self._rgb = bits[:,:,:3:]
#        self._rgb = self._rgb[:,:,::-1]
        return self._rgb

    def cleanup(self):
        """
        Release resources.
        """
        self._windowDC.DeleteDC()
        self._memDC.DeleteDC()
        win32gui.ReleaseDC(self._hwnd, self._w_handle_DC)
        win32gui.DeleteObject(self._dataBitMap.GetHandle())


if __name__ == "__main__":
    from matplotlib import pyplot as plt
    screenshot = grab_screen(wnd_title='Need For Speed III: Hot Pursuit')
    print(screenshot.shape)
    f, axarr = plt.subplots(1, 1)
    axarr.imshow(screenshot)
    plt.show()
