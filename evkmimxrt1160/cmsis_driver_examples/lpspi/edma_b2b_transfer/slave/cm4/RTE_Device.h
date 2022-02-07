/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPSPI1_InitPins();
extern void LPSPI1_DeinitPins();

/*Driver name mapping.*/
/* User needs to provide the implementation of LPSPIX_GetFreq/LPSPIX_InitPins/LPSPIX_DeinitPins for the enabled LPSPI
 * instance. */
#define RTE_SPI1        1
#define RTE_SPI1_DMA_EN 1

/* SPI configuration. */
#define RTE_SPI1_PCS_TO_SCK_DELAY       1000
#define RTE_SPI1_SCK_TO_PSC_DELAY       1000
#define RTE_SPI1_BETWEEN_TRANSFER_DELAY 1000
#define RTE_SPI1_MASTER_PCS_PIN_SEL     (kLPSPI_MasterPcs0)
#define RTE_SPI1_SLAVE_PCS_PIN_SEL      (kLPSPI_SlavePcs0)
#define RTE_SPI1_PIN_INIT               LPSPI1_InitPins
#define RTE_SPI1_PIN_DEINIT             LPSPI1_DeinitPins
#define RTE_SPI1_DMA_TX_CH              0
#define RTE_SPI1_DMA_TX_PERI_SEL        (uint8_t) kDmaRequestMuxLPSPI1Tx
#if __CORTEX_M == 7
#define RTE_SPI1_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_SPI1_DMA_TX_DMA_BASE    DMA0
#else
#define RTE_SPI1_DMA_TX_DMAMUX_BASE DMAMUX1
#define RTE_SPI1_DMA_TX_DMA_BASE    DMA1
#endif
#define RTE_SPI1_DMA_RX_CH       1
#define RTE_SPI1_DMA_RX_PERI_SEL (uint8_t) kDmaRequestMuxLPSPI1Rx
#if __CORTEX_M == 7
#define RTE_SPI1_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_SPI1_DMA_RX_DMA_BASE    DMA0
#else
#define RTE_SPI1_DMA_RX_DMAMUX_BASE DMAMUX1
#define RTE_SPI1_DMA_RX_DMA_BASE    DMA1
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#endif /* _RTE_DEVICE_H */
