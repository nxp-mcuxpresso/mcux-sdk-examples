/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_spdif.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPDIF          SPDIF
#define DEMO_SPDIF_CLOCK_FREQ  CLOCK_GetPllFreq(kCLOCK_Pll_AudioPll)
#define DEMO_SPDIF_SAMPLE_RATE 48000
#define BUFFER_SIZE 768
#define BUFFER_NUM  4
#define PLAY_COUNT  5000
#define SAMPLE_RATE 48000
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};
spdif_handle_t txHandle           = {0};
spdif_handle_t rxHandle           = {0};
static volatile bool isTxFinished = false;
static volatile bool isRxFinished = false;
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t audioBuff[BUFFER_SIZE * BUFFER_NUM], 4);
static volatile uint32_t beginCount   = 0;
static volatile uint32_t sendCount    = 0;
static volatile uint8_t emptyBlock    = BUFFER_NUM;
static volatile uint32_t receiveCount = 0;
static spdif_transfer_t txXfer        = {0};
static spdif_transfer_t rxXfer        = {0};

/*******************************************************************************
 * Code
 ******************************************************************************/
static void txCallback(SPDIF_Type *base, spdif_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_SPDIF_TxIdle)
    {
        sendCount++;
        emptyBlock++;

        if (sendCount == beginCount)
        {
            isTxFinished = true;
            SPDIF_TransferAbortSend(base, handle);
        }
    }
}

static void rxCallback(SPDIF_Type *base, spdif_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_SPDIF_RxCnew)
    {
        /* Copy C channel contents to tx */
        base->STCSCH = base->SRCSH;
        base->STCSCL = base->SRCSL;
    }

    if (status == kStatus_SPDIF_RxIdle)
    {
        emptyBlock--;
        receiveCount++;

        if (receiveCount == beginCount)
        {
            isRxFinished = true;
            SPDIF_TransferAbortReceive(base, handle);
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    spdif_config_t config;
    uint32_t sourceClock = 0;
    uint8_t txIndex = 0, rxIndex = 0;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_InitAudioPll(&audioPllConfig);

    /*Clock setting for SPDIF*/
    CLOCK_SetRootClockMux(kCLOCK_Root_Spdif, 4);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Spdif, 1);

    PRINTF("SPDIF example started!\n\r");

    SPDIF_GetDefaultConfig(&config);
    SPDIF_Init(EXAMPLE_SPDIF, &config);

    sourceClock = CLOCK_GetPllFreq(kCLOCK_PllAudio);
    SPDIF_TxSetSampleRate(EXAMPLE_SPDIF, SAMPLE_RATE, sourceClock);

    SPDIF_TransferTxCreateHandle(EXAMPLE_SPDIF, &txHandle, txCallback, NULL);
    SPDIF_TransferRxCreateHandle(EXAMPLE_SPDIF, &rxHandle, rxCallback, NULL);

    txXfer.dataSize = BUFFER_SIZE;
    rxXfer.dataSize = BUFFER_SIZE;

    beginCount = PLAY_COUNT;

    /* Wait until finished */
    while ((isTxFinished != true) || (isRxFinished != true))
    {
        if ((isTxFinished != true) && (emptyBlock < BUFFER_NUM))
        {
            txXfer.data = audioBuff + txIndex * BUFFER_SIZE;
            if (kStatus_Success == SPDIF_TransferSendNonBlocking(EXAMPLE_SPDIF, &txHandle, &txXfer))
            {
                txIndex = (txIndex + 1) % BUFFER_NUM;
            }
        }

        if ((isRxFinished != true) && (emptyBlock > 0))
        {
            rxXfer.data = audioBuff + rxIndex * BUFFER_SIZE;
            if (kStatus_Success == SPDIF_TransferReceiveNonBlocking(EXAMPLE_SPDIF, &rxHandle, &rxXfer))
            {
                rxIndex = (rxIndex + 1) % BUFFER_NUM;
            }
        }
    }

    PRINTF("\n\r SPDIF example finished!\n\r ");
    while (1)
    {
    }
}
