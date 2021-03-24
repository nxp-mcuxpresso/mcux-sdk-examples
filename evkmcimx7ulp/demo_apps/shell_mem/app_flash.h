/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_FLASH_H_
#define _APP_FLASH_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitFlash(void);
int32_t APP_EraseFlash(uint32_t offset, uint32_t bytes);
int32_t APP_WriteFlash(uint32_t *buf, uint32_t offset, uint32_t bytes);

#endif /* _APP_FLASH_H_ */
