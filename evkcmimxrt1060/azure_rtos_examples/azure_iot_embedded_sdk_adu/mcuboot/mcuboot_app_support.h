/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCUBOOT_APP_SUPPORT_H__
#define __MCUBOOT_APP_SUPPORT_H__

#include "fsl_common.h"
#include "flash_partitioning.h"
#include "sysflash/sysflash.h"
#include "flash_map.h"

#define IMAGE_MAGIC       0x96f3b83d
#define IMAGE_HEADER_SIZE 32

struct image_version
{
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
};

/** Image header.  All fields are in little endian byte order. */
struct image_header
{
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;         /* Size of image header (bytes). */
    uint16_t ih_protect_tlv_size; /* Size of protected TLV area (bytes). */
    uint32_t ih_img_size;         /* Does not include header. */
    uint32_t ih_flags;            /* IMAGE_F_[...]. */
    struct image_version ih_ver;
    uint32_t _pad1;
};

#define IMAGE_TLV_INFO_MAGIC      0x6907
#define IMAGE_TLV_PROT_INFO_MAGIC 0x6908

/** Image TLV header.  All fields in little endian. */
struct image_tlv_info
{
    uint16_t it_magic;
    uint16_t it_tlv_tot; /* size of TLV area (including tlv_info header) */
};


/* Bootloader helper API */
enum
{
    kSwapType_None,         // Default value when there is no upgradable image
    kSwapType_ReadyForTest, // The application needs to switch to this state when finishing the update operation
    kSwapType_Testing,      // The bootloader needs to switch to this state before running the test image
    kSwapType_Permanent,    // The application needs to switch to this state when the self-test is okay
    kSwapType_Fail,
    kSwapType_Fatal,
    kSwapType_Max,
};

typedef struct
{
    uint32_t start;
    uint32_t size;
} partition_t;

typedef int (*bl_hashfunc_t) (uint32_t offset, size_t size, uint8_t sha256[32]);

int32_t bl_verify_image(uint32_t addrphy, uint32_t size);
status_t bl_get_update_partition_info(uint32_t image, partition_t *ptn);
status_t bl_update_image_state(uint32_t image, uint32_t state);
status_t bl_get_image_state(uint32_t image, uint32_t *state);
const char *bl_imgstate_to_str(uint32_t state);
void bl_print_image_info(bl_hashfunc_t hashfunc);
int bl_flash_remap_active(void);
int32_t bl_flash_read(uint32_t addr, uint32_t *buffer, uint32_t len);

#endif
