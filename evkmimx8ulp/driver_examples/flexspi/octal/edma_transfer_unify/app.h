/*
 * Copyright 2021,2023 NXP
 * All rights reserved.
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
#define EXAMPLE_FLEXSPI                 FLEXSPI0
#define EXAMPLE_CACHE                   CACHE64_CTRL0
#define FLASH_SIZE                      0x1000 /* 32Mb/KByte */
#define EXAMPLE_FLEXSPI_AMBA_BASE       FlexSPI0_AMBA_BASE
#define FLASH_PAGE_SIZE                 256
#define EXAMPLE_SECTOR                  100
#define SECTOR_SIZE                     0x1000 /* 4K */
#define EXAMPLE_FLEXSPI_CLOCK           kCLOCK_FlexSpi0
#define FLASH_PORT                      kFLEXSPI_PortA1
#define EXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK kFLEXSPI_ReadSampleClkExternalInputFromDqsPad

#define FLASH_GIGADEVICE_DEVICE_GD25LX256 1
#define CACHE_MAINTAIN                    1

/* DMA related. */
#define EXAMPLE_FLEXSPI_DMA (DMA0)

#define EXAMPLE_TX_DMA_CHANNEL_CLOCK kCLOCK_Dma0Ch0
#define EXAMPLE_RX_DMA_CHANNEL_CLOCK kCLOCK_Dma0Ch1

#define FLEXSPI_TX_DMA_REQUEST_SOURCE kDmaRequestMux0FlexSPI0Tx
#define FLEXSPI_RX_DMA_REQUEST_SOURCE kDmaRequestMux0FlexSPI0Rx

#define FLEXSPI_TX_DMA_CHANNEL 0U
#define FLEXSPI_RX_DMA_CHANNEL 1U

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*${variable:start}*/
typedef struct _flexspi_cache_status
{
    volatile bool CacheEnableFlag;
} flexspi_cache_status_t;
/*${variable:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);

/*${prototype:end}*/

#endif /* _APP_H_ */
