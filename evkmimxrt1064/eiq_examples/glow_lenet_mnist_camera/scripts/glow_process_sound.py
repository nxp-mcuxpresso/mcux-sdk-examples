#
# Copyright 2021 NXP
# All rights reserved.
# 
# SPDX-License-Identifier: BSD-3-Clause
#

""" This is a Glow utility script to serialize sound files into text format (e.g. "sound.txt") or binary format which
    can be later included into a C application as a C buffer array like this:
      uint8_t soundData[] = {
        #include "sound.txt"
      };
"""

import os
import numpy as np
import argparse
import scipy.io.wavfile as wavfile

OUTPUT_FORMAT_OPTS = ['txt', 'bin']
OUTPUT_TYPE_OPTS = ['int8', 'int16', 'int32', 'float32']


# ----------------------------------------------------------------------------------------------------------------------
# Performs pre-processing of a given sound file and serialization to text/binary in order to include in the code.
# Before serialization, the data sample array is split into multiple windows each with a length of 'window_size' samples
# and with the windows overlapping over a length of `window_overlap` samples.
# It requires the following parameters:
# - input_path - full path to the sound file
# - output_path - full path to the output file
# - output_format - 'txt' or 'bin' (default is 'txt')
# - output_type - 'int8', 'int16', 'int32' or 'float32' (default is 'float32')
# - window_size - the size of the window (in samples)
# - window_overlap - the size of the window overlap (in samples)
# Function returns serialized sound file size (bytes).
# ----------------------------------------------------------------------------------------------------------------------
def process_sound(input_path, output_path, output_format, output_type, window_size, window_overlap):

    # Read sound data.
    sample_rate, snd_data = wavfile.read(filename=input_path)
    sample_count = len(snd_data)

    # Extract overlapping windows.
    if window_size <= 0:
        window_size = sample_count
        window_overlap = 0
    snd_data_window_array = np.ndarray((0, window_size))
    window_count = 0
    window_start = 0
    while window_start + window_size <= len(snd_data):
        snd_data_window = snd_data[window_start: (window_start + window_size)]
        snd_data_window = np.reshape(snd_data_window, (1, window_size))
        snd_data_window_array = np.append(snd_data_window_array, snd_data_window, axis=0)
        window_count += 1
        window_start += window_size - window_overlap

    # Windowing validation.
    assert window_overlap >= 0, 'Window overlap should be greater or equal to 0!'
    assert window_overlap < window_size, 'Window overlap should be smaller than window size!'
    assert window_count != 0, 'Window count is 0. Try using a smaller window size!'

    # Convert data to output type.
    if output_type == 'int8':
        snd_data_window_array = snd_data_window_array.astype(np.int8)
    elif output_type == 'int16':
        snd_data_window_array = snd_data_window_array.astype(np.int16)
    elif output_type == 'int32':
        snd_data_window_array = snd_data_window_array.astype(np.int32)
    elif output_type == 'float32':
        snd_data_window_array = snd_data_window_array.astype(np.float32)
    else:
        assert False, 'Output type invalid!'

    # Serialize data.
    snd_data_bytes = bytearray(snd_data_window_array.tobytes(order='C'))
    if output_format == 'txt':
        bytes_per_line = 20
        with open(output_path, 'wt') as f:
            # Write header.
            f.write('// %s\n' % ('-' * 117))
            f.write('// File:\n')
            f.write('//   Path: %s\n' % input_path)
            f.write('//   Rate: %d Hz\n' % sample_rate)
            f.write('//   Size: %d samples (%f seconds)\n' % (sample_count, sample_count / sample_rate))
            f.write('// Windowing:\n')
            f.write('//   Overlap size: %d samples (%f seconds)\n' % (window_overlap, window_overlap / sample_rate))
            f.write('//   Window size: %d samples (%f seconds)\n' % (window_size, window_size / sample_rate))
            f.write('//   Window count: %d\n' % window_count)
            f.write('// Output:\n')
            f.write('//   Shape: [%d, %d]\n' % (window_count, window_size))
            f.write('//   Size: %d bytes\n' % len(snd_data_bytes))
            f.write('//   Type: %s\n' % output_type)
            f.write('// %s\n' % ('-' * 117))
            # Write content.
            idx = 0
            for byte in snd_data_bytes:
                f.write('0X%02X, ' % byte)
                if idx % bytes_per_line == (bytes_per_line - 1):
                    f.write('\n')
                idx = idx + 1
    elif output_format == 'bin':
        with open(output_path, 'wb') as f:
            f.write(snd_data_bytes)
    else:
        assert False, 'Output format invalid! Expected options are \'txt\' or \'bin\'!'

    # Return serialized sound size.
    return len(snd_data_bytes)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument('-input-path', '-sound-path', '-i', action='store', dest='input_path', required=True,
                        help='Input sound file path or a directory.')

    parser.add_argument('-output-path', '-o', action='store', dest='output_path', required=True,
                        help='Output file path or a directory when -input-path is also a directory.')

    parser.add_argument('-output-format', '-f', action='store', dest='output_format', default='txt',
                        choices=OUTPUT_FORMAT_OPTS, help='Output file format: txt, bin. Default is txt.')

    parser.add_argument('-sound-type', '-output-type', '-type', '-t', action='store', dest='output_type',
                        default='float32', choices=OUTPUT_TYPE_OPTS, help='Output type. Default is float32.')

    parser.add_argument('-window-size', '-size', action='store', dest='window_size', type=int, default=0,
                        help='Window size (samples). If window size is 0 or negative no windowing will be performed'
                             '(the whole audio signal will be considered as 1 window). Default is 0.')

    parser.add_argument('-window-overlap', '-overlap', action='store', dest='window_overlap', type=int, default=0,
                        help='Window overlap (samples). Should be strictly smaller than the window size. Default is 0.')

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

            # Skip files which are not sounds.
            if input_ext not in ['.wav']:
                continue

            # Process current sound.
            process_sound(input_path=input_file,
                          output_path=output_file,
                          output_format=args.output_format,
                          output_type=args.output_type,
                          window_size=args.window_size,
                          window_overlap=args.window_overlap)

            print('Sound processed to file \"%s\" ...' % output_file)

    else:

        # Process only one sound.
        process_sound(input_path=args.input_path,
                      output_path=args.output_path,
                      output_format=args.output_format,
                      output_type=args.output_type,
                      window_size=args.window_size,
                      window_overlap=args.window_overlap)

        print('Sound processed to file \"%s\" ...' % args.output_path)
