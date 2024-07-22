/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lpm.h"
#include "fsl_clock.h"
#include "clock_config.h"
#include "fsl_pmu.h"
#include "fsl_debug_console.h"
#include "board.h"

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
#include "fsl_pf5020.h"
#include "board.h"
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t g_savedPrimask;
const char *g_cpuModeNames[] = {"RUN", "WAIT", "STOP", "SUSPEND"};
run_mode_t currentRunMode    = kLPM_OverDrive;

#if (defined(BOARD_USE_EXT_PMIC) & BOARD_USE_EXT_PMIC)
pf5020_handle_t g_pf5020Handle;
#endif /* #if (defined(BOARD_USE_EXT_PMIC) & BOARD_USE_EXT_PMIC) */

/*******************************************************************************
 * Code
 ******************************************************************************/
void GPC_EnableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(CPU_SLICE, irq, true);
}

void GPC_DisableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(CPU_SLICE, irq, false);
}

void GPC_DisableAllWakeupSource(gpc_cpu_slice_t slice)
{
    uint8_t i;

    for (i = 0; i < GPC_CPU_CTRL_CM_IRQ_WAKEUP_MASK_COUNT2; i++)
    {
        GPC_CPU_CTRL->AUTHEN[slice].CM_IRQ_WAKEUP_MASK[i] |= 0xFFFFFFFF;
    }
}

/* Configure each step during CPU mode transition, enable all steps by default. */
void GPC_ConfigCore0CpuModeTransitionFlow(void)
{
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_SleepSsar, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_SleepLpcg, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_SleepPll, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_SleepIso, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_SleepReset, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_SleepPower, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_WakeupPower, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_WakeupReset, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_WakeupIso, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_WakeupPll, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_WakeupLpcg, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU0, kGPC_CM_WakeupSsar, true);
}

/* Configure each step during CPU mode transition, enable all steps by default. */
void GPC_ConfigCore1CpuModeTransitionFlow(void)
{
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_SleepSsar, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_SleepLpcg, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_SleepPll, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_SleepIso, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_SleepReset, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_SleepPower, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_WakeupPower, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_WakeupReset, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_WakeupIso, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_WakeupPll, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_WakeupLpcg, true);
    GPC_CM_EnableCpuModeTransitionStep(kGPC_CPU1, kGPC_CM_WakeupSsar, true);
}

/* Configure each step during system sleep transition, enable all steps by default. */
void GPC_ConfigSystemSleepTransitionFlow(void)
{
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step0In, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step1In, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step2In, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step3In, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_DcdcIn, true);
#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
    /* There is no way to go over this step since PMIC_READY pin is not used on MIMXRT1180A-EVK. We have to skip this
     * step here. */
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_PmicIn, false);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_PmicOut, false);
#else
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_PmicIn, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_PmicOut, true);
#endif /* (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_DcdcOut, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step3Out, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step2Out, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step1Out, true);
    GPC_SS_EnableSystemSleepTransitionStep(GPC_SYS_SLEEP_CTRL, kGPC_SS_Step0Out, true);
}

void GPC_ConfigROSC(void)
{
    GPC_GLOBAL->GPC_ROSC_CTRL |= GPC_GLOBAL_GPC_ROSC_CTRL_ROSC_OFF_EN_MASK;
}

void GPC_ConfigBandgap(void)
{
    ANADIG_PMU->PMU_REF_CTRL |= ANADIG_PMU_PMU_REF_CTRL_REF_STBY_EN_MASK | ANADIG_PMU_PMU_REF_CTRL_REF_CONTROL_MODE_MASK;
}

void GPC_InitConfig()
{
    GPC_AssignCpuDomain(kGPC_CPU0, CM33_DOMAIN_MASK);
    GPC_ConfigCore0CpuModeTransitionFlow();
#ifndef SINGLE_CORE_M33
    GPC_AssignCpuDomain(kGPC_CPU1, CM7_DOMAIN_MASK);
    GPC_ConfigCore1CpuModeTransitionFlow();
#endif
    GPC_ConfigSystemSleepTransitionFlow();
    GPC_ConfigROSC();
    GPC_ConfigBandgap();
}

#if !(defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
void DCDC_InitConfig(void)
{
    dcdc_config_t config;

    DCDC_SetVoltage(kDCDC_CORE0, kDCDC_1P0Target1P1V);
    DCDC_SetVoltage(kDCDC_CORE1, kDCDC_1P0Target1P1V);
    DCDC_GPC_SetVDD1P0LowPowerModeTargetVoltage(DCDC, kDCDC_CORE0, kDCDC_1P0Target0P8V);
    DCDC_GPC_SetVDD1P0LowPowerModeTargetVoltage(DCDC, kDCDC_CORE1, kDCDC_1P0Target0P8V);
    DCDC_GPC_EnableVDD1P0LowPowerMode(DCDC, kDCDC_CORE0, true);
    DCDC_GPC_EnableVDD1P0LowPowerMode(DCDC, kDCDC_CORE1, true);

    DCDC_GetDefaultConfig(&config);
    config.controlMode = kDCDC_GPCControl;
    DCDC_Init(DCDC, &config);
}
#endif

#ifndef SINGLE_CORE_M33
void M7_TCM_Retention(bool enable)
{
    uint32_t tmp32 = CM7PLATFORM_MIX_SLICE->MLPL_CFG;
    tmp32 &= ~SRC_MIX_SLICE_MLPL_CFG_MLPL_HW_PDN_LMEM_MASK;
    if (enable)
    {
        tmp32 |= SRC_MIX_SLICE_MLPL_CFG_MLPL_HW_PDN_LMEM(0x5U);
    }
    else
    {
        tmp32 |= SRC_MIX_SLICE_MLPL_CFG_MLPL_HW_PDN_LMEM(0x6U);
    }
    CM7PLATFORM_MIX_SLICE->MLPL_CFG = tmp32;
}
#endif

void SRC_InitConfig()
{
    uint8_t i;
    static const src_config_t srcCfg[SRC_SLICE_NUM] = SRC_CONFIGURATION_TABLE;

    for (i = 0; i < SRC_SLICE_NUM; i++)
    {
        if (srcCfg[i].ctrlMode == CM33_DOMAIN)
        {
            SRC_SLICE_ControlByCpuLowPowerMode(srcCfg[i].sliceName, CM33_DOMAIN_MASK, srcCfg[i].powerLevel);
            SRC_SLICE_SetWhiteList(srcCfg[i].sliceName, CM33_DOMAIN_MASK);
        }
#ifndef SINGLE_CORE_M33
        else if (srcCfg[i].ctrlMode == CM7_DOMAIN)
        {
            SRC_SLICE_ControlByCpuLowPowerMode(srcCfg[i].sliceName, CM7_DOMAIN_MASK, srcCfg[i].powerLevel);
            SRC_SLICE_SetWhiteList(srcCfg[i].sliceName, CM7_DOMAIN_MASK);
        }
#endif
    }
}

void CCM_InitConfig(void)
{
    uint8_t index;
    static const clock_source_config_t oscpllCfg[CLOCK_OSCPLL_NUM] = CLK_OSCPLL_CONFIGURATION_TABLE;
    static const lpcg_config_t lpcgCfg[CLOCK_LPCG_NUM]             = CLK_LPCG_CONFIGURATION_TABLE;

    for (index = 0; index < CLOCK_OSCPLL_NUM; index++)
    {
        if (oscpllCfg[index].ctrlMode == CM33_DOMAIN)
        {
            CLOCK_OSCPLL_ControlByCpuLowPowerMode((clock_name_t)index, CM33_DOMAIN_MASK, oscpllCfg[index].level);
        }
#ifndef SINGLE_CORE_M33
        else if (oscpllCfg[index].ctrlMode == CM7_DOMAIN)
        {
            CLOCK_OSCPLL_ControlByCpuLowPowerMode((clock_name_t)index, CM7_DOMAIN_MASK, oscpllCfg[index].level);
        }
#endif
    }

    for (index = 0; index < CLOCK_LPCG_NUM; index++)
    {
        if (lpcgCfg[index].ctrlMode == CM33_DOMAIN)
        {
            CLOCK_LPCG_ControlByCpuLowPowerMode((clock_lpcg_t)index, CM33_DOMAIN_MASK, lpcgCfg[index].level);
        }
#ifndef SINGLE_CORE_M33
        else if (lpcgCfg[index].ctrlMode == CM7_DOMAIN)
        {
            CLOCK_LPCG_ControlByCpuLowPowerMode((clock_lpcg_t)index, CM7_DOMAIN_MASK, lpcgCfg[index].level);
        }
#endif
    }
}

void PMU_InitConfig(void)
{
    PMU_SetPllLdoControlMode(ANADIG_PMU, kPMU_GPCMode);
    PMU_EnableLdoStandbyMode(kPMU_PllLdo, true);
    PMU_EnableLdoStandbyMode(kPMU_AonAnaLdo, true);
    PMU_EnableLdoStandbyMode(kPMU_AonDigLdo, true);
    PMU_EnableFBBStandbyMode(ANADIG_PMU, true);
}

/* Configure all clock sources as hardware/software mode */
void CLOCK_ControlMode(clock_control_mode_t mode)
{
    if (mode == kCLOCK_SoftwareMode)
    {
        CLOCK_SetClockSourceControlMode(kCLOCK_OscRc24M, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_OscRc400M, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_Osc24M, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_ArmPll, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd0, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd1, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd2, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd3, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Div2, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd0, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd1, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd2, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd3, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll1, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll1Div2, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll1Div5, kCLOCK_SoftwareMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_AudioPll, kCLOCK_SoftwareMode);
    }
    else if (mode == kCLOCK_GpcMode)
    {
        CLOCK_SetClockSourceControlMode(kCLOCK_OscRc24M, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_OscRc400M, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_Osc24M, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_ArmPll, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd0, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd1, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd2, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll2Pfd3, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Div2, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd0, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd1, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd2, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll3Pfd3, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll1, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll1Div2, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_SysPll1Div5, kCLOCK_GpcMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_AudioPll, kCLOCK_GpcMode);
    }
    else
    {
        assert(0);
    }
}

void ChipInitConfig(void)
{
    clock_root_config_t rootCfg = {0};

    const clock_audio_pll_config_t audioPllConfig = {
        .loopDivider = 47, /* PLL Loop divider, valid range for DIV_SELECT divider value: 27 ~ 54. */
        .postDivider = 0,  /* Divider after PLL, should only be 1, 2, 4, 8, 16, 32 */
        .numerator =
            1, /* 30 bit numerator of fractional loop divider, Fout = Fin * ( loopDivider + numerator / denominator ) */
        .denominator = 50, /* 30 bit denominator of fractional loop divider, Fout = Fin * ( loopDivider + numerator /
                              denominator ) */
    };

#if !(defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
    DCDC_InitConfig();
#endif /* !(defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC) */
    GPC_InitConfig();

    CLOCK_ControlMode(kCLOCK_SoftwareMode);
    CLOCK_OSC_EnableOsc24M();
    CLOCK_OSC_EnableOscRc400M();
    CLOCK_InitAudioPll(&audioPllConfig);

#ifndef SINGLE_CORE_M33
    PMU_EnableFBB(ANADIG_PMU, true);
    CLOCK_InitArmPllWithFreq(800);
    M7_TCM_Retention(true);

    rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
    rootCfg.div = 1;
    CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
#endif

    rootCfg.mux = kCLOCK_M33_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_M33, &rootCfg);

    EdgeLock_SetClock(kCLOCK_EDGELOCK_ClockRoot_MuxOscRc400M, 2);

    rootCfg.mux = kCLOCK_BUS_AON_ClockRoot_MuxSysPll2Out;
    rootCfg.div = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Aon, &rootCfg);

    rootCfg.mux = kCLOCK_BUS_WAKEUP_ClockRoot_MuxSysPll2Out;
    rootCfg.div = 4;
    CLOCK_SetRootClock(kCLOCK_Root_Bus_Wakeup, &rootCfg);

    rootCfg.mux = kCLOCK_WAKEUP_AXI_ClockRoot_MuxSysPll3Out;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Wakeup_Axi, &rootCfg);

    CLOCK_ControlMode(kCLOCK_GpcMode);

    CCM_InitConfig();
    SRC_InitConfig();
    PMU_InitConfig();

    NVIC_DisableIRQ(DAP_IRQn);
    /* Request system sleep when STOP and SUSPEND */
    BLK_CTRL_S_AONMIX->LP_HANDSHAKE = 0x1FFFFF;
}

void PrintSystemStatus(void)
{
    gpc_cpu_mode_t preCpuMode;
    gpc_cpu_mode_t curCpuMode;

    PRINTF("\r\n");

    preCpuMode = GPC_CM_GetPreviousCpuMode(kGPC_CPU0);
    curCpuMode = GPC_CM_GetCurrentCpuMode(kGPC_CPU0);
    PRINTF("M33 previous CPU mode is %s\r\n", GET_CPU_MODE_NAME(preCpuMode));
    PRINTF("M33 current CPU mode is %s\r\n", GET_CPU_MODE_NAME(curCpuMode));
    if (curCpuMode == kGPC_RunMode)
    {
        PRINTF("M33 CLK is %d MHz\r\n", CLOCK_GetRootClockFreq(kCLOCK_Root_M33) / 1000000);
    }
#ifndef SINGLE_CORE_M33
    preCpuMode = GPC_CM_GetPreviousCpuMode(kGPC_CPU1);
    curCpuMode = GPC_CM_GetCurrentCpuMode(kGPC_CPU1);
    PRINTF("M7 previous CPU mode is %s\r\n", GET_CPU_MODE_NAME(preCpuMode));
    PRINTF("M7 current CPU mode is %s\r\n", GET_CPU_MODE_NAME(curCpuMode));
    if (curCpuMode == kGPC_RunMode)
    {
        PRINTF("M7 CLK is %d MHz\r\n", CLOCK_GetRootClockFreq(kCLOCK_Root_M7) / 1000000);
    }
#endif
    PRINTF("EDGELOCK CLK is %d MHz\r\n", CLOCK_GetRootClockFreq(kCLOCK_Root_Edgelock) / 1000000);
    PRINTF("BUS AON CLK is %d MHz\r\n", CLOCK_GetRootClockFreq(kCLOCK_Root_Bus_Aon) / 1000000);
    PRINTF("BUS WAKEUP CLK is %d MHz\r\n", CLOCK_GetRootClockFreq(kCLOCK_Root_Bus_Wakeup) / 1000000);
    PRINTF("WAKEUP AXI CLK is %d MHz\r\n", CLOCK_GetRootClockFreq(kCLOCK_Root_Wakeup_Axi) / 1000000);

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

void CpuModeTransition(gpc_cpu_mode_t cpuMode, bool sysSleepEn)
{
    assert(cpuMode != kGPC_RunMode);

#if __CORTEX_M == 33
    switch (cpuMode)
    {
        case kGPC_WaitMode:
            BLK_CTRL_S_AONMIX->M33_CFG &= ~BLK_CTRL_S_AONMIX_M33_CFG_WAIT_MASK;
            BLK_CTRL_NS_AONMIX->GPC_CFG &= ~BLK_CTRL_NS_AONMIX_GPC_CFG_M33_SLEEP_SEL_MASK;
            break;
        case kGPC_StopMode:
            BLK_CTRL_NS_AONMIX->GPC_CFG |= BLK_CTRL_NS_AONMIX_GPC_CFG_M33_SLEEP_SEL_MASK;
            break;
        case kGPC_SuspendMode:
            BLK_CTRL_NS_AONMIX->GPC_CFG |= BLK_CTRL_NS_AONMIX_GPC_CFG_M33_SLEEP_SEL_MASK;
            break;
        default:
            assert(0);
            break;
    }
#elif __CORTEX_M == 7
    switch (cpuMode)
    {
        case kGPC_WaitMode:
            BLK_CTRL_S_AONMIX->M7_CFG &= ~BLK_CTRL_S_AONMIX_M7_CFG_WAIT_MASK;
            BLK_CTRL_NS_AONMIX->GPC_CFG &= ~BLK_CTRL_NS_AONMIX_GPC_CFG_M7_SLEEP_SEL_MASK;
            break;
        case kGPC_StopMode:
            BLK_CTRL_NS_AONMIX->GPC_CFG |= BLK_CTRL_NS_AONMIX_GPC_CFG_M7_SLEEP_SEL_MASK;
            break;
        case kGPC_SuspendMode:
            BLK_CTRL_NS_AONMIX->GPC_CFG |= BLK_CTRL_NS_AONMIX_GPC_CFG_M7_SLEEP_SEL_MASK;
            break;
        default:
            assert(0);
            break;
    }
#endif

    GPC_CM_SetNextCpuMode(CPU_SLICE, cpuMode);

    if (sysSleepEn == true)
    {
        GPC_CM_RequestSystemSleepMode(CPU_SLICE, cpuMode);
    }
    else
    {
        GPC_CM_ClearSystemSleepModeRequest(CPU_SLICE, cpuMode);
    }
    SystemEnterSleepMode(cpuMode);
}

void RunModeTransition(run_mode_t targetRunMode)
{
    clock_root_config_t rootCfg                   = {0};
    const run_mode_config_t runMode[RUN_MODE_NUM] = RUN_MODE_DEF_TABLE;

    if (currentRunMode == targetRunMode)
    {
        PRINTF("Already in target mode!\r\n");
        return;
    }

    if (runMode[targetRunMode].grade < runMode[currentRunMode].grade) // DCDC first, then FBB, finally clock
    {
#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
        if (kStatus_Success != PF5020_SW1_SetRunStateOption(&g_pf5020Handle, runMode[targetRunMode].pmicVoltage,
                                                            kPF5020_BuckRegulatorPWMMode))
        {
            PRINTF("PMIC voltage set failed\r\n");
            while (1)
                ;
        }
#else
        DCDC_SetVoltage(kDCDC_CORE0, runMode[targetRunMode].targetVoltage);
        DCDC_SetVoltage(kDCDC_CORE1, runMode[targetRunMode].targetVoltage);
#endif

#ifndef SINGLE_CORE_M33
        if (runMode[targetRunMode].cm7Freq > 600)
        {
            PMU_EnableFBB(ANADIG_PMU, true);
        }
        else
        {
            PMU_EnableFBB(ANADIG_PMU, false);
        }
        rootCfg.mux = kCLOCK_M7_ClockRoot_MuxOscRc24M;
        rootCfg.div = 1;
        CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);

        PMU_SetPllLdoControlMode(ANADIG_PMU, kPMU_StaticMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_ArmPll, kCLOCK_SoftwareMode);
        CLOCK_InitArmPllWithFreq(runMode[targetRunMode].cm7Freq);
        CLOCK_SetClockSourceControlMode(kCLOCK_ArmPll, kCLOCK_GpcMode);
        PMU_SetPllLdoControlMode(ANADIG_PMU, kPMU_GPCMode);

        rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
        rootCfg.div = 1;
        CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);
#endif

        rootCfg.mux = runMode[targetRunMode].cm33Source;
        rootCfg.div = runMode[targetRunMode].cm33Divider;
        CLOCK_SetRootClock(kCLOCK_Root_M33, &rootCfg);

        EdgeLock_SetClock(runMode[targetRunMode].edgelockSource, runMode[targetRunMode].edgelockDivider);

        rootCfg.mux = runMode[targetRunMode].busAonSource;
        rootCfg.div = runMode[targetRunMode].busAonDivider;
        CLOCK_SetRootClock(kCLOCK_Root_Bus_Aon, &rootCfg);

        rootCfg.mux = runMode[targetRunMode].busWakeupSource;
        rootCfg.div = runMode[targetRunMode].busWakeupDivider;
        CLOCK_SetRootClock(kCLOCK_Root_Bus_Wakeup, &rootCfg);

        rootCfg.mux = runMode[targetRunMode].wakeupAxiSource;
        rootCfg.div = runMode[targetRunMode].wakeupAxiDivider;
        CLOCK_SetRootClock(kCLOCK_Root_Wakeup_Axi, &rootCfg);
    }
    else if (runMode[targetRunMode].grade > runMode[currentRunMode].grade) // Clock down first, then FBB, finally DCDC
    {
        rootCfg.mux = runMode[targetRunMode].cm33Source;
        rootCfg.div = runMode[targetRunMode].cm33Divider;
        CLOCK_SetRootClock(kCLOCK_Root_M33, &rootCfg);

        EdgeLock_SetClock(runMode[targetRunMode].edgelockSource, runMode[targetRunMode].edgelockDivider);

        rootCfg.mux = runMode[targetRunMode].busAonSource;
        rootCfg.div = runMode[targetRunMode].busAonDivider;
        CLOCK_SetRootClock(kCLOCK_Root_Bus_Aon, &rootCfg);

        rootCfg.mux = runMode[targetRunMode].busWakeupSource;
        rootCfg.div = runMode[targetRunMode].busWakeupDivider;
        CLOCK_SetRootClock(kCLOCK_Root_Bus_Wakeup, &rootCfg);

        rootCfg.mux = runMode[targetRunMode].wakeupAxiSource;
        rootCfg.div = runMode[targetRunMode].wakeupAxiDivider;
        CLOCK_SetRootClock(kCLOCK_Root_Wakeup_Axi, &rootCfg);

#ifndef SINGLE_CORE_M33
        rootCfg.mux = kCLOCK_M7_ClockRoot_MuxOscRc24M;
        rootCfg.div = 1;
        CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);

        PMU_SetPllLdoControlMode(ANADIG_PMU, kPMU_StaticMode);
        CLOCK_SetClockSourceControlMode(kCLOCK_ArmPll, kCLOCK_SoftwareMode);
        CLOCK_InitArmPllWithFreq(runMode[targetRunMode].cm7Freq);
        CLOCK_SetClockSourceControlMode(kCLOCK_ArmPll, kCLOCK_GpcMode);
        PMU_SetPllLdoControlMode(ANADIG_PMU, kPMU_GPCMode);

        rootCfg.mux = kCLOCK_M7_ClockRoot_MuxArmPllOut;
        rootCfg.div = 1;
        CLOCK_SetRootClock(kCLOCK_Root_M7, &rootCfg);

        if (runMode[targetRunMode].cm7Freq > 600)
        {
            PMU_EnableFBB(ANADIG_PMU, true);
        }
        else
        {
            PMU_EnableFBB(ANADIG_PMU, false);
        }
#endif

#if (defined(BOARD_USE_EXT_PMIC) && BOARD_USE_EXT_PMIC)
        if (kStatus_Success != PF5020_SW1_SetRunStateOption(&g_pf5020Handle, runMode[targetRunMode].pmicVoltage,
                                                            kPF5020_BuckRegulatorPWMMode))
        {
            PRINTF("PMIC voltage set failed\r\n");
            while (1)
                ;
        }
#else
        DCDC_SetVoltage(kDCDC_CORE0, runMode[targetRunMode].targetVoltage);
        DCDC_SetVoltage(kDCDC_CORE1, runMode[targetRunMode].targetVoltage);
#endif
    }
    else
    {
        // should define different grade for different run mode.
        assert(0);
    }

    currentRunMode = targetRunMode;
}
