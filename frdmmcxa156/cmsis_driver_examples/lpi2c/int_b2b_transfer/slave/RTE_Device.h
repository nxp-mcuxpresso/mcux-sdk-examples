/*
 * Copyright 2024 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPI2C3_InitPins();
extern void LPI2C3_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPI2CX_GetFreq/LPI2CX_InitPins/LPI2CX_DeinitPins for the enabled LPI2C
 * instance. */
#define RTE_I2C3        1
#define RTE_I2C3_DMA_EN 0

/* LPI2C configuration. */
#define RTE_I2C3_PIN_INIT        LPI2C3_InitPins
#define RTE_I2C3_PIN_DEINIT      LPI2C3_DeinitPins
#define RTE_I2C3_DMA_TX_CH       6
#define RTE_I2C3_DMA_TX_PERI_SEL (uint16_t) kDma0RequestLPI2C3Tx
#define RTE_I2C3_DMA_TX_DMA_BASE DMA0
#define RTE_I2C3_DMA_RX_CH       5
#define RTE_I2C3_DMA_RX_PERI_SEL (uint16_t) kDma0RequestLPI2C3Rx
#define RTE_I2C3_DMA_RX_DMA_BASE DMA0

#endif /* _RTE_DEVICE_H */
