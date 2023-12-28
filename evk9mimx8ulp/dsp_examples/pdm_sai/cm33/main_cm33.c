/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dsp_support.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_fusion.h"
#include "fsl_reset.h"
#include "fsl_rgpio.h"
#include "fsl_iomuxc.h"
#include "fsl_codec_common.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"
#include "fsl_upower.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_AUDIO_MASTER_CLOCK CLOCK_GetIpFreq(kCLOCK_Sai0)

#define DEMO_SAI_CLK_FREQ CLOCK_GetIpFreq(kCLOCK_Sai0)
#define DEMO_I2C_CLK_FREQ CLOCK_GetIpFreq(kCLOCK_Lpi2c0)

#define I2C_RELEASE_SDA_GPIO  GPIOA
#define I2C_RELEASE_SDA_PIN   9U
#define I2C_RELEASE_SCL_GPIO  GPIOA
#define I2C_RELEASE_SCL_PIN   8U
#define I2C_RELEASE_BUS_COUNT 100U

#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100U
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
int BOARD_CODEC_Init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
codec_handle_t g_codecHandle;
wm8960_config_t wm8960Config = {
    .i2cConfig        = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
    .route            = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.sampleRate = kWM8960_AudioSampleRate16KHz, .bitWidth = kWM8960_AudioBitWidth32bit},
    .master_slave     = false,
};
codec_config_t g_boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};

/*******************************************************************************
 * Code
 ******************************************************************************/
int BOARD_CODEC_Init(void)
{
    if (CODEC_Init(&g_codecHandle, &g_boardCodecConfig) != kStatus_Success)
    {
        PRINTF("Codec init failed!\r\n");
        return -1;
    }

    /* Initial volume kept low for hearing safety. */
    /* Adjust it to your needs, 0x0006 for -51 dB, 0x0039 for 0 dB etc. */
    if (CODEC_SetVolume(&g_codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        return -1;
    }

    return 0;
}


static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    rgpio_pin_config_t pin_config;

    pin_config.pinDirection = kRGPIO_DigitalOutput;
    pin_config.outputLogic  = 1U;

    CLOCK_EnableClock(kCLOCK_RgpioA);

    RGPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    RGPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    RGPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        RGPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        RGPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        RGPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    RGPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    RGPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    RGPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    RGPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}
/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrcDiv(kCLOCK_Lpi2c0, kCLOCK_Pcc1BusIpSrcCm33Bus, 0U, 0U);
    RESET_PeripheralReset(kRESET_Lpi2c0);
    /* Use Pll1Pfd2Div clock source 12.288MHz. */
    CLOCK_SetIpSrc(kCLOCK_Sai0, kCLOCK_Cm33SaiClkSrcPll1Pfd2Div);

    /* Power on DMA0 memory partition */
    UPOWER_PowerOnMemPart(0U, (uint32_t)kUPOWER_MP1_DMA0);

    wm8960Config.i2cConfig.codecI2CSourceClock = DEMO_I2C_CLK_FREQ;
    wm8960Config.format.mclk_HZ                = DEMO_SAI_CLK_FREQ;

    PRINTF("\r\nAudio demo started. Initialize pins and codec on core 'Cortex-M33'\r\n");

    BOARD_CODEC_Init();

    /* Print the initial banner */
    PRINTF("Pins and codec initialized.\r\n");

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    while (1)
        ;
}
