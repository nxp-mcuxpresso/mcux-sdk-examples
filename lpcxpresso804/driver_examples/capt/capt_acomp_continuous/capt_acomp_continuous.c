/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_capt.h"
#include "fsl_acomp.h"
#include "fsl_debug_console.h"

#include <stdbool.h>
#include "fsl_power.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ACOMP_BASE           ACOMP
#define DEMO_ACOMP_CAPT_CHANNEL   4U
#define DEMO_CAPT_BASE            CAPT
#define DEMO_CAPT_IRQn            CMP_CAPT_IRQn
#define DEMO_CAPT_IRQHandler      CMP_CAPT_IRQHandler
#define DEMO_CAPT_ENABLE_PINS     kCAPT_X0Pin | kCAPT_X1Pin | kCAPT_X2Pin | kCAPT_X3Pin | kCAPT_X4Pin
#define DEMO_CAPT_ENABLE_PINS_NUM 5
#define DEMO_CAPT_CLOCK_FREQ      CLOCK_GetFroFreq()

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void ACOMP_Configuration(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint16_t g_captRawData[DEMO_CAPT_ENABLE_PINS_NUM];
volatile bool g_captPollDone;

/*******************************************************************************
 * Code
 ******************************************************************************/


/*!
 * @brief ISR for CAPT.
 */
void DEMO_CAPT_IRQHandler(void)
{
    uint32_t intStat;
    capt_touch_data_t s_captData;

    intStat = CAPT_GetInterruptStatusFlags(DEMO_CAPT_BASE);

    CAPT_ClearInterruptStatusFlags(DEMO_CAPT_BASE, intStat);

    /* If timeout happens, the board capt capacitor should be changed. */
    if (intStat &
        (kCAPT_InterruptOfYesTouchStatusFlag | kCAPT_InterruptOfNoTouchStatusFlag | kCAPT_InterruptOfTimeOutStatusFlag))
    {
        CAPT_GetTouchData(DEMO_CAPT_BASE, &s_captData);

        g_captRawData[s_captData.XpinsIndex] = s_captData.count;
    }

    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
        g_captPollDone = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    capt_config_t captConfig;
    uint32_t captClockFreq;

    /* Initialize board hardware. */
    /* Attach 12 MHz clock to USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    /* Attach main clock to CAPT */
    CLOCK_Select(kCAPT_Clk_From_Fro);
    POWER_DisablePD(kPDRUNCFG_PD_ACMP);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    ACOMP_Configuration();

    PRINTF("CAPT ACOMP continuous example.\r\n");

    /* Initialize CAPT module. */
    CAPT_GetDefaultConfig(&captConfig);
    captConfig.triggerMode = kCAPT_ComparatorTriggerMode;
    captConfig.XpinsMode   = kCAPT_InactiveXpinsHighZMode;
    /* Calculate the clock divider to make sure CAPT work in 2Mhz fclk. */
    captClockFreq           = DEMO_CAPT_CLOCK_FREQ;
    captConfig.clockDivider = (captClockFreq / 2000000U - 1U);
    captConfig.enableXpins  = DEMO_CAPT_ENABLE_PINS;

    /* The CAPT FCLK is set to 2MHz, the delay period between two polling rounds
     * is pollCount * 2096, setting pollCount to 250, then the polling round
     * frequency is about 2Hz, user can change this value to smaller value to
     * for faster polling round.
     */
    captConfig.pollCount = 250;

    CAPT_Init(DEMO_CAPT_BASE, &captConfig);

    /* Enable the interrupts. */
    CAPT_EnableInterrupts(DEMO_CAPT_BASE, kCAPT_InterruptOfYesTouchEnable | kCAPT_InterruptOfNoTouchEnable |
                                              kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable);
    NVIC_EnableIRQ(DEMO_CAPT_IRQn);

    /* Set polling mode and start poll. */
    CAPT_SetPollMode(DEMO_CAPT_BASE, kCAPT_PollContinuousMode);

    while (1)
    {
        while (!g_captPollDone)
        {
        }

        /* Output the captured data. */
        PRINTF("CAPT Data: ");

        for (uint8_t i = 0; i < ARRAY_SIZE(g_captRawData); i++)
        {
            PRINTF("%u\t", g_captRawData[i]);
        }

        PRINTF("\r\n");

        g_captPollDone = false;
    }
}

static void ACOMP_Configuration(void)
{
    acomp_config_t acompConfig;
    acomp_ladder_config_t acompLadderConfig;

    acompConfig.enableSyncToBusClk  = false;
    acompConfig.hysteresisSelection = kACOMP_Hysteresis20MVSelection;
    ACOMP_Init(DEMO_ACOMP_BASE, &acompConfig);

    ACOMP_EnableInterrupts(DEMO_ACOMP_BASE, kACOMP_InterruptsDisable);

    ACOMP_SetInputChannel(DEMO_ACOMP_BASE, DEMO_ACOMP_CAPT_CHANNEL, 0U);

    acompLadderConfig.ladderValue      = 0x08U;
    acompLadderConfig.referenceVoltage = kACOMP_LadderRefVoltagePinVDD;
    ACOMP_SetLadderConfig(DEMO_ACOMP_BASE, &acompLadderConfig);
}
