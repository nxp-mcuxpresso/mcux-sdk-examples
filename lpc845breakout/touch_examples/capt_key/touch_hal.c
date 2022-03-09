/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include "touch_hal.h"
#include "fsl_capt.h"
#include "app_config.h"
#include "fsl_acomp.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TOUCH_HAL_DISABLE_CAPT_INTERRUPTS                                                                     \
    CAPT_DisableInterrupts(TOUCH_HAL_CAPT, kCAPT_InterruptOfYesTouchEnable | kCAPT_InterruptOfNoTouchEnable | \
                                               kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable)

#define TOUCH_HAL_ENABLE_CAPT_INTERRUPTS                                                                     \
    CAPT_EnableInterrupts(TOUCH_HAL_CAPT, kCAPT_InterruptOfYesTouchEnable | kCAPT_InterruptOfNoTouchEnable | \
                                              kCAPT_InterruptOfTimeOutEnable | kCAPT_InterruptOfPollDoneEnable)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool captPollDone                           = false;
static volatile uint16_t captRawData[TOUCH_X_CHANNEL_COUNT] = {0U};

/*******************************************************************************
 * Code
 ******************************************************************************/
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

void TOUCH_HAL_Init(void)
{
    capt_config_t captConfig;

    ACOMP_Configuration();

    /* Initialize CAPT module. */
    CAPT_GetDefaultConfig(&captConfig);
    captConfig.triggerMode = kCAPT_ComparatorTriggerMode;
    captConfig.XpinsMode   = kCAPT_InactiveXpinsHighZMode;
    /* Calculate the clock divider to make sure CAPT work in 2Mhz fclk. */
    captConfig.clockDivider = TOUCH_HAL_CAPT_CLK_DIVIDER;
    captConfig.enableXpins  = TOUCH_HAL_CAPT_ENABLE_PINS;
    captConfig.pollCount    = TOUCH_HAL_CAPT_DELAY_BETWEEN_POLL;
    CAPT_Init(TOUCH_HAL_CAPT, &captConfig);

    captPollDone = false;

    /* Enable the interrupts. */
    TOUCH_HAL_ENABLE_CAPT_INTERRUPTS;
    EnableIRQ(TOUCH_HAL_CAPT_IRQn);

    /* Set polling mode and start poll. */
    CAPT_SetPollMode(TOUCH_HAL_CAPT, kCAPT_PollContinuousMode);
}

void TOUCH_HAL_WaitDataReady(int16_t rawData[])
{
    uint8_t i;
    bool dataReady = false;

    while (1)
    {
        TOUCH_HAL_DISABLE_CAPT_INTERRUPTS;

        if (captPollDone)
        {
            for (i = 0; i < TOUCH_X_CHANNEL_COUNT; i++)
            {
                rawData[i] = captRawData[i];
            }
            captPollDone = false;
            dataReady    = true;
        }

        TOUCH_HAL_ENABLE_CAPT_INTERRUPTS;

        if (dataReady)
        {
            return;
        }
    }
}

void TOUCH_HAL_CAPT_IRQHandler(void)
{
    uint32_t intStat;
    capt_touch_data_t captData;

    intStat = CAPT_GetInterruptStatusFlags(TOUCH_HAL_CAPT);

    CAPT_ClearInterruptStatusFlags(TOUCH_HAL_CAPT, intStat);

    if (intStat &
        (kCAPT_InterruptOfYesTouchStatusFlag | kCAPT_InterruptOfNoTouchStatusFlag | kCAPT_InterruptOfTimeOutStatusFlag))
    {
        CAPT_GetTouchData(TOUCH_HAL_CAPT, &captData);

        captRawData[captData.XpinsIndex] = captData.count;

        captPollDone = false;
    }

    if (intStat & kCAPT_InterruptOfPollDoneStatusFlag)
    {
        captPollDone = true;
    }
}
