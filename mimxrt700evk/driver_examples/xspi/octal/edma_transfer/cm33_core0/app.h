/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*${header:start}*/
#include "fsl_power.h"
#include "fsl_cache.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_XSPI                    XSPI0
#define FLASH_SIZE                      0x10000 /* 64MB/KByte */
#define EXAMPLE_XSPI_AMBA_BASE          0x28000000U
#define FLASH_PAGE_SIZE                 256
#define EXAMPLE_SECTOR                  200
#define SECTOR_SIZE                     0x1000 /* 4K */
#define EXAMPLE_XSPI_CLOCK              kCLOCK_Xspi0
#define EXAMPLE_XSPI_RX_SAMPLE_CLOCK    kXSPI_ReadSampleClkExternalInputFromDqsPad


#define NOR_CMD_LUT_SEQ_IDX_READ                   0
#define NOR_CMD_LUT_SEQ_IDX_READ_STATUS            1
#define NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI        2
#define NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE           3
#define NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE_OPI       4
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_OCTAL      7
#define NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR           8
#define NOR_CMD_LUT_SEQ_IDX_READ_ID_SPI            9
#define NOR_CMD_LUT_SEQ_IDX_READ_ID_OPI            10
#define NOR_CMD_LUT_SEQ_IDX_ERASE_CHIP             11
#define NOR_CMD_LUT_SEQ_IDX_ENTER_OPI              12

#define CUSTOM_LUT_LENGTH        80
#define FLASH_BUSY_STATUS_POL    1
#define FLASH_BUSY_STATUS_OFFSET 0
#define FLASH_WE_STATUS_OFFSET   7

#define FLASH_ENABLE_OCTAL_CMD   0x02
/* DMA related. */
#define EXAMPLE_XSPI_DMA (DMA0)

#define XSPI_TX_DMA_CHANNEL 0U
#define XSPI_RX_DMA_CHANNEL 1U

#define XSPI_TX_DMA_IRQn  EDMA0_CH0_IRQn
#define XSPI_TX_DMA_ISR   EDMA0_CH0_DriverIRQHandler
#define XSPI_RX_DMA_IRQn  EDMA0_CH1_IRQn
#define XSPI_RX_DMA_ISR   EDMA0_CH1_DriverIRQHandler


#define XSPI_TX_DMA_REQUEST_SOURCE kDmaRequestMuxXspi0Tx
#define XSPI_RX_DMA_REQUEST_SOURCE kDmaRequestMuxXspi0Rx

#define DEMO_INVALIDATE_CACHES   do { \
  XCACHE_InvalidateCache(XCACHE1);      \
  CACHE64_CTRL0->CCR |= CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK | CACHE64_CTRL_CCR_GO_MASK; \
  while ((CACHE64_CTRL0->CCR & CACHE64_CTRL_CCR_GO_MASK) != 0x00U) \
  {} \
  CACHE64_CTRL0->CCR &= ~(CACHE64_CTRL_CCR_INVW0_MASK | CACHE64_CTRL_CCR_INVW1_MASK); \
  } while(0)

/*${macro:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/
typedef struct _xspi_cache_status
{
    volatile bool codeCacheEnableFlag;
    volatile bool systemCacheEnableFlag;
} xspi_cache_status_t;
/*${variable:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
static inline void xspi_clock_init(void)
{
    POWER_DisablePD(kPDRUNCFG_APD_XSPI0);
    POWER_DisablePD(kPDRUNCFG_PPD_XSPI0);
    POWER_ApplyPD();
}
/*${prototype:end}*/

#endif /* _APP_H_ */
