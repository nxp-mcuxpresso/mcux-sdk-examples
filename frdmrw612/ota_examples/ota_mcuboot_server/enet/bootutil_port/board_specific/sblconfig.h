/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__


/* MCUBoot Flash Config */

#define CONFIG_MCUBOOT_MAX_IMG_SECTORS 1088u

/*
 * Number of image pairs is 1 in the case of the monolithic application.
 * This is mandated by the MATTER specification.
 */
#define CONFIG_UPDATEABLE_IMAGE_NUMBER 1

/*
 * MCUBoot upgrade mode
 *
 * The default MCUBoot configuration is to use swap mechanism. In case the flash
 * remapping functionality is supported by processor the alternative mechanism
 * using direct-xip mode can be used and evaluated by user.
 */
#define CONFIG_MCUBOOT_FLASH_REMAP_ENABLE

/* Board specific register for flash remap functionality */
#define FLASH_REMAP_START_REG           0x40134420      /* RW61x flash remap start address register */
#define FLASH_REMAP_END_REG             0x40134424      /* RW61x flash remap end address register */
#define FLASH_REMAP_OFFSET_REG          0x40134428      /* RW61x flash remap offset register */

/* Crypto Config */
// #define CONFIG_BOOT_OTA_TEST
/* uncomment to generate MCU boot for testing without image signature verification */

#ifdef CONFIG_BOOT_OTA_TEST
#define CONFIG_BOOT_NO_SIGNATURE
#endif
#ifndef CONFIG_BOOT_NO_SIGNATURE
#define COMPONENT_MCUBOOT_SECURE
#define CONFIG_BOOT_SIGNATURE
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN 2048
#endif
#define COMPONENT_MBEDTLS
#define CONFIG_BOOT_BOOTSTRAP

#endif
