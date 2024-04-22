/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/

/* Note: region is [bot:top), the end is open interval. So the bit[2:0] of the end address must be zero. */
#define StartAddr 0x20500000
#define EndAddr   0x2050FFFF

#define S3MU MU_RT_S3MUA

#define ELE_GETINFO_TRNG_STATE_OFFSET   156u
#define ELE_TRNG_STATE_READY            0x3u
   
#define USE_FLASH 0
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
