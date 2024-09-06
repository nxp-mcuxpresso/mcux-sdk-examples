/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__

/* MCUBoot Flash Config */

#define CONFIG_MCUBOOT_MAX_IMG_SECTORS 800

#define CONFIG_UPDATEABLE_IMAGE_NUMBER 1

/* MCUBoot upgrade mode */

/*
 * The default MCUBoot configuration is to use swap mechanism. In case the flash
 * remapping functionality is supported by processor the alternative mechanism
 * using direct-xip mode can be used and evaluated by user.
 * Comment this to enable swap mode.
 */
#define CONFIG_MCUBOOT_FLASH_REMAP_ENABLE

/* Board specific register for flash remap functionality */
#define FLASH_REMAP_OFFSET_REG 0x400AC080 /* RT1060 flash remap offset register */

/* Encrypted XIP support config */

/*
 * Enable extension utilizing on-the-fly decryption of encrypted image.
 * For more information please see readme file.
 */
//#define CONFIG_ENCRYPT_XIP_EXT_ENABLE

/*
 * Encrypted XIP extension uses simpler OVERWRITE_ONLY mode instead of three
 * slot configuration.
 */
//#define CONFIG_ENCRYPT_XIP_EXT_OVERWRITE_ONLY

/* Crypto Config */

#define COMPONENT_MCUBOOT_SECURE
#define CONFIG_BOOT_SIGNATURE
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN 2048
#define COMPONENT_MBEDTLS
#define CONFIG_BOOT_BOOTSTRAP

#endif
