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

#if !defined(CONFIG_ENCRYPT_XIP_EXT_ENABLE)
/* Overwrite-only, swap or direct-xip mode with flash remapping */

#define BOOT_FLASH_ACT_APP              0x60040000
#define BOOT_FLASH_CAND_APP             0x60240000

#elif defined(CONFIG_ENCRYPT_XIP_EXT_OVERWRITE_ONLY)
/* Encrypted XIP extension: modified overwrite-only mode */

#define BOOT_FLASH_ACT_APP              0x60040000
#define BOOT_FLASH_CAND_APP             0x60240000
#define BOOT_FLASH_ENC_META             0x60440000
#define BOOT_FLASH_EXEC_APP             BOOT_FLASH_ACT_APP

#else
/* Encrypted XIP extension: Three slot mode */

#define BOOT_FLASH_EXEC_APP             0x60040000
#define BOOT_FLASH_ACT_APP              0x60240000
#define BOOT_FLASH_CAND_APP             0x60440000
#define BOOT_FLASH_ENC_META             0x60640000

#endif /* !defined(CONFIG_ENCRYPT_XIP_EXT_ENABLE) */

#endif /* _FLASH_PARTITIONING_H_ */

