/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PICTURES_H_
#define _PICTURES_H_

#define DEMO_PIC_NUM        3U   /* Number of pictures. */
#define DEMO_PIC_WIDTH      800U /* Width of the pictures. */
#define DEMO_PIC_HEIGHT     480U /* Height of the pictures. */
#define DEMO_BYTE_PER_PIXEL 2    /* Bytes per pixel. */

#ifndef BUILD_PIC_BIN
#define BUILD_PIC_BIN 0
#endif

/* The pictures. */
#if BUILD_PIC_BIN
extern const uint16_t demoPictures[DEMO_PIC_NUM][DEMO_PIC_HEIGHT * DEMO_PIC_WIDTH];
#else
#define DEMO_PIC_ADDR 0x68000400
extern const uint8_t (*demoPictures)[DEMO_PIC_HEIGHT * DEMO_PIC_WIDTH * DEMO_BYTE_PER_PIXEL];
const uint8_t (*demoPictures)[DEMO_PIC_HEIGHT * DEMO_PIC_WIDTH * DEMO_BYTE_PER_PIXEL] = (void *)DEMO_PIC_ADDR;
#endif

#endif /* _PICTURES_H_ */
