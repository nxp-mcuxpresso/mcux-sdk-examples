/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPI_DEV_H
#define SPI_DEV_H

#include <stdint.h>
#include <stdlib.h>
#include "spi_dev_lpspi.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct _spi_tx_rx_info
{
    uint8_t dataWidth; /* 1, 2, 4. */
    const uint8_t * txData;
    uint8_t *rxData;
    uint32_t dataLen;
} spi_tx_rx_info_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

status_t SPI_DEV_SendReceive(spi_dev_t *spi_dev, const spi_tx_rx_info_t *infos, uint8_t infoLen);

#if defined(__cplusplus)
extern "C" }
#endif

#endif
