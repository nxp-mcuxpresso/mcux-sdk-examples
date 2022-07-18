/*
 * Copyright 2022 NXP
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
#define EXAMPLE_FLEXSPI           FLEXSPI
#define FLASH_SIZE                0x2000 /* 64Mb/KByte */
#define FLASH_PAGE_SIZE           256
#define EXAMPLE_FLEXSPI_AMBA_BASE FlexSPI_AMBA_BASE
#define NOR_FLASH_START_ADDRESS   (20U * 0x1000U)
#define EXAMPLE_FLEXSPI_CLOCK     kCLOCK_FlexSpi
#define CACHE_MAINTAIN            0x01U

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
#include "fsl_cache.h"
#endif

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);

static inline void FLEXSPI_ClockInit(void)
{
#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
    /* Switch to PLL2 for XIP to avoid hardfault during re-initialize clock. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);    /* Set PLL2 PFD2 clock 396MHZ. */
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0x2); /* Choose PLL2 PFD2 clock as flexspi source clock. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 2);   /* flexspi clock 133M. */
#else
    const clock_usb_pll_config_t g_ccmConfigUsbPll = {.loopDivider = 0U};

    CLOCK_InitUsb1Pll(&g_ccmConfigUsbPll);
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 24);   /* Set PLL3 PFD0 clock 360MHZ. */
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0x3); /* Choose PLL3 PFD0 clock as flexspi source clock. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 2);   /* flexspi clock 120M. */
#endif
}
/*${prototype:end}*/

#endif /* _APP_H_ */
