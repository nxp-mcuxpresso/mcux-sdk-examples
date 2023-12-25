/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

#include "board.h"
#include "fsl_cache.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_FLEXSPI                 FLEXSPI
#define EXAMPLE_CACHE                   CACHE64_CTRL0
#define FLASH_SIZE                      0x10000 /* 512Mb/KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI_AMBA_PC_CACHE_BASE
#define FLASH_PAGE_SIZE                 256
#define EXAMPLE_SECTOR                  2048
#define SECTOR_SIZE                     0x1000 /* 4K */
#define FLASH_PORT                      kFLEXSPI_PortA1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackFromDqsPad

#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD     0
#define NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG     1
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE        2
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR        3
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD   4
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP          5
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE 6
#define NOR_CMD_LUT_SEQ_IDX_READ_NORMAL        7
#define NOR_CMD_LUT_SEQ_IDX_READID             8
#define NOR_CMD_LUT_SEQ_IDX_WRITE              9
#define NOR_CMD_LUT_SEQ_IDX_ENTERQPI           10
#define NOR_CMD_LUT_SEQ_IDX_EXITQPI            11
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG      12
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST          13

#define CUSTOM_LUT_LENGTH 60
/* Enable quad and update dummy cycle */
#define FLASH_QUAD_ENABLE        0xC740
#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0

#define EXAMPLE_DMA        DMA0
#define EXAMPLE_TX_CHANNEL 29
#define EXAMPLE_RX_CHANNEL 28

#define CACHE_MAINTAIN 1
#define EXAMPLE_INVALIDATE_FLEXSPI_CACHE()                                                                          \
    do                                                                                                              \
    {                                                                                                               \
        CACHE64_CTRL0->CCR |= CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK | CACHE64_CTRL_CCR_GO_MASK; \
        while (CACHE64_CTRL0->CCR & CACHE64_CTRL_CCR_GO_MASK)                                                       \
        {                                                                                                           \
        }                                                                                                           \
        CACHE64_CTRL0->CCR &= ~(CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK);                         \
    } while (0)
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
static inline void flexspi_clock_init(void)
{
    /* Use aux0_pll_clk / 2 */
    BOARD_SetFlexspiClock(FLEXSPI, 2U, 2U);
}
/*${prototype:end}*/

#endif /* _APP_H_ */
