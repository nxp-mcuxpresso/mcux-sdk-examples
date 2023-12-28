/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _DSP_CONFIG_H_
#define _DSP_CONFIG_H_

#define DSP_EPT_ADDR (30)
#define MCU_EPT_ADDR (40)

/* Address of RAM, where the image for dsp should be copied */
#define DSP_LITERAL_ADDRESS (uint32_t *)0x24000000
#define DSP_BOOT_ADDRESS    (uint32_t *)0x24020000
#define DSP_SRAM_ADDRESS    (uint32_t *)0x00200000
#define DSP_NCACHE_ADDRESS  (uint32_t *)0x20040000

#endif
