/*
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ff.h"
#include "diskio.h"
#include "fsl_sd.h"
#include "sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void wav_header(uint8_t *header, uint32_t sampleRate, uint32_t bitsPerFrame, uint8_t channels, uint32_t fileSize);
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
extern bool sdcard;
extern volatile uint32_t fullBlock;
extern volatile uint32_t emptyBlock;
extern FIL g_fileObject;
/*******************************************************************************
 * Code
 ******************************************************************************/
void wav_header(uint8_t *header, uint32_t sampleRate, uint32_t bitsPerFrame, uint8_t channels, uint32_t fileSize)
{
    uint32_t totalDataLen = fileSize - 8U;
    uint32_t audioDataLen = fileSize - 44U;
    uint32_t byteRate     = sampleRate * (bitsPerFrame / 8U) * channels;
    header[0]             = 'R';
    header[1]             = 'I';
    header[2]             = 'F';
    header[3]             = 'F';
    header[4]             = (totalDataLen & 0xff); /* file-size (equals file-size - 8) */
    header[5]             = ((totalDataLen >> 8U) & 0xff);
    header[6]             = ((totalDataLen >> 16U) & 0xff);
    header[7]             = ((totalDataLen >> 24U) & 0xff);
    header[8]             = 'W'; /* Mark it as type "WAVE" */
    header[9]             = 'A';
    header[10]            = 'V';
    header[11]            = 'E';
    header[12]            = 'f'; /* Mark the format section 'fmt ' chunk */
    header[13]            = 'm';
    header[14]            = 't';
    header[15]            = ' ';
    header[16]            = 16; /* 4 bytes: size of 'fmt ' chunk, Length of format data.  Always 16 */
    header[17]            = 0;
    header[18]            = 0;
    header[19]            = 0;
    header[20]            = 1; /* format = 1 ,Wave type PCM */
    header[21]            = 0;
    header[22]            = channels; /* channels */
    header[23]            = 0;
    header[24]            = (sampleRate & 0xff);
    header[25]            = ((sampleRate >> 8U) & 0xff);
    header[26]            = ((sampleRate >> 16U) & 0xff);
    header[27]            = ((sampleRate >> 24U) & 0xff);
    header[28]            = (byteRate & 0xff);
    header[29]            = ((byteRate >> 8U) & 0xff);
    header[30]            = ((byteRate >> 16U) & 0xff);
    header[31]            = ((byteRate >> 24U) & 0xff);
    header[32]            = (channels * bitsPerFrame / 8); /* block align */
    header[33]            = 0;
    header[34]            = bitsPerFrame; /* bits per sample */
    header[35]            = 0;
    header[36]            = 'd'; /*"data" marker */
    header[37]            = 'a';
    header[38]            = 't';
    header[39]            = 'a';
    header[40]            = (audioDataLen & 0xff); /* data-size (equals file-size - 44).*/
    header[41]            = ((audioDataLen >> 8) & 0xff);
    header[42]            = ((audioDataLen >> 16) & 0xff);
    header[43]            = ((audioDataLen >> 24) & 0xff);
}

void RecordSDCard(I2S_Type *base, uint32_t time_s)
{
    uint32_t i            = 0;
    uint32_t bytesWritten = 0;
    uint32_t bytesRead    = 0;
    uint32_t txindex      = 0;
    uint32_t rxindex      = 0;
    uint32_t sdReadCount  = 0;
    uint8_t header[44]    = {0};
    uint32_t fileSize     = time_s * DEMO_AUDIO_SAMPLE_RATE * 2U * 2U + 44U;
    FRESULT error;
    sai_transfer_t xfer                = {0};
    static const TCHAR wavpathBuffer[] = DEMO_RECORD_WAV_PATH;

    /* Clear the status */
    isrxFinished = false;
    receiveCount = 0;
    istxFinished = false;
    sendCount    = 0;
    sdcard       = true;

    PRINTF("\r\nBegin to record......\r\n");
    PRINTF("\r\nFile path is record/music1.wav\r\n");
    error = f_open(&g_fileObject, (char const *)wavpathBuffer, (FA_WRITE | FA_READ | FA_CREATE_ALWAYS));
    if (error)
    {
        if (error == FR_EXIST)
        {
            PRINTF("File exists.\r\n");
        }
        else
        {
            PRINTF("Open file failed.\r\n");
            return;
        }
    }

    /* Write the data into the sdcard */
    wav_header(header, DEMO_AUDIO_SAMPLE_RATE, 16, 2, fileSize);

    /* Write the wav header */
    error = f_write(&g_fileObject, (void *)header, 44U, (UINT *)&bytesWritten);
    if ((error) || (bytesWritten != 44))
    {
        PRINTF("Write file failed. \r\n");
    }

    /* Reset SAI internal logic */
    SAI_TxSoftwareReset(base, kSAI_ResetTypeSoftware);
    SAI_RxSoftwareReset(base, kSAI_ResetTypeSoftware);

    /* Start to record */
    beginCount = time_s * DEMO_AUDIO_SAMPLE_RATE * 2U * 2U / BUFFER_SIZE;

    /* Start record first */
    memset(audioBuff, 0, BUFFER_SIZE * BUFFER_NUM);
    xfer.dataSize = BUFFER_SIZE;
    for (i = 0; i < BUFFER_NUM; i++)
    {
        xfer.data = audioBuff + i * BUFFER_SIZE;
        SAI_TransferReceiveEDMA(base, &rxHandle, &xfer);
    }

    emptyBlock = 0;
    while ((isrxFinished != true) || (fullBlock != 0))
    {
        if (fullBlock > 0)
        {
            error = f_write(&g_fileObject, audioBuff + txindex * BUFFER_SIZE, BUFFER_SIZE, (UINT *)&bytesWritten);
            if ((error) || (bytesWritten != BUFFER_SIZE))
            {
                PRINTF("Write file failed. \r\n");
                return;
            }

            txindex = (txindex + 1U) % BUFFER_NUM;
            fullBlock--;
            emptyBlock++;
        }

        if ((emptyBlock > 0) && (isrxFinished == false))
        {
            xfer.data = audioBuff + rxindex * BUFFER_SIZE;
            rxindex   = (rxindex + 1U) % BUFFER_NUM;
            SAI_TransferReceiveEDMA(base, &rxHandle, &xfer);
            emptyBlock--;
        }
    }
    f_close(&g_fileObject);
    PRINTF("\r\nRecord is finished!\r\n");

    /* Playback the record file */
    PRINTF("\r\nPlayback the recorded file...");
    txindex    = 0;
    rxindex    = 0;
    emptyBlock = 0;
    fullBlock  = 0;
    memset(audioBuff, 0, BUFFER_SIZE * BUFFER_NUM);
    f_open(&g_fileObject, (char const *)wavpathBuffer, (FA_READ));
    if (f_lseek(&g_fileObject, 44U))
    {
        PRINTF("Set file pointer position failed. \r\n");
    }

    for (i = 0; i < BUFFER_NUM; i++)
    {
        error = f_read(&g_fileObject, (void *)(audioBuff + i * BUFFER_SIZE), BUFFER_SIZE, (UINT *)&bytesRead);
        if ((error) || (bytesRead != BUFFER_SIZE))
        {
            PRINTF("Read file failed. \r\n");
            return;
        }

        sdReadCount++;
        fullBlock++;
    }

    /* Wait for playback finished */
    while (istxFinished != true)
    {
        if ((emptyBlock > 0) && (sdReadCount < beginCount))
        {
            error = f_read(&g_fileObject, (void *)(audioBuff + rxindex * BUFFER_SIZE), BUFFER_SIZE, (UINT *)&bytesRead);
            if ((error) || (bytesRead != BUFFER_SIZE))
            {
                PRINTF("Read file failed. \r\n");
                return;
            }

            rxindex = (rxindex + 1U) % BUFFER_NUM;
            emptyBlock--;
            fullBlock++;
            sdReadCount++;
        }

        if (fullBlock > 0)
        {
            xfer.data = audioBuff + txindex * BUFFER_SIZE;
            txindex   = (txindex + 1U) % BUFFER_NUM;
            SAI_TransferSendEDMA(base, &txHandle, &xfer);
            fullBlock--;
        }
    }
    f_close(&g_fileObject);
    PRINTF("\r\nPlayback is finished!\r\n");
}
