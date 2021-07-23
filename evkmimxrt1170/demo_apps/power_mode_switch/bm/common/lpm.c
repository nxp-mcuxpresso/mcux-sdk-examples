/*
 * Copyright 2020 NXP
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
#include "chip_init_def.h"
#include "fsl_anatop_ai.h"
#include "fsl_ssarc.h"

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

void GPC_ConfigCore0SetpointMapping()
{
    uint8_t i, j;
    uint32_t tmp;
    uint8_t cmSpMapping[17][17] = CPU0_COMPATIBLE_SP_TABLE;

    for (i = 1; i < 17; i++)
    {
        tmp = 0;
        for (j = 1; j < 17; j++)
        {
            tmp |= cmSpMapping[i][j] << cmSpMapping[0][j];
        }
        GPC_CM_SetSetPointMapping(GPC_CPU_MODE_CTRL_0, cmSpMapping[i][0], tmp);
    }
}

void GPC_ConfigCore1SetpointMapping()
{
    uint8_t i, j;
    uint32_t tmp;
    uint8_t cmSpMapping[17][17] = CPU1_COMPATIBLE_SP_TABLE;

    for (i = 1; i < 17; i++)
    {
        tmp = 0;
        for (j = 1; j < 17; j++)
        {
            tmp |= cmSpMapping[i][j] << cmSpMapping[0][j];
        }
        GPC_CM_SetSetPointMapping(GPC_CPU_MODE_CTRL_1, cmSpMapping[i][0], tmp);
    }
}

//---------------------------------------------------------------------------
// Name:gpc_cm_fsm_ctrl
// Function:bypass all steps in the flow of run to idle sleep
//---------------------------------------------------------------------------
void GPC_ConfigCore0CpuModeTransitionFlow()
{
    gpc_tran_step_config_t tranStepConfig;

    tranStepConfig.enableStep = true;
    tranStepConfig.cntMode    = kGPC_StepCounterDisableMode;
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_SleepSsar, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_SleepLpcg, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_SleepPll, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_SleepIso, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_SleepReset, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_SleepPower, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_WakeupPower, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_WakeupReset, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_WakeupIso, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_WakeupPll, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_WakeupLpcg, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_0, kGPC_CM_WakeupSsar, &tranStepConfig);
}

void GPC_ConfigCore1CpuModeTransitionFlow()
{
    gpc_tran_step_config_t tranStepConfig;

    tranStepConfig.enableStep = true;
    tranStepConfig.cntMode    = kGPC_StepCounterDisableMode;
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_SleepSsar, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_SleepLpcg, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_SleepPll, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_SleepIso, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_SleepReset, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_SleepPower, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_WakeupPower, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_WakeupReset, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_WakeupIso, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_WakeupPll, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_WakeupLpcg, &tranStepConfig);
    GPC_CM_ConfigCpuModeTransitionStep(GPC_CPU_MODE_CTRL_1, kGPC_CM_WakeupSsar, &tranStepConfig);
}

//---------------------------------------------------------------------------
// Name:gpc_sp_disable_all_ctrl
// Function:bypass all steps in the flow of SP
//---------------------------------------------------------------------------
void GPC_ConfigSetpointTransitionFlow()
{
    gpc_tran_step_config_t tranStepConfig;

    tranStepConfig.enableStep = true;
    tranStepConfig.cntMode    = kGPC_StepCounterDisableMode;
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_SsarSave, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_LpcgOff, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_GroupDown, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_RootDown, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_PllOff, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_IsoOn, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_ResetEarly, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_PowerOff, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_BiasOff, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_BandgapPllLdoOff, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_LdoPre, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_DcdcDown, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_DcdcUp, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_LdoPost, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_BandgapPllLdoOn, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_BiasOn, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_PowerOn, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_ResetLate, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_IsoOff, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_PllOn, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_RootUp, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_GroupUp, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_LpcgOn, &tranStepConfig);
    GPC_SP_ConfigSetPointTransitionStep(GPC_SET_POINT_CTRL, kGPC_SP_SsarRestore, &tranStepConfig);
}

//---------------------------------------------------------------------------
// Name:gpc_stby_disable_all_ctrl
// Function:bypass all steps in the flow of STBY
//---------------------------------------------------------------------------
void GPC_ConfigStbyTransitionFlow()
{
    gpc_tran_step_config_t tranStepConfig;

    tranStepConfig.enableStep = true;
    tranStepConfig.cntMode    = kGPC_StepCounterDisableMode;
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_LpcgIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_PllIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_BiasIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_PldoIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_BandgapIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_LdoIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_DcdcIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_PmicIn, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_PmicOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_DcdcOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_LdoOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_BandgapOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_PldoOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_BiasOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_PllOut, &tranStepConfig);
    GPC_STBY_ConfigStandbyTransitionStep(GPC_STBY_CTRL, kGPC_STBY_LpcgOut, &tranStepConfig);
}

void GPC_ConfigROSC()
{
    GPC_SET_POINT_CTRL->SP_ROSC_CTRL = ~OSC_RC_16M_STBY_VAL;
}

void GPC_InitConfig()
{
    GPC_ConfigCore0SetpointMapping();
    GPC_ConfigCore0CpuModeTransitionFlow();
#ifndef SINGLE_CORE_M7
    GPC_ConfigCore1SetpointMapping();
    GPC_ConfigCore1CpuModeTransitionFlow();
#endif
    GPC_ConfigSetpointTransitionFlow();
    GPC_ConfigStbyTransitionFlow();
    GPC_ConfigROSC();
}

dcdc_buck_mode_1P8_target_vol_t buck1P8Voltage[16] = {
    kDCDC_1P8BuckTarget1P8V, // SP0
    kDCDC_1P8BuckTarget1P8V, // SP1
    kDCDC_1P8BuckTarget1P8V, // SP2
    kDCDC_1P8BuckTarget1P8V, // SP3
    kDCDC_1P8BuckTarget1P8V, // SP4
    kDCDC_1P8BuckTarget1P8V, // SP5
    kDCDC_1P8BuckTarget1P8V, // SP6
    kDCDC_1P8BuckTarget1P8V, // SP7
    kDCDC_1P8BuckTarget1P8V, // SP8
    kDCDC_1P8BuckTarget1P8V, // SP9
    kDCDC_1P8BuckTarget1P8V, // SP10
    kDCDC_1P8BuckTarget1P8V, // SP11
    kDCDC_1P8BuckTarget1P8V, // SP12
    kDCDC_1P8BuckTarget1P8V, // SP13
    kDCDC_1P8BuckTarget1P8V, // SP14
    kDCDC_1P8BuckTarget1P8V, // SP15
};
dcdc_buck_mode_1P0_target_vol_t buck1P0Voltage[16]       = DCDC_1P0_BUCK_MODE_CONFIGURATION_TABLE;
dcdc_standby_mode_1P8_target_vol_t standby1P8Voltage[16] = {
    kDCDC_1P8StbyTarget1P8V, // SP0
    kDCDC_1P8StbyTarget1P8V, // SP1
    kDCDC_1P8StbyTarget1P8V, // SP2
    kDCDC_1P8StbyTarget1P8V, // SP3
    kDCDC_1P8StbyTarget1P8V, // SP4
    kDCDC_1P8StbyTarget1P8V, // SP5
    kDCDC_1P8StbyTarget1P8V, // SP6
    kDCDC_1P8StbyTarget1P8V, // SP7
    kDCDC_1P8StbyTarget1P8V, // SP8
    kDCDC_1P8StbyTarget1P8V, // SP9
    kDCDC_1P8StbyTarget1P8V, // SP10
    kDCDC_1P8StbyTarget1P8V, // SP11
    kDCDC_1P8StbyTarget1P8V, // SP12
    kDCDC_1P8StbyTarget1P8V, // SP13
    kDCDC_1P8StbyTarget1P8V, // SP14
    kDCDC_1P8StbyTarget1P8V, // SP15
};
dcdc_standby_mode_1P0_target_vol_t standby1P0Voltage[16] = DCDC_1P0_STANDBY_MODE_CONFIGURATION_TABLE;

void DCDC_InitConfig(void)
{
    dcdc_config_t dcdcConfig;
    dcdc_setpoint_config_t dcdcSetpointConfig;

    DCDC_BootIntoDCM(DCDC);

    dcdcSetpointConfig.enableDCDCMap              = DCDC_ONOFF_SP_VAL;
    dcdcSetpointConfig.enableDigLogicMap          = DCDC_DIG_ONOFF_SP_VAL;
    dcdcSetpointConfig.lowpowerMap                = DCDC_LP_MODE_SP_VAL;
    dcdcSetpointConfig.standbyMap                 = DCDC_ONOFF_STBY_VAL;
    dcdcSetpointConfig.standbyLowpowerMap         = DCDC_LP_MODE_STBY_VAL;
    dcdcSetpointConfig.buckVDD1P8TargetVoltage    = buck1P8Voltage;
    dcdcSetpointConfig.buckVDD1P0TargetVoltage    = buck1P0Voltage;
    dcdcSetpointConfig.standbyVDD1P8TargetVoltage = standby1P8Voltage;
    dcdcSetpointConfig.standbyVDD1P0TargetVoltage = standby1P0Voltage;
    DCDC_SetPointInit(DCDC, &dcdcSetpointConfig);

    DCDC_GetDefaultConfig(&dcdcConfig);
    dcdcConfig.controlMode = kDCDC_SetPointControl;
    DCDC_Init(DCDC, &dcdcConfig);
}

typedef struct _pgmc_config
{
    uint8_t ctrlType;
    uint8_t domainType;
    uint8_t sliceID;
    uint8_t ctrlMode;
    uint32_t spConfig;
} pgmc_config_t;

void PGMC_InitConfig()
{
    uint8_t i, index;
    uint32_t spVal;
    PGMC_BPC_Type *bpcBase;
    PGMC_CPC_Type *cpcBase;
    PGMC_PPC_Type *ppcBase;
    pgmc_bpc_cpu_power_mode_option_t bpcCpuModeOption;
    pgmc_bpc_setpoint_mode_option_t bpcSetpointOption;
    static const pgmc_config_t pgmcCfg[POWER_DOMAIN_NUM] = PGMC_CONFIGURATION_TABLE;

    PGMC_BPC_ControlPowerDomainBySoftwareMode(PGMC_BPC4, false);

    for (i = 0; i < POWER_DOMAIN_NUM; i++)
    {
        index = pgmcCfg[i].sliceID;
        spVal = pgmcCfg[i].spConfig;
        if (pgmcCfg[i].ctrlType == BPC)
        {
            bpcBase = (PGMC_BPC_Type *)(PGMC_BPC0_BASE + 0x200 * index);
            if (pgmcCfg[i].ctrlMode == SP_CTRL)
            {
                bpcSetpointOption.stateSave = false;
                bpcSetpointOption.powerOff  = true;
                PGMC_BPC_ControlPowerDomainBySetPointMode(bpcBase, spVal, &bpcSetpointOption);
            }
            else if (pgmcCfg[i].ctrlMode == CM7_DOMAIN)
            {
                bpcCpuModeOption.assignDomain = kPGMC_CM7Core;
                bpcCpuModeOption.stateSave    = false;
                bpcCpuModeOption.powerOff     = true;
                PGMC_BPC_ControlPowerDomainByCpuPowerMode(bpcBase, kPGMC_SuspendMode, &bpcCpuModeOption);
            }
#ifndef SINGLE_CORE_M7
            else if (pgmcCfg[i].ctrlMode == CM4_DOMAIN)
            {
                bpcCpuModeOption.assignDomain = kPGMC_CM4Core;
                bpcCpuModeOption.stateSave    = false;
                bpcCpuModeOption.powerOff     = true;
                PGMC_BPC_ControlPowerDomainByCpuPowerMode(bpcBase, kPGMC_SuspendMode, &bpcCpuModeOption);
            }
#endif
        }
        else if (pgmcCfg[i].ctrlType == CPC)
        {
            cpcBase = (PGMC_CPC_Type *)(PGMC_CPC0_BASE + 0x400 * index);
            if (pgmcCfg[i].domainType == PD_TYPE_CORE)
            {
                PGMC_CPC_CORE_PowerOffByCpuPowerMode(cpcBase, kPGMC_SuspendMode);
            }
            else if (pgmcCfg[i].domainType == PD_TYPE_LMEM)
            {
                if (pgmcCfg[i].ctrlMode == SP_CTRL)
                {
                    PGMC_CPC_LMEM_ControlBySetPointMode(cpcBase, spVal, kPGMC_MLPLArrOffPerOff);
                }
                else if ((pgmcCfg[i].ctrlMode == CM7_DOMAIN))
                {
                    PGMC_CPC_LMEM_ControlByCpuPowerMode(cpcBase, kPGMC_SuspendMode, kPGMC_MLPLArrOffPerOff);
                }
#ifndef SINGLE_CORE_M7
                else if ((pgmcCfg[i].ctrlMode == CM4_DOMAIN))
                {
                    PGMC_CPC_LMEM_ControlByCpuPowerMode(cpcBase, kPGMC_SuspendMode, kPGMC_MLPLArrOffPerOff);
                }
#endif
            }
        }
        else if (pgmcCfg[i].ctrlType == PPC)
        {
            ppcBase = (PGMC_PPC_Type *)(PGMC_PPC0_BASE);
            if (pgmcCfg[i].ctrlMode == SP_CTRL)
            {
                PGMC_PPC_ControlBySetPointMode(ppcBase, spVal, true);
            }
            else if (pgmcCfg[i].ctrlMode == CM7_DOMAIN)
            {
                PGMC_PPC_ControlByCpuPowerMode(ppcBase, kPGMC_SuspendMode);
            }
#ifndef SINGLE_CORE_M7
            else if (pgmcCfg[i].ctrlMode == CM4_DOMAIN)
            {
                PGMC_PPC_ControlByCpuPowerMode(ppcBase, kPGMC_SuspendMode);
            }
#endif
        }
    }
#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    PGMC_BPC2->BPC_SSAR_SAVE_CTRL &= ~PGMC_BPC_BPC_SSAR_SAVE_CTRL_SAVE_AT_SP_MASK;
    PGMC_BPC2->BPC_SSAR_SAVE_CTRL |= PGMC_BPC_BPC_SSAR_SAVE_CTRL_SAVE_AT_SP(PD_WKUP_SP_VAL);
    PGMC_BPC2->BPC_SSAR_RESTORE_CTRL &= ~PGMC_BPC_BPC_SSAR_RESTORE_CTRL_RESTORE_AT_SP_MASK;
    PGMC_BPC2->BPC_SSAR_RESTORE_CTRL |= PGMC_BPC_BPC_SSAR_RESTORE_CTRL_RESTORE_AT_SP(~PD_WKUP_SP_VAL);
#endif
}

typedef struct _src_config
{
    src_reset_slice_name_t sliceName;
    uint8_t ctrlMode;
    uint16_t spConfig;
} src_config_t;

void SRC_InitConfig()
{
    uint8_t i;
    static const src_config_t srcCfg[SRC_SLICE_NUM] = SRC_CONFIGURATION_TABLE;

    SRC_ClearGlobalSystemResetStatus(SRC, 0xffffffff);
    SRC_SetGlobalSystemResetMode(SRC, kSRC_Wdog3Reset, kSRC_DoNotResetSystem);
    SRC_SetGlobalSystemResetMode(SRC, kSRC_Wdog4Reset, kSRC_DoNotResetSystem);

    for (i = 0; i < SRC_SLICE_NUM; i++)
    {
        if (srcCfg[i].ctrlMode == SP_CTRL)
        {
            SRC_SetSliceSetPointConfig(SRC, srcCfg[i].sliceName, srcCfg[i].spConfig);
            SRC_EnableSetPointTransferReset(SRC, srcCfg[i].sliceName, true);
            SRC_LockSliceMode(SRC, srcCfg[i].sliceName);
        }
        else if (srcCfg[i].ctrlMode == CM7_DOMAIN)
        {
            SRC_SetAssignList(SRC, srcCfg[i].sliceName, CM7_DOMAIN);
            SRC_LockAssignList(SRC, srcCfg[i].sliceName);
            SRC_SetSliceDomainModeConfig(SRC, srcCfg[i].sliceName, kSRC_Cpu0SuspendModeAssertReset);
            SRC_EnableDomainModeTransferReset(SRC, srcCfg[i].sliceName, true);
            SRC_LockSliceMode(SRC, srcCfg[i].sliceName);
        }
#ifndef SINGLE_CORE_M7
        else if (srcCfg[i].ctrlMode == CM4_DOMAIN)
        {
            SRC_SetAssignList(SRC, srcCfg[i].sliceName, CM4_DOMAIN);
            SRC_LockAssignList(SRC, srcCfg[i].sliceName);
            SRC_SetSliceDomainModeConfig(SRC, srcCfg[i].sliceName, kSRC_Cpu1SuspendModeAssertReset);
            SRC_EnableDomainModeTransferReset(SRC, srcCfg[i].sliceName, true);
            SRC_LockSliceMode(SRC, srcCfg[i].sliceName);
        }
#endif
    }
}

typedef struct _clock_source_config
{
    uint8_t ctrlMode;
    uint16_t stbyValue;
    uint16_t spValue;
    clock_level_t level;
} clock_source_config_t;

typedef struct _lpcg_config
{
    uint8_t ctrlMode;
    uint16_t stbyValue;
    uint16_t spValue;
    clock_level_t level;
} lpcg_config_t;

void CCM_InitConfig()
{
    uint32_t index;
    const clock_root_setpoint_config_t *spTable;
    static const clock_root_setpoint_config_t m7SpCfg[16]          = CLK_ROOT_M7_SP_TABLE;
    static const clock_root_setpoint_config_t m4SpCfg[16]          = CLK_ROOT_M4_SP_TABLE;
    static const clock_root_setpoint_config_t busSpCfg[16]         = CLK_ROOT_BUS_SP_TABLE;
    static const clock_root_setpoint_config_t busLpsrSpCfg[16]     = CLK_ROOT_BUS_LPSR_SP_TABLE;
    static const uint8_t clkrootCfg[CLOCK_ROOT_NUM]                = CLK_ROOT_CONFIGURATION_TABLE;
    static const clock_source_config_t oscpllCfg[CLOCK_OSCPLL_NUM] = CLK_OSCPLL_CONFIGURATION_TABLE;
    static const lpcg_config_t lpcgCfg[CLOCK_LPCG_NUM]             = CLK_LPCG_CONFIGURATION_TABLE;

    for (index = 0; index < CLOCK_ROOT_NUM; index++)
    {
        if (clkrootCfg[index] == SP_CTRL)
        {
            if (!CLOCK_ROOT_IsSetPointImplemented((clock_root_t)index))
            {
                assert(0);
            }
            switch (index)
            {
                case kCLOCK_Root_M7:
                    spTable = m7SpCfg;
                    break;
                case kCLOCK_Root_M4:
                    spTable = m4SpCfg;
                    break;
                case kCLOCK_Root_Bus:
                    spTable = busSpCfg;
                    break;
                case kCLOCK_Root_Bus_Lpsr:
                    spTable = busLpsrSpCfg;
                    break;
                default:
                    /* please prepare setpoint table for other clock roots. */
                    assert(0);
                    break;
            }
            CLOCK_ROOT_ControlBySetPointMode((clock_root_t)index, spTable);
        }
        else if (clkrootCfg[index] == CM7_DOMAIN)
        {
            CLOCK_ROOT_ControlByDomainMode((clock_root_t)index, CM7_DOMAIN);
        }
#ifndef SINGLE_CORE_M7
        else if (clkrootCfg[index] == CM4_DOMAIN)
        {
            CLOCK_ROOT_ControlByDomainMode((clock_root_t)index, CM4_DOMAIN);
        }
#endif
    }

    for (index = 0; index < CLOCK_OSCPLL_NUM; index++)
    {
        if (oscpllCfg[index].ctrlMode == SP_CTRL)
        {
            if (!CLOCK_OSCPLL_IsSetPointImplemented((clock_name_t)index))
            {
                assert(0);
            }
            // keep clock source init state aligned with set point 0 state.
            if ((oscpllCfg[index].spValue & 0x1) == 0)
            {
                CCM->OSCPLL[index].DIRECT = 0;
            }
            else
            {
                CCM->OSCPLL[index].DIRECT = 1;
            }
            CLOCK_OSCPLL_ControlBySetPointMode((clock_name_t)index, oscpllCfg[index].spValue,
                                               oscpllCfg[index].stbyValue);
        }
        else if (oscpllCfg[index].ctrlMode == CM7_DOMAIN)
        {
            CLOCK_OSCPLL_ControlByCpuLowPowerMode((clock_name_t)index, CM7_DOMAIN, oscpllCfg[index].level,
                                                  oscpllCfg[index].level);
        }
#ifndef SINGLE_CORE_M7
        else if (oscpllCfg[index].ctrlMode == CM4_DOMAIN)
        {
            CLOCK_OSCPLL_ControlByCpuLowPowerMode((clock_name_t)index, CM4_DOMAIN, oscpllCfg[index].level,
                                                  oscpllCfg[index].level);
        }
#endif
    }

    for (index = 0; index < CLOCK_LPCG_NUM; index++)
    {
        if (lpcgCfg[index].ctrlMode == SP_CTRL)
        {
            if (!CLOCK_LPCG_IsSetPointImplemented((clock_lpcg_t)index))
            {
                assert(0);
            }
            CLOCK_LPCG_ControlBySetPointMode((clock_lpcg_t)index, lpcgCfg[index].spValue, lpcgCfg[index].stbyValue);
        }
        else if (lpcgCfg[index].ctrlMode == CM7_DOMAIN)
        {
            CLOCK_LPCG_ControlByCpuLowPowerMode((clock_lpcg_t)index, CM7_DOMAIN, lpcgCfg[index].level,
                                                lpcgCfg[index].level);
        }
#ifndef SINGLE_CORE_M7
        else if (lpcgCfg[index].ctrlMode == CM4_DOMAIN)
        {
            CLOCK_LPCG_ControlByCpuLowPowerMode((clock_lpcg_t)index, CM4_DOMAIN, lpcgCfg[index].level,
                                                lpcgCfg[index].level);
        }
#endif
    }
}

void PMU_InitConfig()
{
    uint8_t i;
    static const pmu_lpsr_dig_target_output_voltage_t anatopLdoCfg[16] = PMU_LDO_CONDIGURATION_TABLE;

    /* Check if FBB need to be enabled in OverDrive(OD) mode.
       Note: FUSE could not be read out if OTP memory is powered off.*/
    if (((OCOTP->FUSEN[7].FUSE & 0x10U) >> 4U) != 1)
    {
        PMU_GPCEnableBodyBias(kPMU_FBB_CM7, PMU_FBB_SP_VAL);
    }
    else
    {
        PMU_GPCEnableBodyBias(kPMU_FBB_CM7, 0);
    }
    PMU_GPCEnableBodyBias(kPMU_RBB_SOC, PMU_RBB_SOC_SP_VAL);
    PMU_GPCEnableBodyBias(kPMU_RBB_LPSR, PMU_RBB_LPSR_SP_VAL);
    PMU_GPCEnableLdo(kPMU_PllLdo, PMU_LDO_PLL_EN_SP_VAL);
    PMU_GPCEnableLdo(kPMU_LpsrAnaLdo, PMU_LDO_ANA_EN_SP_VAL);
    PMU_GPCSetLdoOperateMode(kPMU_LpsrAnaLdo, PMU_LDO_ANA_LP_MODE_SP_VAL, kPMU_HighPowerMode);
    PMU_GPCEnableLdoTrackingMode(kPMU_LpsrAnaLdo, PMU_LDO_ANA_TRACKING_EN_SP_VAL);
    PMU_GPCEnableLdoBypassMode(kPMU_LpsrAnaLdo, PMU_LDO_ANA_BYPASS_EN_SP_VAL);
    PMU_GPCEnableLdo(kPMU_LpsrDigLdo, PMU_LDO_DIG_EN_SP_VAL);
    PMU_GPCSetLdoOperateMode(kPMU_LpsrDigLdo, PMU_LDO_DIG_LP_MODE_SP_VAL, kPMU_HighPowerMode);
    PMU_GPCEnableLdoTrackingMode(kPMU_LpsrDigLdo, PMU_LDO_DIG_TRACKING_EN_SP_VAL);
    PMU_GPCEnableLdoBypassMode(kPMU_LpsrDigLdo, PMU_LDO_DIG_BYPASS_EN_SP_VAL);
    PMU_GPCEnableBandgap(ANADIG_PMU, PMU_BG_SP_VAL);

    PMU_GPCEnableBodyBiasStandbyMode(kPMU_FBB_CM7, PMU_FBB_STBY_VAL);
    PMU_GPCEnableBodyBiasStandbyMode(kPMU_RBB_SOC, PMU_RBB_SOC_STBY_VAL);
    PMU_GPCEnableBodyBiasStandbyMode(kPMU_RBB_LPSR, PMU_RBB_LPSR_STBY_VAL);
    PMU_GPCEnableLdoStandbyMode(kPMU_PllLdo, PMU_LDO_PLL_EN_STBY_VAL);
    PMU_GPCEnableLdoStandbyMode(kPMU_LpsrAnaLdo, PMU_LDO_ANA_EN_STBY_VAL);
    PMU_GPCEnableLdoStandbyMode(kPMU_LpsrDigLdo, PMU_LDO_DIG_EN_STBY_VAL);
    PMU_GPCEnableBandgapStandbyMode(ANADIG_PMU, PMU_BG_STBY_VAL);

    for (i = 0; i < 16; i++)
    {
        PMU_GPCSetLpsrDigLdoTargetVoltage(1 << i, anatopLdoCfg[i]);
    }

    PMU_SetLpsrDigLdoControlMode(ANADIG_LDO_SNVS, kPMU_GPCMode);
    PMU_SetLpsrAnaLdoControlMode(ANADIG_LDO_SNVS, kPMU_GPCMode);
    PMU_SetPllLdoControlMode(ANADIG_PMU, kPMU_GPCMode);
    PMU_SetBandgapControlMode(ANADIG_PMU, kPMU_GPCMode);
    PMU_SetBodyBiasControlMode(ANADIG_PMU, kPMU_FBB_CM7, kPMU_GPCMode);
    PMU_SetBodyBiasControlMode(ANADIG_PMU, kPMU_RBB_SOC, kPMU_GPCMode);
    PMU_SetBodyBiasControlMode(ANADIG_PMU, kPMU_RBB_LPSR, kPMU_GPCMode);
}

// Configure all clock sources as hardware/software mode
void CLOCK_ControlMode(uint8_t cmode)
{
    if (cmode == 0)
    { // SW mode
        ANADIG_OSC->OSC_16M_CTRL &= ~ANADIG_OSC_OSC_16M_CTRL_RC_16M_CONTROL_MODE_MASK;
        ANADIG_OSC->OSC_48M_CTRL &=
            ~(ANADIG_OSC_OSC_48M_CTRL_RC_48M_CONTROL_MODE_MASK | ANADIG_OSC_OSC_48M_CTRL_RC_48M_DIV2_CONTROL_MODE_MASK);
        ANADIG_OSC->OSC_24M_CTRL &= ~ANADIG_OSC_OSC_24M_CTRL_OSC_24M_CONTROL_MODE_MASK;
        ANADIG_OSC->OSC_400M_CTRL1 &= ~ANADIG_OSC_OSC_400M_CTRL1_RC_400M_CONTROL_MODE_MASK;
        ANADIG_PLL->ARM_PLL_CTRL &= ~ANADIG_PLL_ARM_PLL_CTRL_ARM_PLL_CONTROL_MODE_MASK;
        ANADIG_PLL->SYS_PLL1_CTRL &= ~(ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_CONTROL_MODE_MASK |
                                       ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_DIV2_CONTROL_MODE_MASK |
                                       ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_DIV5_CONTROL_MODE_MASK);
        ANADIG_PLL->SYS_PLL3_CTRL &= ~(ANADIG_PLL_SYS_PLL3_CTRL_SYS_PLL3_CONTROL_MODE_MASK |
                                       ANADIG_PLL_SYS_PLL3_CTRL_SYS_PLL3_DIV2_CONTROL_MODE_MASK);
        ANADIG_PLL->SYS_PLL3_UPDATE &=
            ~(ANADIG_PLL_SYS_PLL3_UPDATE_PFD0_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL3_UPDATE_PFD1_CONTROL_MODE_MASK |
              ANADIG_PLL_SYS_PLL3_UPDATE_PDF2_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL3_UPDATE_PFD3_CONTROL_MODE_MASK);
        ANADIG_PLL->SYS_PLL2_CTRL &= ~ANADIG_PLL_SYS_PLL2_CTRL_SYS_PLL2_CONTROL_MODE_MASK;
        ANADIG_PLL->SYS_PLL2_UPDATE &=
            ~(ANADIG_PLL_SYS_PLL2_UPDATE_PFD0_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL2_UPDATE_PFD1_CONTROL_MODE_MASK |
              ANADIG_PLL_SYS_PLL2_UPDATE_PFD2_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL2_UPDATE_PFD3_CONTROL_MODE_MASK);
        ANADIG_PLL->PLL_AUDIO_CTRL &= ~ANADIG_PLL_PLL_AUDIO_CTRL_PLL_AUDIO_CONTROL_MODE_MASK;
        ANADIG_PLL->PLL_VIDEO_CTRL &= ~ANADIG_PLL_PLL_VIDEO_CTRL_PLL_VIDEO_CONTROL_MODE_MASK;
    }
    else
    { // HW MODE
        ANADIG_OSC->OSC_16M_CTRL |= ANADIG_OSC_OSC_16M_CTRL_RC_16M_CONTROL_MODE_MASK;
        ANADIG_OSC->OSC_48M_CTRL |=
            (ANADIG_OSC_OSC_48M_CTRL_RC_48M_CONTROL_MODE_MASK | ANADIG_OSC_OSC_48M_CTRL_RC_48M_DIV2_CONTROL_MODE_MASK);
        ANADIG_OSC->OSC_24M_CTRL |= ANADIG_OSC_OSC_24M_CTRL_OSC_24M_CONTROL_MODE_MASK;
        ANADIG_OSC->OSC_400M_CTRL1 |= ANADIG_OSC_OSC_400M_CTRL1_RC_400M_CONTROL_MODE_MASK;
        ANADIG_PLL->ARM_PLL_CTRL |= ANADIG_PLL_ARM_PLL_CTRL_ARM_PLL_CONTROL_MODE_MASK;
        ANADIG_PLL->SYS_PLL1_CTRL |= (ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_CONTROL_MODE_MASK |
                                      ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_DIV2_CONTROL_MODE_MASK |
                                      ANADIG_PLL_SYS_PLL1_CTRL_SYS_PLL1_DIV5_CONTROL_MODE_MASK);
        ANADIG_PLL->SYS_PLL3_CTRL |= (ANADIG_PLL_SYS_PLL3_CTRL_SYS_PLL3_CONTROL_MODE_MASK |
                                      ANADIG_PLL_SYS_PLL3_CTRL_SYS_PLL3_DIV2_CONTROL_MODE_MASK);
        ANADIG_PLL->SYS_PLL3_UPDATE |=
            (ANADIG_PLL_SYS_PLL3_UPDATE_PFD0_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL3_UPDATE_PFD1_CONTROL_MODE_MASK |
             ANADIG_PLL_SYS_PLL3_UPDATE_PDF2_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL3_UPDATE_PFD3_CONTROL_MODE_MASK);
        ANADIG_PLL->SYS_PLL2_CTRL |= ANADIG_PLL_SYS_PLL2_CTRL_SYS_PLL2_CONTROL_MODE_MASK;
        ANADIG_PLL->SYS_PLL2_UPDATE |=
            (ANADIG_PLL_SYS_PLL2_UPDATE_PFD0_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL2_UPDATE_PFD1_CONTROL_MODE_MASK |
             ANADIG_PLL_SYS_PLL2_UPDATE_PFD2_CONTROL_MODE_MASK | ANADIG_PLL_SYS_PLL2_UPDATE_PFD3_CONTROL_MODE_MASK);
        ANADIG_PLL->PLL_AUDIO_CTRL |= ANADIG_PLL_PLL_AUDIO_CTRL_PLL_AUDIO_CONTROL_MODE_MASK;
        ANADIG_PLL->PLL_VIDEO_CTRL |= ANADIG_PLL_PLL_VIDEO_CTRL_PLL_VIDEO_CONTROL_MODE_MASK;
    }
}

#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
/* clang-format off */
#define PINMUX_DESCRIPTOR_NUM       20
#define PINMUX_DESCRIPTOR_TABLE \
{/*address                                                              ,data ,size                                ,opreation                      ,type                         ,index */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_05]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  0   */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_06]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  1   */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_07]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  2   */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_08]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  3   */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_09]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  4   */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_10]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  5   */\
{ (uint32_t)&IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B2_11]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  6   */\
{ (uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_DQS_FA_SELECT_INPUT]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  7   */\
{ (uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_0] ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  8   */\
{ (uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_1] ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  9   */\
{ (uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_2] ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  10  */\
{ (uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_IO_FA_SELECT_INPUT_3] ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  11  */\
{ (uint32_t)&IOMUXC->SELECT_INPUT[kIOMUXC_FLEXSPI1_I_SCK_FA_SELECT_INPUT]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  12  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_05]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  13  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_06]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  14  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_07]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  15  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_08]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  16  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_09]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  17  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_10]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}, /*  18  */\
{ (uint32_t)&IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B2_11]  ,0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveEnableRestoreEnable ,kSSARC_ReadValueWriteBack}} /*  19  */

#define FLEXSPI_DESCRIPTOR_NUM       34
#define FLEXSPI_DESCRIPTOR_TABLE \
{/*address                            ,data       ,size                                ,opreation                       ,type                       ,index */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0xFFFF1010 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  0   */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0x1        ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_RMWOr           }, /*  1   */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0x1        ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_Polling0        }, /*  2   */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0xFFFF1012 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  3   */\
{ (uint32_t)&FLEXSPI1->MCR1           ,0xFFFFFFFF ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  4   */\
{ (uint32_t)&FLEXSPI1->MCR2           ,0x200001F7 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  5   */\
{ (uint32_t)&FLEXSPI1->AHBCR          ,0x00000078 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  6   */\
{ (uint32_t)&FLEXSPI1->AHBRXBUFCR0[0] ,0x800F0000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  7   */\
{ (uint32_t)&FLEXSPI1->AHBRXBUFCR0[1] ,0x800F0000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  8   */\
{ (uint32_t)&FLEXSPI1->AHBRXBUFCR0[2] ,0x80000020 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  9   */\
{ (uint32_t)&FLEXSPI1->AHBRXBUFCR0[3] ,0x80000020 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  10  */\
{ (uint32_t)&FLEXSPI1->IPRXFCR        ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  11  */\
{ (uint32_t)&FLEXSPI1->IPTXFCR        ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  12  */\
{ (uint32_t)&FLEXSPI1->FLSHCR0[0]     ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  13  */\
{ (uint32_t)&FLEXSPI1->FLSHCR0[1]     ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  14  */\
{ (uint32_t)&FLEXSPI1->FLSHCR0[2]     ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  15  */\
{ (uint32_t)&FLEXSPI1->FLSHCR0[3]     ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  16  */\
{ (uint32_t)&FLEXSPI1->FLSHCR0[0]     ,0x00004000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  17  */\
{ (uint32_t)&FLEXSPI1->FLSHCR1[0]     ,0x00020063 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  18  */\
{ (uint32_t)&FLEXSPI1->FLSHCR2[0]     ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  19  */\
{ (uint32_t)&FLEXSPI1->DLLCR[0]       ,0x00000100 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  20  */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0xFFFF1010 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  21  */\
{ 0                                   ,200        ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_DelayCycles     }, /*  22  */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0xFFFF1012 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  23  */\
{ (uint32_t)&FLEXSPI1->FLSHCR4        ,0x00000003 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  24  */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0xFFFF1010 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  25  */\
{ (uint32_t)&FLEXSPI1->LUTKEY         ,0x5AF05AF0 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  26  */\
{ (uint32_t)&FLEXSPI1->LUTCR          ,0x00000002 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  27  */\
{ (uint32_t)&FLEXSPI1->LUT[0]         ,0x0a1804eb ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  28  */\
{ (uint32_t)&FLEXSPI1->LUT[1]         ,0x26043206 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  29  */\
{ (uint32_t)&FLEXSPI1->LUT[2]         ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  30  */\
{ (uint32_t)&FLEXSPI1->LUT[3]         ,0x00000000 ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_WriteFixedValue }, /*  31  */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0x1        ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_RMWOr           }, /*  32  */\
{ (uint32_t)&FLEXSPI1->MCR0           ,0x1        ,kSSARC_DescriptorRegister32bitWidth ,kSSARC_SaveDisableRestoreEnable ,kSSARC_Polling0        }} /*  33  */
/* clang-format on */

void SSARC_InitConfig(void)
{
    uint16_t i, descriptorIndex = 0;
    uint8_t groupIndex = 0;
    ssarc_group_config_t groupConfig;
    static const ssarc_descriptor_config_t pinmuxDescriptor[PINMUX_DESCRIPTOR_NUM]   = PINMUX_DESCRIPTOR_TABLE;
    static const ssarc_descriptor_config_t flexspiDescriptor[FLEXSPI_DESCRIPTOR_NUM] = FLEXSPI_DESCRIPTOR_TABLE;

    groupConfig.startIndex = descriptorIndex;
    for (i = 0; i < PINMUX_DESCRIPTOR_NUM; i++)
    {
        SSARC_SetDescriptorConfig(SSARC_HP, descriptorIndex++, &pinmuxDescriptor[i]);
    }
    groupConfig.endIndex        = descriptorIndex - 1;
    groupConfig.highestAddress  = 0xFFFFFFFFU;
    groupConfig.lowestAddress   = 0x00000000U;
    groupConfig.saveOrder       = kSSARC_ProcessFromStartToEnd;
    groupConfig.savePriority    = 0U;
    groupConfig.restoreOrder    = kSSARC_ProcessFromStartToEnd;
    groupConfig.restorePriority = 0U;
    groupConfig.powerDomain     = kSSARC_WAKEUPMIXPowerDomain;
    groupConfig.cpuDomain       = kSSARC_CM7Core;
    SSARC_GroupInit(SSARC_LP, groupIndex++, &groupConfig);

    groupConfig.startIndex = descriptorIndex;
    for (i = 0; i < FLEXSPI_DESCRIPTOR_NUM; i++)
    {
        SSARC_SetDescriptorConfig(SSARC_HP, descriptorIndex++, &flexspiDescriptor[i]);
    }
    groupConfig.endIndex        = descriptorIndex - 1;
    groupConfig.highestAddress  = 0xFFFFFFFFU;
    groupConfig.lowestAddress   = 0x00000000U;
    groupConfig.saveOrder       = kSSARC_ProcessFromStartToEnd;
    groupConfig.savePriority    = 0U;
    groupConfig.restoreOrder    = kSSARC_ProcessFromStartToEnd;
    groupConfig.restorePriority = 1U;
    groupConfig.powerDomain     = kSSARC_WAKEUPMIXPowerDomain;
    groupConfig.cpuDomain       = kSSARC_CM7Core;
    SSARC_GroupInit(SSARC_LP, groupIndex++, &groupConfig);
}
#endif

void ChipInitConfig(void)
{
    clock_root_config_t rootCfg = {0};

    DCDC_SetVDD1P0BuckModeTargetVoltage(DCDC, kDCDC_1P0BuckTarget1P0V);

    PMU_StaticEnableLpsrAnaLdoBypassMode(ANADIG_LDO_SNVS, true);
    PMU_StaticEnableLpsrDigLdoBypassMode(ANADIG_LDO_SNVS, true);

    const clock_sys_pll2_config_t sysPll2Config = {
        .ssEnable = false,
    };

    /*
     * Default configuration for audio and video pll.
     * AUDIO_PLL/VIDEO_PLL(20.3125MHZ--- 1300MHZ configureable ) setting:
     * Frequency = Fref * (loopDivider + numerator / numerator ) / 2^postDivider
     *           = 24 * (41 + 67/100) / 2^0 = 1000.08 Mhz
     */
    const clock_av_pll_config_t avPllConfig = {
        .loopDivider = 41,    /* loopDivider: 27---54 */
        .postDivider = 0,     /* postDivider: 0---5 */
        .numerator   = 67,    /* 30 bit numerator of fractional loop divider */
        .denominator = 100,   /* 30 bit denominator of fractional loop divider */
        .ssEnable    = false, /* Enable spread spectrum flag */
    };
    const clock_audio_pll_gpc_config_t avPllGpcConfig = {
        .loopDivider = 41,    /* loopDivider: 27---54 */
        .numerator   = 67,    /* 30 bit numerator of fractional loop divider */
        .denominator = 100,   /* 30 bit denominator of fractional loop divider */
        .ssEnable    = false, /* Enable spread spectrum flag */
    };

    for (uint8_t i = 0; i < CLOCK_ROOT_NUM; i++)
    {
        /* Set clock root to unassigned mode so that software configuration can take effetcs. */
        CLOCK_ROOT_ControlByUnassignedMode((clock_root_t)i);
        /* For clock roots which mux to OscRc48MDiv2, mux to Rc16M so that application could turn off OscRc48MDiv2 to
         * save power. */
        if (CLOCK_GetRootClockMux((clock_root_t)i) == 0)
        {
            CLOCK_SetRootClockMux((clock_root_t)i, 3);
        }
    }
    ANADIG_OSC->OSC_48M_CTRL &= ~ANADIG_OSC_OSC_48M_CTRL_TEN_MASK;

    CLOCK_OSC_EnableOsc24M();
    rootCfg.mux = 1;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);

    CLOCK_ControlMode(0);

    CLOCK_OSC_EnableOscRc400M();
    CLOCK_InitArmPllWithFreq(700);
    CLOCK_InitSysPll2(&sysPll2Config);
    /* Init System Pll2 pfd3. */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    CLOCK_InitSysPll3();
    /* Init System Pll3 pfd3. */
    CLOCK_InitPfd(kCLOCK_PllSys3, kCLOCK_Pfd3, 22);
    CLOCK_InitAudioPll(&avPllConfig);
    CLOCK_GPC_SetAudioPllOutputFreq(&avPllGpcConfig);
    CLOCK_InitVideoPll(&avPllConfig);
    CLOCK_GPC_SetVideoPllOutputFreq(&avPllGpcConfig);

    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
    rootCfg.mux = kCLOCK_BUS_ClockRoot_MuxSysPll2Pfd3;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg);
    rootCfg.mux = kCLOCK_M4_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_M4, &rootCfg);
    rootCfg.mux = kCLOCK_BUS_LPSR_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Lpsr, &rootCfg);

    GPC_InitConfig();
    DCDC_InitConfig();
    PGMC_InitConfig();
    SRC_InitConfig();
    CCM_InitConfig();
    PMU_InitConfig();
#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    SSARC_InitConfig();
#endif

    CLOCK_ControlMode(1);

    /* SW should write 1 to OCOTP->PDN bit 0 to shut off power to the OTP memory and avoid unnecessary power wasting.
       Note: FUSE could not be read out if OTP memory is powered off. */
    OCOTP->PDN |= OCOTP_PDN_PDN_MASK;
    IOMUXC_SNVS_GPR->GPR37 |= IOMUXC_SNVS_GPR_GPR37_SNVS_TAMPER_PUE_MASK;
    /* CM7 SLEEPING is sent to GPC to start sleep sequence */
    IOMUXC_GPR->GPR16 |= IOMUXC_GPR_GPR16_M7_GPC_SLEEP_SEL_MASK;
    /* CM4 SLEEPING is sent to GPC to start sleep sequence */
    IOMUXC_LPSR_GPR->GPR34 |= IOMUXC_LPSR_GPR_GPR34_M4_GPC_SLEEP_SEL_MASK;
}

void PrintSystemStatus(void)
{
    gpc_cpu_mode_t preCpuMode;
    gpc_cpu_mode_t curCpuMode;

    PRINTF("\r\n");

    PRINTF("System previous setpoint is %d\r\n", GPC_SP_GetPreviousSetPoint(GPC_SET_POINT_CTRL));
    PRINTF("System current setpoint is %d\r\n", GPC_SP_GetCurrentSetPoint(GPC_SET_POINT_CTRL));

    preCpuMode = GPC_CM_GetPreviousCpuMode(GPC_CPU_MODE_CTRL_0);
    curCpuMode = GPC_CM_GetCurrentCpuMode(GPC_CPU_MODE_CTRL_0);
    PRINTF("M7 previous CPU mode is %s\r\n", GET_CPU_MODE_NAME(preCpuMode));
    PRINTF("M7 current CPU mode is %s\r\n", GET_CPU_MODE_NAME(curCpuMode));
    if (curCpuMode == kGPC_RunMode)
    {
        PRINTF("M7 CLK is %d MHz\r\n", CLOCK_GetFreqFromObs(CCM_OBS_M7_CLK_ROOT) / 1000000);
    }
#ifndef SINGLE_CORE_M7
    preCpuMode = GPC_CM_GetPreviousCpuMode(GPC_CPU_MODE_CTRL_1);
    curCpuMode = GPC_CM_GetCurrentCpuMode(GPC_CPU_MODE_CTRL_1);
    PRINTF("M4 previous CPU mode is %s\r\n", GET_CPU_MODE_NAME(preCpuMode));
    PRINTF("M4 current CPU mode is %s\r\n", GET_CPU_MODE_NAME(curCpuMode));
    if (curCpuMode == kGPC_RunMode)
    {
        PRINTF("M4 CLK is %d MHz\r\n", CLOCK_GetFreqFromObs(CCM_OBS_M4_CLK_ROOT) / 1000000);
    }
#endif

    PRINTF("\r\n");
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

/* Get core0 status based on different set point. */
core_status_t getCore0Status(uint8_t setpoint)
{
    core_status_t status = kCORE_NormalRun;

    if (setpoint > 8)
    {
        status = kCORE_ClockGated;
    }
    if (!((DCDC_ONOFF_SP_VAL >> setpoint) & 0x1))
    {
        status = kCORE_PowerGated;
    }

    return status;
}

/* Get core1 status based on different set point. */
core_status_t getCore1Status(uint8_t setpoint)
{
    core_status_t status = kCORE_NormalRun;

    if (setpoint == 6 || setpoint == 7 || setpoint == 10 || setpoint == 14 || setpoint == 15)
    {
        status = kCORE_ClockGated;
    }

    return status;
}
