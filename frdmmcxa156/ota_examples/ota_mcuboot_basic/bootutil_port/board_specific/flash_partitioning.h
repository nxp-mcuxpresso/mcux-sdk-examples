/*
 * Copyright 2021 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

/*
  MCUBoot region       0x0     - 0x08000  : 32kB
  Primary slot         0x8000  - 0x84000  : 496kB (0x7c000 bytes)
  Secondary slot       0x84000 - 0x100000 : 496kB (0x7c000 bytes)  
*/

#define BOOT_FLASH_BASE     0x00000000
#define BOOT_FLASH_ACT_APP  0x00008000
#define BOOT_FLASH_CAND_APP 0x00084000

#endif
