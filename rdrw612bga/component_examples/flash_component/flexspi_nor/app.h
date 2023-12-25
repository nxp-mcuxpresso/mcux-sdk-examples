/*
 * Copyright 2019-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_FLEXSPI           FLEXSPI
#define FLASH_SIZE                0x10000 /* 512Mb/KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE FlexSPI_AMBA_PC_CACHE_BASE
#define FLASH_PAGE_SIZE           256
#define NOR_FLASH_START_ADDRESS   (2048U * 0x1000U)
#define XIP_EXTERNAL_FLASH        (1) /* Set this macro to skip chip erase. */

#define CACHE_MAINTAIN 0x01U
#define EXAMPLE_CACHE64    (CACHE64_CTRL0)
#define EXAMPLE_ENABLE_CACHE_OF_FLASH   CACHE64_EnableCache(EXAMPLE_CACHE64)
#define EXAMPLE_DISABLE_CACHE_OF_FLASH  CACHE64_DisableCache(EXAMPLE_CACHE64) 

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
    flexspi_transfer_t flashXfer;
    status_t status1, status2;
    uint32_t lookupTable[8] = {0U};

    /* Use aux0_pll_clk / 4 */
    BOARD_SetFlexspiClock(FLEXSPI, 2U, 4U);

    /* Need to reset it in case flash already configured. */
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
/*${prototype:end}*/

#endif /* _APP_H_ */
