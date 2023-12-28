/*
 * Copyright 2019 NXP
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
static float32_t do_fft(
    uint32_t sampleRate, uint32_t bitWidth, uint8_t *buffer, float32_t *fftData, float32_t *fftResult);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static float32_t ffData[2 * BUFFER_SIZE];
static float32_t ffResult[BUFFER_SIZE];
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
/*******************************************************************************
 * Code
 ******************************************************************************/
static float32_t do_fft(
    uint32_t sampleRate, uint32_t bitWidth, uint8_t *buffer, float32_t *fftData, float32_t *fftResult)
{
    /* Counter variable for navigating buffers */
    uint32_t counter;

    /* Return value for wav frequency in hertz */
    float32_t wavFreqHz;

    /* CMSIS status & FFT instance */
    arm_status status;                /* ARM status variable */
    arm_cfft_radix2_instance_f32 fft; /* ARM FFT instance */

    /* Frequency analysis variables */
    float32_t maxValue;           /* max value for greatest FFT bin amplitude */
    uint32_t testIndex       = 0; /* value for storing the bin location with maxValue */
    uint32_t complexBuffSize = BUFFER_SIZE * 2;
    uint32_t fftSize         = BUFFER_SIZE; /* FFT bin size */
    uint32_t ifftFlag        = 0;           /* Flag for the selection of CFFT/CIFFT */
    uint32_t doBitReverse    = 1;           /* Flag for selection of normal order or bit reversed order */
    float32_t hzPerBin       = 2 * ((float32_t)sampleRate / (float32_t)fftSize); /* Calculate hz per FFT bin */

    uint8_t *temp8; /* Point to data for 8 bit samples */
    uint8_t temp8Data;

    uint16_t *temp16; /* Point to data for 16 bit samples */
    int16_t temp16Data;

    uint32_t *temp32; /* Point to data for 32 bit samples */
    int32_t temp32Data;

    /* Set status as success */
    status = ARM_MATH_SUCCESS;

    /* Wav data variables */
    switch (bitWidth)
    {
        case 8:
            temp8     = (uint8_t *)buffer;
            temp8Data = 0;

            /* Copy wav data to fft input array */
            for (counter = 0; counter < complexBuffSize; counter++)
            {
                if (counter % 2 == 0)
                {
                    temp8Data        = (uint8_t)*temp8;
                    fftData[counter] = (float32_t)temp8Data;
                    temp8++;
                }
                else
                {
                    fftData[counter] = 0.0;
                }
            }

            /* Set instance for Real FFT */
            status = arm_cfft_radix2_init_f32(&fft, fftSize, ifftFlag, doBitReverse);

            /* Perform Real FFT on fftData */
            arm_cfft_radix2_f32(&fft, fftData);

            /* Populate FFT bins */
            arm_cmplx_mag_f32(fftData, fftResult, fftSize);

            /* Zero out non-audible, low-frequency noise from FFT Results. */
            fftResult[0] = 0.0;

            /* Find max bin and location of max (first half of bins as this is the only valid section) */
            arm_max_f32(fftResult, fftSize, &maxValue, &testIndex);

            break;

        case 16:
            temp16     = (uint16_t *)buffer;
            temp16Data = 0;

            /* Copy wav data to fft input array */
            for (counter = 0; counter < complexBuffSize; counter++)
            {
                if (counter % 2 == 0)
                {
                    temp16Data       = (int16_t)*temp16;
                    fftData[counter] = (float32_t)temp16Data;
                    temp16++;
                }
                else
                {
                    fftData[counter] = 0.0;
                }
            }

            /* Set instance for Real FFT */
            status = arm_cfft_radix2_init_f32(&fft, fftSize, ifftFlag, doBitReverse);

            /* Perform Real FFT on fftData */
            arm_cfft_radix2_f32(&fft, fftData);

            /* Populate FFT bins */
            arm_cmplx_mag_f32(fftData, fftResult, fftSize);

            /* Zero out non-audible, low-frequency noise from FFT Results. */
            fftResult[0] = 0.0;

            /* Find max bin and location of max (first half of bins as this is the only valid section) */
            arm_max_f32(fftResult, fftSize, &maxValue, &testIndex);

            break;

        case 32:
            temp32     = (uint32_t *)buffer;
            temp32Data = 0;

            /* Copy wav data to fft input array */
            for (counter = 0; counter < complexBuffSize; counter++)
            {
                if (counter % 2 == 0)
                {
                    temp32Data       = (int32_t)*temp32;
                    fftData[counter] = (float32_t)temp32Data;
                    temp32++;
                }
                else
                {
                    fftData[counter] = 0.0;
                }
            }

            /* Set instance for Real FFT */
            status = arm_cfft_radix2_init_f32(&fft, fftSize, ifftFlag, doBitReverse);

            /* Perform Real FFT on fftData */
            arm_cfft_radix2_f32(&fft, fftData);

            /* Populate FFT bins */
            arm_cmplx_mag_f32(fftData, fftResult, fftSize);

            /* Zero out non-audible, low-frequency noise from FFT Results. */
            fftResult[0] = 0.0;

            /* Find max bin and location of max (first half of bins as this is the only valid section) */
            arm_max_f32(fftResult, fftSize, &maxValue, &testIndex);

            break;

        default:
            __asm("NOP");
            break;
    }

    if (status != ARM_MATH_SUCCESS)
    {
        wavFreqHz = 0; /* If an error has occurred set frequency of wav data to 0Hz */
        PRINTF("\r\nFFT compuation error.\r\n");
    }
    else
    {
        /* Set wavFreqHz to bin location of max amplitude multiplied by the hz per bin */
        wavFreqHz = testIndex * hzPerBin;
    }

    return wavFreqHz;
}

void PlaybackSine(I2S_Type *base, uint32_t SineWaveFreqHz, uint32_t time_s)
{
    uint32_t count      = DEMO_AUDIO_SAMPLE_RATE / SineWaveFreqHz;
    uint32_t i          = 0;
    uint32_t val        = 0;
    sai_transfer_t xfer = {0};
    float32_t freq      = 0;
    uint32_t totalNum = 0, index = 0;

    /* Clear the status */
    istxFinished = false;
    sendCount    = 0;
    emptyBlock   = BUFFER_NUM;

    /* Gnerate the sine wave data */
    for (i = 0; i < count; i++)
    {
        val                   = arm_sin_q15(0x8000 * i / count);
        audioBuff[4 * i]      = val & 0xFFU;
        audioBuff[4 * i + 1U] = (val >> 8U) & 0xFFU;
        audioBuff[4 * i + 2U] = val & 0xFFU;
        audioBuff[4 * i + 3U] = (val >> 8U) & 0xFFU;
    }

    /* Repeat the cycle */
    for (i = 1; i < (BUFFER_SIZE * BUFFER_NUM) / (4 * count); i++)
    {
        memcpy(audioBuff + (i * 4 * count), audioBuff, (4 * count));
    }

    /* Send times according to the time need to playback */
    beginCount = DEMO_AUDIO_SAMPLE_RATE * time_s * 4 / BUFFER_SIZE;

    /* Compute the frequency of the data using FFT */
    freq = do_fft(DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH, audioBuff, ffData, ffResult);

    PRINTF("\r\n Data frequency is %f\r\n", freq);

    /* Reset SAI Tx internal logic */
    SAI_TxSoftwareReset(base, kSAI_ResetTypeSoftware);
    /* Do the playback */
    xfer.data     = audioBuff;
    xfer.dataSize = BUFFER_SIZE;

    while (totalNum < beginCount)
    {
        /* Transfer data already prepared, so while there is any
        empty slot, just transfer */
        if (emptyBlock > 0)
        {
            xfer.data = audioBuff + index * BUFFER_SIZE;
            /* Shall make sure the sai buffer queue is not full */
            if (SAI_TransferSendEDMA(base, &txHandle, &xfer) == kStatus_Success)
            {
                index = (index + 1U) % BUFFER_NUM;
                totalNum++;
                emptyBlock--;
            }
        }
    }

    /* Wait for the send finished */
    while (istxFinished != true)
    {
    }
}
