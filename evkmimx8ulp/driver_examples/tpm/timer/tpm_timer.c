/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_tpm.h"

#include "fsl_common.h"
#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* define instance */
#define BOARD_TPM TPM0
/* Interrupt number and interrupt handler for the TPM instance used */
#define BOARD_TPM_IRQ_NUM TPM0_IRQn
#define BOARD_TPM_HANDLER TPM0_IRQHandler
/* Get source clock for TPM driver */
#define TPM_SOURCE_CLOCK (CLOCK_GetTpmClkFreq(0) / 4)

#ifndef DEMO_TIMER_PERIOD_US
/* Set counter period to 1ms */
#define DEMO_TIMER_PERIOD_US (1000U)
#endif
#ifndef TPM_PRESCALER
/* Calculate the clock division based on the PWM frequency to be obtained */
#define TPM_PRESCALER TPM_CalculateCounterClkDiv(BOARD_TPM, 1000000U / DEMO_TIMER_PERIOD_US, TPM_SOURCE_CLOCK);
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool tpmIsrFlag           = false;
volatile uint32_t milisecondCounts = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t cnt;
    uint32_t loop       = 2;
    uint32_t secondLoop = 1000U;
    const char *signals = "-|";
    tpm_config_t tpmInfo;

    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(kCLOCK_Tpm0, kCLOCK_Pcc1BusIpSrcCm33Bus);
    RESET_PeripheralReset(kRESET_Tpm0);

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

    /* Print a note to terminal */
    PRINTF("\r\nTPM example to simulate a timer\r\n");

    TPM_GetDefaultConfig(&tpmInfo);

    /* TPM clock divide by TPM_PRESCALER */
    tpmInfo.prescale = TPM_PRESCALER;

    /* Initialize TPM module */
    TPM_Init(BOARD_TPM, &tpmInfo);

    /* Set timer period */
    TPM_SetTimerPeriod(BOARD_TPM, USEC_TO_COUNT(DEMO_TIMER_PERIOD_US, TPM_SOURCE_CLOCK / (1U << tpmInfo.prescale)));

    TPM_EnableInterrupts(BOARD_TPM, kTPM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_TPM_IRQ_NUM);

    PRINTF("Press any key to start timer!\r\n");
    GETCHAR();

    PRINTF("\r\nYou will see a \"-\" or \"|\" in terminal every 1 second:\r\n");
    TPM_StartTimer(BOARD_TPM, kTPM_SystemClock);

    cnt = 0;
    while (true)
    {
        if (tpmIsrFlag)
        {
            milisecondCounts++;
            tpmIsrFlag = false;
            if (milisecondCounts >= secondLoop)
            {
                PRINTF("%c", signals[cnt & 1]);
                cnt++;
                if (cnt >= loop)
                {
                    cnt = 0;
                }
                milisecondCounts = 0U;
            }
        }
        __WFI();
    }
}

void BOARD_TPM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    TPM_ClearStatusFlags(BOARD_TPM, kTPM_TimeOverflowFlag);
    tpmIsrFlag = true;
    SDK_ISR_EXIT_BARRIER;
}
