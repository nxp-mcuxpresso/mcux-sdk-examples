/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__

#define SOC_LPC55S69_SERIES
#define ARCH_ARM_CORTEX_M33
#define ARCH_ARM_CORTEX_FPU
#define SOC_LPC_SERIES

#define BOARD_BOOTCLOCKRUN_CORE_CLOCK BOARD_BOOTCLOCKFROHF96M_CORE_CLOCK

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

/* Flash IAP */

#define COMPONENT_FLASHIAP
#define COMPONENT_FLASHIAP_ROM

/* Crypto */

#define COMPONENT_MCUBOOT_SECURE
#define CONFIG_BOOT_SIGNATURE
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA
#define CONFIG_BOOT_SIGNATURE_TYPE_RSA_LEN 2048
#define COMPONENT_MBEDTLS
#define CONFIG_BOOT_BOOTSTRAP

/* Serial Manager */

#define COMPONENT_SERIAL_MANAGER
#define COMPONENT_SERIAL_MANAGER_LPUART
#define SERIAL_PORT_TYPE_UART 1

/* Platform Drivers Config */

#define BOARD_FLASH_SUPPORT
#define SOC_CPU_LPC55S69JBD100_cm33_core0

/* On-chip Peripheral Drivers */

#define SOC_GPIO
#define SOC_LPUART
#define SOC_LPUART_2
#define SOC_INPUTMUX
#define SOC_PINT
#define SOC_GINT

/* Board extended module Drivers */

#endif
