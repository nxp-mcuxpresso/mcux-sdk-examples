/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsp_ipc.h"
#include "fsl_debug_console.h"


#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "main_cm33.h"
#include "app_dsp_ipc.h"
#include "dsp_support.h"
#include "rtp_buffer.h"
#include "rtp_receiver.h"
#include "wifi_client.h"

#include "dsp_config.h"
#include "fsl_cs42448.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_power.h"
#include "fsl_pca9420.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CODEC_VOLUME 100U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int BOARD_CODEC_Init(void);

int BOARD_CODEC_Init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
codec_handle_t g_codecHandle;
cs42448_config_t g_cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 16U},
    .bus          = kCS42448_BusI2S,
    .slaveAddress = CS42448_I2C_ADDR,
};
codec_config_t g_boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &g_cs42448Config};

static app_handle_t app;

/*******************************************************************************
 * Code
 ******************************************************************************/

int BOARD_CODEC_Init(void)
{
    PRINTF("Configure cs42448 codec\r\n");

    if (CODEC_Init(&g_codecHandle, &g_boardCodecConfig) != kStatus_Success)
    {
        PRINTF("cs42448_Init failed!\r\n");
        return -1;
    }

    if (CODEC_SetVolume(&g_codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        return -1;
    }

    return 0;
}

/*!
 * @brief Main function.
 */
int main(void)
{
    int ret;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* Clear MUA reset before run DSP core */
    RESET_PeripheralReset(kMU_RST_SHIFT_RSTn);

    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 1);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    g_cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    g_cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    PRINTF("\r\n");
    PRINTF("******************************\r\n");
    PRINTF("RTP demo start\r\n");
    PRINTF("******************************\r\n");
    PRINTF("\r\n");

    /* Initialize audio HW codec. */
    ret = BOARD_CODEC_Init();
    if (ret)
    {
        PRINTF("CODEC_Init failed!\r\n");
        return -1;
    }

    /* Initialize RPMsg IPC interface between ARM and DSP cores. */
    dsp_ipc_init();

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

#if DSP_IMAGE_COPY_TO_RAM
    PRINTF("DSP image copied to DSP TCM\r\n");
#endif

    /* Initialize application context */
    memset(&app, 0U, sizeof(app_handle_t));

    /* Initialize RTP data buffers. */
    rtp_buffer_init(&app);

    /* Start Wi-Fi connection task. */
    wifi_client_init(&app);

    /* Start IPC processing task. */
    app_dsp_ipc_init(&app);

    /* Start RTP receiving task. */
    rtp_receiver_init(&app);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Shoud not reach this statement. */
    return 0;
}
