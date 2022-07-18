/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _DSP_CONFIG_H_
#define _DSP_CONFIG_H_

/* Address of RAM, where the image for dsp should be copied */
#define DSP_BOOT_ADDRESS   (uint32_t *)0x24000000
#define DSP_SRAM_ADDRESS   (uint32_t *)0x00200000
#define DSP_NCACHE_ADDRESS (uint32_t *)0x20040000

#endif
