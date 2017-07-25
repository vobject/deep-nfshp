import cv2


# Window title to search for
WINDOW_TITLE = "Need For Speed III: Hot Pursuit"

# 1024x768 / 4; large enough for further processing
CAPTURE_SIZE = (256,192)

# Window offsets in Windows 10
WINDOW_OFFSETS = (4,28,4,4)

IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS = 66, 200, 3
INPUT_SHAPE = (IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNELS)


def crop(image):
    """
    Crop the image (remove the sky)
    """
    return image[112:, :, :]


def resize(image):
    """
    Resize the image to the input shape used by the network model
    """
    return cv2.resize(image, (IMAGE_WIDTH, IMAGE_HEIGHT), cv2.INTER_AREA)


def rgb2yuv(image):
    """
    Convert the image from RGB to YUV (This is what the NVIDIA model does)
    """
    return cv2.cvtColor(image, cv2.COLOR_RGB2YUV)


def preprocess(image):
    """
    Combine all preprocess functions into one
    """
    image = crop(image)
    image = resize(image)
    image = rgb2yuv(image)
    return image
