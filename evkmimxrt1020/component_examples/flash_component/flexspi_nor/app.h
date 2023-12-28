/*
 * Copyright 2019, 2023 NXP
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
#include "fsl_flexspi.h"
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);

static inline void FLEXSPI_ClockInit(void)
{
#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
    flexspi_transfer_t flashXfer;
    status_t status1, status2;
    uint32_t lookupTable[8] = {0U};

    if ((FLEXSPI->MCR0 & FLEXSPI_MCR0_MDIS_MASK) != FLEXSPI_MCR0_MDIS_MASK)
    {
        /* In the BOOT phase, Flash's read dummy cycles changed to 8, current implementation of the FLASH component is
        unable to get these updates (in SFDP, the default read dummy cycles are 4). To avoid a mismatch between the
        host and external flash, reset the external flash to its default settings. */
        /* Set reset enable and reset commands for LUT. */
        lookupTable[0] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x66, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00);
        lookupTable[4] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x99, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0x00);
        /* Update LUT table. */
        FLEXSPI_UpdateLUT(FLEXSPI, 56U, lookupTable, 8);

        flashXfer.deviceAddress = 0U;
        flashXfer.port          = kFLEXSPI_PortA1;
        flashXfer.cmdType       = kFLEXSPI_Command;
        flashXfer.SeqNumber     = 1;
        flashXfer.seqIndex      = 14;
        status1 = FLEXSPI_TransferBlocking(FLEXSPI, &flashXfer);
        flashXfer.seqIndex      = 15;
        status2 = FLEXSPI_TransferBlocking(FLEXSPI, &flashXfer);
        if ((status1 != kStatus_Success) || (status2 != kStatus_Success))
        {
            assert(false);
        }
    }
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
