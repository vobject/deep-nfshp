from tensorflow import keras

import cv2
import numpy as np


class TinyModel:
    """
    Self-driving car in a simulator with a tiny neural network:
    https://medium.com/@xslittlegrass/self-driving-car-in-a-simulator-with-a-tiny-neural-network-13d33b871234
    """
    NAME = 'tiny'
    IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS = 16, 32, 1
    INPUT_SHAPE = (IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS)

    def __init__(self):
        self.model = None

    def get(self):
        if not self.model:
            self.model = keras.models.Sequential()
            self.model.add(keras.layers.Lambda(lambda x: x/127.5-1.0, input_shape=TinyModel.INPUT_SHAPE))
            self.model.add(keras.layers.Conv2D(2, kernel_size=3, padding='valid', activation='elu', input_shape=TinyModel.INPUT_SHAPE))
            self.model.add(keras.layers.MaxPooling2D(pool_size=4, strides=4, padding='valid'))
            self.model.add(keras.layers.Dropout(0.25))
            self.model.add(keras.layers.Flatten())
            self.model.add(keras.layers.Dense(1))
        return self.model

    @staticmethod
    def preprocess(image):
        image = cv2.cvtColor(image, cv2.COLOR_RGB2HSV)
        image = image[:,:,1]
        image = cv2.resize(image, (TinyModel.IMAGE_WIDTH, TinyModel.IMAGE_HEIGHT))
        image = np.expand_dims(image, axis=2)
        return image


def main():
    pass


if __name__ == '__main__':
    main()
