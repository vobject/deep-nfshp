from tensorflow import keras

import cv2


class NvidiaModel:
    """
    Modified NVIDIA model based on https://github.com/naokishibuya/car-behavioral-cloning
    """
    NAME = 'nvidia'
    IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS = 66, 200, 3
    INPUT_SHAPE = (IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS)

    def __init__(self):
        self.model = None

    def get(self):
        if not self.model:
            self.model = keras.models.Sequential()
            self.model.add(keras.layers.Lambda(lambda x: x/127.5-1.0, input_shape=NvidiaModel.INPUT_SHAPE))
            self.model.add(keras.layers.Conv2D(24, kernel_size=5, strides=2, activation='elu'))
            self.model.add(keras.layers.Conv2D(36, kernel_size=5, strides=2, activation='elu'))
            self.model.add(keras.layers.Conv2D(48, kernel_size=5, strides=2, activation='elu'))
            self.model.add(keras.layers.Conv2D(64, kernel_size=3, activation='elu'))
            self.model.add(keras.layers.Conv2D(64, kernel_size=3, activation='elu'))
            self.model.add(keras.layers.Dropout(0.5))
            self.model.add(keras.layers.Flatten())
            self.model.add(keras.layers.Dense(100, activation='elu'))
            self.model.add(keras.layers.Dense(50, activation='elu'))
            self.model.add(keras.layers.Dense(10, activation='elu'))
            self.model.add(keras.layers.Dense(1))
        return self.model

    @staticmethod
    def preprocess(image):
        image = image[112:, :, :]
        image = cv2.resize(image, (NvidiaModel.IMAGE_WIDTH, NvidiaModel.IMAGE_HEIGHT), cv2.INTER_AREA)
        image = cv2.cvtColor(image, cv2.COLOR_RGB2YUV)
        return image


def main():
    pass


if __name__ == '__main__':
    main()
