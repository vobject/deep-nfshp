from tensorflow import keras

import cv2


class CommaModel:
    """
    comma.ai's steering model architecture:
    https://github.com/commaai/research/blob/master/train_steering_model.py
    """
    NAME = 'comma'
    IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS = 160, 320, 3
    INPUT_SHAPE = (IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS)

    def __init__(self):
        self.model = None

    def get(self):
        if not self.model:
            self.model = keras.models.Sequential()
            self.model.add(keras.layers.Lambda(lambda x: x/127.5-1.0, input_shape=CommaModel.INPUT_SHAPE, output_shape=CommaModel.INPUT_SHAPE))
            self.model.add(keras.layers.Conv2D(16, kernel_size=8, strides=4, padding='same', activation='elu'))
            self.model.add(keras.layers.Conv2D(32, kernel_size=5, strides=2, padding='same', activation='elu'))
            self.model.add(keras.layers.Conv2D(64, kernel_size=5, strides=2, padding='same'))
            self.model.add(keras.layers.Flatten())
            self.model.add(keras.layers.Dropout(0.2))
            self.model.add(keras.layers.ELU())
            self.model.add(keras.layers.Dense(512))
            self.model.add(keras.layers.Dropout(0.5))
            self.model.add(keras.layers.ELU())
            self.model.add(keras.layers.Dense(1))
        return self.model

    @staticmethod
    def preprocess(image):
        image = image[56:, :, :]
        image = cv2.resize(image, (CommaModel.IMAGE_WIDTH, CommaModel.IMAGE_HEIGHT), cv2.INTER_LINEAR)
        return image


def main():
    pass


if __name__ == '__main__':
    main()
