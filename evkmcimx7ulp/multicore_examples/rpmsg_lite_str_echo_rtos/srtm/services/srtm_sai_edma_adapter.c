/*
 * Copyright 2017-2018, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "srtm_sai_edma_adapter.h"
#include "srtm_heap.h"

#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "srtm_dispatcher.h"
#include "srtm_message.h"
#include "srtm_service_struct.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct _srtm_sai_edma_buf_runtime
{
    uint32_t leadIdx;          /* ready period index for playback or recording. */
    uint32_t chaseIdx;         /* consumed period index for playback or recording. */
    uint32_t loadIdx;          /* used to indicate period index preloaded either to DMA transfer or to local buffer. */
    uint32_t remainingPeriods; /* periods to be consumed/filled */
    uint32_t remainingLoadPeriods; /* periods to be preloaded either to DMA transfer or to local buffer. */
    uint32_t offset;               /* period offset to copy */
} * srtm_sai_edma_buf_runtime_t;

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
struct _srtm_sai_edma_local_period
{
    uint32_t dataSize;     /* bytes of copied data */
    uint32_t endRemoteIdx; /* period index of remote buffer if local period contains remote buffer end. */
    uint32_t remoteIdx;    /* save remote period index which the local period end points to */
    uint32_t remoteOffset; /* save remote period offset which the local period end points to */
};

struct _srtm_sai_edma_local_runtime
{
    uint32_t periodSize;
    struct _srtm_sai_edma_buf_runtime bufRtm;
    struct _srtm_sai_edma_local_period periodsInfo[SRTM_SAI_EDMA_MAX_LOCAL_BUF_PERIODS];
};
#endif

typedef struct _srtm_sai_edma_runtime
{
    srtm_audio_state_t state;
    sai_edma_handle_t saiHandle;
    sai_word_width_t bitWidth;
    sai_mono_stereo_t channels;
    uint32_t srate;
    uint8_t *bufAddr;
    uint32_t bufSize;
    uint32_t periodSize;
    uint32_t periods;
    uint32_t readyIdx;                        /* period ready index. */
    srtm_procedure_t proc;                    /* proc message to trigger DMA transfer in SRTM context. */
    struct _srtm_sai_edma_buf_runtime bufRtm; /* buffer provided by audio client. */
#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    srtm_sai_edma_local_buf_t localBuf;
    struct _srtm_sai_edma_local_runtime localRtm; /* buffer set by application. */
#endif
    bool freeRun;               /* flag to indicate that no periodReady will be sent by audio client. */
    uint32_t finishedBufOffset; /* offset from bufAddr where the data transfer has completed. */
} * srtm_sai_edma_runtime_t;

/* SAI EDMA adapter */
typedef struct _srtm_sai_edma_adapter
{
    struct _srtm_sai_adapter adapter;
    uint32_t index;

    I2S_Type *sai;
    DMA_Type *dma;
    srtm_sai_edma_config_t txConfig;
    srtm_sai_edma_config_t rxConfig;
    edma_handle_t txDmaHandle;
    edma_handle_t rxDmaHandle;

    struct _srtm_sai_edma_runtime rxRtm;
    struct _srtm_sai_edma_runtime txRtm;
} * srtm_sai_edma_adapter_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const sai_word_width_t saiBitsWidthMap[] = {kSAI_WordWidth16bits, kSAI_WordWidth24bits};
static const sai_mono_stereo_t saiChannelMap[]  = {kSAI_MonoLeft, kSAI_MonoRight, kSAI_Stereo};
#ifdef SRTM_DEBUG_MESSAGE_FUNC
static const char *saiDirection[] = {"Rx", "Tx"};
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SRTM_SaiEdmaAdapter_RecycleTxMessage(srtm_message_t msg, void *param)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)param;

    assert(handle->txRtm.proc == NULL);

    handle->txRtm.proc = msg;
}

static void SRTM_SaiEdmaAdapter_RecycleRxMessage(srtm_message_t msg, void *param)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)param;

    assert(handle->rxRtm.proc == NULL);

    handle->rxRtm.proc = msg;
}

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
static void SRTM_SaiEdmaAdaptor_ResetLocalBuf(srtm_sai_edma_runtime_t rtm)
{
    uint32_t i, n;

    if (rtm->localBuf.buf)
    {
        memset(&rtm->localRtm.bufRtm, 0, sizeof(struct _srtm_sai_edma_buf_runtime));
        rtm->localRtm.periodSize =
            (rtm->localBuf.bufSize / rtm->localBuf.periods) & (~SRTM_SAI_EDMA_MAX_LOCAL_PERIOD_ALIGNMENT_MASK);
        /* Calculate how many local periods each remote period */
        n                        = (rtm->periodSize + rtm->localRtm.periodSize - 1) / rtm->localRtm.periodSize;
        rtm->localRtm.periodSize = ((rtm->periodSize + n - 1) / n + SRTM_SAI_EDMA_MAX_LOCAL_PERIOD_ALIGNMENT_MASK) &
                                   (~SRTM_SAI_EDMA_MAX_LOCAL_PERIOD_ALIGNMENT_MASK);
        for (i = 0; i < SRTM_SAI_EDMA_MAX_LOCAL_BUF_PERIODS; i++)
        {
            rtm->localRtm.periodsInfo[i].dataSize     = 0;
            rtm->localRtm.periodsInfo[i].endRemoteIdx = UINT32_MAX;
            rtm->localRtm.periodsInfo[i].remoteIdx    = 0;
            rtm->localRtm.periodsInfo[i].remoteOffset = 0;
        }
    }
}
#endif

static void SRTM_SaiEdmaAdapter_GetXfer(srtm_sai_edma_runtime_t rtm, sai_transfer_t *xfer)
{
    srtm_sai_edma_buf_runtime_t bufRtm;

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    if (rtm->localBuf.buf)
    {
        bufRtm         = &rtm->localRtm.bufRtm;
        xfer->dataSize = rtm->localRtm.periodsInfo[bufRtm->loadIdx].dataSize;
        xfer->data     = rtm->localBuf.buf + bufRtm->loadIdx * rtm->localRtm.periodSize;
    }
    else
#endif
    {
        bufRtm         = &rtm->bufRtm;
        xfer->dataSize = rtm->periodSize;
        xfer->data     = rtm->bufAddr + bufRtm->loadIdx * rtm->periodSize;
    }
}

static void SRTM_SaiEdmaAdapter_DmaTransfer(srtm_sai_edma_adapter_t handle, srtm_audio_dir_t dir)
{
    srtm_sai_edma_runtime_t rtm = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;
    srtm_sai_edma_buf_runtime_t bufRtm;
    uint32_t i;
    status_t status;
    uint32_t periods;
    sai_transfer_t xfer;
    uint32_t num;

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    if (rtm->localBuf.buf)
    {
        bufRtm  = &rtm->localRtm.bufRtm;
        periods = rtm->localBuf.periods;
    }
    else
#endif
    {
        bufRtm  = &rtm->bufRtm;
        periods = rtm->periods;
    }

    num = bufRtm->remainingLoadPeriods;

    for (i = 0; i < num; i++)
    {
        SRTM_SaiEdmaAdapter_GetXfer(rtm, &xfer);
        if (dir == SRTM_AudioDirTx)
        {
            status = SAI_TransferSendEDMA(handle->sai, &rtm->saiHandle, &xfer);
        }
        else
        {
            status = SAI_TransferReceiveEDMA(handle->sai, &rtm->saiHandle, &xfer);
        }
        if (status != kStatus_Success)
        {
            /* Audio queue full */
            break;
        }
        bufRtm->loadIdx = (bufRtm->loadIdx + 1U) % periods;
        bufRtm->remainingLoadPeriods--;
    }
}

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
static void SRTM_SaiEdmaAdapter_CopyData(srtm_sai_edma_adapter_t handle)
{
    srtm_sai_edma_runtime_t rtm;
    uint32_t srcSize, dstSize, size;
    srtm_sai_edma_buf_runtime_t srcRtm, dstRtm;
    uint8_t *src, *dst;

    rtm    = &handle->txRtm;
    srcRtm = &rtm->bufRtm;
    dstRtm = &rtm->localRtm.bufRtm;

    while (srcRtm->remainingLoadPeriods && (rtm->localBuf.periods - dstRtm->remainingPeriods))
    {
        src     = rtm->bufAddr + srcRtm->loadIdx * rtm->periodSize;
        dst     = rtm->localBuf.buf + dstRtm->leadIdx * rtm->localRtm.periodSize;
        srcSize = rtm->periodSize - srcRtm->offset;
        dstSize = rtm->localRtm.periodSize - dstRtm->offset;
        size    = MIN(srcSize, dstSize);
        memcpy(dst + dstRtm->offset, src + srcRtm->offset, size);

        srcRtm->offset += size;
        dstRtm->offset += size;
        if (srcRtm->offset == rtm->periodSize) /* whole remote buffer loaded */
        {
            rtm->localRtm.periodsInfo[dstRtm->leadIdx].endRemoteIdx = srcRtm->loadIdx;
            srcRtm->loadIdx                                         = (srcRtm->loadIdx + 1) % rtm->periods;
            srcRtm->offset                                          = 0;
            srcRtm->remainingLoadPeriods--;
        }

        if (dstRtm->offset == rtm->localRtm.periodSize || srcRtm->offset == 0)
        {
            /* local period full or remote period ends */
            rtm->localRtm.periodsInfo[dstRtm->leadIdx].dataSize     = dstRtm->offset;
            rtm->localRtm.periodsInfo[dstRtm->leadIdx].remoteIdx    = srcRtm->loadIdx;
            rtm->localRtm.periodsInfo[dstRtm->leadIdx].remoteOffset = srcRtm->offset;
            dstRtm->leadIdx                                         = (dstRtm->leadIdx + 1) % rtm->localBuf.periods;
            dstRtm->remainingPeriods++;
            dstRtm->remainingLoadPeriods++;
            dstRtm->offset = 0;
        }
    }
}
#endif

static void SRTM_SaiEdmaAdapter_AddNewPeriods(srtm_sai_edma_runtime_t rtm, uint32_t periodIdx)
{
    srtm_sai_edma_buf_runtime_t bufRtm = &rtm->bufRtm;
    uint32_t newPeriods;
    uint32_t primask;

    assert(periodIdx < rtm->periods);

    newPeriods = (periodIdx + rtm->periods - bufRtm->leadIdx) % rtm->periods;
    if (newPeriods == 0U) /* in case buffer is empty and filled all */
    {
        newPeriods = rtm->periods;
    }
    bufRtm->leadIdx = periodIdx;

    primask = DisableGlobalIRQ();
    bufRtm->remainingPeriods += newPeriods;
    EnableGlobalIRQ(primask);
    bufRtm->remainingLoadPeriods += newPeriods;
}

static void SRTM_SaiEdmaAdapter_Transfer(srtm_sai_edma_adapter_t handle, srtm_audio_dir_t dir)
{
    srtm_sai_edma_runtime_t rtm = &handle->txRtm;

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    if (dir == SRTM_AudioDirTx && rtm->localBuf.buf)
    {
        if (rtm->localRtm.bufRtm.remainingPeriods <= rtm->localBuf.threshold)
        {
            /* Copy data from remote buffer to local buffer. */
            SRTM_SaiEdmaAdapter_CopyData(handle);
        }
    }
#endif
    /* Trigger DMA if having more data to playback/record. */
    SRTM_SaiEdmaAdapter_DmaTransfer(handle, dir);

    if (rtm->freeRun && rtm->bufRtm.remainingPeriods < rtm->periods)
    {
        /* In free run, we assume consumed period is filled immediately. */
        SRTM_SaiEdmaAdapter_AddNewPeriods(rtm, rtm->bufRtm.chaseIdx);
    }
}

static void SRTM_SaiEdmaAdapter_TxTransferProc(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)param1;
    srtm_sai_edma_runtime_t rtm    = &handle->txRtm;

    if (rtm->state == SRTM_AudioStateStarted)
    {
        SRTM_SaiEdmaAdapter_Transfer(handle, SRTM_AudioDirTx);
    }
}

static void SRTM_SaiEdmaAdapter_RxTransferProc(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)param1;
    srtm_sai_edma_runtime_t rtm    = &handle->rxRtm;

    if (rtm->state == SRTM_AudioStateStarted)
    {
        /* Trigger DMA if having more buffer to record. */
        SRTM_SaiEdmaAdapter_Transfer(handle, SRTM_AudioDirRx);
    }
}

static void SRTM_SaiEdmaTxCallback(I2S_Type *sai, sai_edma_handle_t *edmaHandle, status_t status, void *userData)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)userData;
    srtm_sai_edma_runtime_t rtm    = &handle->txRtm;
    srtm_sai_adapter_t adapter     = &handle->adapter;
    bool consumed                  = true;

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    if (rtm->localBuf.buf)
    {
        if (rtm->localRtm.periodsInfo[rtm->localRtm.bufRtm.chaseIdx].endRemoteIdx < rtm->periods)
        {
            /* The local buffer contains data from remote buffer end */
            rtm->bufRtm.remainingPeriods--; /* Now one of the remote buffer has been consumed. */
            rtm->bufRtm.chaseIdx = (rtm->bufRtm.chaseIdx + 1) % rtm->periods;
            rtm->localRtm.periodsInfo[rtm->localRtm.bufRtm.chaseIdx].endRemoteIdx = UINT32_MAX;
        }
        else
        {
            /* Remote period not consumed. */
            consumed = false;
        }

        rtm->finishedBufOffset = rtm->localRtm.periodsInfo[rtm->localRtm.bufRtm.chaseIdx].remoteIdx * rtm->periodSize +
                                 rtm->localRtm.periodsInfo[rtm->localRtm.bufRtm.chaseIdx].remoteOffset;
        rtm->localRtm.bufRtm.remainingPeriods--;
        rtm->localRtm.bufRtm.chaseIdx = (rtm->localRtm.bufRtm.chaseIdx + 1) % rtm->localBuf.periods;
    }
    else
#endif
    {
        rtm->bufRtm.remainingPeriods--;
        rtm->bufRtm.chaseIdx   = (rtm->bufRtm.chaseIdx + 1U) % rtm->periods;
        rtm->finishedBufOffset = rtm->bufRtm.chaseIdx * rtm->periodSize;
    }

    /* Notify period done message */
    if ((adapter->service != NULL) && (adapter->periodDone != NULL) && consumed &&
        (rtm->freeRun || (rtm->bufRtm.remainingPeriods <= handle->txConfig.threshold)))
    {
        /* In free run, we need to make buffer as full as possible, threshold is ignored. */
        (void)adapter->periodDone(adapter->service, SRTM_AudioDirTx, handle->index, rtm->bufRtm.chaseIdx);
    }

    if ((adapter->service != NULL) && (rtm->state == SRTM_AudioStateStarted) && (rtm->proc != NULL))
    {
        /* Fill data or add buffer to DMA scatter-gather list if there's remaining buffer to send */
        (void)SRTM_Dispatcher_PostProc(adapter->service->dispatcher, rtm->proc);
        rtm->proc = NULL;
    }
}

static void SRTM_SaiEdmaRxCallback(I2S_Type *sai, sai_edma_handle_t *edmaHandle, status_t status, void *userData)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)userData;
    srtm_sai_edma_runtime_t rtm    = &handle->rxRtm;
    srtm_sai_adapter_t adapter     = &handle->adapter;

    rtm->bufRtm.remainingPeriods--;
    rtm->bufRtm.chaseIdx   = (rtm->bufRtm.chaseIdx + 1U) % rtm->periods;
    rtm->finishedBufOffset = rtm->bufRtm.chaseIdx * rtm->periodSize;

    /* Rx is always freeRun, we assume filled period is consumed immediately. */
    SRTM_SaiEdmaAdapter_AddNewPeriods(rtm, rtm->bufRtm.chaseIdx);

    if ((adapter->service != NULL) && (adapter->periodDone != NULL))
    {
        /* Rx is always freeRun */
        (void)adapter->periodDone(adapter->service, SRTM_AudioDirRx, handle->index, rtm->bufRtm.chaseIdx);
    }

    if ((adapter->service != NULL) && (rtm->state == SRTM_AudioStateStarted) && (rtm->proc != NULL))
    {
        /* Add buffer to DMA scatter-gather list if there's remaining buffer to send */
        (void)SRTM_Dispatcher_PostProc(adapter->service->dispatcher, rtm->proc);
        rtm->proc = NULL;
    }
}

static void SRTM_SaiEdmaAdapter_InitSAI(srtm_sai_edma_adapter_t handle, srtm_audio_dir_t dir)
{
    if (dir == SRTM_AudioDirTx)
    {
        EDMA_CreateHandle(&handle->txDmaHandle, handle->dma, handle->txConfig.dmaChannel);
        SAI_TxInit(handle->sai, &handle->txConfig.config);
        SAI_TransferTxCreateHandleEDMA(handle->sai, &handle->txRtm.saiHandle, SRTM_SaiEdmaTxCallback, (void *)handle,
                                       &handle->txDmaHandle);
    }
    else
    {
        EDMA_CreateHandle(&handle->rxDmaHandle, handle->dma, handle->rxConfig.dmaChannel);
        SAI_RxInit(handle->sai, &handle->rxConfig.config);
        SAI_TransferRxCreateHandleEDMA(handle->sai, &handle->rxRtm.saiHandle, SRTM_SaiEdmaRxCallback, (void *)handle,
                                       &handle->rxDmaHandle);
    }
}

static void SRTM_SaiEdmaAdapter_DeinitSAI(srtm_sai_edma_adapter_t handle, srtm_audio_dir_t dir)
{
    if (dir == SRTM_AudioDirTx)
    {
        SAI_TxReset(handle->sai);
    }
    else
    {
        SAI_RxReset(handle->sai);
    }
}

static void SRTM_SaiEdmaAdapter_SetXferFormat(sai_transfer_format_t *fmt,
                                              srtm_sai_edma_runtime_t rtm,
                                              srtm_sai_edma_config_t *cfg)
{
    fmt->channel = cfg->dataLine;
#if defined(FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) && (FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER)
    fmt->masterClockHz = cfg->mclk;
#endif
    fmt->protocol           = cfg->config.protocol;
    fmt->watermark          = cfg->watermark;
    fmt->sampleRate_Hz      = rtm->srate;
    fmt->bitWidth           = (uint32_t)rtm->bitWidth;
    fmt->stereo             = rtm->channels;
    fmt->isFrameSyncCompact = false;
}

static void SRTM_SaiEdmaAdapter_SetFormat(srtm_sai_edma_adapter_t handle, srtm_audio_dir_t dir, bool sync)
{
    sai_transfer_format_t xferFormat;
    srtm_sai_edma_config_t *cfg;

    (void)memset(&xferFormat, 0, sizeof(sai_transfer_format_t));
    if (dir == SRTM_AudioDirTx)
    {
        cfg = &handle->txConfig;
        SRTM_SaiEdmaAdapter_SetXferFormat(&xferFormat, sync ? &handle->rxRtm : &handle->txRtm, cfg);
        SAI_TransferTxSetFormatEDMA(handle->sai, &handle->txRtm.saiHandle, &xferFormat, cfg->mclk, cfg->bclk);
    }
    else
    {
        cfg = &handle->rxConfig;
        SRTM_SaiEdmaAdapter_SetXferFormat(&xferFormat, sync ? &handle->txRtm : &handle->rxRtm, cfg);
        SAI_TransferRxSetFormatEDMA(handle->sai, &handle->rxRtm.saiHandle, &xferFormat, cfg->mclk, cfg->bclk);
    }
}

static srtm_status_t SRTM_SaiEdmaAdapter_Open(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t rtm    = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    /* Record the index */
    handle->index = index;

    if (rtm->state != SRTM_AudioStateClosed)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: %s in wrong state %d!\r\n", __func__, saiDirection[dir],
                           rtm->state);
        return SRTM_Status_InvalidState;
    }

    rtm->state   = SRTM_AudioStateOpened;
    rtm->freeRun = true;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_Close(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t rtm    = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (rtm->state == SRTM_AudioStateClosed)
    {
        /* Stop may called when audio service reset. */
        return SRTM_Status_Success;
    }

    if (rtm->state != SRTM_AudioStateOpened)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: %s in wrong state %d!\r\n", __func__, saiDirection[dir],
                           rtm->state);
        return SRTM_Status_InvalidState;
    }

    rtm->state = SRTM_AudioStateClosed;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_Start(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t thisRtm, otherRtm;
    srtm_sai_edma_config_t *thisCfg, *otherCfg;
    srtm_audio_dir_t otherDir;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (dir == SRTM_AudioDirTx)
    {
        thisRtm  = &handle->txRtm;
        otherRtm = &handle->rxRtm;
        thisCfg  = &handle->txConfig;
        otherCfg = &handle->rxConfig;
        otherDir = SRTM_AudioDirRx;
    }
    else
    {
        thisRtm  = &handle->rxRtm;
        otherRtm = &handle->txRtm;
        thisCfg  = &handle->rxConfig;
        otherCfg = &handle->txConfig;
        otherDir = SRTM_AudioDirTx;
    }

    if (thisRtm->state != SRTM_AudioStateOpened)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: %s in wrong state %d!\r\n", __func__, saiDirection[dir],
                           thisRtm->state);
        return SRTM_Status_InvalidState;
    }

    if (thisRtm->periods == 0U)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: %s valid buffer not set!\r\n", __func__, saiDirection[dir]);
        return SRTM_Status_InvalidState;
    }

    if (thisRtm->srate == 0U)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: %s valid format param not set!\r\n", __func__,
                           saiDirection[dir]);
        return SRTM_Status_InvalidState;
    }

    if (otherCfg->config.syncMode == kSAI_ModeSync)
    {
        /* The other direction in sync mode, it will initialize both directions. */
        if (otherRtm->state != SRTM_AudioStateStarted)
        {
            /* Only when the other direction is not started, we can initialize, else the device setting is reused. */
            SRTM_SaiEdmaAdapter_InitSAI(handle, dir);
            /* Use our own format. */
            SRTM_SaiEdmaAdapter_SetFormat(handle, dir, false);
        }
    }
    else
    {
        /* The other direction has dedicated clock, it will not initialize this direction.
           Do initialization by ourselves. */
        SRTM_SaiEdmaAdapter_InitSAI(handle, dir);
        /* Use our own format. */
        SRTM_SaiEdmaAdapter_SetFormat(handle, dir, false);

        if (thisCfg->config.syncMode == kSAI_ModeSync && otherRtm->state != SRTM_AudioStateStarted)
        {
            /* This direction in sync mode and the other not started, need to initialize the other direction. */
            SRTM_SaiEdmaAdapter_InitSAI(handle, otherDir);
            /* Set other direction format to ours. */
            SRTM_SaiEdmaAdapter_SetFormat(handle, otherDir, true);
        }
    }

    thisRtm->state = SRTM_AudioStateStarted;

    /* Reset buffer index */
    thisRtm->bufRtm.loadIdx    = thisRtm->bufRtm.chaseIdx;
    thisRtm->bufRtm.offset     = 0;
    thisRtm->finishedBufOffset = thisRtm->bufRtm.chaseIdx * thisRtm->periodSize;
    if (thisRtm->freeRun)
    {
        /* Assume buffer full in free run */
        thisRtm->readyIdx = thisRtm->bufRtm.leadIdx;
    }

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    SRTM_SaiEdmaAdaptor_ResetLocalBuf(thisRtm);
#endif

    SRTM_SaiEdmaAdapter_AddNewPeriods(thisRtm, thisRtm->readyIdx);
    SRTM_SaiEdmaAdapter_Transfer(handle, dir);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_End(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index, bool stop)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t thisRtm, otherRtm;
    srtm_sai_edma_config_t *thisCfg, *otherCfg;
    srtm_audio_dir_t otherDir;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (dir == SRTM_AudioDirTx)
    {
        thisRtm  = &handle->txRtm;
        otherRtm = &handle->rxRtm;
        thisCfg  = &handle->txConfig;
        otherCfg = &handle->rxConfig;
        otherDir = SRTM_AudioDirRx;
    }
    else
    {
        thisRtm  = &handle->rxRtm;
        otherRtm = &handle->txRtm;
        thisCfg  = &handle->rxConfig;
        otherCfg = &handle->txConfig;
        otherDir = SRTM_AudioDirTx;
    }

    if (thisRtm->state == SRTM_AudioStateClosed)
    {
        /* Stop may called when audio service reset. */
        return SRTM_Status_Success;
    }

    if (thisRtm->state == SRTM_AudioStateStarted)
    {
        if (dir == SRTM_AudioDirTx)
        {
            SAI_TransferTerminateSendEDMA(handle->sai, &thisRtm->saiHandle);
        }
        else
        {
            SAI_TransferTerminateReceiveEDMA(handle->sai, &thisRtm->saiHandle);
        }

        if (otherCfg->config.syncMode == kSAI_ModeSync)
        {
            /* The other direction in sync mode, it will deinitialize this direction when it's stopped. */
            if (otherRtm->state != SRTM_AudioStateStarted)
            {
                /* The other direction not started, we can deinitialize this direction. */
                SRTM_SaiEdmaAdapter_DeinitSAI(handle, dir);
            }
        }
        else
        {
            /* The other direction has dedicated clock, its stop will not affect this direction.
               Do deinitialization by ourselves. */
            SRTM_SaiEdmaAdapter_DeinitSAI(handle, dir);
            if (thisCfg->config.syncMode == kSAI_ModeSync && otherRtm->state != SRTM_AudioStateStarted)
            {
                /* This direction in sync mode and the other not started, need to deinitialize the other direction. */
                SRTM_SaiEdmaAdapter_DeinitSAI(handle, otherDir);
            }
        }

        if (otherRtm->state != SRTM_AudioStateStarted)
        {
            /* If both Tx and Rx are not running, we can deinitialize this SAI instance. */
            SAI_Deinit(handle->sai);
        }
    }

    thisRtm->bufRtm.remainingPeriods = thisRtm->bufRtm.remainingLoadPeriods = 0UL;
    if (!thisRtm->freeRun)
    {
        thisRtm->readyIdx = thisRtm->bufRtm.leadIdx;
        thisRtm->freeRun  = stop; /* Reset to freeRun if stopped. */
    }
    thisRtm->bufRtm.leadIdx = thisRtm->bufRtm.chaseIdx;

    thisRtm->state = SRTM_AudioStateOpened;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_Stop(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    return SRTM_SaiEdmaAdapter_End(adapter, dir, index, true);
}

static srtm_status_t SRTM_SaiEdmaAdapter_Pause(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (dir == SRTM_AudioDirTx)
    {
        /* Disable request */
        SAI_TxEnableDMA(handle->sai, kSAI_FIFORequestDMAEnable, false);
        /* Disable SAI */
        SAI_TxEnable(handle->sai, false);
    }
    else
    {
        /* Disable request*/
        SAI_RxEnableDMA(handle->sai, kSAI_FIFORequestDMAEnable, false);
        /* Disable SAI */
        SAI_RxEnable(handle->sai, false);
    }

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_Restart(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (dir == SRTM_AudioDirTx)
    {
        /* Enable request */
        SAI_TxEnableDMA(handle->sai, kSAI_FIFORequestDMAEnable, true);
        /* Enable SAI */
        SAI_TxEnable(handle->sai, true);
    }
    else
    {
        /* Enable request*/
        SAI_RxEnableDMA(handle->sai, kSAI_FIFORequestDMAEnable, true);
        /* Enable SAI */
        SAI_RxEnable(handle->sai, true);
    }

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_SetParam(
    srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index, uint8_t format, uint8_t channels, uint32_t srate)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t rtm    = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d. fmt %d, ch %d, srate %d\r\n", __func__, saiDirection[dir],
                       index, format, channels, srate);

    if (rtm->state != SRTM_AudioStateOpened)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: %s in wrong state %d!\r\n", __func__, saiDirection[dir],
                           rtm->state);
        return SRTM_Status_InvalidState;
    }

    if (format >= ARRAY_SIZE(saiBitsWidthMap) || channels >= ARRAY_SIZE(saiChannelMap))
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: %s unsupported format or channels %d, %d!\r\n", __func__,
                           saiDirection[dir], format, channels);
        return SRTM_Status_InvalidParameter;
    }

    rtm->bitWidth = saiBitsWidthMap[format];
    rtm->channels = saiChannelMap[channels];
    rtm->srate    = srate;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_SetBuf(srtm_sai_adapter_t adapter,
                                                srtm_audio_dir_t dir,
                                                uint8_t index,
                                                uint8_t *bufAddr,
                                                uint32_t bufSize,
                                                uint32_t periodSize,
                                                uint32_t periodIdx)
{
    srtm_sai_edma_adapter_t handle     = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t rtm        = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;
    srtm_sai_edma_buf_runtime_t bufRtm = &rtm->bufRtm;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d. buf [0x%x, 0x%x]; prd size 0x%x, idx %d\r\n", __func__,
                       saiDirection[dir], index, bufAddr, bufSize, periodSize, periodIdx);

    if (rtm->state != SRTM_AudioStateOpened)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: %s in wrong state %d!\r\n", __func__, saiDirection[dir],
                           rtm->state);
        return SRTM_Status_InvalidState;
    }

    rtm->bufAddr    = bufAddr;
    rtm->periodSize = periodSize;
    rtm->periods    = (periodSize != 0U) ? bufSize / periodSize : 0U;
    rtm->bufSize    = periodSize * rtm->periods;

    assert(periodIdx < rtm->periods);

    bufRtm->chaseIdx = periodIdx;
    bufRtm->leadIdx  = periodIdx;

    bufRtm->remainingPeriods = bufRtm->remainingLoadPeriods = 0UL;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_Suspend(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_status_t status           = SRTM_Status_Success;
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t thisRtm;
    srtm_sai_edma_config_t *thisCfg;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (dir == SRTM_AudioDirTx)
    {
        thisRtm = &handle->txRtm;
        thisCfg = &handle->txConfig;
    }
    else
    {
        thisRtm = &handle->rxRtm;
        thisCfg = &handle->rxConfig;
    }

    if (thisRtm->state == SRTM_AudioStateStarted && thisCfg->stopOnSuspend)
    {
        status = SRTM_SaiEdmaAdapter_End(adapter, dir, index, false);
    }

    return status;
}

static srtm_status_t SRTM_SaiEdmaAdapter_Resume(srtm_sai_adapter_t adapter, srtm_audio_dir_t dir, uint8_t index)
{
    srtm_status_t status           = SRTM_Status_Success;
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t thisRtm;
    srtm_sai_edma_config_t *thisCfg;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    if (dir == SRTM_AudioDirTx)
    {
        thisRtm = &handle->txRtm;
        thisCfg = &handle->txConfig;
    }
    else
    {
        thisRtm = &handle->rxRtm;
        thisCfg = &handle->rxConfig;
    }

    if (thisRtm->state == SRTM_AudioStateStarted && thisCfg->stopOnSuspend)
    {
        status = SRTM_SaiEdmaAdapter_Start(adapter, dir, index);
    }

    return status;
}

static srtm_status_t SRTM_SaiEdmaAdapter_GetBufOffset(srtm_sai_adapter_t adapter,
                                                      srtm_audio_dir_t dir,
                                                      uint8_t index,
                                                      uint32_t *pOffset)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t rtm    = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d\r\n", __func__, saiDirection[dir], index);

    *pOffset = rtm->finishedBufOffset;

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SaiEdmaAdapter_PeriodReady(srtm_sai_adapter_t adapter,
                                                     srtm_audio_dir_t dir,
                                                     uint8_t index,
                                                     uint32_t periodIdx)
{
    srtm_status_t status           = SRTM_Status_Success;
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;
    srtm_sai_edma_runtime_t rtm    = dir == SRTM_AudioDirTx ? &handle->txRtm : &handle->rxRtm;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %s%d - period %d\r\n", __func__, saiDirection[dir], index,
                       periodIdx);

    if (rtm->state == SRTM_AudioStateStarted)
    {
        if (dir == SRTM_AudioDirTx)
        {
            SRTM_SaiEdmaAdapter_AddNewPeriods(rtm, periodIdx);
            /* Add buffer to DMA scatter-gather list if there's remaining buffer to send.
               Needed in case buffer xflow */
            SRTM_SaiEdmaAdapter_Transfer(handle, dir);
        }
    }
    else
    {
        rtm->freeRun  = false;
        rtm->readyIdx = periodIdx;
    }

    return status;
}

srtm_sai_adapter_t SRTM_SaiEdmaAdapter_Create(I2S_Type *sai,
                                              DMA_Type *dma,
                                              srtm_sai_edma_config_t *txConfig,
                                              srtm_sai_edma_config_t *rxConfig)
{
    srtm_sai_edma_adapter_t handle;

    assert((sai != NULL) && (dma != NULL));

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_sai_edma_adapter_t)SRTM_Heap_Malloc(sizeof(struct _srtm_sai_edma_adapter));
    assert(handle);
    (void)memset(handle, 0, sizeof(struct _srtm_sai_edma_adapter));

    handle->sai = sai;
    handle->dma = dma;
    if (txConfig != NULL)
    {
        (void)memcpy(&handle->txConfig, txConfig, sizeof(srtm_sai_edma_config_t));
        handle->txRtm.proc = SRTM_Procedure_Create(SRTM_SaiEdmaAdapter_TxTransferProc, handle, NULL);
        assert(handle->txRtm.proc);
        SRTM_Message_SetFreeFunc(handle->txRtm.proc, SRTM_SaiEdmaAdapter_RecycleTxMessage, handle);
    }
    if (rxConfig != NULL)
    {
        (void)memcpy(&handle->rxConfig, rxConfig, sizeof(srtm_sai_edma_config_t));
        handle->rxRtm.proc = SRTM_Procedure_Create(SRTM_SaiEdmaAdapter_RxTransferProc, handle, NULL);
        assert(handle->rxRtm.proc);
        SRTM_Message_SetFreeFunc(handle->rxRtm.proc, SRTM_SaiEdmaAdapter_RecycleRxMessage, handle);
    }

    /* Adapter interfaces. */
    handle->adapter.open         = SRTM_SaiEdmaAdapter_Open;
    handle->adapter.start        = SRTM_SaiEdmaAdapter_Start;
    handle->adapter.pause        = SRTM_SaiEdmaAdapter_Pause;
    handle->adapter.restart      = SRTM_SaiEdmaAdapter_Restart;
    handle->adapter.stop         = SRTM_SaiEdmaAdapter_Stop;
    handle->adapter.close        = SRTM_SaiEdmaAdapter_Close;
    handle->adapter.setParam     = SRTM_SaiEdmaAdapter_SetParam;
    handle->adapter.setBuf       = SRTM_SaiEdmaAdapter_SetBuf;
    handle->adapter.suspend      = SRTM_SaiEdmaAdapter_Suspend;
    handle->adapter.resume       = SRTM_SaiEdmaAdapter_Resume;
    handle->adapter.getBufOffset = SRTM_SaiEdmaAdapter_GetBufOffset;
    handle->adapter.periodReady  = SRTM_SaiEdmaAdapter_PeriodReady;

    return &handle->adapter;
}

void SRTM_SaiEdmaAdapter_Destroy(srtm_sai_adapter_t adapter)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)(void *)adapter;

    assert(adapter);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    if (handle->txRtm.proc != NULL)
    {
        SRTM_Message_SetFreeFunc(handle->txRtm.proc, NULL, NULL);
        SRTM_Procedure_Destroy(handle->txRtm.proc);
    }

    if (handle->rxRtm.proc != NULL)
    {
        SRTM_Message_SetFreeFunc(handle->rxRtm.proc, NULL, NULL);
        SRTM_Procedure_Destroy(handle->rxRtm.proc);
    }

    SRTM_Heap_Free(handle);
}

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
void SRTM_SaiEdmaAdapter_SetTxLocalBuf(srtm_sai_adapter_t adapter, srtm_sai_edma_local_buf_t *localBuf)
{
    srtm_sai_edma_adapter_t handle = (srtm_sai_edma_adapter_t)adapter;

    assert(adapter);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    if (localBuf)
    {
        assert(localBuf->periods <= SRTM_SAI_EDMA_MAX_LOCAL_BUF_PERIODS);
        memcpy(&handle->txRtm.localBuf, localBuf, sizeof(srtm_sai_edma_local_buf_t));
    }
    else
    {
        handle->txRtm.localBuf.buf = NULL;
    }
}
#endif
