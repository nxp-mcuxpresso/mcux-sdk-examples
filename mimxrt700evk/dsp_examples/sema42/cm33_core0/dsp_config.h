/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _DSP_CONFIG_H_
#define _DSP_CONFIG_H_

#if defined(MIMXRT798S_cm33_core0_SERIES)
/* Cortex-M33 core0 copies to HiFi4 */
#define DSP_LITERAL_ADDRESS (uint32_t *)0x24000000
#define DSP_BOOT_ADDRESS    (uint32_t *)0x24020000
#define DSP_SRAM_ADDRESS    (uint32_t *)0x20400000
#else
/* Cortex-M33 core1 copies to HiFi1 */
#define DSP_BOOT_ADDRESS (uint32_t *)0x580000
#define DSP_TEXT_ADDRESS (uint32_t *)0x680000
#define DSP_DATA_ADDRESS (uint32_t *)0x20700000
#endif

#endif
