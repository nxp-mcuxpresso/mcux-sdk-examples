#
# Copyright 2019 NXP
# All rights reserved.
# 
# SPDX-License-Identifier: BSD-3-Clause
#

import os
import numpy as np
import argparse
from imageio import imread

IMAGE_MODE_OPTS = ['neg1to1', '0to1', '0to255', 'neg128to127']
IMAGE_LAYOUT_OPTS = ['NHWC', 'NCHW']
IMAGE_CHANNEL_ORDER_OPTS = ['RGB', 'BGR']
OUTPUT_FORMAT_OPTS = ['txt','bin']

# Performs pre-processing of a given image and binary serialization.
# It requires the following parameters:
# - image_path - full path to the image
# - image_mode - image normalization mode: '0to1', 'neg1to1'
# - image_layout - 'NCHW' or 'NHWC'
# - image_channel_order - 'RGB' or 'BGR'
# - num_batch - the image will be repeated in order to obtain a batch (default is 1)
# Function returns serialized image bytes.
def process_image(image_path, image_mode, image_layout, image_channel_order, num_batch):

    # Validate parameters
    assert os.path.isfile(image_path), 'Image path invalid!'
    assert image_mode in IMAGE_MODE_OPTS, 'Image mode invalid1'
    assert image_layout in IMAGE_LAYOUT_OPTS, 'Image layout invalid!'
    assert image_channel_order in IMAGE_CHANNEL_ORDER_OPTS, 'Image channel invalid!'

    # Read image as float, channel order RGB and HWC format.
    # For gray scale image add channel dimension
    img_data = imread(image_path).astype(np.float32)
    if img_data.ndim == 2:
        img_data = np.reshape(img_data, (img_data.shape[0], img_data.shape[1], 1))

    # If image has ALPHA channel drop it
    if img_data.shape[2] == 4:
        img_data = img_data[:, :, (0, 1, 2)]

    # Convert from RGB to BGR
    if image_channel_order == 'BGR':
        if img_data.shape[2] == 3:
            img_data = img_data[:, :, (2, 1, 0)]

    # Convert image from HWC to CHW
    if image_layout == 'NCHW':
        img_data = np.transpose(img_data, (2, 0, 1))

    # Image normalization
    if image_mode == 'neg1to1':
        range_min = -1.0
        range_max = 1.0
    elif image_mode == '0to1':
        range_min = 0.0
        range_max = 1.0
    elif image_mode == '0to255':
        range_min = 0.0
        range_max = 255.0
    elif image_mode == 'neg128to127':
        range_min = -128.0
        range_max = 127.0
    else:
        assert False, 'Image mode invalid!'
    scale = (range_max - range_min) / 255.0
    bias = range_min
    img_data = img_data * scale + bias

    # Repeat image for batch processing (resulting tensor is NCHW or NHWC)
    img_data = np.reshape(img_data, (1, img_data.shape[0], img_data.shape[1], img_data.shape[2]))
    img_data = np.repeat(img_data, num_batch, axis=0)
    img_data = np.reshape(img_data, (num_batch, img_data.shape[1], img_data.shape[2], img_data.shape[3]))

    # Return serialized image
    return bytearray(img_data.tobytes(order='C'))


# Returns the PNG image paths from a given folder
def get_image_files(data_path, max_num_images=None):
    image_files = []
    for file in os.listdir(data_path):
        if file.endswith(".png"):
            image_files.append(data_path + '/' + file)
    if not image_files:
        print('Images not found in folder %s!' % data_path)
    if max_num_images:
        image_files = image_files[0:max_num_images]
    return image_files


# Maximum number of images to serialize
IMG_MAX_NUM = 100;

# Image folder
IMG_PATH = './dataset'

# Output binary path
OUT_PATH = './test'

# Image labels
IMG_LABELS = [
'airplane',
'automobile',
'bird',
'cat',
'deer',
'dog',
'frog',
'horse',
'ship',
'truck'
]

# Parse all PNG images
inp_data = bytearray()
out_data = []
for img_path in get_image_files(IMG_PATH, IMG_MAX_NUM):
    # Process image
    img_data = process_image(image_path=img_path,
                             image_mode='0to1',
                             image_layout='NCHW',
                             image_channel_order='BGR',
                             num_batch=1)
    # Concatenate data
    inp_data += img_data

    # Concatenate image label (last part of the name)
    img_name = os.path.basename(img_path)
    img_label = img_name[img_name.rfind('_')+1:img_name.find('.')]
    
    
    # Find label index in list
    img_label_idx = IMG_LABELS.index(img_label)
    img_label_idx = np.int32(img_label_idx)
    out_data.append(img_label_idx)

    # Print info
    print('Serialized image: %s (Label %d) ...' % (img_path, img_label_idx))

# Serialize input data
with open(OUT_PATH + '/' + 'input.bin', 'wb') as f:
    f.write(inp_data)

# Serialize input data
with open(OUT_PATH + '/' + 'output.bin', 'wb') as f:
    for item in out_data:
      f.write(np.int32(item))