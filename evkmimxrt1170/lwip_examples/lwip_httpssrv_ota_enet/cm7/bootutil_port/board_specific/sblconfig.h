/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SBL_CONFIG_H__
#define SBL_CONFIG_H__

/* MCU-SBL RT1170 Configuration */

#define SOC_IMXRT1170_SERIES
#define ARCH_ARM_CORTEX_M7
#define ARCH_ARM_CORTEX_FPU
#define SOC_IMXRTYYYY_SERIES

/* MCU SBL core */

#define CONFIG_MCUBOOT_MAX_IMG_SECTORS 800

/*
 * MCUBoot upgrade mode
 *
 * The default MCUBoot configuration is to use swap mechanism. In case the flash
 * remapping functionality is supported by processor the alternative mechanism
 * using direct-xip mode can be used and evaluated by user.
 */
#define CONFIG_MCUBOOT_FLASH_REMAP_ENABLE 1

/*
 * Downgrade is not supported in MCUBoot using direct-xip mode because the active
 * image with highest version is always chosen. This define enables code in
 * bootloader and OTA application to support downgrade.
 */
#define CONFIG_MCUBOOT_FLASH_REMAP_DOWNGRADE_SUPPORT 1

/* Board specific register for flash remap functionality */
#define FLASH_REMAP_OFFSET_REG 0x400CC428 /* RT1170 flash remap offset register */

/* MCU SBL Flash Map */

/* MCU SBL metadata header */

/* MCU SBL Component */

/* Flash IAP */

#define COMPONENT_FLASHIAP
#define COMPONENT_FLASHIAP_ROM

/* Flash device parameters */

#define COMPONENT_FLASHIAP_SIZE 16777216

/* secure */

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

/* mcu isp support */

/* Platform Drivers Config */

#define BOARD_FLASH_SUPPORT
#define ISSI_IS25WPxxxA
#define SOC_MIMXRT1176DVMAA

/* On-chip Peripheral Drivers */

#define SOC_GPIO
#define SOC_LPUART
#define SOC_LPUART_1
#define SOC_FLEXSPI
#define SOC_FLEXSPI_1

/* Onboard Peripheral Drivers */

/* Board extended module Drivers */

#endif
