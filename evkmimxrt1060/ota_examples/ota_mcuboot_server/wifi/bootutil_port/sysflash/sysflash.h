/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SYSFLASH_H__
#define __SYSFLASH_H__

#include <string.h>
#include "fsl_common.h"
#include "mcuboot_config.h"

/* Currently only these two strategies are used:
 * - MCUBOOT_SWAP_USING_MOVE
 * - MCUBOOT_OVERWRITE_ONLY
 *
 * They both require two slots (primary & secondary) for each image.
 */

#define FLASH_AREA_IMAGE_PRIMARY(x)   (((x) < MCUBOOT_IMAGE_NUMBER) ? ((x)*2) : 255)
#define FLASH_AREA_IMAGE_SECONDARY(x) (((x) < MCUBOOT_IMAGE_NUMBER) ? ((x)*2 + 1) : 255)

#define MCUBOOT_IMAGE_SLOT_NUMBER (MCUBOOT_IMAGE_NUMBER * 2)

/* So far only a single storage device supported */
#define FLASH_DEVICE_ID 1

int flash_area_id_from_multi_image_slot(int image_index, int slot);

#endif /* __SYSFLASH_H__ */
