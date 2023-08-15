/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"	
#include "fsl_debug_console.h"	
#include "fsl_mu.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#define APP_CHN_MU_REG_NUM (0U)
#define APP_MU_GENINT_ENABLE (kMU_GenInt0InterruptEnable)
#define APP_MU_RXFULLINT_ENABLE (kMU_Rx0FullInterruptEnable)

#define APP_SETPOINT0_CONSTRAINTS       \
    1U, PM_RESC_CORE_DOMAIN_WAIT
#define APP_SETPOINT1_CONSTRAINTS       \
    3U, PM_RESC_CORE_DOMAIN_WAIT, PM_RESC_SYS_PLL1_ON, PM_RESC_LPSR_DIG_LDO_OFF
#define APP_SETPOINT10_CONSTRAINTS      \
    2U, PM_RESC_CORE_DOMAIN_SUSPEND, PM_RESC_WAKEUP_MIX_ON
#define APP_SETPOINT15_CONSTRAINTS      \
    2U, PM_RESC_CORE_DOMAIN_SUSPEND, PM_RESC_OSC_RC_16M_ON

/*******************************************************************************
* Prototypes
******************************************************************************/
void APP_InitWakeupSource(void);
void APP_RegisterNotify(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);
uint32_t APP_GetWakeupTimeout(void);
#define BOOT_FLAG 0x01U
#define REV_TARGET_MODE_FLAG 0x02U
#define REV_TIMEOUT_VALUE_FLAG 0x03U
/*******************************************************************************
 * Variables
 ******************************************************************************/
pm_wakeup_source_t g_MuWakeup;
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);

static volatile uint32_t g_msgRecv;
static volatile bool isMsgReceived = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_RegisterNotify(void)
{
    return;
}

void APP_InitWakeupSource(void)
{
    // For secondary core, set MU interrupt as wakeup source.
    // Primary core will trigger general interrupt to wake up secondary core.
    PM_InitWakeupSource(&g_MuWakeup, PM_WSID_MUB_IRQ, NULL, true);
}

void APP_SetConstraints(uint8_t powerState)
{
    switch(powerState)
    {
        // setpoint 0
        case 0:
        {
            PM_SetConstraints(PM_LP_STATE_SP0, APP_SETPOINT0_CONSTRAINTS);
            break;
        }
        // setpoint 1
        case 1:
        {
            PM_SetConstraints(PM_LP_STATE_SP1, APP_SETPOINT1_CONSTRAINTS);
            break;
        }
        // setpoint 10
        case 2:
        {
            PM_SetConstraints(PM_LP_STATE_SP10, APP_SETPOINT10_CONSTRAINTS);
            break;
        }
        // setpoint 15
        case 3:
        {
            PM_SetConstraints(PM_LP_STATE_SP15, APP_SETPOINT15_CONSTRAINTS);
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerState)
{
    switch(powerState)
    {
        // setpoint 0
        case 0:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP0, APP_SETPOINT0_CONSTRAINTS);
            break;
        }
        // setpoint 1
        case 1:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP1, APP_SETPOINT1_CONSTRAINTS);
            break;
        }
        // setpoint 10
        case 2:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP10, APP_SETPOINT10_CONSTRAINTS);
            break;
        }
        // setpoint 15
        case 3:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SP15, APP_SETPOINT15_CONSTRAINTS);
            break;
        }
    }
}


void MU_IRQ_HANDLER(void)
{
    if (kMU_Rx0FullFlag & MU_GetStatusFlags(MU_BASE))
    {
        g_msgRecv     = MU_ReceiveMsgNonBlocking(MU_BASE, APP_CHN_MU_REG_NUM);
        isMsgReceived = true;
        /* We do not disable MU interrupt here since we always get input from core0. */
    }
    if (kMU_GenInt0Flag & MU_GetStatusFlags(MU_BASE))
    {
        // Primary core trigger general interrupt.
        MU_ClearStatusFlags(MU_BASE, kMU_GenInt0Flag);
    }
    SDK_ISR_EXIT_BARRIER;
}

int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
  
    MU_Init(MU_BASE);

    // Sync with primary core.
    MU_SetFlags(MU_BASE, BOOT_FLAG);
    // Enable General Interrupt and Receive Interrupt.
    MU_EnableInterrupts(MU_BASE, APP_MU_GENINT_ENABLE | APP_MU_RXFULLINT_ENABLE);
    EnableIRQ(MU_IRQ);

    /* Init Power Manager */
    PM_CreateHandle(&g_pmHandle);

    APP_RegisterNotify();
    APP_InitWakeupSource();
    while (1)
    {
        // Polling to get target power mode from primary core.
        while(!isMsgReceived)
        {
        }
        isMsgReceived = false;
        g_targetPowerMode = (uint8_t)g_msgRecv;

        // Inform primary core that target power mode is received.
        MU_SetFlags(MU_BASE, REV_TARGET_MODE_FLAG);

        while(!isMsgReceived)
        {
        }
        isMsgReceived = false;
        uint32_t timeoutUs = g_msgRecv;
        // Inform primary core that timeout is received.
        MU_SetFlags(MU_BASE, REV_TIMEOUT_VALUE_FLAG);

        APP_SetConstraints(g_targetPowerMode);
        g_irqMask = DisableGlobalIRQ();
        PM_EnablePowerManager(true);
        PM_EnterLowPower(timeoutUs);
        PM_EnablePowerManager(false);
        EnableGlobalIRQ(g_irqMask);
        APP_ReleaseConstraints(g_targetPowerMode);
    }
}
