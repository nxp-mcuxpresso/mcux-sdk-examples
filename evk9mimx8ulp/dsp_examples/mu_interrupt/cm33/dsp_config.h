/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _DSP_CONFIG_H_
#define _DSP_CONFIG_H_

/* Address of RAM, where the image for DSP should be copied */
/* These addresses are accessed by the ARM M33 core. */
#define DSP_RESET_ADDRESS (uint32_t *)0x20008000
#define DSP_TEXT_ADDRESS  (uint32_t *)0x20008400
#define DSP_SRAM_ADDRESS  (uint32_t *)0x20020000

#endif
