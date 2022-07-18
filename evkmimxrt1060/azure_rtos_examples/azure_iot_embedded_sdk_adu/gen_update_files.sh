#!/bin/bash

# Before executing this script, please install imgtool and create-adu-import-manifest.sh

IMGTOOL=imgtool
GEN_MANIFEST="./create-adu-import-manifest.sh"
HEADER_FILE="./sample_config.h"

SAMPLE_UPDATE_ID_PROVIDER=$(grep "SAMPLE_UPDATE_ID_PROVIDER" ${HEADER_FILE} | cut -d '"' -f 2 -)
SAMPLE_UPDATE_ID_NAME=$(grep "SAMPLE_UPDATE_ID_NAME" ${HEADER_FILE} | cut -d '"' -f 2 -)
SAMPLE_UPDATE_ID_VERSION=$(grep "SAMPLE_UPDATE_ID_VERSION" ${HEADER_FILE} | cut -d '"' -f 2 -)

SAMPLE_DEVICE_MANUFACTURER=$(grep "SAMPLE_DEVICE_MANUFACTURER" ${HEADER_FILE} | cut -d '"' -f 2 -)
SAMPLE_DEVICE_MODEL=$(grep "SAMPLE_DEVICE_MODEL" ${HEADER_FILE} | cut -d '"' -f 2 -)

PRIV_KEY="./keys/sign-rsa2048-priv.pem"

if [ $# -eq 1 ]
then
    BINFILE=$1
    FILENAME=$(basename $1)
else
    echo "Please input a binary file. (*.bin)"
    exit 1
fi

if [ "${FILENAME##*.}" != "bin" ]
then
    echo "Please input a binary file. (*.bin)"
    exit 1
fi

OUTIMG=${SAMPLE_UPDATE_ID_NAME}.${FILENAME%.*}.${SAMPLE_UPDATE_ID_VERSION}.signed.bin

MANIFEST_FILE=${SAMPLE_UPDATE_ID_PROVIDER}.${SAMPLE_UPDATE_ID_NAME}.${SAMPLE_UPDATE_ID_VERSION}.importmanifest.json

$IMGTOOL sign --key ${PRIV_KEY}                       \
              --align 4                               \
              --header-size 0x400                     \
              --pad-header                            \
              --slot-size 0x200000                    \
              --max-sectors 800                       \
              --version "$SAMPLE_UPDATE_ID_VERSION"   \
              "$BINFILE"                              \
              "$OUTIMG"

$GEN_MANIFEST -p "${SAMPLE_UPDATE_ID_PROVIDER}" -n "${SAMPLE_UPDATE_ID_NAME}" -v "${SAMPLE_UPDATE_ID_VERSION}"   \
   -c "deviceManufacturer:${SAMPLE_DEVICE_MANUFACTURER}" -c "deviceModel:${SAMPLE_DEVICE_MODEL}"                 \
   -h "microsoft/swupdate:1" -r "installedCriteria:1.0" "$OUTIMG" > "$MANIFEST_FILE"

echo "The binary file for update is: " "$OUTIMG"
echo "The manifest file for update is: " "$MANIFEST_FILE"

