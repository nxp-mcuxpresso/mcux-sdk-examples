/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPI_DEV_LPSPI_H
#define SPI_DEV_LPSPI_H

#include "fsl_lpspi.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef void (*lpspi_cs_func_t)(uint8_t level);

typedef struct spi_dev_lpspi
{
    LPSPI_Type *lpspi;
    uint32_t reg_tcr;
    lpspi_cs_func_t cs_func;
} spi_dev_lpspi_t;

typedef spi_dev_lpspi_t spi_dev_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

status_t SPI_DEV_Init(spi_dev_t *spi_dev, LPSPI_Type* lpspi, lpspi_cs_func_t cs_func);

#if defined(__cplusplus)
extern "C" }
#endif

#endif