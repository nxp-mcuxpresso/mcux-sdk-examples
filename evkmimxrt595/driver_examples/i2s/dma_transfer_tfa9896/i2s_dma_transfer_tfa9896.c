/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP

 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_dma.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_tfa9896.h"
#include "fsl_codec_common.h"
#include "music.h"

#include <stdbool.h>
#include "fsl_codec_adapter.h"
#include "fsl_inputmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_AUDIO_PROTOCOL             kCODEC_BusI2S
#define DEMO_AUDIO_BIT_WIDTH            (16)
#define DEMO_AUDIO_SAMPLE_RATE          (48000)

#define DEMO_I2S_TX (I2S1)

#define DEMO_DMA            (DMA0)
#define DEMO_I2S_TX_CHANNEL (3U)

#define DEMO_I2S_CLOCK_DIVIDER (CLOCK_GetMclkClkFreq() / DEMO_AUDIO_SAMPLE_RATE / DEMO_AUDIO_BIT_WIDTH / 2U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void StartSoundPlayback(void);

static void TxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
tfa9896_config_t tfa9896ConfigL = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
    .slaveAddress = TFA9896_I2C_ADDRESS_LEFT,
    .protocol     = kTFA9896_ProtocolI2S,
    .format       = {.sampleRate = ktfa9896_SampleRate48kHz, .bitWidth = ktfa9896_BitWidth16},
    .calibrate    = false,
};
tfa9896_config_t tfa9896ConfigR = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
    .slaveAddress = TFA9896_I2C_ADDRESS_RIGHT,
    .protocol     = kTFA9896_ProtocolI2S,
    .format       = {.sampleRate = ktfa9896_SampleRate48kHz, .bitWidth = ktfa9896_BitWidth16},
    .calibrate    = false,
};

codec_config_t boardCodecConfigL = {.codecDevType = kCODEC_TFA9896, .codecDevConfig = &tfa9896ConfigL};
codec_config_t boardCodecConfigR = {.codecDevType = kCODEC_TFA9896, .codecDevConfig = &tfa9896ConfigR};

static dma_handle_t s_DmaTxHandle;
static i2s_config_t s_TxConfig;
static i2s_dma_handle_t s_TxHandle;
static i2s_transfer_t s_TxTransfer;
extern codec_config_t boardCodecConfigL;
extern codec_config_t boardCodecConfigR;
codec_handle_t codecHandleLeft;
codec_handle_t codecHandleRight;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < 100; i++)
    {
        __NOP();
    }
}

void BOARD_I3C_ReleaseBus(void)
{
    uint8_t i = 0;

    GPIO_PortInit(GPIO, 2);
    BOARD_InitI3CPinsAsGPIO();

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(GPIO, 2, 30, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(GPIO, 2, 29, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(GPIO, 2, 30, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(GPIO, 2, 29, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(GPIO, 2, 29, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIO, 2, 30, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIO, 2, 29, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIO, 2, 30, 1U);
    i2c_release_bus_delay();
}


/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_I3C_ReleaseBus();
    BOARD_InitI3CPins();

    /* Configure DMAMUX. */
    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    INPUTMUX_Init(INPUTMUX);
    /* Enable DMA request */
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm1TxToDmac0Ch3RequestEna, true);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    /* Attach main clock to I3C, 396MHz / 16 = 24.75MHz. */
    CLOCK_AttachClk(kMAIN_CLK_to_I3C_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivI3cClk, 16);

    /* attach AUDIO PLL clock to FLEXCOMM1 (I2S1) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM1);
    /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 1);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    tfa9896ConfigL.i2cConfig.codecI2CSourceClock = CLOCK_GetI3cClkFreq();
    tfa9896ConfigR.i2cConfig.codecI2CSourceClock = tfa9896ConfigL.i2cConfig.codecI2CSourceClock;

    /* Set shared signal set 0: SCK, WS from Flexcomm1 */
    SYSCTL1->SHAREDCTRLSET[0] = SYSCTL1_SHAREDCTRLSET_SHAREDSCKSEL(1) | SYSCTL1_SHAREDCTRLSET_SHAREDWSSEL(1);
    /* Set flexcomm3 SCK, WS from shared signal set 0 */
    SYSCTL1->FCCTRLSEL[3] = SYSCTL1_FCCTRLSEL_SCKINSEL(1) | SYSCTL1_FCCTRLSEL_WSINSEL(1);

    PRINTF("Configure I2S\r\n");

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
    s_TxConfig.divider = DEMO_I2S_CLOCK_DIVIDER;

    I2S_TxInit(DEMO_I2S_TX, &s_TxConfig);

    DMA_Init(DEMO_DMA);

    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_DmaTxHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);

    StartSoundPlayback();

    PRINTF("Initialize left TFA9896\r\n");
    if (CODEC_Init(&codecHandleLeft, &boardCodecConfigL) != kStatus_Success)
    {
        PRINTF("TFA9896_Init failed for left device!\r\n");
        assert(false);
    }
    PRINTF("Initialize right TFA9896\r\n");
    if (CODEC_Init(&codecHandleRight, &boardCodecConfigR) != kStatus_Success)
    {
        PRINTF("TFA9896_Init failed for right device!\r\n");
        assert(false);
    }
#ifdef TFA9896_VOLUME_DEMO
#define ANY_CHANNEL 0x1
    int vol_iter = 0, i = 0;
    for (vol_iter = 0; vol_iter < 100; vol_iter++)
    {
        if (CODEC_SetVolume(&codecHandleLeft, ANY_CHANNEL, vol_iter) != kStatus_Success)
        {
            PRINTF("TFA9896_SetVolume failed for left device!\r\n");
            assert(false);
        }
        for (i = 0; i < 0xfff; i++)
        {
        }
        if (CODEC_SetVolume(&codecHandleRight, ANY_CHANNEL, vol_iter) != kStatus_Success)
        {
            PRINTF("TFA9896_SetVolume failed for right device!\r\n");
            assert(false);
        }
        for (i = 0; i < 4 * 0xfff; i++)
        {
        }
    }

    for (vol_iter = 100; vol_iter > 0; vol_iter--)
    {
        if (CODEC_SetVolume(&codecHandleLeft, ANY_CHANNEL, vol_iter) != kStatus_Success)
        {
            PRINTF("TFA9896_SetVolume failed for left device!\r\n");
            assert(false);
        }
        for (i = 0; i < 0xfff; i++)
        {
        }
        if (CODEC_SetVolume(&codecHandleRight, ANY_CHANNEL, vol_iter) != kStatus_Success)
        {
            PRINTF("TFA9896_SetVolume failed for right device!\r\n");
            assert(false);
        }
        for (i = 0; i < 10 * 0xfff; i++)
        {
        }
    }
    for (vol_iter = 0; vol_iter < 100; vol_iter++)
    {
        if (CODEC_SetVolume(&codecHandleLeft, ANY_CHANNEL, vol_iter) != kStatus_Success)
        {
            PRINTF("TFA9896_SetVolume failed for left device!\r\n");
            assert(false);
        }
        for (i = 0; i < 0xfff; i++)
        {
            ;
        }
        if (CODEC_SetVolume(&codecHandleRight, ANY_CHANNEL, vol_iter) != kStatus_Success)
        {
            PRINTF("TFA9896_SetVolume failed for right device!\r\n");
            assert(false);
        }
        for (i = 0; i < 10 * 0xfff; i++)
        {
        }
    }
#endif
    while (1)
    {
    }
}

static void StartSoundPlayback(void)
{
    PRINTF("Setup looping playback of sine wave\r\n");

    s_TxTransfer.data     = &g_Music[0];
    s_TxTransfer.dataSize = sizeof(g_Music);

    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_TxHandle, &s_DmaTxHandle, TxCallback, (void *)&s_TxTransfer);
    /* need to queue two transmit buffers so when the first one
     * finishes transfer, the other immediatelly starts */
    I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_TxHandle, s_TxTransfer);
    I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_TxHandle, s_TxTransfer);
}

static void TxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    /* Enqueue the same original buffer all over again */
    i2s_transfer_t *transfer = (i2s_transfer_t *)userData;
    I2S_TxTransferSendDMA(base, handle, *transfer);
}
