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

#define FLASH_AREA_IMAGE_1_OFFSET (BOOT_FLASH_ACT_APP - BOOT_FLASH_BASE)
#define FLASH_AREA_IMAGE_1_SIZE   (BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP)
#define FLASH_AREA_IMAGE_2_OFFSET (FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE)
#define FLASH_AREA_IMAGE_2_SIZE   FLASH_AREA_IMAGE_1_SIZE // image2 slot is the same size as image1
#define FLASH_AREA_IMAGE_3_OFFSET (FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE)

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

#define BOOT_MAX_ALIGN 8
#define BOOT_FLAG_SET  1

struct image_trailer
{
    uint8_t swap_type;
    uint8_t pad1[BOOT_MAX_ALIGN - 1];
    uint8_t copy_done;
    uint8_t pad2[BOOT_MAX_ALIGN - 1];
    uint8_t image_ok;
    uint8_t pad3[BOOT_MAX_ALIGN - 1];
    uint8_t magic[16];
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

extern int32_t bl_verify_image(uint32_t addrphy, uint32_t size);

extern status_t bl_get_update_partition_info(uint32_t image, partition_t *ptn);
extern status_t bl_update_image_state(uint32_t image, uint32_t state);
extern status_t bl_get_image_state(uint32_t image, uint32_t *state);
const char *bl_imgstate_to_str(uint32_t state);

#endif
