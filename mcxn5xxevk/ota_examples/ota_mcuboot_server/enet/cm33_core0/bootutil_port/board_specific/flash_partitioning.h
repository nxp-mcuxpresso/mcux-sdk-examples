/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

/* 2MB Internal Flash (0x0000_0000 - 0x001F_FFFF) */
/*
0x0000_0000  +------------------------+ Flash Start
             | MCU Boot               | 128 kB
0x0002_0000  +------------------------+
             | Application            | 896 kB
0x0010_0000  +------------------------+
             | Candidate application  | 896 kB
0x001E_0000  +------------------------+
             | Unused                 | 128 kB
0x0020_0000  +------------------------+ Flash End

NOTES:

 - Core1 image is part of the application image if applicable.

 - The rest of flash from 0x001E_0000 can be used for purposes like NVM data or it can be
   included in the OTA memory layout if additional 64 kB is needed.

*/

#define BOOT_FLASH_BASE     0x00000000
#define BOOT_FLASH_ACT_APP  0x00020000
#define BOOT_FLASH_CAND_APP 0x00100000

#endif
