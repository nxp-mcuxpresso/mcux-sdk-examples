/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
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
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "music.h"
#include "fsl_codec_common.h"
#include <stdbool.h>
#include "fsl_codec_adapter.h"
#include "fsl_wm8904.h"
#include "fsl_iopctl.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_AUDIO_PROTOCOL             kCODEC_BusI2S
#define DEMO_AUDIO_BIT_WIDTH            (16)
#define DEMO_AUDIO_SAMPLE_RATE          (48000)

#define DEMO_I2S_TX      (I2S3)
#define DEMO_I2S_TX_MODE (kI2S_MasterSlaveNormalSlave)

#define DEMO_I2S_CLOCK_DIVIDER (CLOCK_GetMclkClkFreq() / DEMO_AUDIO_SAMPLE_RATE / DEMO_AUDIO_BIT_WIDTH / 2U)
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 30U
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void StartSoundPlayback(void);

static void TxCallback(I2S_Type *base, i2s_handle_t *handle, status_t completionStatus, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8904_config_t wm8904Config = {
    .i2cConfig          = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
    .recordSource       = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate48kHz, .bitWidth = kWM8904_BitWidth16},
    .master             = true,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};

static i2s_config_t s_TxConfig;
static i2s_handle_t s_TxHandle;
static i2s_transfer_t s_TxTransfer;
extern codec_config_t boardCodecConfig;
codec_handle_t codecHandle;

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

    wm8904Config.i2cConfig.codecI2CSourceClock = CLOCK_GetI3cClkFreq();
    wm8904Config.mclk_HZ                       = CLOCK_GetMclkClkFreq();

    /* Set shared signal set 0: SCK, WS from Flexcomm1 */
    SYSCTL1->SHAREDCTRLSET[0] = SYSCTL1_SHAREDCTRLSET_SHAREDSCKSEL(1) | SYSCTL1_SHAREDCTRLSET_SHAREDWSSEL(1);
    /* Set flexcomm3 SCK, WS from shared signal set 0 */
    SYSCTL1->FCCTRLSEL[3] = SYSCTL1_FCCTRLSEL_SCKINSEL(1) | SYSCTL1_FCCTRLSEL_WSINSEL(1);

    PRINTF("Configure codec\r\n");

    /* protocol: i2s
     * sampleRate: 48K
     * bitwidth:16
     */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("codec_Init failed!\r\n");
        assert(false);
    }

    /* Initial volume kept low for hearing safety.
     * Adjust it to your needs, 0-100, 0 for mute, 100 for maximum volume.
     */
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }

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
    s_TxConfig.divider     = DEMO_I2S_CLOCK_DIVIDER;
    s_TxConfig.masterSlave = DEMO_I2S_TX_MODE;
    I2S_TxInit(DEMO_I2S_TX, &s_TxConfig);

    StartSoundPlayback();

    while (1)
    {
    }
}

static void StartSoundPlayback(void)
{
    PRINTF("Setup looping playback of sine wave\r\n");

    s_TxTransfer.data     = &g_Music[0];
    s_TxTransfer.dataSize = sizeof(g_Music);

    I2S_TxTransferCreateHandle(DEMO_I2S_TX, &s_TxHandle, TxCallback, (void *)&s_TxTransfer);
    I2S_TxTransferNonBlocking(DEMO_I2S_TX, &s_TxHandle, s_TxTransfer);
}

static void TxCallback(I2S_Type *base, i2s_handle_t *handle, status_t completionStatus, void *userData)
{
    /* Enqueue the same original s_Buffer all over again */
    i2s_transfer_t *transfer = (i2s_transfer_t *)userData;
    I2S_TxTransferNonBlocking(base, handle, *transfer);
}
