/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__



/* Flash device parameters */
#if !(defined MONOLITHIC_APP && (MONOLITHIC_APP != 0))
#define CONFIG_MCUBOOT_MAX_IMG_SECTORS 800
#else
#include "flash_partitioning.h"
#endif

#if !(defined MONOLITHIC_APP && (MONOLITHIC_APP != 0))
/* Number of image pairs (CPU3 A/B + CPU1 A/B)
 * Translates to MCUBOOT_IMAGE_NUMBER
 */
#define CONFIG_UPDATEABLE_IMAGE_NUMBER 2
#else
/*
 * Number of image pairs is 1 in the case of the monolithic application.
 * This is mandated by the MATTER specification.
 */
#define CONFIG_UPDATEABLE_IMAGE_NUMBER 1
#endif

/* Crypto */
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
