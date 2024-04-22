/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_mu.h"
#include "tee_fault_common.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_MU           MU1_MUB
#define APP_MU_IRQn       MU1_IRQn

#define DEMO_INVALID_DATA_ADDR  (0x20480000U)   /* OCRAM1 */

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t userOption = 0U;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DEMO_SwitchToUntrustedDomain(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_SwitchToUntrustedDomain(void)
{

}

static void Fault_Handler(void)
{
    switch (userOption)
    {
        case DEMO_MU_MSG_INVALID_DATA_ACCESS:
            MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_MU_MSG_INVALID_DATA_ACCESS_DONE);
            break;

        case DEMO_MU_MSG_INVALID_PARAM:
            MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_MU_MSG_INVALID_PARAM_DONE);
            break;

        default:
            break;
    }

    while (1)
        ;
}

void HardFault_Handler(void)
{
    Fault_Handler();
}

void BusFault_Handler(void)
{
    Fault_Handler();
}

static void DEMO_InvalidDataAccess(void)
{
    /*
     * The DEMO_INVALID_DATA_ADDR is inaccessible for untrusted domain,
     * so the access results to hardfault.
     */
    (*(volatile uint32_t *)DEMO_INVALID_DATA_ADDR)++;

    /* Should never get here */
    while (1)
        ;
}

static void DEMO_InvalidParameters(void)
{
    /*
     * The DEMO_INVALID_DATA_ADDR is inaccessible for untrusted domain,
     * so the access results to hardfault.
     */
    memcpy((char *)DEMO_INVALID_DATA_ADDR, (char *)(DEMO_INVALID_DATA_ADDR + 4), 4);

    /* Should never get here */
    while (1)
        ;
}

int main(void)
{
    BOARD_ConfigMPU();
    NVIC_EnableIRQ(APP_MU_IRQn);

    MU_Init(DEMO_MU);

    /* Notify core 0 that ready for the demo. */
    MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_MU_MSG_CORE1_READY);

    /* Handle the command from core 0. */
    while (1)
    {
        userOption = MU_ReceiveMsg(DEMO_MU, DEMO_MU_CH);

        switch (userOption)
        {
            case DEMO_MU_MSG_INVALID_DATA_ACCESS:
                DEMO_SwitchToUntrustedDomain();
                DEMO_InvalidDataAccess();
                break;

            case DEMO_MU_MSG_INVALID_PARAM:
                DEMO_SwitchToUntrustedDomain();
                DEMO_InvalidParameters();
                break;

            default:
                break;
        }
    }
}
