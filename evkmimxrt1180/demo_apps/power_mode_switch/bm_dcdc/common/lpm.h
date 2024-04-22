/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include <stdint.h>
#include "fsl_gpc.h"
#include "fsl_dcdc.h"
#include "fsl_soc_src.h"
#include "chip_init_def.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if __CORTEX_M == 33
#define CPU_SLICE kGPC_CPU0
#elif __CORTEX_M == 7
#define CPU_SLICE kGPC_CPU1
#endif
#define GET_CPU_MODE_NAME(mode) (g_cpuModeNames[(uint8_t)mode])

typedef enum _run_mode
{
    kLPM_OverDrive  = 0x0UL,
    kLPM_NoramlRun  = 0x1UL,
    kLPM_UnderDrive = 0x2UL,
} run_mode_t;

typedef struct _run_mode_config
{
    uint8_t grade;
    uint8_t pmicVoltage; /* PMIC voltage control register value. */
    dcdc_1P0_target_vol_t targetVoltage;
    uint16_t cm7Freq;
    clock_root_mux_source_t cm33Source;
    uint8_t cm33Divider;
    clock_root_mux_source_t edgelockSource;
    uint8_t edgelockDivider;
    clock_root_mux_source_t busAonSource;
    uint8_t busAonDivider;
    clock_root_mux_source_t busWakeupSource;
    uint8_t busWakeupDivider;
    clock_root_mux_source_t wakeupAxiSource;
    uint8_t wakeupAxiDivider;
} run_mode_config_t;

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceTimer, /*!< Wakeup by Timer.        */
    kAPP_WakeupSourcePin,   /*!< Wakeup by external pin. */
} app_wakeup_source_t;

typedef struct _src_config
{
    SRC_MIX_SLICE_Type *sliceName;
    uint8_t ctrlMode;
    src_power_level_t powerLevel;
} src_config_t;

typedef struct _mix_config
{
    uint8_t ctrlMode;
    SRC_MIX_SLICE_Type *mix;
    src_power_level_t level;
} mix_config_t;

typedef struct _clock_source_config
{
    uint8_t ctrlMode;
    clock_level_t level;
} clock_source_config_t;

typedef struct _lpcg_config
{
    uint8_t ctrlMode;
    clock_level_t level;
} lpcg_config_t;

typedef struct _oscpll_config
{
    uint8_t ctrlMode;
    clock_level_t level;
} oscpll_config_t;

typedef enum _gpc_enable
{
    kGPC_Disable = 0UL,
    kGPC_Enable  = 1UL,
} gpc_enable_t;
/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

extern const char *g_cpuModeNames[];

void GPC_EnableWakeupSource(uint32_t irq);
void GPC_DisableWakeupSource(uint32_t irq);
void GPC_DisableAllWakeupSource(gpc_cpu_slice_t slice);
void ChipInitConfig(void);
void CpuModeTransition(gpc_cpu_mode_t cpuMode, bool sysSleepEn);
void RunModeTransition(run_mode_t TargetRun);
void PrintSystemStatus(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _LPM_H_ */
