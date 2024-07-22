/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BOOT_H__
#define __BOOT_H__

#include <stdint.h>
#include <stdbool.h>

#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "flash_map.h"

#define MAJOR_VERSION  2
#define MINOR_VERSION  0
#define REVISE_VERSION 0

#define VERSION_SPACER     "."
#define STR2(VERSION)      #VERSION
#define STR(VERSION)       STR2(VERSION)
#define BOOTLOADER_VERSION STR(MAJOR_VERSION) VERSION_SPACER STR(MINOR_VERSION) VERSION_SPACER STR(REVISE_VERSION)

int sbl_boot_main(void);

#endif
