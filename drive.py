import argparse

import cv2
import numpy as np
from tensorflow import keras

import utils

# Emulate an Xbox 360 controller:
# https://github.com/tidzo/pyvjoy
import pyvjoy

# Get the pixel data of a window:
# Based on https://github.com/speeli/pygta5/blob/master/grabscreen.py
from grabscreen import grab_screen


def get_model_preprocessing(name):
    from models.nvidia import NvidiaModel
    from models.comma import CommaModel
    from models.tiny import TinyModel

    if name:
        # assume NVIDIA model by default
        preproc = NvidiaModel.preprocess
        if CommaModel.NAME in name:
            preproc = CommaModel.preprocess
        elif TinyModel.NAME in name:
            preproc = TinyModel.preprocess
        return preproc


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-m', dest='model_filepath',   type=str, help='trained model file')
    args = parser.parse_args()

    model = keras.models.load_model(args.model_filepath)
    preproc = get_model_preprocessing(args.model_filepath)

    MAX_VJOY = 32767
    j = pyvjoy.VJoyDevice(1)

    while True:
        # accelerate permanently
        j.set_button(13, 1)

        screen = grab_screen(wnd_title=utils.WINDOW_TITLE,
                             dst_size=utils.CAPTURE_SIZE,
                             src_offset=utils.WINDOW_OFFSETS)
        screen_mod = preproc(screen)

        # predict the steering angle for the current image
        steering_angle = float(model.predict(np.array([screen_mod]), batch_size=1))

        # translate to vjoy axis value
        j_steering = np.interp(steering_angle, [-.5, +.5], [0, MAX_VJOY])
        print('{} angle={}'.format('LEFT' if steering_angle < 0 else 'RIGHT', steering_angle))

        j.data.wAxisX = int(j_steering)
        j.update()

        cv2.imshow('screen', screen_mod)
        if cv2.waitKey(10) & 0xFF == 27: # ESC
            j.reset()
            j.update()
            cv2.destroyAllWindows()
            break


if __name__ == '__main__':
    main()
