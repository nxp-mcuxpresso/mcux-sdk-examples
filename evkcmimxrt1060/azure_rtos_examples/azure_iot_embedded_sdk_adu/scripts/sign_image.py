#!/usr/bin/env python3

# one flash partition size by default
FLASH_PARTITION_SIZE = 0x200000

import os
import sys
import argparse

import datetime

try:
    from imgtool import image
except ImportError:
    print("Please install imgtool.")
    sys.exit(1)

import imgtool.keys as keys
from imgtool.version import decode_version


def adu_sign_image(keyfile, firmware_version, infile, firstboot=False, outfile=None, slot_size=FLASH_PARTITION_SIZE):
    """

    Args:
        keyfile: private key file for image signature
        firmware_version: firmware version, for example, '1.0.0'
        infile: input image file path
        firstboot: optional, True: the app image is first boot
        outfile: output file path
        slot_size: specify the image size when generating a full-size image for the first boot
    Returns:    out file path

    """
    assert os.path.exists(keyfile)
    assert firmware_version
    assert os.path.exists(infile)

    # get file name without extension
    infile_name_no_ext = os.path.splitext(os.path.basename(infile))[0]

    # make an image with the size of slot_size if it's for the first boot
    confirm = True if firstboot else False
    pad = True if firstboot else False

    img = image.Image(
                align=4,
                header_size=0x400,
                pad_header=True,
                slot_size=slot_size,
                max_sectors=800,
                version=decode_version(firmware_version),
                confirm=confirm,
                pad=pad)

    img.load(infile)

    key = keys.load(keyfile)
    if key is None:
        print("Key error!")
        sys.exit(1)

    img.create(key=key, public_key_format='hash', enckey=None)

    boot = '.boot' if firstboot else ''
    timestamp = int(datetime.datetime.utcnow().timestamp())
    outfile_name = f"{infile_name_no_ext}.{firmware_version}.signed.{str(timestamp)}{boot}.bin"

    outfile_path = outfile if outfile else os.path.join(os.path.dirname(infile), outfile_name)

    img.save(outfile_path, None)

    print('The image file is: ' + outfile_path)
    return outfile_path


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-k', '--key', help='the private key for signing image', required=True)
    parser.add_argument('-v', '--version', help='the firmware version', required=True)
    parser.add_argument('-i', '--infile', help='the raw image file', required=True)
    parser.add_argument('-f', '--firstboot', action='store_true', help='generate a full-size image for the first boot')
    parser.add_argument('-s', '--slotsize', help='specify the image size when generating a full-size image, 2097152 by default (2M)',
                        type=int, default=FLASH_PARTITION_SIZE)
    args = parser.parse_args()

    adu_sign_image(args.key, args.version, args.infile, firstboot=args.firstboot, slot_size=args.slotsize)
