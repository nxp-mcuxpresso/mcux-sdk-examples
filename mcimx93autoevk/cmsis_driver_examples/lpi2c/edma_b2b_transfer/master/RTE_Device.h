/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPI2C1_InitPins();
extern void LPI2C1_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPI2CX_GetFreq/LPI2CX_InitPins/LPI2CX_DeinitPins for the enabled
 * LPI2C instance. */
#define RTE_I2C1        1
#define RTE_I2C1_DMA_EN 1

/* LPI2C configuration. */

#define RTE_I2C1_PIN_INIT        LPI2C1_InitPins
#define RTE_I2C1_PIN_DEINIT      LPI2C1_DeinitPins
#define RTE_I2C1_DMA_TX_CH       (uint8_t) kDma3RequestMuxLPI2C1Tx
#define RTE_I2C1_DMA_TX_PERI_SEL (uint8_t) kDma3RequestMuxLPI2C1Tx
#define RTE_I2C1_DMA_TX_DMA_BASE DMA3
#define RTE_I2C1_DMA_RX_CH       (uint8_t) kDma3RequestMuxLPI2C1Rx
#define RTE_I2C1_DMA_RX_PERI_SEL (uint8_t) kDma3RequestMuxLPI2C1Rx
#define RTE_I2C1_DMA_RX_DMA_BASE DMA3

#endif /* _RTE_DEVICE_H */
