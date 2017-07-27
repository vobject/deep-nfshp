## Train a convolutional neural network to play Need For Speed III

Deep neural network trained to drive [Need For Speed III: Hot Pursuit](https://en.wikipedia.org/wiki/Need_for_Speed_III:_Hot_Pursuit) (1998).

The model's input is an image of the current situation on screen and outputs a steering angle.

See it in action here: https://youtu.be/V_uwjSzxC84

## Overview
TODO: files, parameters, preconditions

File | Description
------------ | -------------
collect.py | Capture training data from a running NFS process while driving
train.py | Train the model
drive.py | Load a trained model and predict steering for a running NFS process
model-XYZ.h5 | Trained model weights

## Training
TODO: screen captures, input captures, image proprocessing, data generation, model training, epochs, Tensorboard

## Testing
TODO: load model, predict steering angle, send input

## Future
A few ideas for improvements. Suggestions or pull requests welcome.

### Drive faster
The model has no sense of speed. The speed is artificially capped around 80-90 km/h by using manual transmission and always drive in gear 1. I have a few ideas on how to approach this but nothing backed by research or experience.

### Drive all tracks in the game
NFS III: HP comes with 9 race tracks. The current model is only trained on the Summit track and performs well. It also does a decent job driving the never seen Country Woods but it is useless on all other tracks. A future goal is be to train and generalize enough to drive all the tracks in the game.

### Stretch goals
- Avoid traffic. The game fills the tracks with traffic if opponents are turned off.
- Manual transmission.
- Script and control the game process from within Python. Would make development and evaluation easier.
- Perform with different camera settings. The CNN was trainged solely on the bumper cam. Other settings work much poorer.
- Perform with different resolutions. The CNN was trainged running the game in 1024x768 windows mode. Other settings work poorer.

## References
- Original inspiration was Harrison Kinsley's pygta5:
https://github.com/Sentdex/pygta5

- NVIDIA End-to-End Deep Learning for Self-Driving Cars:
https://devblogs.nvidia.com/parallelforall/deep-learning-self-driving-cars/

- Modified NVIDIA model:
https://github.com/naokishibuya/car-behavioral-cloning

- VEG's Need For Speed III Modern Patch v1.6.1:
http://veg.by/en/projects/nfs3/

- Xbox-360-Controller-for-Python:
https://github.com/r4dian/Xbox-360-Controller-for-Python

- pyvjoy:
https://github.com/tidzo/pyvjoy
