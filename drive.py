import argparse
import os
import time

import cv2
import numpy as np
from tensorflow import keras

import utils
import control

import requests

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
    parser.add_argument("-m", dest="model_filepath", type=str, help="Trained model file")
    parser.add_argument("-p", dest="nfs3_filepath", type=str, help="Path to the NFS3 executable")
    args = parser.parse_args()

    model = keras.models.load_model(args.model_filepath)
    preproc = get_model_preprocessing(args.model_filepath)

    if not args.nfs3_filepath:
        args.nfs3_filepath = r"D:\Games\nfs3_modern_base_eng_beta\nfs3.exe"
    dinput_proxy_filepath = os.path.join(os.path.dirname(os.path.realpath(__file__)), "dinput_proxy/Release/dinput_proxy.dll")
    withdll_filepath = os.path.join(os.path.dirname(os.path.realpath(__file__)), "external/Detours/bin.X86/withdll.exe")

    proc = control.Nfs3()
    proc.launch_or_find(args.nfs3_filepath, dinput_proxy_filepath, withdll_filepath)
    proc.menu.racenow()

    time.sleep(6)

    proc.drive.shift(1)
    proc.drive.acc(1)

    while True:
        screen = grab_screen(wnd_title=utils.WINDOW_TITLE,
                             dst_size=utils.CAPTURE_SIZE,
                             src_offset=utils.WINDOW_OFFSETS)
        screen_mod = preproc(screen)

        # predict the steering angle for the current image
        steering_angle = float(model.predict(np.array([screen_mod]), batch_size=1))

        # translate to vjoy axis value
        j_steering = np.interp(steering_angle, [-.5, +.5], [0, 65535])
        print('{} raw={}'.format('LEFT' if steering_angle < 0 else 'RIGHT', j_steering))

        proc.drive.stear(int(j_steering))

        cv2.imshow('screen', screen_mod)
        if cv2.waitKey(10) & 0xFF == 27: # ESC
            cv2.destroyAllWindows()

            try:
                proc.menu.exit()
            except:
                pass

            break


if __name__ == '__main__':
    main()
