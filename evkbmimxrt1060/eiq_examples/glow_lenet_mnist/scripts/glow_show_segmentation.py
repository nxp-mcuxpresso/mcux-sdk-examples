#
# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

""" This is a Glow utility script to overlay on top of an image the segmentation map resulting from the 3D tensor of
    scores retrieved by an image segmentation model.
    This script plots or saves the resulting image for visual inspection.
"""

import os
import re
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
from PIL import Image
import argparse

SCORES_FORMAT_OPTS = ['txt', 'bin']


# Utility function to convert a N x 3 matrix from RGB color space to YUV color space.
# Both input and output values are in the range [0.0, 255.0].
def rgb2yuv(rgb):
    # Transform.
    mat = np.array([[0.29900, -0.14713,  0.61500],
                    [0.58700, -0.28886, -0.51499],
                    [0.11400,  0.43600, -0.10001]])
    yuv = np.dot(rgb, mat)
    # Adjust UV values from [U_MIN, U_MAX] and [V_MIN, V_MAX] to [0, 255] range.
    u_min = -0.43600 * 255.0
    u_max = +0.43600 * 255.0
    v_min = -0.61500 * 255.0
    v_max = +0.61500 * 255.0
    yuv[:, 1] = (yuv[:, 1] - u_min) / (u_max - u_min) * 255.0
    yuv[:, 2] = (yuv[:, 2] - v_min) / (v_max - v_min) * 255.0
    # Clip values in [0, 255] range.
    yuv = np.maximum(yuv, 0.0)
    yuv = np.minimum(yuv, 255.0)
    return yuv


# Utility function to convert a N x 3 matrix from YUV color space to RGB color space.
# Both input and output values are in the range [0.0, 255.0].
def yuv2rgb(yuv):
    # Adjust UV values from [0, 255] to [U_MIN, U_MAX] and [V_MIN, V_MAX] range.
    u_min = -0.43600 * 255.0
    u_max = +0.43600 * 255.0
    v_min = -0.61500 * 255.0
    v_max = +0.61500 * 255.0
    yuv[:, 1] = yuv[:, 1] / 255.0 * (u_max - u_min) + u_min
    yuv[:, 2] = yuv[:, 2] / 255.0 * (v_max - v_min) + v_min
    # Transform.
    mat = np.array([[1.00000,  1.00000, 1.00000],
                    [0.00000, -0.39465, 2.03211],
                    [1.13983, -0.58060, 0.00000]])
    rgb = np.dot(yuv, mat)
    # Clip values in [0, 255] range.
    rgb = np.maximum(rgb, 0.0)
    rgb = np.minimum(rgb, 255.0)
    return rgb


# Utility function to generate N RGB colors as a N x 3 matrix.
# The colors should be generated to be maximally distinguishable (maximally distanced in the RGB space).
def gen_rgb_colors(num):

    # We split the RGB domain in N x N x N grid where N is approximately the cube root of the number of colors.
    grid_len = int(np.ceil(np.cbrt(num)))
    assert grid_len > 1, 'The number of colors should be greater than 1!'
    delta = (255.0 - 0.0) / (grid_len - 1)
    grid = [grid_idx * delta for grid_idx in range(grid_len)]
    grid = [int(np.round(val)) for val in grid]
    grid = [max([val, 0]) for val in grid]
    grid = [min([val, 255]) for val in grid]

    # We generate the cartesian product of the 3D grid.
    colors = [(r, g, b) for r in grid for g in grid for b in grid]
    colors = np.array(colors)

    # We remove the extra colors which are closest to [0, 0, 0] (black) or [255, 255, 255] (white).
    black = np.array([0, 0, 0])
    white = np.array([255, 255, 255])
    black_dist = np.array([np.linalg.norm(black - colors[cidx, :]) for cidx in range(colors.shape[0])])
    white_dist = np.array([np.linalg.norm(white - colors[cidx, :]) for cidx in range(colors.shape[0])])
    dist = np.minimum.reduce([black_dist, white_dist])
    rem_idx_list = list()
    for rem_idx in range(len(colors) - num):
        min_val = np.amin(dist)
        min_idx = np.where(dist == min_val)[0][0]
        dist[min_idx] = np.Inf
        rem_idx_list.append(min_idx)
    colors = np.delete(colors, rem_idx_list, axis=0)

    return colors


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument('-image-path', '-image', '-i', action='store', dest='image_path', required=True,
                        help='Input image file path.')

    parser.add_argument('-scores-path', '-scores', '-s', action='store', dest='scores_path', required=True,
                        help='Inputs scores file path. The scores is a float32 3D tensor with HWC layout. The '
                             'number of values is scores-height x scores-width x scores-classes. For text format file '
                             'the float32 values are comma separated. For binary format the float32 values are packed.')

    parser.add_argument('-scores-height', '-height', action='store', dest='height', required=True, type=int,
                        help='The height of the scores tensor.')

    parser.add_argument('-scores-width', '-width', action='store', dest='width', required=True, type=int,
                        help='The height of the scores tensor.')

    parser.add_argument('-scores-classes', '-classes', action='store', dest='classes', required=True, type=int,
                        help='Number of classes for the scores tensors.')

    parser.add_argument('-scores-format', '-f', action='store', dest='scores_format', required=False, default='txt',
                        choices=SCORES_FORMAT_OPTS, help='Inputs scores file format: txt, bin. Default is txt.')

    parser.add_argument('-labels-path', '-labels', '-l', action='store', dest='labels_path', required=False,
                        help='Classes labels file path. The labels are given as one row with comma separated values or '
                             'one value per row.')

    parser.add_argument('-output-path', '-output', '-o', action='store', dest='output_path', required=False,
                        help='Output file path to save the image with the segmentation map (optional).')

    parser.add_argument('-alpha', '-a', action='store', dest='alpha', required=False, type=float, default=0.5,
                        help='The level of opacity of the image versus the score segmentation colors.')

    parser.add_argument('-show-classes', action='store', dest='show_classes', required=False, type=str,
                        help='A comma separated list of classes (numbered from 0) which will be displayed, for example '
                             '"-show-classes=0,1,2". If this option is not used then all the classes are displayed.')

    args = parser.parse_args()

    # Validate parameters.
    assert os.path.isfile(args.image_path), 'Input image path is invalid!'
    assert os.path.isfile(args.scores_path), 'Input scores path is invalid!'
    assert (0 <= args.alpha) and (args.alpha <= 1), 'Alpha parameter must be between 0 and 1!'
    assert args.height > 0, 'Scores height must be bigger than 0!'
    assert args.width > 0, 'Scores width must be bigger than 0!'
    assert args.classes > 0, 'Scores classes must be bigger than 0!'

    # Parse classes to display.
    show_classes = list(range(args.classes))
    if args.show_classes:
        show_classes = [int(c) for c in args.show_classes.split(',')]
        for c in show_classes:
            assert c in range(args.classes), \
                ('Class value %d is invalid! Should be in the range [0, %d]!' % (c, args.classes - 1))

    # Read scores.
    height = args.height
    width = args.width
    classes = args.classes
    scores_data = []
    if args.scores_format == 'txt':
        with open(args.scores_path, 'rt') as f:
            val_list = f.read().split(',')
            val_list.pop()
            scores_data = [float(val) for val in val_list]
    elif args.scores_format == 'bin':
        scores_data = np.fromfile(args.scores_path, dtype=np.float32, count=-1, sep='')
    else:
        assert False, 'Scores format invalid! Expected options are \'txt\' or \'bin\'!'
    scores_data_len = len(scores_data)
    expected_len = height * width * classes
    scores_len_err_msg = 'The scores file "%s" contains %d values ' % (args.scores_path, scores_data_len)
    scores_len_err_msg += 'while the expected number of values is H x W x C '
    scores_len_err_msg += '= %d x %d x %d = %d!' % (height, width, classes, expected_len)
    assert scores_data_len == (height * width * classes), scores_len_err_msg
    scores_data = np.reshape(scores_data, (height, width, classes))

    # Read the input image. If the size is different than the scores then the input image is resized to match.
    input_image = Image.open(args.image_path).convert('RGB')
    input_data = np.array(input_image)
    image_width, image_height = input_image.size
    if (image_width != width) or (image_height != height):
        input_image = input_image.resize((width, height))

    # Generate random RGB colors for segmentation map (C x 3 random array).
    seg_colors = gen_rgb_colors(classes)

    # Compute segmentation data by choosing the class with maximum score for each pixel.
    seg_data = np.ndarray(shape=(height, width, 3))
    for h_idx in range(height):
        for w_idx in range(width):
            # Find maximum score for this pixel.
            max_val = -np.inf
            max_idx = 0
            for c_idx in range(classes):
                val = scores_data[h_idx, w_idx, c_idx]
                if val >= max_val:
                    max_val = val
                    max_idx = c_idx
            # Use the color associated to the class. If the class should not be displayed then we use the pixel from
            # the original input image.
            if max_idx in show_classes:
                seg_data[h_idx, w_idx, :] = seg_colors[max_idx, :]
            else:
                seg_data[h_idx, w_idx, :] = input_data[h_idx, w_idx]
    seg_image = Image.fromarray(seg_data.astype(np.uint8))

    # Blend the input image with the segmentation image.
    blended_image = Image.blend(input_image, seg_image, args.alpha)

    # Save blended image (if path is provided).
    if args.output_path:
        blended_image.save(args.output_path)

    # Read labels.
    if args.labels_path:
        assert os.path.isfile(args.labels_path), 'Labels file path is invalid!'
        with open(args.labels_path, 'rt') as f:
            legend_labels = re.split(r'[\r\n]|,|;', f.read())
            while not legend_labels[-1]:
                legend_labels.pop()
            assert len(legend_labels) == classes, 'The labels file "%s" contains %d classes while %d classes where ' \
                                                  'expected!' % (args.labels_path, len(legend_labels), classes)
            for idx in range(len(legend_labels)):
                legend_labels[idx] = str(idx) + ' : ' + legend_labels[idx]
            legend_labels = [legend_labels[idx] for idx in show_classes]
    else:
        legend_labels = show_classes

    # Plot blended image with legend.
    legend_colors = [Rectangle((0, 0), 1, 1, color=seg_colors[idx, :] / 255) for idx in show_classes]
    plt.legend(legend_colors,
               legend_labels,
               fontsize='xx-large',
               ncol=1,
               handleheight=1.5,
               labelspacing=0.01,
               bbox_to_anchor=(1.05, 1),
               loc='upper left')
    plt.imshow(blended_image)
    plt.show()
