#
# Copyright 2019 NXP
# All rights reserved.
# 
# SPDX-License-Identifier: BSD-3-Clause
#

""" This is a Glow utility script to serialize image files into text format (e.g. "image.txt") or binary format which
    can be later included into a C application as a C buffer array like this:
      uint8_t imageData[] = {
        #include "image.txt"
      };
"""

import os
import numpy as np
import argparse
from imageio import imread

OUTPUT_FORMAT_OPTS = ['txt', 'bin']
OUTPUT_TYPE_OPTS = ['int8', 'uint8', 'float32']
IMAGE_MODE_OPTS = ['neg1to1', 'neg1to1i8', '0to1', '0to255', 'neg128to127']
IMAGE_LAYOUT_OPTS = ['NHWC', 'NCHW']
IMAGE_CHANNEL_ORDER_OPTS = ['RGB', 'BGR']


# ----------------------------------------------------------------------------------------------------------------------
# Performs pre-processing of a given image file and serialization to text/binary in order to include in the code.
# It requires the following parameters:
# - input_path - full path to the image file
# - output_path - full path to the output file
# - output_format - 'txt' or 'bin' (default is 'txt')
# - output_type - 'int8', 'uint8' or 'float32' (default is 'float32')
# - image_mode - image normalization mode: '0to1', 'neg1to1'
# - image_layout - 'NCHW' or 'NHWC'
# - image_channel_order - 'RGB' or 'BGR'
# - image_batch - the image will be repeated in order to obtain a batch (default is 1)
# Function returns serialized image size (bytes).
# ----------------------------------------------------------------------------------------------------------------------
def process_image(input_path, output_path, output_format, output_type, image_mode, image_layout, image_channel_order,
                  image_batch):
    # Validate parameters.
    assert os.path.isfile(input_path), 'Input path invalid!'
    assert output_type in OUTPUT_TYPE_OPTS, 'Output type invalid!'
    assert image_mode in IMAGE_MODE_OPTS, 'Image mode invalid!'
    assert image_layout in IMAGE_LAYOUT_OPTS, 'Image layout invalid!'
    assert image_channel_order in IMAGE_CHANNEL_ORDER_OPTS, 'Image channel invalid!'

    # Read image as UINT8, channel order RGB and HWC format.
    # For gray scale image add channel dimension
    img_data = imread(input_path).astype(np.uint8)
    if img_data.ndim == 2:
        img_data = np.reshape(img_data, (img_data.shape[0], img_data.shape[1], 1))

    # If image has ALPHA channel drop it.
    if img_data.shape[2] == 4:
        img_data = img_data[:, :, (0, 1, 2)]

    # Convert from RGB to BGR.
    if image_channel_order == 'BGR':
        if img_data.shape[2] == 3:
            img_data = img_data[:, :, (2, 1, 0)]

    # Convert image from HWC to CHW.
    if image_layout == 'NCHW':
        img_data = np.transpose(img_data, (2, 0, 1))

    # Data type conversion.
    if output_type == 'int8':
        img_data = img_data - 128
        img_data = img_data.astype(np.int8)
    elif output_type == 'uint8':
        img_data = img_data.astype(np.uint8)
    elif output_type == 'float32':
        img_data = img_data.astype(np.float32)
    else:
        assert False, 'Output type invalid!'

    # Image normalization (for float32 only).
    if output_type == 'float32':
        if image_mode == 'neg1to1':
            range_min = -1.0
            range_max = 1.0
        elif image_mode == 'neg1to1i8':
            range_min = -128.0 / 128.0
            range_max = 127.0 / 128.0
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
        img_data = img_data.astype(np.float32)

    # Repeat image for batch processing (resulting tensor is NCHW or NHWC).
    img_data = np.reshape(img_data, (1, img_data.shape[0], img_data.shape[1], img_data.shape[2]))
    img_data = np.repeat(img_data, image_batch, axis=0)
    img_data = np.reshape(img_data, (image_batch, img_data.shape[1], img_data.shape[2], img_data.shape[3]))

    # Serialize image batch.
    img_data_bytes = bytearray(img_data.tobytes(order='C'))
    if output_format == 'txt':
        bytes_per_line = 20
        with open(output_path, 'wt') as f:
            # Write header.
            f.write('// %s\n' % ('-' * 117))
            f.write('// Image: %s\n' % input_path)
            f.write('// Shape: [%d, %d, %d, %d]\n' % (img_data.shape[0],
                                                      img_data.shape[1],
                                                      img_data.shape[2],
                                                      img_data.shape[3]))
            f.write('// Type: %s\n' % output_type)
            f.write('// Size: %d [bytes]\n' % len(img_data_bytes))
            if output_type == 'float32':
                f.write('// Mode: %s\n' % image_mode)
            f.write('// Layout: %s\n' % image_layout)
            f.write('// Channel order: %s\n' % image_channel_order)
            f.write('// %s\n' % ('-' * 117))
            # Write content.
            idx = 0
            for byte in img_data_bytes:
                f.write('0X%02X, ' % byte)
                if idx % bytes_per_line == (bytes_per_line - 1):
                    f.write('\n')
                idx = idx + 1
    elif output_format == 'bin':
        with open(output_path, 'wb') as f:
            f.write(img_data_bytes)
    else:
        assert False, 'Output format invalid! Expected options are \'txt\' or \'bin\'!'

    # Return serialized image size.
    return len(img_data_bytes)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument('-input-path', '-image-path', '-i', action='store', dest='input_path', required=True,
                        help='Input image file path or a directory.')

    parser.add_argument('-output-path', '-o', action='store', dest='output_path', required=True,
                        help='Output file path or a directory when -input-path is also a directory.')

    parser.add_argument('-output-format', '-f', action='store', dest='output_format', default='txt',
                        choices=OUTPUT_FORMAT_OPTS, help='Output file format: txt, bin. Default is txt.')

    parser.add_argument('-image-type', '-output-type', '-type', '-t', action='store', dest='output_type',
                        default='float32', choices=OUTPUT_TYPE_OPTS, help='Output type. Default is float32.')

    parser.add_argument('-image-mode', '-mode', '-m', action='store', dest='image_mode', default='0to255',
                        choices=IMAGE_MODE_OPTS, help='Image normalization mode. Default is 0to255.')

    parser.add_argument('-image-layout', '-layout', '-l', action='store', dest='image_layout', default='NHWC',
                        choices=IMAGE_LAYOUT_OPTS, help='Image output layout. Default is NHWC.')

    parser.add_argument('-image-channel-order', '-channel-order', '-c', action='store', dest='image_channel_order',
                        default='RGB',
                        choices=IMAGE_CHANNEL_ORDER_OPTS, help='Image channel order. Default is RGB.')

    parser.add_argument('-image-batch', '-batch', '-b', action='store', dest='image_batch', type=int, default=1,
                        help='Input batch size. Image will be repeated to obtain a batch worth of data. Default is 1.')

    args = parser.parse_args()

    # When input path is a directory we process all the files in the directory.
    if os.path.isdir(args.input_path):

        # Validate that output path is NOT a file path.
        assert not os.path.isfile(args.output_path), 'When -input-path is a directory -output-path must a directory!'

        # Create output directory if not exists.
        if not os.path.isdir(args.output_path):
            os.mkdir(args.output_path)

        # Traverse all the input directory.
        for file in os.listdir(args.input_path):

            # Input/output filename.
            input_file = os.path.join(args.input_path, file)
            input_name, input_ext = os.path.splitext(file)
            output_file = os.path.join(args.output_path, input_name) + '.' + args.output_format

            # Skip files which are not images.
            if input_ext not in ['.png', '.jpg', '.jpeg']:
                continue

            # Process current image.
            process_image(input_path=input_file,
                          output_path=output_file,
                          output_format=args.output_format,
                          output_type=args.output_type,
                          image_mode=args.image_mode,
                          image_layout=args.image_layout,
                          image_channel_order=args.image_channel_order,
                          image_batch=args.image_batch)

            print('Image processed to file \"%s\" ...' % output_file)

    else:

        # Process only one image.
        process_image(input_path=args.input_path,
                      output_path=args.output_path,
                      output_format=args.output_format,
                      output_type=args.output_type,
                      image_mode=args.image_mode,
                      image_layout=args.image_layout,
                      image_channel_order=args.image_channel_order,
                      image_batch=args.image_batch)

        print('Image processed to file \"%s\" ...' % args.output_path)
