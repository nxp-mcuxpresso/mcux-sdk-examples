/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IAP_H__
#define __IAP_H__

#include <stdint.h>
#include <stdbool.h>

#include "bootutil/image.h"
#include "bootutil/bootutil.h"
#include "flash_map.h"

typedef void (*iapfun)(void);
#define FLASH_IMAGE_START_ADDR 0x60080000
#define FLASH_APP_ADDR         (FLASH_IMAGE_START_ADDR + 0x2000)

#define FLASH_IMAGE_OFFSET   0x180000
#define FLASH_IMAGE_END_ADDR (FLASH_IMAGE_START_ADDR + FLASH_IMAGE_OFFSET)

#define FLASH_IMAGE_INFO_ADDR (FLASH_IMAGE_END_ADDR + FLASH_IMAGE_OFFSET)

#define OTA_GET_VERSION     0x01
#define OTA_UPDATE_REQ      0x02
#define OTA_UPDATE_START    0x03
#define OTA_UPDATE_CONTINUE 0x04
#define OTA_UPDATE_RETRANS  0x05
#define OTA_UPDATE_DATA     0x06
#define OTA_UPDATE_FINISH   0x07
#define OTA_START_LOG       0x08
#define OTA_ENTER_CONSOLE   0x09
#define OTA_CMD_ERASE       0x0A
#define OTA_CMD_UPDATE      0x0B
#define OTA_CMD_ROLLBACK    0x0C
#define OTA_CMD_ERASE_OK    0x0D
#define OTA_CMD_ERASE_ERR   0x0E

#define SIZE_PER_PACKAGE 2048
#define BUFFER_SIZE      2060

#define FLASH_IMAGE_FLAG 0x5A

typedef struct
{
    uint16_t header;
    int16_t index;
    uint8_t cmd;
    uint8_t Reserved[3];
    uint16_t length;
    uint16_t checksum;
} package_header_t;

typedef struct
{
    package_header_t cmd_info;
    char name[16];
} cmd_info_t;

typedef struct
{
    package_header_t package_info;
    uint8_t payload[SIZE_PER_PACKAGE];
} image_package_t;

typedef struct
{
    uint16_t version;
    uint8_t flag;
    uint8_t Reserved[253];
} image_info_t;

void do_boot(struct boot_rsp *rsp);

#endif
