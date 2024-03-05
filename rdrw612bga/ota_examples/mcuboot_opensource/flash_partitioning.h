/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

#define BOOT_FLASH_BASE 0x08000000

/* Define MONOLITHIC_APP only if MCUBOOT is to handle images wrapping the 
 * 3 core binaries. It must be so for MATTER applications requiring OTA
  * MONOLITHIC_APP must be defined at project level. If deined herein must include
  * flash_partitioning.h in sblconfig.h
 */

/*
The memory is allocated as follows:
    - BOOTLOADER: 0x020000 bytes @ 0x08000000
    - APP_ACT:    0x1F0000 bytes @ 0x08020000
    - APP_CAND:   0x1F0000 bytes @ 0x08210000
    - WIFI_ACT:   0x0A0000 bytes @ 0x08400000
    - WIFI_CAND:  0x0A0000 bytes @ 0x084A0000
*/


#if !(defined MONOLITHIC_APP && (MONOLITHIC_APP != 0))
#define BOOT_FLASH_ACT_APP  0x08020000

#define BOOT_FLASH_CAND_APP 0x08210000

#define BOOT_FLASH_ACT_WIFI  0x08400000
#define BOOT_FLASH_CAND_WIFI 0x084A0000

#define BOOT_FLASH_ACT_BLE  0x08540000
#define BOOT_FLASH_CAND_BLE 0x08590000

#define BOOT_FLASH_ACT_154   0x085E0000
#define BOOT_FLASH_CAND_154  0x08630000
#else

/* Must be aligned with FW_ACTIVE_APP_START and FW_UPDATE_STORAGE_START
 * of application's linker script
 */

#define CONFIG_MCUBOOT_MAX_IMG_SECTORS (((BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP)/4096u))
#define BOOT_FLASH_ACT_APP  0x08020000
#define BOOT_FLASH_CAND_APP 0x08460000
#define BOOT_MAX_IMG_NB_SECTORS  CONFIG_MCUBOOT_MAX_IMG_SECTORS

#endif
#endif
