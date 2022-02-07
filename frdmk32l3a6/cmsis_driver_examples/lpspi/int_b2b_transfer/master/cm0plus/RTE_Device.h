/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPSPI0_InitPins();
extern void LPSPI0_DeinitPins();
extern void LPSPI3_InitPins();
extern void LPSPI3_DeinitPins();

/* M4 core configuration */
#ifdef CPU_K32L3A60VPJ1A_cm4
/*Driver name mapping.*/
/* User needs to provide the implementation of LPSPIX_GetFreq/LPSPIX_InitPins/LPSPIX_DeinitPins for the enabled LPSPI
 * instance. */
#define RTE_SPI0        1
#define RTE_SPI0_DMA_EN 0

/* SPI configuration. */
#define RTE_SPI0_PCS_TO_SCK_DELAY       1000
#define RTE_SPI0_SCK_TO_PSC_DELAY       1000
#define RTE_SPI0_BETWEEN_TRANSFER_DELAY 1000
#define RTE_SPI0_MASTER_PCS_PIN_SEL     (kLPSPI_MasterPcs2)
#define RTE_SPI0_SLAVE_PCS_PIN_SEL      (kLPSPI_SlavePcs2)
#define RTE_SPI0_PIN_INIT               LPSPI0_InitPins
#define RTE_SPI0_PIN_DEINIT             LPSPI0_DeinitPins
#define RTE_SPI0_DMA_TX_CH              0
#define RTE_SPI0_DMA_TX_PERI_SEL        (uint8_t) kDmaRequestMux0LPSPI0Tx
#define RTE_SPI0_DMA_TX_DMAMUX_BASE     DMAMUX0
#define RTE_SPI0_DMA_TX_DMA_BASE        DMA0
#define RTE_SPI0_DMA_RX_CH              1
#define RTE_SPI0_DMA_RX_PERI_SEL        (uint8_t) kDmaRequestMux0LPSPI0Rx
#define RTE_SPI0_DMA_RX_DMAMUX_BASE     DMAMUX0
#define RTE_SPI0_DMA_RX_DMA_BASE        DMA0
#endif

/* M0+ core configuration */
#ifdef CPU_K32L3A60VPJ1A_cm0plus
/*Driver name mapping.*/
#define RTE_SPI3        1
#define RTE_SPI3_DMA_EN 0

/* SPI configuration. */
#define RTE_SPI3_PCS_TO_SCK_DELAY       1000
#define RTE_SPI3_SCK_TO_PSC_DELAY       1000
#define RTE_SPI3_BETWEEN_TRANSFER_DELAY 1000
#define RTE_SPI3_MASTER_PCS_PIN_SEL     (kLPSPI_MasterPcs1)
#define RTE_SPI3_SLAVE_PCS_PIN_SEL      (kLPSPI_SlavePcs1)
#define RTE_SPI3_PIN_INIT               LPSPI3_InitPins
#define RTE_SPI3_PIN_DEINIT             LPSPI3_DeinitPins
#define RTE_SPI3_DMA_TX_CH              5
#define RTE_SPI3_DMA_TX_PERI_SEL        (uint8_t) kDmaRequestMux1LPSPI3Tx
#define RTE_SPI3_DMA_TX_DMAMUX_BASE     DMAMUX1
#define RTE_SPI3_DMA_TX_DMA_BASE        DMA1
#define RTE_SPI3_DMA_RX_CH              6
#define RTE_SPI3_DMA_RX_PERI_SEL        (uint8_t) kDmaRequestMux1LPSPI3Rx
#define RTE_SPI3_DMA_RX_DMAMUX_BASE     DMAMUX1
#define RTE_SPI3_DMA_RX_DMA_BASE        DMA1
#endif

#endif /* _RTE_DEVICE_H */
