import os
import argparse
import datetime

import cv2
import keras.models
import keras.layers
import keras.callbacks
import keras.optimizers
import numpy as np
import sklearn.model_selection

import utils


def training_generator(image_paths, steering_angles, batch_size):
    """
    Generate training data.
    """
    images = np.empty([batch_size, utils.IMAGE_HEIGHT, utils.IMAGE_WIDTH, utils.IMAGE_CHANNELS])
    steers = np.empty(batch_size)
    while True:
        for i in range(0, batch_size, 2):
            index = np.random.choice(len(image_paths))
            image = cv2.imread(image_paths[index])
            steering_angle = steering_angles[index]

            image = utils.preprocess(image)

            # add the image and steering angle to the batch
            images[i] = image
            steers[i] = steering_angle

            # add flipped the image and steering angle to the batch
            images[i+1] = cv2.flip(image, 1)
            steers[i+1] = -steering_angle
        yield images, steers


def validation_generator(image_paths, steering_angles, batch_size):
    """
    Generate cross validation data.
    """
    images = np.empty([batch_size, utils.IMAGE_HEIGHT, utils.IMAGE_WIDTH, utils.IMAGE_CHANNELS])
    steers = np.empty(batch_size)
    while True:
        for i in range(batch_size):
            index = np.random.choice(len(image_paths))
            image = cv2.imread(image_paths[index])
            steering_angle = steering_angles[index]

            images[i] = utils.preprocess(image)
            steers[i] = steering_angle
        yield images, steers


def read_datapoint(path):
    """
    Read an image and it's metadata from disk.
    """
    fname = os.path.basename(path)
    fname = os.path.splitext(fname)[0]
    elems = fname.split("_")
    if len(elems) != 3:
        raise Exception('Invalid data set: ' + path)

    # return id, timestamp, steering value
    return int(elems[0]), int(elems[1]), float(elems[2])


def load_data(data_dir, test_size):
    """
    Load data from disk and split it into training and validation sets.
    """
    X = []
    y = []

    sub_dirs = os.listdir(data_dir)
    for sub_dir in sub_dirs:
        sub_dir_path = os.path.join(data_dir, sub_dir)
        img_files = os.listdir(sub_dir_path)
        for img_file in img_files:
            img_path = os.path.join(sub_dir_path, img_file)
            _, _, steering = read_datapoint(img_path)
            X.append(img_path)
            y.append(steering)

    return sklearn.model_selection.train_test_split(X, y, test_size=test_size, random_state=0)


def build_model():
    """
    Modified NVIDIA model based on https://github.com/naokishibuya/car-behavioral-cloning.
    """
    model = keras.models.Sequential()
    model.add(keras.layers.Lambda(lambda x: x/127.5-1.0, input_shape=utils.INPUT_SHAPE))
    model.add(keras.layers.Conv2D(24, 5, 5, activation='elu', subsample=(2, 2)))
    model.add(keras.layers.Conv2D(36, 5, 5, activation='elu', subsample=(2, 2)))
    model.add(keras.layers.Conv2D(48, 5, 5, activation='elu', subsample=(2, 2)))
    model.add(keras.layers.Conv2D(64, 3, 3, activation='elu'))
    model.add(keras.layers.Conv2D(64, 3, 3, activation='elu'))
    model.add(keras.layers.Dropout(0.5))
    model.add(keras.layers.Flatten())
    model.add(keras.layers.Dense(100, activation='elu'))
    model.add(keras.layers.Dense(50, activation='elu'))
    model.add(keras.layers.Dense(10, activation='elu'))
    model.add(keras.layers.Dense(1))
    model.summary()

    return model


def train_model(model, epochs, batch_size, X_train, X_valid, y_train, y_valid):
    """
    Train the model.
    """
    logging = keras.callbacks.TensorBoard(log_dir='logs')

    checkpoint = keras.callbacks.ModelCheckpoint('model-{epoch:03d}.h5',
                                                 monitor='val_loss',
                                                 verbose=0,
                                                 save_best_only=True,
                                                 mode='auto')

    model.compile(loss='mean_squared_error', optimizer=keras.optimizers.Adam(lr=1.0e-4))

    model.fit_generator(training_generator(X_train, y_train, batch_size),
                        steps_per_epoch=len(X_train) // batch_size * 2,
                        epochs=epochs,
                        max_q_size=1,
                        validation_data=validation_generator(X_valid, y_valid, batch_size),
                        validation_steps=len(X_valid),
                        callbacks=[logging, checkpoint],
                        verbose=1)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', dest='data_dir',   type=str,   default='data', help='data directory')
    parser.add_argument('-t', dest='test_size',  type=float, default=0.1,    help='test size fraction')
    parser.add_argument('-n', dest='epochs',     type=int,   default=10,     help='number of epochs')
    parser.add_argument('-b', dest='batch_size', type=int,   default=40,     help='batch size')
    args = parser.parse_args()

    print('{} start'.format(datetime.datetime.now()))
    data = load_data(args.data_dir, args.test_size)
    model = build_model()
    train_model(model, args.epochs, args.batch_size, *data)
    print('{} done'.format(datetime.datetime.now()))


if __name__ == '__main__':
    main()
