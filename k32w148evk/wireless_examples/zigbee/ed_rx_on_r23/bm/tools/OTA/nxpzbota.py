#
# Copyright 2023 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

import argparse
import os
import sys
from collections import namedtuple
import re
import argparse
import subprocess
import struct
import binascii
import string
import crccheck
import logging
import tempfile
from packaging.version import Version
import spsdk

# Inform User that specific SPSDK packages version is required
assert Version(spsdk.__version__) < Version('2.0.0') and Version(spsdk.__version__) >= Version('1.0.0'), 'SPSDK Version should be < "2.0.0" and > "1.0.0"'

from Crypto.Cipher import AES

from spsdk.image.mbimg import get_mbi_class
from spsdk.sbfile.sb31.images import SB3_SCH_FILE, SecureBinary31
from spsdk.utils.schema_validator import (
    ValidationSchemas,
    check_config,
)

import spsdk.utils.misc

from spsdk.utils.misc import (
    get_abs_path,
    load_configuration
)

# Init logger
FORMAT = '%(asctime)s,%(msecs)03d %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s'

# Make logger to print on stdout */
stdout_handler = logging.StreamHandler(sys.stdout)

logging.basicConfig(format=FORMAT, datefmt='%Y-%m-%d:%H:%M:%S', handlers=[stdout_handler])
logger = logging.getLogger('nxpzbota')
script_path = None
version_file_path = None

if getattr(sys, 'frozen', False):
    # If the application is run as a bundle, the PyInstaller bootloader
    # extends the sys module by a flag frozen=True and sets the app sys executable.
    script_path = os.path.dirname(sys.executable)
    base_path = sys._MEIPASS
    version_file_path = os.path.join(base_path, 'version.txt')
else:
    script_path = os.path.dirname(os.path.abspath(__file__))
    version_file_path = os.path.join(script_path, 'version.txt')

# Get SCRIPT DIRECTORY, used for local referenced files (SB3 jsons, keys & certs)
SCRIPT_DIRECTORY = os.path.abspath(os.path.realpath(script_path))

# Get SCRIPT VERSION
if os.path.exists(version_file_path):
    with open(version_file_path) as f:
        SCRIPT_VERSION = f.readlines()[0].strip()
else:
    # Running on a versionless build
    SCRIPT_VERSION = 'versionless'

# SPSDK SB3 Generation files
MBI_SIGN_JSON = os.path.join(SCRIPT_DIRECTORY, 'SB3', 'sign_mcu_file_json_example.json')
SB3_WIHTOUT_NBU = os.path.join(SCRIPT_DIRECTORY, 'SB3', 'sb3_json_example_without_nbu.json')

# Constants
# NONCE, LINKKEY and OTA HDR sections with their hardocded names and min sizes
NONCE_SECTION_NAME = '.ro_nonce'
NONCE_SECTION_SIZE = 16

LINK_KEY_SECTION_NAME = '.ro_se_lnkKey'
LINK_KEY_SECTION_SIZE = 16

# Normally OTA HDR is 56 bytes, but there is an additional 13 bytes for EXTRA HDR (optional)
# resulting in 69 bytes, but it is aligned to 16 so it will be padded until the size is 80
# 4 bytes of this space is used for CRC
OTA_HDR_SECTION_NAME = '.ro_ota_header'
OTA_HDR_SECTION_SIZE = 69
OTA_HDR_TOTAL_SIZE = 80
OTA_HDR_CRC_SIZE = 4

def parse_sections(file):
    sections = {}

    Section = namedtuple('Section', ['idx', 'name', 'size', 'vma', 'lma', 'offset', 'align', 'flags'])

    objdump = subprocess.check_output(['arm-none-eabi-objdump', '-h', file])
    objdump = objdump.decode('utf-8')

    section_re = re.compile(
        r'(?P<idx>[0-9]+)\s'
        r'(?P<name>.{13,})s*'
        r'(?P<size>[0-9a-f]{8})\s*'
        r'(?P<vma>[0-9a-f]{8})\s*'
        r'(?P<lma>[0-9a-f]{8})\s*'
        r'(?P<offset>[0-9a-f]{8})\s*'
        r'(?P<align>[0-9*]*)\s*'
        r'(?P<flags>[\[[\w]*[, [\w]*]*)'
    )

    for match in re.finditer(section_re, objdump):
        sec_dict = match.groupdict()

        sec_dict['idx'] = int(sec_dict['idx'])

        for attr in ['vma', 'lma', 'size', 'offset']:
            sec_dict[attr] = int(sec_dict[attr], 16)

        sec_dict['align'] = eval(sec_dict['align'])

        # sections[sec_dict['name']] = Section(**sec_dict)
        sections[sec_dict['idx']] = Section(**sec_dict)

    return sections

def gen_binary(elf):
    data = b''

    temp_bin = tempfile.NamedTemporaryFile(delete=False)
    subprocess.run(['arm-none-eabi-objcopy', '-O', 'binary', elf, temp_bin.name], check = True)
    temp_bin.seek(0)
    data = temp_bin.read()

    # Make sure to close and delete temp file
    temp_bin.close()
    os.unlink(temp_bin.name)

    return data

def image_size(elf):
    return len(gen_binary(elf))

def file_size(file):
    size = os.path.getsize(file)
    return size

def copy_file(src, dest):
    # Copy input file in output file
    with open(src, 'rb') as src, open(dest, 'wb') as dst:
        dst.write(src.read())

def read_file(file, offset, size):
    data = b''
    with open(file, 'r+b') as file:
        file.seek(offset)
        data = file.read(size)
    return data

def write_file(file, data, offset):
    with open(file, 'r+b') as file:
        file.seek(offset)
        file.write(data)

def pad16_image(elf):
    current_size = image_size(elf)

    padding_len = (16 - (current_size % 16)) & 15

    sections = parse_sections(elf)

    if 1:
        print("Section 16 byte alignment %d" % (padding_len))

    if padding_len != 0:
        padding_bytes = bytearray(padding_len)

        last_section = get_last_section(sections)
        if last_section == None:
            return

        last_section_name = last_section.name.rstrip()

        # and add quotes around the section name
        last_section_name = r"%s" % (last_section_name)

        with open(elf, 'r+b') as elf_file:
            elf_file.seek(last_section.offset, 0)
            initial_last_section_content = elf_file.read(last_section.size)
            elf_file.close()

        # Update last section to include padding
        with open('temp_last_section.bin', 'wb') as tmp_file:
            tmp_file.write(initial_last_section_content)
            tmp_file.write(padding_bytes)

        ret = subprocess.call(['arm-none-eabi-objcopy',
                                                  '--update-section',
                                                  '%s=temp_last_section.bin' % last_section_name,
                                                  elf,
                                                  elf])
        if ret != 0:
            print('Error padding')

        os.remove('temp_last_section.bin')

def get_last_section(sections):
    last_section = None
    for _, section in sections.items():
        if 'LOAD' in section.flags:
            if last_section is None or section.lma > last_section.lma:
                if section.size > 0:
                    last_section = section

    return last_section

def get_section(sections, section_name):
    section_found = None
    for _, section in sections.items():
        # if 1:
        #     print("Section: {:s} {:s} {:x} {:x}".format(section.name, section.flags, section.lma, section.size))
        if section.name.rstrip() == section_name and section.size != 0:
            section_found = section
    return section_found

#
# Fill in the Nonce section that is used for AES image encryption and decryption
# Populating the .ro_nonce overwrites the section not changing its size
#
def PopulateNonceSection(elf, nonce):
    if not elf:
        raise Exception(f'Error with input elf file: {elf}')

    nonce_section = get_section(parse_sections(elf), NONCE_SECTION_NAME)
    if nonce_section is None:
        raise Exception(f'{NONCE_SECTION_NAME} section not found in ELF file')
    else:
        if nonce_section.size < NONCE_SECTION_SIZE:
            raise Exception(f'{NONCE_SECTION_NAME} section is smaller ({nonce_section.size}) than required {NONCE_SECTION_SIZE}')

        if nonce.find("0x") == -1:
            sNonceString = nonce
        else:
            # skip the 0x
            sNonceString = nonce[2:]
        sNonceString = sNonceString.strip()

        if len(sNonceString) != NONCE_SECTION_SIZE * 2:
            raise Exception(f'Nonce {sNonceString} must be a 32 character string composed of hex digits')
        if not all(c in string.hexdigits for c in sNonceString):
            raise Exception(f'Nonce {sNonceString} must be composed of hex characters')

        # Get bytes from HEX string eg. 'BA' -> b'\xba'
        nonce = binascii.unhexlify(sNonceString)

        write_file(elf, nonce, nonce_section.offset)


def GetNonceFromSection(elf):
    if not elf:
        raise Exception(f'Error with input elf file: {elf}')

    nonce_section = get_section(parse_sections(elf), NONCE_SECTION_NAME)
    if nonce_section is None:
        raise Exception(f'{NONCE_SECTION_NAME} section not found in ELF file')
    else:
        if nonce_section.size < NONCE_SECTION_SIZE:
            raise Exception(f'{NONCE_SECTION_NAME} section is smaller ({nonce_section.size}) than required {NONCE_SECTION_SIZE}')

        nonce = read_file(elf, nonce_section.offset, NONCE_SECTION_SIZE)

        # Get HEX string from bytes eg. b'\xba' -> 'BA'
        nonce_str = nonce.hex()

        return nonce_str

#
# ota_ext_hdr_value determines the format of the OTA Header extension optional fields
#  
#
def gen_ota_hdr_ext(ota_ext_hdr_value, args):
    # write 13 bytes in all cases, the padding size is adjusted so that the place to wrtie the CRC to is always after
    if ota_ext_hdr_value == 0:
        ota_hdr_pad0_crc = struct.Struct("<13B")  # padding only till CRC
        ota_hdr_ext = ota_hdr_pad0_crc.pack(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                            0xff)
        hdr_ext_sz = 0
    elif ota_ext_hdr_value == 1:
        ota_hdr_ext_security = struct.Struct("<B12B")  # only security version optional field
        ota_hdr_ext = ota_hdr_ext_security.pack(args.OtaSecurityVersion, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                0xff, 0xff, 0xff, 0xff)
        hdr_ext_sz = ota_hdr_ext_security.size - 12
    elif ota_ext_hdr_value == 2:
        ota_hdr_ext_mac = struct.Struct("<Q5B")  # only OtaDestinationMac optional field
        ota_hdr_ext = ota_hdr_ext_mac.pack(args.OtaDestinationMac, 0xff, 0xff, 0xff, 0xff, 0xff)
        hdr_ext_sz = ota_hdr_ext_mac.size - 5
    elif ota_ext_hdr_value == 3:
        ota_hdr_ext_security_mac = struct.Struct("<BQ4B")
        ota_hdr_ext = ota_hdr_ext_security_mac.pack(args.OtaSecurityVersion, args.OtaDestinationMac, 0xff, 0xff, 0xff,
                                                    0xff)
        hdr_ext_sz = ota_hdr_ext_security_mac.size - 4
    elif ota_ext_hdr_value == 4:
        ota_hdr_ext_hardware = struct.Struct("<HH9B")
        ota_hdr_ext = ota_hdr_ext_hardware.pack(args.OtaHwVersionMin, args.OtaHwVersionMax, 0xff, 0xff, 0xff, 0xff,
                                                0xff, 0xff, 0xff, 0xff, 0xff)
        hdr_ext_sz = ota_hdr_ext_hardware.size - 9
    elif ota_ext_hdr_value == 5:
        ota_hdr_ext_security_hardware = struct.Struct("<BHH8B")
        ota_hdr_ext = ota_hdr_ext_security_hardware.pack(args.OtaSecurityVersion, args.OtaHwVersionMin,
                                                         args.OtaHwVersionMax, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                         0xff)
        hdr_ext_sz = ota_hdr_ext_security_hardware.size - 8
    elif ota_ext_hdr_value == 6:
        ota_hdr_ext_mac_hardware = struct.Struct("<QHHB")  # single padding byte
        ota_hdr_ext = ota_hdr_ext_mac_hardware.pack(args.OtaDestinationMac, args.OtaHwVersionMin, args.OtaHwVersionMax,
                                                    0xff)
        hdr_ext_sz = ota_hdr_ext_mac_hardware.size - 1
    elif ota_ext_hdr_value == 7:  # all optional fields present no padding
        ota_hdr_ext_s = struct.Struct("<BQHH")
        ota_hdr_ext = ota_hdr_ext_s.pack(args.OtaSecurityVersion, args.OtaDestinationMac, args.OtaHwVersionMin,
                                         args.OtaHwVersionMax)
        hdr_ext_sz = ota_hdr_ext_s.size
    else:
        ota_hdr_ext = b''
        hdr_ext_sz = 0

    return ota_hdr_ext, hdr_ext_sz

#
# Populating the .ro_ota_header overwrites the section not changing its size
# Note that img_size must have been determined already
#
def gen_ota_hdr(args, img_size):
    # the OTA header section is a placeholder that must contain the 56 mandatory bytes 
    # + 13 optional bytes 
    # + 4 CRC bytes --> hence the 73 
    # The actual section size is normally rounded to 80 bytes
    OTA_HEADER_MARKER = 0x0beef11e
    ota_hdr_s = struct.Struct("<I5HIH32sI")
    ota_ext_hdr = False

    ota_ext_hdr_value = 0
    manufacturer = int(args.OtaManufacturer, 0)
    imageType = int(args.OtaImageType, 0)
    imageVersion = int(args.OtaFileVersionNb, 0)

    if args.OtaVersionString is None or len(args.OtaVersionString) != 32:
        raise Exception(f'OTA Version string {args.OtaVersionString} must be a 32 character string')

    # The OTA header may contain 3 optional fields:
    #  Hw Version range defined by Min and Max HH
    #  Security version
    # if either OtaHwVersionMin or OtaHwVersionMax or both are specified
    # a Hw restriction applies and the extension must be added
    if args.OtaHwVersionMin is not None or args.OtaHwVersionMax is not None:
        if args.OtaHwVersionMax > 0 and args.OtaHwVersionMax <= 0xffff:
            if args.OtaHwVersionMax >= args.OtaHwVersionMin:
                ota_ext_hdr = True
                ota_ext_hdr_value |= 4
    # if either OtaSecurityVersion is defined
    # a Hw restriction applies and the extension must be added
    if (args.OtaSecurityVersion != None):
        ota_ext_hdr_value |= 1

    # OtaDestinationMac is to fill the u64UpgradeFileDest optional field
    if (args.OtaDestinationMac != None):
        ota_ext_hdr = True
        ota_ext_hdr_value |= 2

    if ota_ext_hdr_value != 0:
        ota_ext_hdr = True

    #####################
    # PopulateOtaHeaderOptionalFields call
    # hdr_ext_sz returns the number of significant extension bytes
    # but PopulateOtaHeaderOptionalFields fills padding up to CRC location

    ota_hdr_ext, hdr_ext_sz = gen_ota_hdr_ext(ota_ext_hdr_value, args)
    header_size = ota_hdr_s.size
    header_size += hdr_ext_sz

    total_image_size = header_size + img_size + 6
    crc_s = struct.Struct("<I")
    crc = crc_s.pack(0)
    version_string = args.OtaVersionString.encode()
    version_array = bytearray(version_string)
    ota_hdr = ota_hdr_s.pack(
        OTA_HEADER_MARKER,
        int(args.OtaHeaderVersionNb, 0),
        header_size,
        ota_ext_hdr_value if ota_ext_hdr else 0,
        manufacturer,
        imageType,
        imageVersion,
        int(args.OtaStackVersionNb, 0),
        version_array,
        total_image_size)

    # TODO: Better handle this case
    ota_hdr_ext_size = hdr_ext_sz

    return ota_hdr, ota_hdr_ext, ota_hdr_ext_size, crc

def PopulateOtaHdrSection(elf, ota_hdr, ota_hdr_ext, crc):
    if not elf:
        raise Exception(f'Error with input elf file: {elf}')

    ota_hdr_section = get_section(parse_sections(elf), OTA_HDR_SECTION_NAME)
    if ota_hdr_section is None:
        raise Exception(f'{OTA_HDR_SECTION_NAME} section not found in ELF file')
    else:
        if ota_hdr_section.size < OTA_HDR_TOTAL_SIZE:
            raise Exception(f'{OTA_HDR_SECTION_NAME} section is smaller ({ota_hdr_section.size}) than required {OTA_HDR_SECTION_SIZE}')

        if not ota_hdr or not ota_hdr_ext or not crc:
            raise Exception(f'Failed to generate OTA HDR, EXT HDR and CRC')

        data = ota_hdr + ota_hdr_ext + crc
        write_file(elf, data, ota_hdr_section.offset)

def PatchOtaHeaderImageCrc(elf, crc_value):
    if not elf:
        raise Exception(f'Error with input elf file: {elf}')

    ota_hdr_section = get_section(parse_sections(elf), OTA_HDR_SECTION_NAME)
    if ota_hdr_section is None:
        raise Exception(f'{OTA_HDR_SECTION_NAME} section not found in ELF file')
    else:
        crc_s = struct.Struct("<I")
        crc = crc_s.pack(crc_value)
        write_file(elf, crc, ota_hdr_section.offset + OTA_HDR_SECTION_SIZE)

# Populating the .ro_se_LnkKey overwrites the section not changing its size
#
def PopulateLnkKeySection(elf, linkkey):
    if not elf:
        raise Exception(f'Error with input elf file: {elf}')

    lnk_key_section = get_section(parse_sections(elf), LINK_KEY_SECTION_NAME)
    if lnk_key_section is None:
        raise Exception(f'{LINK_KEY_SECTION_NAME} section not found in ELF file')
    else:
        if lnk_key_section.size < LINK_KEY_SECTION_SIZE:
            raise Exception(f'{LINK_KEY_SECTION_NAME} section is smaller ({lnk_key_section.size}) than required {LINK_KEY_SECTION_SIZE}')

        if linkkey.find("0x") == -1:
            sKeyString = linkkey
        else:
            # skip the 0x
            sKeyString = linkkey[2:]
        sKeyString = sKeyString.strip()

        if len(sKeyString) != LINK_KEY_SECTION_SIZE * 2:
            raise Exception(f'Link key {sKeyString} must be a 32 character string composed of hex digits')
        if not all(c in string.hexdigits for c in sKeyString):
            raise Exception(f'Link key {sKeyString} must be composed of hex characters')

        lnk_key = binascii.unhexlify(sKeyString)
        write_file(elf, lnk_key, lnk_key_section.offset)

def GetLnkKeyFromSection(elf):
    if not elf:
        raise Exception(f'Error with input elf file: {elf}')

    lnk_key_section = get_section(parse_sections(elf), LINK_KEY_SECTION_NAME)
    if lnk_key_section is None:
        raise Exception(f'{LINK_KEY_SECTION_NAME} section not found in ELF file')
    else:
        if lnk_key_section.size < LINK_KEY_SECTION_SIZE:
            raise Exception(f'{LINK_KEY_SECTION_NAME} section is smaller ({lnk_key_section.size}) than required {LINK_KEY_SECTION_SIZE}')

        key = read_file(elf, lnk_key_section.offset, LINK_KEY_SECTION_SIZE)
        key_str = key.hex()

        return key_str

def sb3_export(input, output, config):
    config_data = load_configuration(config)

    # Adjust input and output
    # Assuming that 0x0 address load command is input binary
    config_data['containerOutputFile'] = output
    for comm in config_data['commands']:
        if 'load' in comm and comm['load']['address'] == '0x0':
            comm['load']['file'] = input
            break

    def patch_config_rel_dir(config, key):
        return os.path.join(SCRIPT_DIRECTORY, config[key])

    config_data['containerKeyBlobEncryptionKey'] = patch_config_rel_dir(config_data, 'containerKeyBlobEncryptionKey')
    config_data['rootCertificate0File'] = patch_config_rel_dir(config_data, 'rootCertificate0File')
    config_data['rootCertificate1File'] = patch_config_rel_dir(config_data, 'rootCertificate1File')
    config_data['rootCertificate2File'] = patch_config_rel_dir(config_data, 'rootCertificate2File')
    config_data['rootCertificate3File'] = patch_config_rel_dir(config_data, 'rootCertificate3File')
    config_data['mainRootCertPrivateKeyFile'] = patch_config_rel_dir(config_data, 'mainRootCertPrivateKeyFile')
    config_data['signingCertificateFile'] = patch_config_rel_dir(config_data, 'signingCertificateFile')
    config_data['signingCertificatePrivateKeyFile'] = patch_config_rel_dir(config_data, 'signingCertificatePrivateKeyFile')

    config_dir = os.path.dirname(config)
    check_config(config_data, SecureBinary31.get_validation_schemas_family())
    schemas = SecureBinary31.get_validation_schemas(
        config_data["family"], include_test_configuration=True
    )
    schemas.append(ValidationSchemas.get_schema_file(SB3_SCH_FILE)["sb3_output"])
    check_config(config_data, schemas, search_paths=[config_dir])
    sb3 = SecureBinary31.load_from_config(config_data, search_paths=[config_dir])

    sb3_data = sb3.export()
    sb3_output_file_path = get_abs_path(config_data["containerOutputFile"], config_dir)
    spsdk.utils.misc.write_file(sb3_data, sb3_output_file_path, mode="wb")

def mbi_export(input, output, config):
    config_data = load_configuration(config)

    def patch_config_rel_dir(config, key):
        return os.path.join(SCRIPT_DIRECTORY, config[key])

    config_data['inputImageFile'] = input
    config_data['masterBootOutputFile'] = output
    config_data['rootCertificate0File'] = patch_config_rel_dir(config_data, 'rootCertificate0File')
    config_data['rootCertificate1File'] = patch_config_rel_dir(config_data, 'rootCertificate1File')
    config_data['rootCertificate2File'] = patch_config_rel_dir(config_data, 'rootCertificate2File')
    config_data['rootCertificate3File'] = patch_config_rel_dir(config_data, 'rootCertificate3File')
    config_data['mainRootCertPrivateKeyFile'] = patch_config_rel_dir(config_data, 'mainRootCertPrivateKeyFile')
    config_data['signingCertificateFile'] = patch_config_rel_dir(config_data, 'signingCertificateFile')
    config_data['signingCertificatePrivateKeyFile'] = patch_config_rel_dir(config_data, 'signingCertificatePrivateKeyFile')

    config_dir = os.path.dirname(config)

    mbi_cls = get_mbi_class(config_data)
    check_config(config_data, mbi_cls.get_validation_schemas(), search_paths=[config_dir])
    mbi_obj = mbi_cls()
    mbi_obj.load_from_config(config_data, search_paths=[config_dir])
    mbi_data = mbi_obj.export()

    mbi_output_file_path = get_abs_path(config_data["masterBootOutputFile"], config_dir)
    spsdk.utils.misc.write_file(mbi_data, mbi_output_file_path, mode="wb")

def generate_sb3(elf):
    # Generate binary to be used by MBI
    # Create temporary files to hold the binary and master boot output
    bin_temp = tempfile.NamedTemporaryFile(delete=False)
    mbr_temp = tempfile.NamedTemporaryFile(delete=False)
    sb3_temp = tempfile.NamedTemporaryFile(delete=False)

    bin_temp.write(gen_binary(elf))
    bin_temp.close()

    mbi_export(bin_temp.name, mbr_temp.name, MBI_SIGN_JSON)
    sb3_export(mbr_temp.name, sb3_temp.name, SB3_WIHTOUT_NBU)

    data = sb3_temp.read()

    # Make sure to close and delete temporary files
    os.unlink(bin_temp.name)
    mbr_temp.close()
    os.unlink(mbr_temp.name)
    sb3_temp.close()
    os.unlink(sb3_temp.name)

    return data

def auto_int(x):
    return int(x, 0)

def pad16_data(data):
    if (len(data) % 16) != 0:
        remainder = 16 - (len(data) % 16)
        for _ in range(remainder):
            data = data + b'\xff'

    return data


def encryptFlashData(nonce, key, data):
    encyptedBlock = []

    data = pad16_data(data)

    r = AES.new(key, AES.MODE_ECB)

    nonce = [int.from_bytes(nonce[0:4], byteorder='big'),
             int.from_bytes(nonce[4:8], byteorder='big'),
             int.from_bytes(nonce[8:12], byteorder='big'),
             int.from_bytes(nonce[12:16], byteorder='big')]

    for x in range(len(data) // 16):
        # use nonce value to create encrpyted chunk
        encryptNonce = b''
        for i in nonce:
            encryptNonce += i.to_bytes(4, 'big')

        encChunk = r.encrypt(encryptNonce)

        # increment the nonce value
        if (nonce[3] == 0xffffffff):
            nonce[3] = 0
        else:
            nonce[3] += 1

        # xor encypted junk with data chunk
        chunk = data[x*16:(x+1)*16]        # Read 16 byte chucks. 128 bits

        loutChunk = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

        for i in range(16):
            loutChunk[i] = chunk[i] ^ encChunk[i]
            encyptedBlock.append(chunk[i] ^ encChunk[i])

    return bytes(encyptedBlock)
 
def main():

    parser = argparse.ArgumentParser(
                    prog='nxpzbota',
                    description='NXP Zigbee OTA image generator',
                    epilog='Currently only K32W1 is supported.')

    parser.add_argument('-i', '--input', help='ELF/AXF input file.', required=True)
    parser.add_argument('-o', '--output', help='ELF/AXF populated output file if --embed, OTA binary output file name otherwise.', required=True)
    parser.add_argument('-d', '--device', help='Device type', required=True, choices=['K32W1'])

    parser.add_argument('--embed', help='Only populate OTA HDR, Nonce, Link Key of the input AXF. Don\'t generate OTA file.', action='store_true')

    parser.add_argument('-e', '--encrypt', help='Enable NXP encryption of OTA image.', action='store_true')
    parser.add_argument("-k", '--key', help='NXP key used to encrypt OTA image.')
    parser.add_argument("-n", '--nonce', help="Nonce Initial Vector for encryption 16 hex bytes.")
    parser.add_argument("-l", '--linkkey', help="Link key encryption/decryption 16 hex bytes.")
    parser.add_argument('-log',
                '--loglevel',
                default='INFO',
                help='Provide logging level. Example --loglevel DEBUG, default INFO',
                choices=logging._nameToLevel.keys())

    parser.add_argument('-v', '--version', action='version', version='%(prog)s-' + SCRIPT_VERSION)

    parser.add_argument("-OtaMan", '--OtaManufacturer', default="0x1037", help="OTA manufacturer code.")
    parser.add_argument("-OtaVersionStr", '--OtaVersionString', help="OTA version string.", required=True)

    parser.add_argument("-OtaImgTyp", '--OtaImageType', default="0x0001", help="OTA header image type")
    parser.add_argument("-OtaHdrV", '--OtaHeaderVersionNb', default="0x0100", help="OTA header version")
    parser.add_argument("-OtaFileVersion", '--OtaFileVersionNb', default="0x0001", help="OTA File version")
    parser.add_argument("-OtaStackVersion", '--OtaStackVersionNb', default="0x002", help="OTA stack version")
    parser.add_argument("-OtaDestMac", '--OtaDestinationMac', type=int,
                        help="IEEE address of destination node targeted by OTA")
    parser.add_argument("-OtaSecV", '--OtaSecurityVersion', type=auto_int, help="Ota security credential version")
    parser.add_argument('-OtaHwMin', '--OtaHwVersionMin', type=int,
                        help="define min of Hw versions compatible with OTA file. Implies inclusion in OTA header")
    parser.add_argument('-OtaHwMax', '--OtaHwVersionMax', type=int,
                        help="define max of Hw versions compatible with OTA file. Implies inclusion in OTA header")

    args = parser.parse_args()
    logger.setLevel(args.loglevel.upper())

    if args.encrypt and (args.key is None):
        parser.error('encryption is used, please provide a key with -k argument')

    elf_file_input = os.path.abspath(args.input)
    file_output = os.path.abspath(args.output)

    logger.info(f'NXPZBOTA generation tool')

    if args.embed:
        logger.info(f'Embedding provided OTA info')
    else:
        logger.info(f'Generate OTA binary file')

    logger.info(f'Input  file: {elf_file_input}')
    logger.info(f'Output file: {file_output}')

    # Temprary file to populate the various sections
    temp_elf_file = tempfile.NamedTemporaryFile(delete=False).name
    logger.debug(f'Using temp file {temp_elf_file} to populate sections, input AXF file will not modify.')

    # Copy INPUT file to temp one
    copy_file(elf_file_input, temp_elf_file)

    if not args.nonce:
        logger.debug(f'Nonce not provided, reading from ELF section.')
        args.nonce = GetNonceFromSection(temp_elf_file)

    logger.debug(f'NONCE: {args.nonce}')

    if not args.linkkey:
        logger.debug(f'Link key not provided, reading from ELF section.')
        args.linkkey = GetLnkKeyFromSection(temp_elf_file)

    logger.debug(f'LNKKEY: {args.linkkey}')

    logger.debug(f'Populate nonce section with: "{args.nonce}"')
    PopulateNonceSection(temp_elf_file, args.nonce)

    logger.debug(f'Populate link key section with: "{args.linkkey}"')
    PopulateLnkKeySection(temp_elf_file, args.linkkey)

    img_total_len = image_size(temp_elf_file)

    # Force CRC to 0 at this stage
    ota_hdr, ota_hdr_ext, _, crc = gen_ota_hdr(args, img_total_len)
    logger.debug(f'Populate OTA and optional headers')
    PopulateOtaHdrSection(temp_elf_file, ota_hdr, ota_hdr_ext, crc)

    # The ELF file is patched at this stage : let's proceeed to the CRC computation
    imagecrc = crccheck.crc.Crc32Bzip2().process(gen_binary(temp_elf_file)).finalhex()

    logger.debug(f'Populate image with computed CRC: "{imagecrc}"')
    PatchOtaHeaderImageCrc(temp_elf_file, int("0x" + imagecrc, 16))

    if args.embed:
        logger.debug(f'Embed only build, writing the patched AXF/ELF.')
        # Copy file to output file
        copy_file(temp_elf_file, file_output)
    else:
        # Actual OTA image generation (NXP OTA img format + ZB OTA image format)
        logger.debug(f'Starting OTA image generation')

        ota_file_data = generate_sb3(temp_elf_file)

        logger.debug(f'Generated SB3, bin len: {len(ota_file_data)}')

        size_before = len(ota_file_data)
        ota_file_data = binascii.unhexlify(args.linkkey) + ota_file_data
        logger.debug(f'Added NXP link key: "{args.linkkey}", bin len: {len(ota_file_data)} diff: {len(ota_file_data) - size_before}')

        size_before = len(ota_file_data)
        ota_file_data = pad16_data(ota_file_data)
        logger.debug(f'Padding image to multiple of 16, bin len: {len(ota_file_data)} diff {len(ota_file_data) - size_before} (padded)')

        # Keep the encrypted data separate, CRC calculation is done on plain data
        # If encryption is not needed, it will be the same plain data
        ota_file_data_encrypted = ota_file_data
        if args.encrypt:
            size_before = len(ota_file_data)
            ota_file_data_encrypted = encryptFlashData(binascii.unhexlify(args.nonce), binascii.unhexlify(args.key), ota_file_data)
            logger.debug(f'Encrypted binary, bin len: {len(ota_file_data)}, diff: {len(ota_file_data) - size_before} (padded, should be 0)')
        else:
            logger.debug(f'Encryption skipped')

        size_before = len(ota_file_data)
        ota_file_data = binascii.unhexlify(args.nonce) + ota_file_data
        logger.debug(f'Added NXP Nonce: "{args.nonce}", bin len: {len(ota_file_data)}, diff: {len(ota_file_data) - size_before}')

        ota_hdr, ota_hdr_ext, _, crc = gen_ota_hdr(args, len(ota_file_data))

        size_before = len(ota_file_data)
        if ota_hdr != None and ota_hdr_ext != None and crc != None:
            remainder = OTA_HDR_TOTAL_SIZE - len(ota_hdr + ota_hdr_ext + crc)
            copy_ota_file_data = ota_hdr + ota_hdr_ext + crc + (b'\xff' * remainder) + ota_file_data

            # Compute CRC on the copy data file
            imagecrc = crccheck.crc.Crc32Bzip2().process(copy_ota_file_data).finalhex()

            crc_s = struct.Struct("<I")
            crc = crc_s.pack(int('0x' + imagecrc, 16))

            ota_file_data = copy_ota_file_data[0:OTA_HDR_SECTION_SIZE] + crc + copy_ota_file_data[OTA_HDR_SECTION_SIZE + OTA_HDR_CRC_SIZE:]

        # Patch encrypted binary, CRC had to be calculated on plain binary
        offset = len(ota_file_data) - len(ota_file_data_encrypted)
        ota_file_data = ota_file_data[0:offset] + ota_file_data_encrypted

        logger.debug(f'Added NXP OTA header and CRC "{imagecrc}", bin len: {len(ota_file_data)}, diff: {len(ota_file_data) - size_before}')

        logger.debug(f'NXP OTA Image finished, bin len: {len(ota_file_data)}')

        # ZB OTA file format generation
        logger.debug(f'Starting ZB OTA image creation, bin len: {len(ota_file_data)}')

        ota_hdr, ota_hdr_ext, ota_hdr_ext_size, _ = gen_ota_hdr(args, len(ota_file_data))

        element_hdr_s = struct.Struct("<HI")
        image_hdr = element_hdr_s.pack(
            0,
            len(ota_file_data))

        size_before = len(ota_file_data)
        ota_file_data = image_hdr + ota_file_data
        logger.debug(f'Added Element HDR (for the binary image), bin len: {len(ota_file_data)}, diff: {len(ota_file_data) - size_before}')

        size_before = len(ota_file_data)

        if ota_hdr_ext:
            ota_file_data = ota_hdr_ext[0:ota_hdr_ext_size] + ota_file_data
            logger.debug(f'Added ZB EXT HDR, bin len: {len(ota_file_data)}, diff: {len(ota_file_data) - size_before}')

        size_before = len(ota_file_data)

        if ota_hdr:
            ota_file_data = ota_hdr + ota_file_data
            logger.debug(f'Added ZB OTA HDR, bin len: {len(ota_file_data)}, diff: {len(ota_file_data) - size_before}')

        logger.info(f'Final binary size: {len(ota_file_data)}')

        with open(file_output, 'wb') as bin_file:
            bin_file.write(ota_file_data)

    # Clean temp files
    logger.debug(f'Cleanup temp file: {temp_elf_file}')
    os.unlink(temp_elf_file)

if __name__ == '__main__':
    main()
