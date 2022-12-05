/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_afe.h"

#include "fsl_vref.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_AFE_CHANNEL 0U
/* About 23 mV */
#define DEMO_AFE_VREF_TRIM 2U
#define DEMO_AFE_BASEADDR  AFE

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_VREF_Config(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_VREF_Config(void)
{
    vref_config_t config;

    /* Get vref default configure */
    /*
     * config.bufferMode = kVREF_ModeHighPowerBuffer;
     * config.bufferMode = kVREF_ModeTightRegulationBuffer;
     * config.enableExternalVoltRef = false;
     * config.enableLowRef = false;
     */
    VREF_GetDefaultConfig(&config);
#if defined(FSL_FEATURE_VREF_HAS_LOW_REFERENCE) && FSL_FEATURE_VREF_HAS_LOW_REFERENCE
    /* Enable low reference volt */
    config.enableLowRef = true;
#endif /* FSL_FEATURE_VREF_HAS_LOW_REFERENCE */
    /* Init vref */
    VREF_Init(VREF, &config);
#if defined(FSL_FEATURE_VREF_HAS_LOW_REFERENCE) && FSL_FEATURE_VREF_HAS_LOW_REFERENCE
    /* Vref set trim reference */
    VREF_SetLowReferenceTrimVal(VREF, 3U);
#endif /* FSL_FEATURE_VREF_HAS_LOW_REFERENCE */
    /* Vref set trim, this value will not be the same with every boards */
    VREF_SetTrimVal(VREF, DEMO_AFE_VREF_TRIM);
}

int main(void)
{
    afe_config_t afeExampleStruct;
    afe_channel_config_t afeChnExampleStruct;
    int32_t result0;

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Select clkSrc for AFEclk */
    CLOCK_SetAfeClkSrc(1U);
    /* Init vref. */
    APP_VREF_Config();
    /* Get AFE config default */
    /*
     * afeExampleStruct.enableLowPower = false;
     * afeExampleStruct.resultFormat = kAFE_ResultFormatRight;
     * afeExampleStruct.clockDivider = kAFE_ClockDivider2;
     * afeExampleStruct.clockSource = kAFE_ClockSource1;
     * afeExampleStruct.startupCount = 2U;
     */
    AFE_GetDefaultConfig(&afeExampleStruct);
    afeExampleStruct.startupCount = 80U; /* startupCnt = (Clk_freq/Clk_div)*20e-6 */

    /* Get channel config default */
    AFE_GetDefaultChannelConfig(&afeChnExampleStruct);
    /* Init AFE module */
    AFE_Init(DEMO_AFE_BASEADDR, &afeExampleStruct);
    /* AFE_Init AFE channel 0 */
    AFE_SetChannelConfig(DEMO_AFE_BASEADDR, DEMO_AFE_CHANNEL, &afeChnExampleStruct);
    /* Disable interrupt */
    AFE_DisableChannelInterrupts(DEMO_AFE_BASEADDR, kAFE_Channel0InterruptEnable);
    /* Disable DMA */
    AFE_EnableChannelDMA(DEMO_AFE_BASEADDR, (uint32_t)kAFE_Channel0DMAEnable, false);

    PRINTF("AFE Polling example.\r\n");

    while (1)
    {
        /* Wait next triger from user */
        PRINTF("Press any key to trigger AFE conversion\r\n");
        GETCHAR();
        /* Software trigger conversion */
        AFE_DoSoftwareTriggerChannel(DEMO_AFE_BASEADDR, kAFE_Channel0Trigger);
        /* Wait until conversion complete */
        while ((kAFE_Channel0ConversionCompleteFlag & AFE_GetChannelStatusFlags(DEMO_AFE_BASEADDR)) == 0)
        {
        }
        /* Read result conversion */
        result0 = (int32_t)AFE_GetChannelConversionValue(DEMO_AFE_BASEADDR, DEMO_AFE_CHANNEL);
        /* Print result */
        PRINTF("AFE value  = %d\r\n", result0);
    }
}
