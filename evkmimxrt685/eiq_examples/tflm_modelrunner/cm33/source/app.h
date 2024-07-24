/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016,2024 NXP
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

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);

#ifdef __cplusplus
extern "C" {
#endif

int64_t getTime();
int gethostname(char* name);

#ifdef __cplusplus
}
#endif
/*${prototype:end}*/

#endif /* _APP_H_ */
