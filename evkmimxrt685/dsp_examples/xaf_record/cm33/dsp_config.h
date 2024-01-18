/*
 * Copyright 2019-2023 NXP
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
#define DSP_NCACHE_ADDRESS  (uint32_t *)0x20060000

/* Inter processor communication common RAM */
#define RPMSG_LITE_LINK_ID    (RL_PLATFORM_IMXRT600_LINK_ID)
#define RPMSG_LITE_SHMEM_BASE (void *)0x20070000
#define RPMSG_LITE_SHMEM_SIZE (64 * 1024)

/* DSP-private uncached audio memory buffers */
/* Each buffer is 32k in size in a separate SRAM partition */
#define DSP_AUDIO_BUFFER_1_PING (0x20000000)
#define DSP_AUDIO_BUFFER_1_PONG (0x20008000)
#define DSP_AUDIO_BUFFER_2_PING (0x20010000)
#define DSP_AUDIO_BUFFER_2_PONG (0x20018000)

/* Audio ping/pong buffers for streaming DSP handling.
 * Assign each buffer to distinct shared RAM partition to maximize performance. */
#define AUDIO_SHARED_BUFFER_1      0x20020000
#define AUDIO_SHARED_BUFFER_1_SIZE (128 * 1024)
#define AUDIO_SHARED_BUFFER_2      0x20040000
#define AUDIO_SHARED_BUFFER_2_SIZE (128 * 1024)

#define AUDIO_I2S_CAPTURER_DEVICE 4
#define AUDIO_I2S_RENDERER_DEVICE 1

#endif
