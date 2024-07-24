/*
 * Copyright 2021,2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

/*
The memory is allocated as follows:
    - BOOTLOADER:  0x020000 bytes @ 0x08000000
    - APP_ACT:     0x440000 bytes @ 0x08020000
    - APP_CAND:    0x440000 bytes @ 0x08460000
Note: Application and wifi binary data are embedded into one monolithic output.
See rw61x_wifi_bin.c
*/
#define BOOT_FLASH_BASE     0x08000000
#define BOOT_FLASH_ACT_APP  0x08020000
#define BOOT_FLASH_CAND_APP 0x08460000

#endif
