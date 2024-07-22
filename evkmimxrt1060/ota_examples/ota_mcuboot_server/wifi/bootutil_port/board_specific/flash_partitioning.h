/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "sblconfig.h"
#include "mflash_drv.h"

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

#define BOOT_FLASH_BASE                 0x60000000

#ifndef CONFIG_MCUBOOT_ENCRYPTED_XIP_SUPPORT

/* Swap or direct-xip mode with flash remapping */
#define BOOT_FLASH_ACT_APP              0x60040000
#define BOOT_FLASH_CAND_APP             0x60240000

#else 

/* Three slot mode with encrypted XIP */
#define BOOT_FLASH_ENC_DEVICE_ID        FLASH_DEVICE_ID
#define BOOT_FLASH_ENC_APP              0x60040000
#define BOOT_FLASH_ACT_APP              0x60240000
#define BOOT_FLASH_CAND_APP             0x60440000

#endif /*CONFIG_MCUBOOT_ENCRYPTED_XIP_SUPPORT*/

#endif /* _FLASH_PARTITIONING_H_ */

