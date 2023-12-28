/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_STREAMER_PCM_H_
#define _FSL_STREAMER_PCM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "fsl_dmamux.h"
#include "fsl_sai_edma.h"
#include "FreeRTOS.h"
#include "portable.h"
#include "semphr.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief PCM interface structure */
typedef struct _pcm_rtos_t
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
    SemaphoreHandle_t semaphoreTX;

    uint8_t isFirstRx;
    uint8_t isFirstTx;

    bool dummy_tx_enable;
} pcm_rtos_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Initialize the PCM interface
 *
 * This function initializes the PCM interface for audio data output to a
 * speaker or input from a microphone on a specific target hardware platform.
 * This function should be executed directly by the application or other
 * middleware library, and should be called before any other PCM interface functions.
 * This function is responsible for initializing and configuring audio output
 * peripherals, DMA and codecs so that they are ready to utilitize the PCM
 * interface library.
 */
void streamer_pcm_init(void);

/*!
 * @brief Open the PCM interface for writing and configuring
 *
 * This function returns a handle to the PCM interface that can be used for
 * configuration and writing the data.
 *
 * This function is called from the streamer when it is placed into a playing
 * state, before audio output starts.
 *
 * @param num_buffers Number of buffers to be used
 * @return 0 for success
 */
int streamer_pcm_tx_open(uint32_t num_buffers);

/*!
 * @brief Open the receive PCM interface for reading and configuring
 *
 * This function returns a handle to the PCM interface that can be used for
 * configuration and reading the data.
 *
 * This function is called from the streamer when it is placed into a playing
 * state, before audio input starts.
 *
 * @param num_buffers Number of buffers to be used
 * @return 0 for success
 */
int streamer_pcm_rx_open(uint32_t num_buffers);

/*!
 * @brief Close the PCM interface
 *
 * This function closes the handle to the PCM interface
 *
 * This function is called from the streamer when it is placed into a stopped
 * state.
 */
void streamer_pcm_tx_close(void);

/*!
 * @brief Close the receive PCM interface
 *
 * This function closes the handle to the PCM interface
 *
 * This function is called from the streamer when it is placed into a stopped
 * state.
 */
void streamer_pcm_rx_close(void);

/*!
 * @brief Write audio data to the PCM interface
 *
 * This function writes raw PCM data to the PCM output interface
 *
 * This function is called from the streamer to output decoded audio data.
 *
 * @param data Pointer to data buffer with PCM data
 * @param size Size in bytes of the data buffer
 * @return 0 on success, non-zero on failure
 */
int streamer_pcm_write(uint8_t *data, uint32_t size);

/*!
 * @brief Read audio data from the input PCM interface
 *
 * This function reads raw PCM data from the PCM input interface
 *
 * This function is called from the streamer to input raw audio data.
 *
 * @param data Pointer to next data buffer with PCM data
 * @param size Size in bytes of the data buffer
 * @return 0 on success, non-zero on failure
 */
int streamer_pcm_read(uint8_t *data, uint32_t size);

/*!
 * @brief Set PCM interface parameters
 *
 * This function is called from the streamer after the start of audio data
 * decode, once the parameters of the data format are known. The PCM interface
 * should be configured to match these parameters. The pcm handle should
 * reflect the values configured.
 *
 * If the PCM interface cannot support these parameters, the pcm handle should
 * be set to the nearest values the interface can support and success should be
 * returned.  These will be queried by the streamer with streamer_pcm_getparams,
 * and the streamer will resample or reconfigure the audio format to fit the
 * desired output.
 *
 * @param sample_rate Sample rate in Hz of the PCM data
 * @param bit_width Size of each PCM data sample, in bits
 * @param num_channels Number of channels of audio data for each PCM frame
 * @param transfer Flag for setting params of input/output pcm interface
 * @param dummy_tx Dummy tx setting for clock enablement
 * @param volume Output volume
 * @return 0 on success, non-zero on failure
 */
int streamer_pcm_setparams(
    uint32_t sample_rate, uint32_t bit_width, uint8_t num_channels, bool transfer, bool dummy_tx, int volume);

/*!
 * @brief Get PCM interface parameters
 *
 * This function is called by the streamer after streamer_pcm_setparams, and
 * should reflect what parameters can be supported by the PCM interface. If
 * these do not match what was requested in streamer_pcm_setparams, the streamer
 * will attempt to configure SRC or other behavior to adjust the data format.
 * If the streamer cannot support this, it will return a failure status to the
 * application via the message task.
 *
 * @param sample_rate Pointer to pcm handle sample rate
 * @param bit_width Pointer to pcm handle bit width
 * @param num_channels Pointer to pcm handle number of channels
 */
void streamer_pcm_getparams(uint32_t *sample_rate, uint32_t *bit_width, uint8_t *num_channels);

/*!
 * @brief Set PCM interface mute status
 *
 * This function is called by the streamer while entering the stopped state to
 * mute the PCM output codec. This function is optional to implement as not all
 * codecs will support this functionality.
 *
 * @param mute Boolean indicated true/false requested codec mute state
 * @return 0 on success, non-zero on failure
 */
int streamer_pcm_mute(bool mute);

/*!
 * @brief Set PCM interface output volume
 *
 * This function is called by the application interface to adjust the output
 * volume for the PCM interface.
 *
 * @param volume Volume on a scale from 0-100
 * @return 0 on success, non-zero on failure
 */
int streamer_pcm_set_volume(int volume);

/* Weak definition of master clock config function - only implemented on some platforms */
__attribute__((weak)) void BOARD_MASTER_CLOCK_CONFIG(void)
{
}

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
