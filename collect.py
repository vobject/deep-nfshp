import os
import datetime

import cv2

import utils

# Get the pixel data of a window:
# Based on https://github.com/speeli/pygta5/blob/master/grabscreen.py
from grabscreen import grab_screen

# Read Xbox 360 Controller input on Windows:
# https://github.com/r4dian/Xbox-360-Controller-for-Python
from xinput import xinput


def create_data_dir(parent_dir='data'):
    """
    Create and return a subdirectory in the given data parent directory.
    """
    DATA_FOLDER_FORMAT = '%Y%m%d%H%M%S'
    dir_name = datetime.datetime.now().strftime(DATA_FOLDER_FORMAT)
    dir_path = os.path.join(parent_dir, dir_name)
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)
    return dir_path


def write_datapoint(out_dir, data_id, image, steering):
    """
    Write an image and it's metadata to disk.
    """
    timestamp = int(datetime.datetime.utcnow().timestamp())
    image_path = os.path.join(out_dir, '{:06d}_{}_{}.jpg'.format(data_id, timestamp, steering))
    cv2.imwrite(image_path, image)


def create_joystick(index, on_accelerate, on_steer):
    joysticks = xinput.XInputJoystick.enumerate_devices()
    if not joysticks:
        raise Exception('No joystick found')

    j = joysticks[index]

    @j.event
    def on_button(button, pressed):
        # Button 13 ('A') accelerates
        if button == 13:
            on_accelerate(pressed == 1)

    @j.event
    def on_axis(axis, value):
        # Left thumb stick is for steering
        if axis == 'l_thumb_x':
            on_steer(value)
            
    return j


class InputHandler:
    record_dir = None
    image = None
    image_id = 0
    
    @staticmethod
    def on_accelerate(active):
        # Recording is active as long as the user accelerates. Create a new
        # directory for the image+input data.
        InputHandler.record_dir = create_data_dir() if active else None
        InputHandler.image_id = InputHandler.image_id if active else 0

    @staticmethod
    def on_steer(value):
        if InputHandler.record_dir:
            write_datapoint(InputHandler.record_dir, InputHandler.image_id, InputHandler.image, value)
            InputHandler.image_id += 1
            

def main():
    j = create_joystick(0, InputHandler.on_accelerate, InputHandler.on_steer)

    while True:
        screen = grab_screen(wnd_title=utils.WINDOW_TITLE,
                             dst_size=utils.CAPTURE_SIZE,
                             src_offset=utils.WINDOW_OFFSETS)
        
        InputHandler.image = screen
        j.dispatch_events()

        cv2.imshow('screen', screen)
        if cv2.waitKey(50) & 0xFF == 27: # ESC
            cv2.destroyAllWindows()
            break


if __name__ == '__main__':
    main()
