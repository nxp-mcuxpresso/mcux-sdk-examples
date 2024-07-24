/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__

#define SOC_LPC55S69_SERIES

/* Flash device parameters */

/* 128kB mcuboot + 192kB AppImage + 192kB AppImageNew */
#define COMPONENT_FLASHIAP_SIZE 524288

/* CONFIG_MCUBOOT_MAX_IMG_SECTORS >= (AppImageSize / SectorSize) */
#define CONFIG_MCUBOOT_MAX_IMG_SECTORS 400

/*
 * LPC55S69 with ECC Flash limits use of revert strategies (move/swap).
 * At least with current MCUBoot implementation.
 */
#define MCUBOOT_OVERWRITE_ONLY

/* Crypto */

#define COMPONENT_MCUBOOT_SECURE
#define CONFIG_BOOT_SIGNATURE
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN 2048
#define COMPONENT_MBEDTLS
#define CONFIG_BOOT_BOOTSTRAP

#endif
