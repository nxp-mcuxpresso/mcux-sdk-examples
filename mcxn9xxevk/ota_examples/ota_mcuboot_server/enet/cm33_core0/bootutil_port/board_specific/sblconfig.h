/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__

/* HW Flash Swapping feature is used */

#define CONFIG_MCUBOOT_FLASH_REMAP_ENABLE
#define CONFIG_MCUBOOT_FLASH_REMAP_BY_SWAP

#define FLASH_REMAP_OFFSET_REG (&(NPX0->REMAP))


/* MCUBoot Flash Config */

/* MCX N10 series has ECC Flash with minimum 16 byte write */

#define MCUBOOT_BOOT_MAX_ALIGN 16

/* Slot size being 896 kB divided by 8 kB sector size gives 112 sectors as minimum value */
/* 256 * minimum write alignment 16 B gives 4 kB swap status size */

#define CONFIG_MCUBOOT_MAX_IMG_SECTORS 256

/* Crypto Config */

#define COMPONENT_MCUBOOT_SECURE
#define CONFIG_BOOT_SIGNATURE
#define CONFIG_BOOT_SIGNATURE_TYPE_ECDSA_P256
#define CONFIG_BOOT_BOOTSTRAP
#define MCUBOOT_USE_TINYCRYPT
#endif
