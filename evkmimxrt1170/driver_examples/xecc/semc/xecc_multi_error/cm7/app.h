/*
 * Copyright 2020 NXP
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
/* XECC */
#define EXAMPLE_XECC                  XECC_SEMC
#define EXAMPLE_XECC_IRQ              XECC_SEMC_FATAL_INT_IRQn
#define EXAMPLE_XECC_IRQ_HANDLER      XECC_SEMC_FATAL_INT_IRQHandler
#define EXAMPLE_XECC_BUSFAULT_HANDLER BusFault_Handler
#define EXAMPLE_XECC_AREA_SIZE        0x1000U

/* SEMC */
#define EXAMPLE_SEMC               SEMC
#define EXAMPLE_SEMC_START_ADDRESS (0x80000000U)
#define EXAMPLE_SEMC_CLK_FREQ      CLOCK_GetRootClockFreq(kCLOCK_Root_Semc)

#define CACHE_MAINTAIN 0x01U

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
#include "fsl_cache.h"
#endif
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
