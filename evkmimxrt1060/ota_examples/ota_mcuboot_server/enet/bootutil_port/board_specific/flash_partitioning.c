/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "flash_partitioning.h"
#include "flash_map.h"
#include "mcuboot_config.h"
#include "sysflash/sysflash.h"

const char *boot_image_names[MCUBOOT_IMAGE_NUMBER] = {"APP"};

struct flash_area boot_flash_map[MCUBOOT_IMAGE_SLOT_NUMBER] = {
    /* Image 0; slot 0 - Main Application Primary Slot  */
    {.fa_id        = 0,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_ACT_APP - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP,
     .fa_name      = "APP_PRIMARY"},

    /* Image 0; slot 1 - Main Application Secondary Slot  */
    {.fa_id        = 1,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_CAND_APP - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP,
     .fa_name      = "APP_SECONDARY"}};

#ifdef CONFIG_MCUBOOT_ENCRYPTED_XIP_SUPPORT

struct flash_area boot_enc_flash_map[1] = {
    /* Image 0; slot virtual - Main Application Encrypted Slot */
    {.fa_id        = 0,
     .fa_device_id = FLASH_DEVICE_ID,
     .fa_off       = BOOT_FLASH_ENC_APP - BOOT_FLASH_BASE,
     .fa_size      = BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP,
     .fa_name      = "APP_ENCRYPTED"}};

#endif