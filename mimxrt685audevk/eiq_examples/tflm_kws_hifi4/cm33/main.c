/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "dsp_support.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_cs42448.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "pin_mux.h"
#include "board.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

static cs42448_config_t s_cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 16000U, .bitWidth = 16U},
    .bus          = kCS42448_BusI2S,
    .slaveAddress = CS42448_I2C_ADDR,
};
static codec_config_t s_boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &s_cs42448Config};
static codec_handle_t s_codecHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    BOARD_Init();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to FLEXCOMM1 (I2S1) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM1);
    /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);

    CLOCK_AttachClk(kAUDIO_PLL_to_DMIC_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 3);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    s_cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    s_cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    if (CODEC_Init(&s_codecHandle, &s_boardCodecConfig) != kStatus_Success)
    {
        PRINTF("Error: Could not initialize audio codec! Please, reconnect the board power supply.\r\n");
        for (;;)
            ;
    }

    /* Initial volume kept low for hearing safety.
     * Adjust it to your needs, 0-100, 0 for mute, 100 for maximum volume.
     */
    if (CODEC_SetVolume(&s_codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 100U) !=
        kStatus_Success)
    {
        PRINTF("Warning: Could not set volume!\r\n");
    }

    /* Print the initial banner */
    PRINTF("\r\nStarting Xtensa example from Cortex-M33 core\r\n");

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    for (;;)
        ;
}
