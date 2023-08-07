/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "board.h"
#include "demo_config.h"

#include "fsl_wm8904.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_i2s.h"

#include "music.h"

#include "task.h"
#include "event_groups.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AUDIO_SAMPLING_RATE_KHZ (48)
#define AUDIO_SAMPLING_RATE     (AUDIO_SAMPLING_RATE_KHZ * 1000)
#define AUDIO_FORMAT_CHANNELS   (0x02)
#define AUDIO_FORMAT_BITS       (16)
#define AUDIO_FORMAT_SIZE       (0x02)

/*******************************************************************************
 * Variables
 ******************************************************************************/
__ALIGN_BEGIN static uint8_t s_Buffer[400] __ALIGN_END; /* 100 samples => time about 2 ms */
static i2s_config_t s_TxConfig;
static i2s_config_t s_RxConfig;
static i2s_transfer_t s_TxTransfer;
static i2s_transfer_t s_RxTransfer;
static i2s_handle_t s_TxHandle;
static i2s_handle_t s_RxHandle;

extern EventGroupHandle_t g_errorEvent;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_InitCodec(void);

static void TxCallback(I2S_Type *base, i2s_handle_t *handle, status_t completionStatus, void *userData);
static void RxCallback(I2S_Type *base, i2s_handle_t *handle, status_t completionStatus, void *userData);
/*******************************************************************************
 * Code
 ******************************************************************************/

void audio_init(void)
{
    BOARD_InitCodec();

    if (xEventGroupGetBits(g_errorEvent) == 0U)
    {
        /*
         * masterSlave = kI2S_MasterSlaveNormalMaster;
         * mode = kI2S_ModeI2sClassic;
         * rightLow = false;
         * leftJust = false;
         * pdmData = false;
         * sckPol = false;
         * wsPol = false;
         * divider = 1;
         * oneChannel = false;
         * dataLength = 16;
         * frameLength = 32;
         * position = 0;
         * watermark = 4;
         * txEmptyZero = true;
         * pack48 = false;
         */
        I2S_TxGetDefaultConfig(&s_TxConfig);
        s_TxConfig.divider     = DEMO_I2S_CLOCK_DIVIDER;
        s_TxConfig.masterSlave = DEMO_I2S_TX_MODE;
        I2S_TxInit(DEMO_I2S_TX, &s_TxConfig);

        I2S_RxGetDefaultConfig(&s_RxConfig);
        s_TxConfig.divider     = DEMO_I2S_CLOCK_DIVIDER;
        s_RxConfig.masterSlave = DEMO_I2S_RX_MODE;
        I2S_RxInit(DEMO_I2S_RX, &s_RxConfig);

        PRINTF("AUDIO Loopback started!\r\n");
        PRINTF("Headphones will play what is input into Audio Line-In connector.\r\n");

        s_RxTransfer.data     = &s_Buffer[0];
        s_RxTransfer.dataSize = sizeof(s_Buffer);
        s_TxTransfer.data     = &s_Buffer[0];
        s_TxTransfer.dataSize = sizeof(s_Buffer);

        I2S_TxTransferCreateHandle(DEMO_I2S_TX, &s_TxHandle, TxCallback, (void *)&s_TxTransfer);
        I2S_RxTransferCreateHandle(DEMO_I2S_RX, &s_RxHandle, RxCallback, (void *)&s_RxTransfer);

        I2S_RxTransferNonBlocking(DEMO_I2S_RX, &s_RxHandle, s_RxTransfer);
        I2S_TxTransferNonBlocking(DEMO_I2S_TX, &s_TxHandle, s_TxTransfer);
    }
}

static void TxCallback(I2S_Type *base, i2s_handle_t *handle, status_t completionStatus, void *userData)
{
    /* Enqueue the same original s_Buffer all over again */
    i2s_transfer_t *transfer = (i2s_transfer_t *)userData;
    I2S_TxTransferNonBlocking(base, handle, *transfer);
}

static void RxCallback(I2S_Type *base, i2s_handle_t *handle, status_t completionStatus, void *userData)
{
    /* Enqueue the same original s_Buffer all over again */
    i2s_transfer_t *transfer = (i2s_transfer_t *)userData;
    I2S_RxTransferNonBlocking(base, handle, *transfer);
}
