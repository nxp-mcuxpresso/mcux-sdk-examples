/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_tsi_v6.h"
#include "fsl_debug_console.h"
#include "fsl_lptmr.h"
#include "fsl_inputmux.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Available PAD names on board */
#define PAD_TSI_MUTUAL_CAP_1_NAME "KEY3"

/* IRQ related redefinitions for specific SOC */
#define TSI0_IRQHandler TSI_END_OF_SCAN_IRQHandler
#define TSI0_IRQn       TSI_END_OF_SCAN_IRQn

/* Define the delta value to indicate a touch event */
#define TOUCH_DELTA_VALUE 100U

/* TSI indication led of electrode 1 */
#define LED1_INIT()   LED_BLUE_INIT(LOGIC_LED_OFF)
#define LED1_TOGGLE() LED_BLUE_TOGGLE()
#define LED2_INIT()   LED_RED_INIT(LOGIC_LED_OFF)

/* Get source clock for LPTMR driver */
#define LPTMR_SOURCE_CLOCK (16000)
/* Define LPTMR microseconds count value */
#define LPTMR_USEC_COUNT (100000)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* The array stores the un-touched sample counter for mutual-cap pad */
uint16_t mutualCalibratedData[1]     = {0U}; /* This example demo only 1 mutual-cap pad usage */
static volatile bool s_tsiInProgress = true;
/* Array of TSI peripheral base address. */
#if defined(TSI0)
#define APP_TSI TSI0
#elif defined(TSI)
#define APP_TSI TSI
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void TSI0_IRQHandler(void)
{
    if (TSI_GetRxMutualCapMeasuredChannel(APP_TSI) == (tsi_mutual_rx_channel_t)(BOARD_TSI_MUTUAL_RX_ELECTRODE_1 - 8U))
    {
        if (TSI_GetCounter(APP_TSI) > (uint16_t)(mutualCalibratedData[0] + TOUCH_DELTA_VALUE))
        {
            LED1_TOGGLE(); /* Toggle the touch event indicating LED */
            s_tsiInProgress = false;
        }
    }

    /* Clear endOfScan flag */
    TSI_ClearStatusFlags(APP_TSI, kTSI_EndOfScanFlag);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    tsi_mutualCap_config_t tsiConfig_mutualCap;
    lptmr_config_t lptmrConfig;
    memset((void *)&lptmrConfig, 0, sizeof(lptmrConfig));

    /* Initialize standard SDK demo application pins */
    /* Enables the clock for INPUTMUX: Enables clock */
    CLOCK_EnableClock(kCLOCK_InputMux0);

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Enables the clk_16k[1] */
    CLOCK_SetupClk16KClocking(kCLOCK_Clk16KToVsys);
    /* attach FRO HF to SCT */
    CLOCK_SetClkDiv(kCLOCK_DivTsiClk, 1u);
    CLOCK_AttachClk(kCLK_IN_to_TSI);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    /* Init tsi Leds in Demo app */
    LED1_INIT();
    LED2_INIT();

    /* Configure LPTMR */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* TSI default hardware configuration for normal mode */
    TSI_GetMutualCapModeDefaultConfig(&tsiConfig_mutualCap);

    /* Initialize the LPTMR */
    LPTMR_Init(LPTMR0, &lptmrConfig);
    /* Initialize the TSI */
    TSI_InitMutualCapMode(APP_TSI, &tsiConfig_mutualCap);

    /* Set timer period */
    LPTMR_SetTimerPeriod(LPTMR0, USEC_TO_COUNT(LPTMR_USEC_COUNT, LPTMR_SOURCE_CLOCK));

    NVIC_EnableIRQ(TSI0_IRQn);
    NVIC_EnableIRQ(TSI_OUT_OF_SCAN_IRQn);
    TSI_EnableModule(APP_TSI, true); /* Enable module */

    PRINTF("\r\nTSI_V6 Mutual-Cap mode Example Start!\r\n");
    /*********  CALIBRATION PROCESS ************/
    PRINTF("\r\nPlease do not touch pad %s when in calibration process!\r\n", PAD_TSI_MUTUAL_CAP_1_NAME);
    TSI_EnableHardwareTriggerScan(APP_TSI, false); /* Enable software trigger scan */
    TSI_DisableInterrupts(APP_TSI, kTSI_EndOfScanInterruptEnable);
    TSI_ClearStatusFlags(APP_TSI, kTSI_EndOfScanFlag);
    /* Select one mutual-cap pad as detecting pad. */
    TSI_SetMutualCapTxChannel(APP_TSI, (tsi_mutual_tx_channel_t)BOARD_TSI_MUTUAL_TX_ELECTRODE_1);
    TSI_SetMutualCapRxChannel(APP_TSI, (tsi_mutual_rx_channel_t)(BOARD_TSI_MUTUAL_RX_ELECTRODE_1 - 8U));
    TSI_StartSoftwareTrigger(APP_TSI);
    while (!(TSI_GetStatusFlags(APP_TSI) & kTSI_EndOfScanFlag))
    {
    }
    mutualCalibratedData[0] = TSI_GetCounter(APP_TSI);
    PRINTF("Calibrated counters for mutual-cap pad %s is: %d \r\n", PAD_TSI_MUTUAL_CAP_1_NAME, mutualCalibratedData[0]);

    /********** SOFTWARE TRIGGER SCAN USING POLLING METHOD ********/
    PRINTF("\r\nNOW, comes to the software trigger scan using polling method!\r\n");
    TSI_EnableHardwareTriggerScan(APP_TSI, false); /* Enable software trigger scan */
    TSI_DisableInterrupts(APP_TSI, kTSI_EndOfScanInterruptEnable);
    TSI_ClearStatusFlags(APP_TSI, kTSI_EndOfScanFlag);
    /* Select one mutual-cap pad as detecting pad. */
    TSI_SetMutualCapTxChannel(APP_TSI, (tsi_mutual_tx_channel_t)BOARD_TSI_MUTUAL_TX_ELECTRODE_1);
    TSI_SetMutualCapRxChannel(APP_TSI, (tsi_mutual_rx_channel_t)(BOARD_TSI_MUTUAL_RX_ELECTRODE_1 - 8U));
    TSI_StartSoftwareTrigger(APP_TSI);
    while (!(TSI_GetStatusFlags(APP_TSI) & kTSI_EndOfScanFlag))
    {
    }
    PRINTF("Mutual-cap pad %s Normal mode counter is: %d \r\n", PAD_TSI_MUTUAL_CAP_1_NAME, TSI_GetCounter(APP_TSI));
    TSI_ClearStatusFlags(APP_TSI, kTSI_EndOfScanFlag);
    TSI_ClearStatusFlags(APP_TSI, (uint32_t)kTSI_OutOfRangeFlag);

    /********** SOFTWARE TRIGGER SCAN USING INTERRUPT METHOD ********/
    PRINTF("\r\nNOW, comes to the software trigger scan using interrupt method!\r\n");
    TSI_EnableInterrupts(APP_TSI, kTSI_GlobalInterruptEnable);
    TSI_EnableInterrupts(APP_TSI, kTSI_EndOfScanInterruptEnable);
    TSI_ClearStatusFlags(APP_TSI, kTSI_EndOfScanFlag);
    /* Select one mutual-cap pad as detecting pad. */
    TSI_SetMutualCapTxChannel(APP_TSI, (tsi_mutual_tx_channel_t)BOARD_TSI_MUTUAL_TX_ELECTRODE_1);
    TSI_SetMutualCapRxChannel(APP_TSI, (tsi_mutual_rx_channel_t)(BOARD_TSI_MUTUAL_RX_ELECTRODE_1 - 8U));
    while (s_tsiInProgress)
    {
        TSI_StartSoftwareTrigger(APP_TSI);
    }
    PRINTF("Mutual-cap pad %s Normal mode counter is: %d \r\n", PAD_TSI_MUTUAL_CAP_1_NAME, TSI_GetCounter(APP_TSI));

    /********** HARDWARE TRIGGER SCAN ********/
    PRINTF("\r\nNOW, comes to the hardware trigger scan method!\r\n");
    PRINTF("After running, touch pad %s each time, you will see LED toggles.\r\n", PAD_TSI_MUTUAL_CAP_1_NAME);
    TSI_EnableModule(APP_TSI, false);
    TSI_EnableHardwareTriggerScan(APP_TSI, true);
    TSI_EnableInterrupts(APP_TSI, kTSI_GlobalInterruptEnable);
    TSI_EnableInterrupts(APP_TSI, kTSI_EndOfScanInterruptEnable);
    TSI_ClearStatusFlags(APP_TSI, kTSI_EndOfScanFlag);

    /* Select one mutual-cap pad as detecting pad. */
    TSI_SetMutualCapTxChannel(APP_TSI, (tsi_mutual_tx_channel_t)BOARD_TSI_MUTUAL_TX_ELECTRODE_1);
    TSI_SetMutualCapRxChannel(APP_TSI, (tsi_mutual_rx_channel_t)(BOARD_TSI_MUTUAL_RX_ELECTRODE_1 - 8U));
    TSI_EnableModule(APP_TSI, true);
    INPUTMUX_AttachSignal(INPUTMUX0, 0U, kINPUTMUX_Lptmr0ToTsiTrigger);
    LPTMR_StartTimer(LPTMR0); /* Start LPTMR triggering */

    while (1)
    {
    }
}
