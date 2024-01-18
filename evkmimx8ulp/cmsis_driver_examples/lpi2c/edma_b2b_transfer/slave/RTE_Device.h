/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPI2C0_InitPins();
extern void LPI2C0_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPI2CX_GetFreq/LPI2CX_InitPins/LPI2CX_DeinitPins for the enabled
 * LPI2C instance. */
#define RTE_I2C0        1
#define RTE_I2C0_DMA_EN 0

/* LPI2C configuration. */

#define RTE_I2C0_PIN_INIT        LPI2C0_InitPins
#define RTE_I2C0_PIN_DEINIT      LPI2C0_DeinitPins
#define RTE_I2C0_DMA_TX_CH       0
#define RTE_I2C0_DMA_TX_PERI_SEL (uint8_t) kDmaRequestMux0LPI2C0Tx
#define RTE_I2C0_DMA_TX_DMA_BASE DMA0
#define RTE_I2C0_DMA_RX_CH       1
#define RTE_I2C0_DMA_RX_PERI_SEL (uint8_t) kDmaRequestMux0LPI2C0Rx
#define RTE_I2C0_DMA_RX_DMA_BASE DMA0

#endif /* _RTE_DEVICE_H */
