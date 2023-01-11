/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_STREAMER_PCM_APP_H_
#define _FSL_STREAMER_PCM_APP_H_

#include "fsl_dmamux.h"
#include "fsl_sai_edma.h"
#include "FreeRTOS.h"
#include "portable.h"
#include "semphr.h"

#include "streamer_pcm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief PCM interface structure */
struct _pcm_rtos_t
{
    sai_transfer_t saiTx;
    sai_edma_handle_t saiTxHandle;
    edma_handle_t dmaTxHandle;

    sai_transfer_t saiRx;
    sai_edma_handle_t saiRxHandle;
    edma_handle_t dmaRxHandle;

    uint32_t sample_rate;
    uint32_t bit_width;
    uint8_t num_channels;

    SemaphoreHandle_t semaphoreRX;

    uint8_t isFirstRx;

    bool dummy_tx_enable;
};

#endif
