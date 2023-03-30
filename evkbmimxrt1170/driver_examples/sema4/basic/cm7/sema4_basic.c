/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sema4.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SEMA4    SEMA4
#define DEMO_PROC_NUM 0 /* Fixed value by system integration. */

#define DEMO_CHECK(x)                                    \
    if (!(x))                                            \
    {                                                    \
        PRINTF("Example error, line: %d\r\n", __LINE__); \
        for (;;)                                         \
            ;                                            \
    }

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void DEMO_SEMA4_Lock(void);
static void DEMO_SEMA4_ResetGate(void);
static void DEMO_SEMA4_ResetAllGate(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("SEMA4 basic example start\r\n");
    PRINTF("Proc number is %d\r\n", DEMO_PROC_NUM);

    SEMA4_Init(DEMO_SEMA4);

    DEMO_SEMA4_Lock();
    DEMO_SEMA4_ResetGate();
    DEMO_SEMA4_ResetAllGate();

    SEMA4_Deinit(DEMO_SEMA4);

    PRINTF("\r\n\r\nSEMA4 basic example finished successfully\r\n");

    while (1)
    {
    }
}

static void DEMO_SEMA4_Lock(void)
{
    uint32_t gateNum = 0U;
    uint32_t i       = 0U;

    PRINTF("\r\n\r\nSEMA42 GATE LOCK AND UNLOCK\r\n");

    DEMO_CHECK(kStatus_Success == SEMA4_ResetAllGates(DEMO_SEMA4));

    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        PRINTF("Gate %d/%d\r\n", gateNum + 1, FSL_FEATURE_SEMA4_GATE_COUNT);

        /*
         * Non-blocking lock.
         */
        PRINTF("Lock with non-blocking method\r\n");

        DEMO_CHECK(kStatus_Success == SEMA4_TryLock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM));

        /* Now SEMA4 gate is locked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == DEMO_PROC_NUM);

        /* Check other gates are not locked by mistake. */
        for (i = 0U; i < gateNum; i++)
        {
            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == -1);
        }
        for (i++; i < FSL_FEATURE_SEMA4_GATE_COUNT; i++)
        {
            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == -1);
        }

        SEMA4_Unlock(DEMO_SEMA4, gateNum);

        /* Now SEMA4 gate is unlocked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == -1);

        PRINTF("Gate lock&unlock success\r\n");

        /*
         * Blocking lock.
         */
        PRINTF("Lock with blocking method\r\n");

        SEMA4_Lock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM);

        /* Now SEMA4 gate is locked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == DEMO_PROC_NUM);

        /* Check other gates are not locked by mistake. */
        for (i = 0U; i < gateNum; i++)
        {
            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == -1);
        }
        for (i++; i < FSL_FEATURE_SEMA4_GATE_COUNT; i++)
        {
            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == -1);
        }

        SEMA4_Unlock(DEMO_SEMA4, gateNum);

        /* Now SEMA4 gate is unlocked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == -1);

        PRINTF("Gate lock&unlock success\r\n");
    }
}

static void DEMO_SEMA4_ResetGate(void)
{
    uint32_t gateNum = 0U;
    uint32_t i       = 0U;

    PRINTF("\r\n\r\nSEMA42 GATE RESET\r\n");

    DEMO_CHECK(kStatus_Success == SEMA4_ResetAllGates(DEMO_SEMA4));

    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        PRINTF("Lock gate %d then reset it\r\n", gateNum);

        SEMA4_Lock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM);

        /* Now SEMA4 gate is locked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == DEMO_PROC_NUM);

        /* Reset the gate. */
        DEMO_CHECK(kStatus_Success == SEMA4_ResetGate(DEMO_SEMA4, gateNum));

        /* Now SEMA4 gate is unlocked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == -1);
    }

    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        PRINTF("Lock all gates then reset gate %d\r\n", gateNum);

        /* Lock all gates. */
        for (i = 0U; i < FSL_FEATURE_SEMA4_GATE_COUNT; i++)
        {
            SEMA4_Lock(DEMO_SEMA4, i, DEMO_PROC_NUM);

            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == DEMO_PROC_NUM);
        }

        /* Reset the gate. */
        DEMO_CHECK(kStatus_Success == SEMA4_ResetGate(DEMO_SEMA4, gateNum));

        /* Now SEMA4 gate is unlocked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == -1);

        /* Check other gate state to make sure no unlock by mistake. */
        for (i = 0U; i < gateNum; i++)
        {
            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == DEMO_PROC_NUM);
        }
        for (i++; i < FSL_FEATURE_SEMA4_GATE_COUNT; i++)
        {
            DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, i) == DEMO_PROC_NUM);
        }

        DEMO_CHECK(kStatus_Success == SEMA4_ResetAllGates(DEMO_SEMA4));
    }
}

static void DEMO_SEMA4_ResetAllGate(void)
{
    uint32_t gateNum = 0U;

    PRINTF("\r\n\r\nSEMA42 ALL GATE RESET\r\n");

    DEMO_CHECK(kStatus_Success == SEMA4_ResetAllGates(DEMO_SEMA4));

    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        PRINTF("Lock gate %d and reset all\r\n", gateNum);

        SEMA4_Lock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM);

        /* Now SEMA4 gate is locked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == DEMO_PROC_NUM);

        /* Reset all gates. */
        DEMO_CHECK(kStatus_Success == SEMA4_ResetAllGates(DEMO_SEMA4));

        /* Now SEMA4 gate is unlocked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == -1);
    }

    PRINTF("Lock all gates and reset all\r\n");

    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        SEMA4_Lock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM);

        /* Now SEMA4 gate is locked. */
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == DEMO_PROC_NUM);
    }

    /* Reset all the gates. */
    DEMO_CHECK(kStatus_Success == SEMA4_ResetAllGates(DEMO_SEMA4));

    /* Check the gate status, should be unlocked. */
    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        DEMO_CHECK(SEMA4_GetLockProc(DEMO_SEMA4, gateNum) == -1);
    }
}
