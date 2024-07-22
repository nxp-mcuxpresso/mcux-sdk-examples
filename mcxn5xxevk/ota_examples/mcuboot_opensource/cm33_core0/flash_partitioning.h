/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

/*
 Bootloader located in Bank 1 IFR 0 Region (0x0100_8000 - 0x0100_FFFF)

0x0000_0000  +------------------------+ Flash Start
             | Application_Primary    | 1024 kB
0x0010_0000  +------------------------+
             | Application_Secondary  | 1024 kB
0x0020_0000  +------------------------+
                        ...
0x0100_8000  +------------------------+
             | Bootloader             | 32 kB
0x0100_FFFF  +------------------------+
*/

#define BOOT_FLASH_BASE     0x00000000
#define BOOT_FLASH_ACT_APP  0x00000000
#define BOOT_FLASH_CAND_APP 0x00100000

#endif
