#
# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

""" This is a Glow utility script to overlay on top of an image the rectangular coordinates corresponding to the boxes
    detected by an object detection model.
    This script plots or saves the resulting image for visual inspection.
"""

import os
import re
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import matplotlib.image as mpimg
import argparse


# Notes:
# A reference box is a list with 5 elements: [class, x_min, y_min, x_max, y_max].
# A detected box is a list with 6 elements: [class, x_min, y_min, x_max, y_max, score].

# Utility function to denormalize box coordinates.
# The denormalized integer coordinates will be between 0 and size-1.
def denorm_box(box, xsize, ysize):
    # Denormalize coordinates.
    x_min = int(round(box[1] * xsize))
    y_min = int(round(box[2] * ysize))
    x_max = int(round(box[3] * xsize))
    y_max = int(round(box[4] * ysize))

    # Saturate coordinates.
    def sat(val, size):
        val = val if val >= 0 else 0
        val = val if val < size else size - 1
        return val
    x_min = sat(x_min, xsize)
    y_min = sat(y_min, ysize)
    x_max = sat(x_max, xsize)
    y_max = sat(y_max, ysize)

    # Return denormalized box.
    box_d = box
    box_d[1] = x_min
    box_d[2] = y_min
    box_d[3] = x_max
    box_d[4] = y_max
    return box_d


# ----------------------------------------------------------------------------------------------------------------------
# Utility to read the detected boxes from a CSV file.
# The CSV file is assumed to have the following format:
#   class1, x_min1, y_min1, x_max1, y_max1, score1,
#   class2, x_min2, y_min2, x_max2, y_max2, score2,
#   .........................................
# This function will return a list of boxes where each box is a list with 6 elements:
#   [class, x_min, y_min, x_max, y_max, score]
# ----------------------------------------------------------------------------------------------------------------------
def read_det_boxes(file_name):
    box_list = list()
    with open(file_name) as det_file:
        for line in det_file.readlines():
            fields = line.strip('\n').split(',')
            assert (len(fields) == 6), ('Detected box file %s is invalid!' % file_name)
            box = list()
            box.append(int(fields[0]))
            box.append(float(fields[1]))
            box.append(float(fields[2]))
            box.append(float(fields[3]))
            box.append(float(fields[4]))
            box.append(float(fields[5]))
            box_list.append(box)
        return box_list


# ----------------------------------------------------------------------------------------------------------------------
# Utility to plot the detected boxes to current axis object.
# ----------------------------------------------------------------------------------------------------------------------
def plot_det_boxes(axis_obj, det_boxes_filename, image_height, image_width, labels):
    det_boxes = read_det_boxes(det_boxes_filename)
    for box in det_boxes:
        # Denormalize boxes.
        box_d = denorm_box(box, xsize=image_width, ysize=image_height)
        class_idx = box_d[0]
        x_min = box_d[1]
        y_min = box_d[2]
        x_max = box_d[3]
        y_max = box_d[4]
        class_score = box_d[5]
        # Text to print.
        if labels:
            box_txt = '%s (%.3f)' % (labels[class_idx], class_score)
        else:
            box_txt = '%d (%.3f)' % (class_idx, class_score)
        # Create a rectangle and add to the image.
        rect = Rectangle((x_min, y_min), x_max - x_min, y_max - y_min, linewidth=2, edgecolor='r', facecolor='none')
        axis_obj.add_patch(rect)
        axis_obj.text(x_min + 5, y_min + 5, box_txt, color='r', fontsize=15, verticalalignment='top',
                      horizontalalignment='left')


# ----------------------------------------------------------------------------------------------------------------------
# Utility to read the reference (ground truth) boxes from a CSV file.
# The CSV file is assumed to have the following format:
#   image_name, class1, x_min1, y_min1, x_max1, y_max1, class2, x_min2, y_min2, x_max2, y_max2, ...
# This function will return a list of boxes where each box is a list with 5 elements:
#   [class, x_min, y_min, x_max, y_max]
# Returns None if no image is found.
# ----------------------------------------------------------------------------------------------------------------------
def read_ref_boxes(file_name, image_name):
    box_list = list()
    with open(file_name) as ref_file:
        for line in ref_file.readlines():
            fields = line.strip('\n').split(',')
            while not fields[-1]:
                fields.pop()
            if fields[0] != image_name:
                continue
            assert ((len(fields) - 1) % 5) == 0, 'Reference box file is invalid!' % file_name
            num_boxes = int((len(fields) - 1) / 5)
            field_idx = 1
            for box_idx in range(num_boxes):
                box = list()
                box.append(int(fields[field_idx + 0]))
                box.append(float(fields[field_idx + 1]))
                box.append(float(fields[field_idx + 2]))
                box.append(float(fields[field_idx + 3]))
                box.append(float(fields[field_idx + 4]))
                box_list.append(box)
                field_idx += 5
            return box_list
        return None


# ----------------------------------------------------------------------------------------------------------------------
# Utility to plot the reference (ground truth) boxes to current axis object.
# ----------------------------------------------------------------------------------------------------------------------
def plot_ref_boxes(axis_obj, ref_boxes_filename, image_name, image_height, image_width, labels):
    ref_boxes = read_ref_boxes(ref_boxes_filename, os.path.basename(image_name))
    for box in ref_boxes:
        # Denormalize box.
        box_d = denorm_box(box, xsize=image_width, ysize=image_height)
        class_idx = box_d[0]
        x_min = box_d[1]
        y_min = box_d[2]
        x_max = box_d[3]
        y_max = box_d[4]
        # Text to print.
        if labels:
            box_txt = '%s' % labels[class_idx]
        else:
            box_txt = '%d' % class_idx
        # Create a rectangle.
        rect = Rectangle((x_min, y_min), x_max - x_min, y_max - y_min, linewidth=2, edgecolor='g', facecolor='none')
        # Add to image.
        axis_obj.add_patch(rect)
        axis_obj.text(x_min + 5, y_min + 5, box_txt, color='g', fontsize=15, verticalalignment='top',
                      horizontalalignment='left')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument('-image-path', '-image', '-i', action='store', dest='image_path', required=True,
                        help='Input image file path.')

    parser.add_argument('-det-boxes-path', '-det-boxes', '-boxes', action='store', dest='det_boxes', required=True,
                        help='Detected boxes csv file. The file contains one detected box for each row, with the'
                             'fields being comma separated, for example:\n'
                             'class1, x_min1, y_min1, x_max1, y_max1, score1,\n'
                             'class2, x_min2, y_min2, x_max2, y_max2, score2,\n'
                             '..............................................')

    parser.add_argument('-ref-boxes-path', '-ref-boxes', action='store', dest='ref_boxes', required=False,
                        help='Reference boxes csv file.')

    parser.add_argument('-labels-path', '-labels', '-l', action='store', dest='labels_path', required=False,
                        help='Classes labels file path. The labels are given as one row with comma separated values or '
                             'one value per row.')

    args = parser.parse_args()

    # Validate parameters.
    assert os.path.isfile(args.image_path), 'Input image path is invalid!'
    assert os.path.isfile(args.det_boxes), 'Detected boxes file path is invalid!'

    # Plot image.
    img = mpimg.imread(args.image_path)
    plt.imshow(img)
    axis = plt.gca()

    # Get image shape.
    img_height = img.shape[0]
    img_width = img.shape[1]

    # Read labels.
    labels_list = list()
    if args.labels_path:
        assert os.path.isfile(args.labels_path), 'Labels file path is invalid!'
        with open(args.labels_path, 'rt') as f:
            labels_list = re.split(r'[\r\n]|,|;', f.read())
            while not labels_list[-1]:
                labels_list.pop()

    # Add detected boxes to the graph.
    plot_det_boxes(axis, args.det_boxes, img_height, img_width, labels_list)

    # Add reference boxes.
    if args.ref_boxes:
        assert os.path.isfile(args.ref_boxes), 'Reference boxes file path is invalid!'
        plot_ref_boxes(axis, args.ref_boxes, args.image_path, img_height, img_width, labels_list)

    # Show final image.
    plt.show()
