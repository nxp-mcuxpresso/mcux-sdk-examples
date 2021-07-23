/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sema4.h"
#include "fsl_mu.h"
#include "demo_common.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_MU       MUB
#define DEMO_SEMA4    SEMA4
#define DEMO_PROC_NUM 1 /* Fixed value by system integration. */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * For the dual core project, generally primary core starts first, initializes
 * the system, then starts the secondary core to run.
 * In the case that debugging dual-core at the same time (for example, using IAR+DAPLink),
 * the secondary core is started by debugger. Then the secondary core might
 * run when the primary core initialization not finished. The SRC->GPR is used
 * here to indicate whether secondary core could go. When started, the secondary core
 * should check and wait the flag in SRC->GPR, the primary core sets the
 * flag in SRC->GPR when its initialization work finished.
 */
#define BOARD_SECONDARY_CORE_GO_FLAG 0xa5a5a5a5u
#define BOARD_SECONDARY_CORE_SRC_GPR kSRC_GeneralPurposeRegister20

void BOARD_WaitAndClearSecondaryCoreGoFlag(void)
{
    while (BOARD_SECONDARY_CORE_GO_FLAG != SRC_GetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR))
    {
    }

    SRC_SetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR, 0x0);
}


int main(void)
{
    uint32_t cmd;
    uint32_t gateNum;

    BOARD_WaitAndClearSecondaryCoreGoFlag();
    BOARD_InitPins();

    MU_Init(DEMO_MU);

    SEMA4_Init(DEMO_SEMA4);

    /* Notify core 0 that ready for the demo. */
    MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_STAT_CORE1_READY);

    /* Handle the command from core 0. */
    while (1)
    {
        cmd = MU_ReceiveMsg(DEMO_MU, DEMO_MU_CH);

        if (DEMO_IS_LOCK_CMD(cmd))
        {
            gateNum = DEMO_GET_LOCK_CMD_GATE(cmd);

            SEMA4_Lock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM);

            MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_STAT_GATE_LOCKED(gateNum));
        }
        else if (DEMO_IS_UNLOCK_CMD(cmd))
        {
            gateNum = DEMO_GET_LOCK_CMD_GATE(cmd);

            SEMA4_Unlock(DEMO_SEMA4, gateNum);

            MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_STAT_GATE_UNLOCKED(gateNum));
        }
        else
        {
            /* Empty. */
        }
    }
}
