/*
 * Copyright 2023-2024 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*${header:start}*/
#include "fsl_cache.h"
/*${header:end}*/
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_FLEXSPI                 FLEXSPI
#define FLASH_SIZE                      0x4000 /* 16 KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI_AMBA_BASE
#define FLASH_PAGE_SIZE                 256
#define EXAMPLE_SECTOR                  50
#define SECTOR_SIZE                     0x1000 /* 4K */
#define EXAMPLE_FLEXSPI_CLOCK           kCLOCK_Root_Flexspi1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkLoopbackInternally

#define NOR_CMD_LUT_SEQ_IDX_READ             0
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG    1
#define NOR_CMD_LUT_SEQ_IDX_READID           2
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_OPI  3
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR      4
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD 5
#define NOR_CMD_LUT_SEQ_IDX_ENABLEQUAD       6
#define NOR_CMD_LUT_SEQ_IDX_WRITE            7
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE      8
#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD   9
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP        10

#define CUSTOM_LUT_LENGTH        60
#define MT25Q_FLASH_QUAD_ENABLE  1
#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0
#define CACHE_MAINTAIN           1

/*
 * If cache is enabled, this example should maintain the cache to make sure
 * CPU core accesses the memory, not cache only.
 */
/* DMA related. */
#define EXAMPLE_FLEXSPI_DMA (DMA4)

#define FLEXSPI_TX_DMA_REQUEST_SOURCE kDma4RequestMuxFlexSPI1Tx
#define FLEXSPI_RX_DMA_REQUEST_SOURCE kDma4RequestMuxFlexSPI1Rx

#define FLEXSPI_TX_DMA_CHANNEL 1U
#define FLEXSPI_RX_DMA_CHANNEL 2U

/*${macro:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/

/*${variable:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
static inline void flexspi_clock_init(void)
{
    /*Clock setting for flexspi1*/
    const clock_root_config_t flexspiClkCfg = {.clockOff = false,
                                               .mux      = 0, // 24MHz oscillator source
                                               .div      = 2};
    CLOCK_SetRootClock(kCLOCK_Root_Flexspi1, &flexspiClkCfg);
}
/*${prototype:end}*/

#endif /* _APP_H_ */
