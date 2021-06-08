/*
 * Copyright 2017-2018, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SRTM_SAI_EDMA_ADAPTER_H__
#define __SRTM_SAI_EDMA_ADAPTER_H__

#include "srtm_audio_service.h"
#include "fsl_sai_edma.h"

/*!
 * @addtogroup srtm_service
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
#define SRTM_SAI_EDMA_LOCAL_BUF_ENABLE (0)
#endif

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
#define SRTM_SAI_EDMA_MAX_LOCAL_BUF_PERIODS           (4)
#define SRTM_SAI_EDMA_MAX_LOCAL_PERIOD_ALIGNMENT      (4U)
#define SRTM_SAI_EDMA_MAX_LOCAL_PERIOD_ALIGNMENT_MASK (SRTM_SAI_EDMA_MAX_LOCAL_PERIOD_ALIGNMENT - 1)
#endif

typedef struct _srtm_sai_edma_config
{
    sai_config_t config;
    uint8_t dataLine;  /* SAI data line number for transaction */
    uint8_t watermark; /* SAI DMA hardware watermark */
    uint32_t mclk;
    uint32_t bclk;
    uint32_t dmaChannel;
    bool stopOnSuspend;
    uint32_t threshold; /* threshold period number: under which will trigger periodDone notification. */
} srtm_sai_edma_config_t;

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
typedef struct _srtm_sai_edma_local_buf
{
    uint8_t *buf;
    uint32_t bufSize;   /* bytes of the whole local buffer */
    uint32_t periods;   /* periods in local buffer */
    uint32_t threshold; /* Threshold period number: under which will trigger copy from share buf to local buf
                           in playback case. */
} srtm_sai_edma_local_buf_t;
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Create SAI EDMA adapter.
 *
 * @param sai SAI base address.
 * @param dma DMA base address.
 * @param txConfig SAI Tx channel configuration.
 * @param rxConfig SAI Rx channel configuration.
 * @return SRTM SAI EDMA adapter on success or NULL on failure.
 */
srtm_sai_adapter_t SRTM_SaiEdmaAdapter_Create(I2S_Type *sai,
                                              DMA_Type *dma,
                                              srtm_sai_edma_config_t *txConfig,
                                              srtm_sai_edma_config_t *rxConfig);

/*!
 * @brief Destroy SAI EDMA adapter.
 *
 * @param adapter SAI EDMA adapter to destroy.
 */
void SRTM_SaiEdmaAdapter_Destroy(srtm_sai_adapter_t adapter);

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
/*!
 * @brief Set local buffer to use in DMA transfer. If local buffer is set, the audio data will be copied
 * from shared buffer to local buffer and then transfered to I2S interface. Otherwise the data will be
 * transfered from shared buffer to I2S interface directly.
 * NOTE: it must be called before service start.
 *
 * @param adapter SAI EDMA adapter to set.
 * @param localBuf Local buffer information to be set to the adapter TX path.
 */
void SRTM_SaiEdmaAdapter_SetTxLocalBuf(srtm_sai_adapter_t adapter, srtm_sai_edma_local_buf_t *localBuf);
#endif

#ifdef __cplusplus
}
#endif

/*! @} */

#endif /* __SRTM_SAI_EDMA_ADAPTER_H__ */
