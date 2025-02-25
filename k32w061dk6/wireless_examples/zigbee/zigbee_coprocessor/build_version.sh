#!/bin/sh
#
# Copyright 2023 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# check if environmental variable NXP_SDK_ROOT is set
#     if not, then use "../../../" as a possible path
# then  check if NXP_SDK_ROOT contains a valid SDK
#     if SW-Content-Register.txt is found
#         then NXP_SDK_ROOT points to a released SDK
#     else if .gitignore is found
#         then NXP_SDK_ROOT points to a cloned repository

if [ -z "$NXP_SDK_ROOT" ]; then
    NXP_SDK_ROOT="../../../../../../../"
fi

if [ -z "$VERSION_FILE" ]; then
    VERSION_FILE="../../src/version.h"
fi

if [ -f $NXP_SDK_ROOT/.gitmodules -o -f $NXP_SDK_ROOT/west.yml ]; then
    SDK_RELEASE=0
# OT NXP check
elif [ -f $NXP_SDK_ROOT/SW-Content-Register.txt ]; then
    SDK_RELEASE=1
# MCUX check
elif [ -f ../.cproject ]; then
    SDK_RELEASE=2
# SDK3.0 check
elif [ ! -z "$1" ]; then
    SDK_RELEASE=3
else
    echo "Could not found a valid SDK package!"
    exit 1
fi

if [ "$SDK_RELEASE" = "1" ]; then
    file=$NXP_SDK_ROOT/SW-Content-Register.txt;
    #Get SDK version number
    sdk_version=`grep "Release Version" $file | cut -d ":" -f 2 | sed 's/[-\.a-zA-Z ]//g'`;

    #Get Zigbee version number
    start=`grep -n "Zigbee" $file | cut -d: -f 1`;
    zigbee_version=`sed -n ''"$start"','"$(($start+5))"'p' $file | grep "Version" | cut -d ":" -f 2 | sed 's/[\. ]//g'`;

    if [ "$zigbee_version" = "NA" ]; then
        zigbee_version=0;
    fi
elif [ "$SDK_RELEASE" = "2" ]; then 
    file="../.cproject"
    VERSION_FILE="../source/version.h"
            
    #Get SDK version number; use sed options that are available both on FreeBSD sed & GNU sed
    sdk_version=`grep "<sdkVersion>" $file | sed -e 's/<sdkVersion>//;s/<\/sdkVersion>//;s/[\. ]//g'`;
    
    #Get Zigbee version number
    zigbee_version=0

elif [ "$SDK_RELEASE" = "3" ]; then
    file=$1/MCUX_VERSION;

    #Version file will be created in build directory
    VERSION_FILE="version.h"

    #Get SDK version number
    sdk_version=`grep VERSION_MAJOR $file | sed -e 's/VERSION_MAJOR//g;s/\ =\ //g'`;
    sdk_version+=`grep VERSION_MINOR $file | sed -e 's/VERSION_MINOR//g;s/\ =\ //g'`;

    #For now there is no zigbee version
    zigbee_version=0;
else
    #Get SDK version number
    sdk_version=0;

    SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
    #Get Zigbee commit SHA
    zigbee_version=`cd ${SCRIPT_DIR}; git log --oneline | head -n 1 | cut -d " " -f 1 | cut -b 1-8`;
fi


if [ -f $VERSION_FILE ]; then
    rm $VERSION_FILE
fi
touch $VERSION_FILE
echo "#define ZIGBEE_VERSION 0x"$zigbee_version >> $VERSION_FILE
echo "#define SDK_VERSION "$sdk_version >> $VERSION_FILE
echo $zigbee_version
