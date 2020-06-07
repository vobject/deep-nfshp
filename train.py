import os
import argparse
import datetime

import cv2
import numpy as np
import sklearn.model_selection

from tensorflow import keras

from models.nvidia import NvidiaModel
from models.comma import CommaModel
from models.tiny import TinyModel


def training_generator(image_paths, steering_angles, preproc, batch_size):
    """
    Generate training data.
    """
    fourth_batch_size = batch_size // 4
    slices_per_epoch = len(image_paths) // batch_size
    datagen = keras.preprocessing.image.ImageDataGenerator(
        rotation_range=10,
        width_shift_range=0.1,
        height_shift_range=0.1,
        shear_range=0.1,
        zoom_range=0.1,
        channel_shift_range=.1,
        fill_mode='nearest')

    training_data_slice = 0
    while True:
        slice_begin = training_data_slice * fourth_batch_size
        slice_end = slice_begin + fourth_batch_size

        # original frames
        orig_images = [preproc(cv2.imread(path)) for path in image_paths[slice_begin:slice_end]]
        orig_steers = steering_angles[slice_begin:slice_end]

        # original frames mirrored
        orig_flip_images = [cv2.flip(x, 1) for x in orig_images]
        orig_flip_steers = [-x for x in orig_steers]

        # generated frames
        gen = datagen.flow(
            x=np.stack(orig_images, axis=0),
            y=np.stack(orig_steers, axis=0),
            batch_size=fourth_batch_size,
            shuffle=False)
        gen_images, gen_steers = next(gen)

        # generated frames mirrored
        gen_flip_images = [cv2.flip(x, 1) for x in gen_images]
        gen_flip_steers = [-x for x in gen_steers]

        images = np.concatenate((orig_images, orig_flip_images, gen_images, gen_flip_images), axis=0)
        steers = np.concatenate((orig_steers, orig_flip_steers, gen_steers, gen_flip_steers), axis=0)

        yield images, steers
        training_data_slice = (training_data_slice + 1) % slices_per_epoch


def validation_generator(image_paths, steering_angles, preproc, batch_size):
    """
    Generate cross validation data.
    """
    while True:
        indices = [np.random.choice(len(image_paths)) for x in range(batch_size)]
        images = [preproc(cv2.imread(image_paths[i])) for i in indices]
        steers = [steering_angles[i] for i in indices]
        yield images, steers


def load_validation_data(image_paths, steering_angles, preproc):
    """
    Load validation frames into memory.
    """
    images = [preproc(cv2.imread(x)) for x in image_paths]
    return np.array(images), np.array(steering_angles)


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

    return sklearn.model_selection.train_test_split(X, y, test_size=test_size, random_state=None)


def build_model(name, print_layer_summary=True):
    if name == NvidiaModel.NAME:
        model = NvidiaModel()
    elif name == CommaModel.NAME:
        model = CommaModel()
    # FIXME: adapt training_generator() for 1-channel images of TinyModel
    # elif name == TinyModel.NAME:
    #     model = TinyModel()

    if print_layer_summary:
        model.get().summary()
    return model


def train_model(model, epochs, batch_size, X_train, X_valid, y_train, y_valid):
    """
    Train the model.
    """
    logging = keras.callbacks.TensorBoard(log_dir='logs')

    checkpoint = keras.callbacks.ModelCheckpoint('model-' + model.NAME + '-{epoch:03d}-{val_loss:.4f}.h5',
                                 monitor='val_loss',
                                 save_best_only=True)

    kmodel = model.get()
    kmodel.compile(loss='mean_squared_error', optimizer=keras.optimizers.Adam(lr=1.0e-4))
    print('{} model compiled'.format(datetime.datetime.now()))

    # 3/4 of training data per batch is generated; we need 4x steps to get once
    # through the whole (physical) training set.
    steps_per_epoch = len(X_train) * 4 // batch_size

    # Pre-loading all validation data in memory speeds up training 10-15%.
    # Switch back to validation data generator if the data get's too large.
    X_valid_data, y_valid_data = load_validation_data(X_valid, y_valid, model.preprocess)

    kmodel.fit_generator(
        training_generator(X_train, y_train, model.preprocess, batch_size),
        steps_per_epoch=steps_per_epoch,
        epochs=epochs,
        validation_data=(X_valid_data, y_valid_data),
        #validation_data=training_generator(X_valid, y_valid, model.preprocess, batch_size),
        #validation_steps=len(X_valid) // batch_size,
        #workers=4,
        callbacks=[logging, checkpoint],
        verbose=1)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', dest='data_dir',   type=str,   default='data',    help='data directory')
    parser.add_argument('-t', dest='test_size',  type=float, default=0.2,       help='test size fraction')
    parser.add_argument('-n', dest='epochs',     type=int,   default=50,        help='number of epochs')
    parser.add_argument('-b', dest='batch_size', type=int,   default=64,        help='batch size')
    parser.add_argument('-m', dest='model_name', type=str,   default='nvidia',  help='model architecture ({}, {}, or {})'.format(NvidiaModel.NAME, CommaModel.NAME, TinyModel.NAME))
    args = parser.parse_args()

    print('{} start'.format(datetime.datetime.now()))
    data = load_data(args.data_dir, args.test_size)
    model = build_model(args.model_name)
    print('{} model built'.format(datetime.datetime.now()))
    train_model(model, args.epochs, args.batch_size, *data)
    print('{} done'.format(datetime.datetime.now()))


if __name__ == '__main__':
    main()
