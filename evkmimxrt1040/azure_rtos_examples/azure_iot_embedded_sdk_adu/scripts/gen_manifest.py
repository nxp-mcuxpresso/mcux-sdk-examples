#!/usr/bin/env python3

UPDATE_PRVIDER = "NXP"
UPDATE_NAME = "MIMXRT1xxx"

DEVICE_MANUFACTURER = "NXP"
DEVICE_MODEL = "MIMXRT1xxx"

manifest_template = """
{
    "updateId": {
        "provider": "",
        "name": "",
        "version": ""
    },
    "isDeployable": true,
    "compatibility": [
        {
            "deviceManufacturer": "",
            "deviceModel": ""
        }
    ],
    "instructions": {
        "steps": [
            {
                "type": "inline",
                "handler": "microsoft/swupdate:1",
                "files": [
                    ""
                ],
                "handlerProperties": {
                    "installedCriteria": "1.0"
                }
            }
        ]
    },
    "files": [
        {
            "filename": "",
            "sizeInBytes": 0,
            "hashes": {
                "sha256": ""
            }
        }
    ],
    "createdDateTime": "",
    "manifestVersion": "4.0"
}
"""

import os
import json
from datetime import datetime
import hashlib
import base64
import argparse


def generate_adu_manifest(infile,
                          fm_version,
                          update_provider=UPDATE_PRVIDER,
                          update_name=UPDATE_NAME,
                          device_manufacturer=DEVICE_MANUFACTURER,
                          device_model=DEVICE_MODEL):
    """

    Args:
        infile: input image file path
        fm_version: firmware version, for example, '1.0.0'
        outfile: optional, output manifest file path
        update_provider: optional, iot device update provider
        update_name: optional, iot device update name
        device_manufacturer: optional, iot device manufacturer
        device_model: optional, iot device model

    Returns: out file path

    """
    assert os.path.exists(infile)
    assert fm_version

    # get in file name
    infile_name = os.path.basename(infile)
    infile_name_no_ext = os.path.splitext(infile_name)[0]

    manifest = json.loads(manifest_template)

    manifest["updateId"]["provider"] = update_provider
    manifest["updateId"]["name"] = update_name
    manifest["updateId"]["version"] = fm_version

    manifest["compatibility"][0]["deviceManufacturer"] = device_manufacturer
    manifest["compatibility"][0]["deviceModel"] = device_model

    manifest["instructions"]["steps"][0]["files"][0] = infile_name

    with open(infile, 'rb') as f:
        date = f.read()
        hash_value = hashlib.sha256(date)

    manifest["files"][0]["filename"] = infile_name
    manifest["files"][0]["sizeInBytes"] = os.path.getsize(infile)
    manifest["files"][0]["hashes"]["sha256"] = str(base64.b64encode(hash_value.digest()), "utf-8")

    manifest["createdDateTime"] = datetime.utcnow().isoformat() + 'Z'

    manifest_file_name = f'{infile_name_no_ext}.importmanifest.json'
    manifest_file = os.path.join(os.path.dirname(infile), manifest_file_name)

    with open(manifest_file, 'w') as f:
        f.write(json.dumps(manifest, indent=4))

    print('The manifest file: %s' % manifest_file)
    return manifest_file

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--infile', help='the image file', required=True)
    parser.add_argument('-v', '--version', help='the firmware version', required=True)
    args = parser.parse_args()

    generate_adu_manifest(args.infile, args.version)
