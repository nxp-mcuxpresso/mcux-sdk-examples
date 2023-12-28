/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lpm.h"
#include "fsl_pgmc.h"
#include "fsl_clock.h"
#include "fsl_dcdc.h"
#include "fsl_soc_src.h"
#include "fsl_pmu.h"
#include "fsl_debug_console.h"
#include "fsl_anatop_ai.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t g_savedPrimask;
const char *g_cpuModeNames[] = {"RUN", "WAIT", "STOP", "SUSPEND"};

/*******************************************************************************
 * Code
 ******************************************************************************/
void GPC_EnableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(GPC_CPU_MODE_CTRL, irq, true);
}

void GPC_DisableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(GPC_CPU_MODE_CTRL, irq, false);
}

void GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL_Type *base)
{
    uint8_t i;

    for (i = 0; i < GPC_CPU_MODE_CTRL_CM_IRQ_WAKEUP_MASK_COUNT; i++)
    {
        base->CM_IRQ_WAKEUP_MASK[i] |= 0xFFFFFFFF;
    }
}

AT_QUICKACCESS_SECTION_CODE(void SystemEnterSleepMode(gpc_cpu_mode_t cpuMode));
void SystemEnterSleepMode(gpc_cpu_mode_t cpuMode)
{
    assert(cpuMode != kGPC_RunMode);

    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();

    if (cpuMode == kGPC_WaitMode)
    {
        /* Clear the SLEEPDEEP bit to go into sleep mode (WAIT) */
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }
    else /* STOP and SUSPEND mode */
    {
        /* Set the SLEEPDEEP bit to enable deep sleep mode (STOP) */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
    /* WFI instruction will start entry into WAIT/STOP mode */
    __WFI();

    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

/*!
 * @brief CpuModeTransition
 *
 * The function set the CPU mode and trigger a sleep event.
 *
 * @param cpuMode The CPU mode that the core will transmit to, refer to "gpc_cpu_mode_t"
 * @param stbyEn=true only means a standby request will be asserted to GPC when CPU in a low power mode.
 *          a) For single-core system: GPC will push system to standby mode as request.
 *          b) For multi-core system: GPC pushes system to standby mode only when ALL cores request standby mode.
 *                                 Thus system may be NOT in standby mode if any core is still in RUN mode.
 */
void CpuModeTransition(gpc_cpu_mode_t cpuMode, bool stbyEn)
{
    assert(cpuMode != kGPC_RunMode);

    GPC_CM_SetNextCpuMode(GPC_CPU_MODE_CTRL, cpuMode);
    GPC_CM_EnableCpuSleepHold(GPC_CPU_MODE_CTRL, true);

    GPC_CPU_MODE_CTRL->CM_NON_IRQ_WAKEUP_MASK |=
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_EVENT_WAKEUP_MASK_MASK |
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_DEBUG_WAKEUP_MASK_MASK; /* Mask debugger wakeup */

    if (stbyEn == true)
    {
        GPC_CM_RequestStandbyMode(GPC_CPU_MODE_CTRL, cpuMode);
    }
    else
    {
        GPC_CM_ClearStandbyModeRequest(GPC_CPU_MODE_CTRL, cpuMode);
    }
    SystemEnterSleepMode(cpuMode);
}

/*!
 * @brief PowerModeTransition
 *
 * The function which will trigger system power mode transition.
 *
 * @param cpuMode The CPU mode that the core will transmit to, refer to "gpc_cpu_mode_t"
 * @param sleepSp The setpoint in CPU low power mode(WAIT/STOP/SUSPEND)
 * @param wakeupSp The wakeup setpoint when wakeupSel is 0.
 * @param wakeupSel wake up setpoint selection.
               0: system will go to the 'wakeupSp' setpoint.
               1: system will go back to the setpoint before entering low power mode.

 * @note 1. In multi-core system, CPU can independently decide which setpoint it wants to go.
 *          GPC will arbitrate and decide the final target setpoint per the allowed setpoint list of
 *          each CPU(CM_RUN_MODE_MAPPING/CM_WAIT_MODE_MAPPING/CM_STOP_MODE_MAPPING/CM_SUSPEND_MODE_MAPPING).
 *       2. stbyEn=true only means a standby request will be asserted to GPC when CPU in a low power mode.
 *          a) For single-core system: GPC will push system to standby mode as request.
 *          b) For multi-core system: GPC pushes system to standby mode only when ALL cores request standby mode.
 *                                    Thus system may be NOT in standby mode if any core is still in RUN mode.
 */
void PowerModeTransition(
    gpc_cpu_mode_t cpuMode, uint8_t sleepSp, uint8_t wakeupSp, gpc_cm_wakeup_sp_sel_t wakeupSel, bool stbyEn)
{
    assert(cpuMode != kGPC_RunMode);

    GPC_CM_RequestSleepModeSetPointTransition(GPC_CPU_MODE_CTRL, sleepSp, wakeupSp, wakeupSel);
    CpuModeTransition(cpuMode, stbyEn);
}

/* Get corresponding CPU mode based on core status. */
gpc_cpu_mode_t getCpuMode(core_status_t coreStatus)
{
    gpc_cpu_mode_t cpuMode = kGPC_RunMode;

    switch (coreStatus)
    {
        case kCORE_NormalRun:
            cpuMode = kGPC_RunMode;
            break;
        case kCORE_ClockGated:
            /* CPU mode could be wait or stop when core clock gated. Here we return a lower power CPU mode. */
            cpuMode = kGPC_StopMode;
            break;
        case kCORE_PowerGated:
            cpuMode = kGPC_SuspendMode;
            break;
        default:
            assert(0);
            break;
    }

    return cpuMode;
}
