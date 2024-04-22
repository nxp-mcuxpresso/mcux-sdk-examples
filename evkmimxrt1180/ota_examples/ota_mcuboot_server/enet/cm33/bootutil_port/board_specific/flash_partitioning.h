/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

/*
 * 256 kB for bootloader
 *   2 MB for application
 */

#define BOOT_FLASH_BASE     0x28000000
#define BOOT_FLASH_ACT_APP  0x28040000
#define BOOT_FLASH_CAND_APP 0x28240000

#endif
