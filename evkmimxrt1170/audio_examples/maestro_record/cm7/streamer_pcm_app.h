/*
 * Copyright 2021-2022 NXP
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
#include "fsl_pdm.h"
#include "fsl_pdm_edma.h"

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

    pdm_edma_transfer_t pdmRx;
    pdm_edma_handle_t pdmRxHandle;
    edma_handle_t dmaRxHandle;

    uint32_t sample_rate;
    uint32_t bit_width;
    uint8_t num_channels;

    SemaphoreHandle_t semaphoreRX;
    SemaphoreHandle_t semaphoreTX;

    uint8_t isFirstRx;
    uint8_t isFirstTx;

    bool dummy_tx_enable;
};

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Set PCM interface master clock
 *
 * This function is called by the application interface to align the master
 * clock of the audio peripherals with the sample rate.
 *
 * @param sample_rate Desired sample rate
 * @return master clock that has been set
 */
int streamer_set_master_clock(int sample_rate);

#if defined(__cplusplus)
}
#endif

#endif
