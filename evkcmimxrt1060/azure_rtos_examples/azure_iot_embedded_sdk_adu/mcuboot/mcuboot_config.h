/*
 * Copyright (c) 2018 Open Source Foundries Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MCUBOOT_CONFIG_H__
#define __MCUBOOT_CONFIG_H__

#include <sblconfig.h>
#include <sbldef.h>
/*
 * Template configuration file for MCUboot.
 *
 * When porting MCUboot to a new target, copy it somewhere that your
 * include path can find it as mcuboot_config/mcuboot_config.h, and
 * make adjustments to suit your platform.
 *
 * For examples, see:
 *
 * boot/zephyr/include/mcuboot_config/mcuboot_config.h
 * boot/mynewt/mcuboot_config/include/mcuboot_config/mcuboot_config.h
 */

/*
 * Signature types
 *
 * You must choose exactly one signature type.
 */

/* Uncomment to enable RSA signature */
#ifdef CONFIG_BOOT_SIGNATURE_TYPE_RSA
#define MCUBOOT_SIGN_RSA
#if (CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN != 2048 && CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN != 3072)
#error "Invalid RSA key size (must be 2048 or 3072)"
#else
#define MCUBOOT_SIGN_RSA_LEN CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN
#endif
#elif defined(CONFIG_BOOT_SIGNATURE_TYPE_ECDSA_P256)
#define MCUBOOT_SIGN_EC256
#elif defined(CONFIG_BOOT_SIGNATURE_TYPE_ED25519)
#define MCUBOOT_SIGN_ED25519
#endif

/* Uncomment to enable BOOTROM signature */
#ifdef CONFIG_BOOT_SIGNATURE_TYPE_ROM
#define MCUBOOT_SIGN_ROM
#ifdef SOC_LPC55S69_SERIES
#define HAB_IVT_OFFSET 0x0u
#else
#define HAB_IVT_OFFSET 0x1000u
#endif
#endif

/* Uncomment to enable MBED_TLS */
#ifdef COMPONENT_MBEDTLS
#define MCUBOOT_USE_MBED_TLS
#endif

/* Uncomment to enable Hardware Key */
#ifdef CONFIG_BOOT_HW_KEY
#define MCUBOOT_HW_KEY
#endif

/*
 * Upgrade mode
 *
 * The default is to support A/B image swapping with rollback.  A
 * simpler code path, which only supports overwriting the
 * existing image with the update image, is also available.
 *
 * In case of supported flash remap funcionality in the used processor the
 * direct-xip mode is configured with user support for downgrade.
 */

/* Uncomment to enable the overwrite-only code path. */
/* #define MCUBOOT_OVERWRITE_ONLY */

#ifndef MCUBOOT_OVERWRITE_ONLY

#ifdef CONFIG_MCUBOOT_FLASH_REMAP_ENABLE

#define MCUBOOT_DIRECT_XIP
#define MCUBOOT_DIRECT_XIP_REVERT

#ifdef CONFIG_MCUBOOT_FLASH_REMAP_DOWNGRADE_SUPPORT
/* Enable hook funcionality to support downgrade functionality in direct-xip
 * mode, see hooks implementation in bootutil_hooks.c */
#define MCUBOOT_IMAGE_ACCESS_HOOKS
#endif /* CONFIG_MCUBOOT_FLASH_REMAP_DOWNGRADE_SUPPORT */

#else

#define CONFIG_BOOT_SWAP_USING_MOVE
#define MCUBOOT_SWAP_USING_MOVE 1

#endif /* CONFIG_MCUBOOT_FLASH_REMAP_ENABLE */

#endif /* MCUBOOT_OVERWRITE_ONLY */

#ifdef MCUBOOT_OVERWRITE_ONLY
/* Uncomment to only erase and overwrite those primary slot sectors needed
 * to install the new image, rather than the entire image slot. */

/* #define MCUBOOT_OVERWRITE_ONLY_FAST */

#endif

/*
 * Cryptographic settings
 *
 * You must choose between mbedTLS and Tinycrypt as source of
 * cryptographic primitives. Other cryptographic settings are also
 * available.
 */

/* Uncomment to use ARM's mbedTLS cryptographic primitives */
#ifdef COMPONENT_MBEDTLS
#define MCUBOOT_USE_MBED_TLS
/* Uncomment to use Tinycrypt's. */
/* #define MCUBOOT_USE_TINYCRYPT */
#endif

#ifdef COMPONENT_MCUBOOT_SECURE
/*
 * Always check the signature of the image in the primary slot before booting,
 * even if no upgrade was performed. This is recommended if the boot
 * time penalty is acceptable.
 */
#ifdef CONFIG_BOOT_SIGNATURE
#define MCUBOOT_VALIDATE_PRIMARY_SLOT
#endif
#endif

#ifdef CONFIG_UPDATEABLE_IMAGE_NUMBER
#define MCUBOOT_IMAGE_NUMBER CONFIG_UPDATEABLE_IMAGE_NUMBER
#else
#define MCUBOOT_IMAGE_NUMBER 1
#endif

/*
 * Flash abstraction
 */

/* Uncomment if your flash map API supports flash_area_get_sectors().
 * See the flash APIs for more details. */
#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS

/* Default maximum number of flash sectors per image slot; change
 * as desirable. */
#ifdef CONFIG_MCUBOOT_MAX_IMG_SECTORS
#define MCUBOOT_MAX_IMG_SECTORS CONFIG_MCUBOOT_MAX_IMG_SECTORS
#else
#error "CONFIG_MCUBOOT_MAX_IMG_SECTORS is not defined"
#endif

/*
 * Logging
 */

/*
 * If logging is enabled the following functions must be defined by the
 * platform:
 *
 *    MCUBOOT_LOG_MODULE_REGISTER(domain)
 *      Register a new log module and add the current C file to it.
 *
 *    MCUBOOT_LOG_MODULE_DECLARE(domain)
 *      Add the current C file to an existing log module.
 *
 *    MCUBOOT_LOG_ERR(...)
 *    MCUBOOT_LOG_WRN(...)
 *    MCUBOOT_LOG_INF(...)
 *    MCUBOOT_LOG_DBG(...)
 *
 * The function priority is:
 *
 *    MCUBOOT_LOG_ERR > MCUBOOT_LOG_WRN > MCUBOOT_LOG_INF > MCUBOOT_LOG_DBG
 */
#ifndef CONFIG_MCUBOOT_DISABLE_LOGGING
#define MCUBOOT_HAVE_LOGGING
#endif

/*
 * Assertions
 */

/* Uncomment if your platform has its own mcuboot_config/mcuboot_assert.h.
 * If so, it must provide an ASSERT macro for use by bootutil. Otherwise,
 * "assert" is used. */
/* #define MCUBOOT_HAVE_ASSERT_H */

#ifdef CONFIG_BOOT_ENCRYPT_RSA
#define MCUBOOT_ENC_IMAGES
#define MCUBOOT_ENCRYPT_RSA
#endif

#ifdef CONFIG_BOOT_BOOTSTRAP
#define MCUBOOT_BOOTSTRAP 1
#endif

/* Not enabled, no feed activity */
#define MCUBOOT_WATCHDOG_FEED() \
    do                          \
    {                           \
    } while (0)

#endif /* __MCUBOOT_CONFIG_H__ */
