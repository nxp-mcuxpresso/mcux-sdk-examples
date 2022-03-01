/*
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sai.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern sai_edma_handle_t txHandle;
extern sai_edma_handle_t rxHandle;
extern uint8_t audioBuff[BUFFER_SIZE * BUFFER_NUM];
extern volatile bool istxFinished;
extern volatile bool isrxFinished;
extern volatile uint32_t beginCount;
extern volatile uint32_t sendCount;
extern volatile uint32_t receiveCount;
extern volatile uint32_t fullBlock;
extern volatile uint32_t emptyBlock;
/*******************************************************************************
 * Code
 ******************************************************************************/
void RecordPlayback(I2S_Type *base, uint32_t time_s)
{
    sai_transfer_t xfer    = {0};
    uint32_t playbackCount = 0, recordCount = 0;
    uint32_t txindex = 0, rxindex = 0;

    /* First clear the buffer */
    memset(audioBuff, 0, BUFFER_SIZE * BUFFER_NUM);
    istxFinished = false;
    isrxFinished = false;
    sendCount    = 0;
    receiveCount = 0;

    /* Reset SAI internal logic */
    SAI_TxSoftwareReset(base, kSAI_ResetTypeSoftware);
    SAI_RxSoftwareReset(base, kSAI_ResetTypeSoftware);

    /* Compute the begin count */
    beginCount = time_s * DEMO_AUDIO_SAMPLE_RATE * 4u / BUFFER_SIZE;

    xfer.dataSize = BUFFER_SIZE;
    /* Wait for playback finished */
    while ((recordCount < beginCount) || (playbackCount < beginCount))
    {
        if ((emptyBlock > 0) && (recordCount < beginCount))
        {
            xfer.data = audioBuff + rxindex * BUFFER_SIZE;
            if (SAI_TransferReceiveEDMA(base, &rxHandle, &xfer) == kStatus_Success)
            {
                rxindex = (rxindex + 1U) % BUFFER_NUM;
                emptyBlock--;
                recordCount++;
            }
        }

        if ((fullBlock > 0) && (playbackCount < beginCount))
        {
            xfer.data = audioBuff + txindex * BUFFER_SIZE;
            if (SAI_TransferSendEDMA(base, &txHandle, &xfer) == kStatus_Success)
            {
                txindex = (txindex + 1U) % BUFFER_NUM;
                fullBlock--;
                playbackCount++;
            }
        }
    }

    /* Wait for record and playback finished */
    while ((istxFinished != true) || (isrxFinished != true))
    {
    }
}
