/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_sd.h"
#include "ff.h"
#include "diskio.h"
#include "ffconf.h"
#include "fsl_sd_disk.h"
#include "fsl_codec_common.h"
#include "sdmmc_config.h"
#include <stdbool.h>
#include "fsl_codec_adapter.h"
#include "fsl_cs42448.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_I2S_CLOCK_DIVIDER          (24576000 / DEMO_AUDIO_SAMPLE_RATE / 32 / 8)
#define DEMO_AUDIO_BIT_WIDTH            (16)
#define DEMO_AUDIO_SAMPLE_RATE          (48000)
#define DEMO_AUDIO_PROTOCOL             kCODEC_BusI2S
#define DEMO_I2S_TX                     (I2S3)
#define DEMO_DMA                        (DMA0)
#define DEMO_I2S_TX_CHANNEL             (7)
#define DEMO_CODEC_I2C_BASEADDR         I2C2
#define DEMO_CODEC_I2C_INSTANCE         2U
#define DEMO_TDM_DATA_START_POSITION    1U
#define FIFO_DEPTH           (15U)
#define PLAYBACK_BUFFER_SIZE (1024)
#define PLAYBACK_BUFFER_NUM  (2U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
static status_t DEMO_MountFileSystem(void);
extern void BORAD_CodecReset(bool state);
static void DEMO_InitCodec(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};
static i2s_dma_handle_t s_i2sTxHandle;
static dma_handle_t s_i2sTxDmaHandle;
static i2s_config_t tx_config;
extern codec_config_t boardCodecConfig;
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[PLAYBACK_BUFFER_NUM * PLAYBACK_BUFFER_SIZE], 4);
volatile bool isFinished      = false;
volatile uint32_t finishIndex = 0U;
volatile uint32_t emptyBlock  = PLAYBACK_BUFFER_NUM;
/*! @brief Card descriptor. */
extern sd_card_t g_sd;
static uint32_t volatile s_writeIndex = 0U;
static uint32_t volatile s_readIndex  = 0U;
static uint32_t volatile s_emptyBlock = PLAYBACK_BUFFER_NUM;
static FATFS s_fileSystem; /* File system object */
static FIL s_fileObject;
static FILINFO s_fileInfo;
static volatile bool s_i2sTransferFinish = false;
codec_handle_t codecHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    s_i2sTransferFinish = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    i2s_transfer_t i2sTxTransfer;
    UINT oneTimeRW = 0U;
    UINT bytesRead;
    FRESULT error;
    uint32_t leftWAVData = 0U;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 1);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();
    BOARD_SD_Config(&g_sd, NULL, BOARD_SDMMC_SD_HOST_IRQ, NULL);

    PRINTF("\r\nI2S dma TDM example started.\n\r");

    /* i2s configurations */
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
     * fifoLevel = 4;
     */
    I2S_TxGetDefaultConfig(&tx_config);
    tx_config.divider     = DEMO_I2S_CLOCK_DIVIDER;
    tx_config.mode        = kI2S_ModeDspWsShort;
    tx_config.wsPol       = true;
    tx_config.dataLength  = 32U;
    tx_config.frameLength = 32 * 8U;
    tx_config.position    = DEMO_TDM_DATA_START_POSITION;

    I2S_TxInit(DEMO_I2S_TX, &tx_config);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel1, false, 64 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel2, false, 128 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel3, false, 192 + DEMO_TDM_DATA_START_POSITION);

    DMA_Init(DEMO_DMA);
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, i2s_Callback, NULL);

    if (DEMO_MountFileSystem() != kStatus_Success)
    {
        PRINTF("Mount file system failed, make sure card is formatted.\r\n");
        return -1;
    }

    /* codec initialization */
    DEMO_InitCodec();

    /* The 8_TDM.wav file process flow:
     * 1.Full fill the transfer buffer firstly, it is important to make sure the audio data is transferred continuously.
     * 2.Send one buffer block through SAI when there is at least one buffer block is full.
     * 3.read one buffer block from sdcard when there is at least one buffer block is empty and transfer done.
     * with step2/step3 repeat, audio data will be sent out continuously.
     * */
    if (s_fileInfo.fsize > PLAYBACK_BUFFER_SIZE * PLAYBACK_BUFFER_NUM)
    {
        oneTimeRW = PLAYBACK_BUFFER_SIZE * PLAYBACK_BUFFER_NUM;
    }
    else
    {
        oneTimeRW = s_fileInfo.fsize;
    }
    error = f_read(&s_fileObject, s_buffer, oneTimeRW, &bytesRead);
    if ((error) || (bytesRead != oneTimeRW))
    {
        PRINTF("Read file failed.\r\n");
        return -1;
    }
    s_emptyBlock -= PLAYBACK_BUFFER_NUM;
    s_readIndex += PLAYBACK_BUFFER_NUM;
    leftWAVData = s_fileInfo.fsize - oneTimeRW;
    oneTimeRW   = PLAYBACK_BUFFER_SIZE;

    PRINTF("\r\nStart play 8_TDM.wav file.\n\r");

    while (leftWAVData > PLAYBACK_BUFFER_SIZE)
    {
        /* wait at least one buffer block is full */
        if (s_emptyBlock < PLAYBACK_BUFFER_NUM)
        {
            if (s_writeIndex >= PLAYBACK_BUFFER_NUM)
            {
                s_writeIndex = 0U;
            }

            /* xfer structure */
            i2sTxTransfer.data     = (uint8_t *)((uint32_t)s_buffer + PLAYBACK_BUFFER_SIZE * s_writeIndex);
            i2sTxTransfer.dataSize = PLAYBACK_BUFFER_SIZE;
            /* Wait for available queue. */
            if (kStatus_Success == I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, i2sTxTransfer))
            {
                s_writeIndex++;
                s_emptyBlock++;
            }
        }
        /* wait at least one buffer block is empty and transfer done. */
        if ((s_emptyBlock) && (s_i2sTransferFinish))
        {
            s_i2sTransferFinish = false;

            if (s_readIndex >= PLAYBACK_BUFFER_NUM)
            {
                s_readIndex = 0U;
            }

            error = f_read(&s_fileObject, (uint8_t *)((uint32_t)s_buffer + s_readIndex * PLAYBACK_BUFFER_SIZE),
                           PLAYBACK_BUFFER_SIZE, &bytesRead);
            if ((error) || (bytesRead != PLAYBACK_BUFFER_SIZE))
            {
                PRINTF("Read file failed.\r\n");
                return -1;
            }

            s_readIndex++;
            s_emptyBlock--;
            leftWAVData -= PLAYBACK_BUFFER_SIZE;
        }
    }
    f_close(&s_fileObject);
    /* Once transfer finish, disable SAI instance. */
    I2S_TransferAbortDMA(DEMO_I2S_TX, &s_i2sTxHandle);
    I2S_Deinit(DEMO_I2S_TX);
    PRINTF("\r\nI2S TDM DMA example finished.\n\r ");
    while (1)
    {
    }
}

static status_t DEMO_MountFileSystem(void)
{
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
    FRESULT error;

    if (f_mount(&s_fileSystem, driverNumberBuffer, 0U))
    {
        PRINTF("Mount volume failed.\r\n");
        return kStatus_Fail;
    }

#if (FF_FS_RPATH >= 2U)
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        PRINTF("Change drive failed.\r\n");
        return kStatus_Fail;
    }
#endif

    error = f_open(&s_fileObject, _T("/8_TDM.wav"), FA_READ);
    if (error)
    {
        if (error == FR_EXIST)
        {
            PRINTF("8_TDM.wav File exists.\r\n");
        }
        else
        {
            PRINTF("8_TDM file not exist.\r\n");
            return kStatus_Fail;
        }
    }

    error = f_stat(_T("/8_TDM.wav"), &s_fileInfo);
    if (error != FR_OK)
    {
        PRINTF("Get file status failed\r\n");
        return kStatus_Fail;
    }

    PRINTF("\r\n8_TDM.wav File is available\r\n");

    return kStatus_Success;
}

static void DEMO_InitCodec(void)
{
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }

    PRINTF("\r\nCodec Init Done.\r\n");
}
