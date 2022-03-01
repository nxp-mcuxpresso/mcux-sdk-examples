/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include <stdint.h>
#include "fsl_gpc.h"
#include "chip_init_def.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if __CORTEX_M == 7
#define GPC_CPU_MODE_CTRL GPC_CPU_MODE_CTRL_0
#elif __CORTEX_M == 4
#define GPC_CPU_MODE_CTRL GPC_CPU_MODE_CTRL_1
#endif
#define GET_CPU_MODE_NAME(mode) (g_cpuModeNames[(uint8_t)mode])

typedef enum _core_status
{
    kCORE_NormalRun  = 0x0UL,
    kCORE_ClockGated = 0x1UL,
    kCORE_PowerGated = 0x2UL,
} core_status_t;

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceTimer, /*!< Wakeup by Timer.        */
    kAPP_WakeupSourcePin,   /*!< Wakeup by external pin. */
} app_wakeup_source_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

extern const char *g_cpuModeNames[];

gpc_cpu_mode_t getCpuMode(core_status_t coreStatus);
core_status_t getCore0Status(uint8_t setpoint);
core_status_t getCore1Status(uint8_t setpoint);
void GPC_EnableWakeupSource(uint32_t irq);
void GPC_DisableWakeupSource(uint32_t irq);
void GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL_Type *base);
void ChipInitConfig(void);
void CpuModeTransition(gpc_cpu_mode_t cpuMode, bool stbyEn);
void PowerModeTransition(
    gpc_cpu_mode_t cpuMode, uint8_t sleepSp, uint8_t wakeupSp, gpc_cm_wakeup_sp_sel_t wakeupSel, bool stbyEn);
void PrintSystemStatus(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _LPM_H_ */
