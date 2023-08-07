/*
 * Copyright 2020, 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lpm.h"
#include "fsl_gpc.h"
#include "fsl_dcdc.h"
#include "fsl_gpt.h"
#include "power_mode_switch.h"
#include "specific.h"

#ifdef SDK_OS_FREE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPM_GPC_IMR_NUM (sizeof(GPC->IMR) / sizeof(GPC->IMR[0]))

#ifdef SDK_OS_FREE_RTOS
/* Define the counter clock of the systick (GPT). For accuracy purpose,
 * please make LPM_SYSTICK_COUNTER_FREQ divisible by 32768, and times of
 * configTICK_RATE_HZ.
 */
#define LPM_SYSTICK_COUNTER_FREQ (32768)
#define LPM_COUNT_PER_TICK       (LPM_SYSTICK_COUNTER_FREQ / configTICK_RATE_HZ)

struct _lpm_power_mode_listener
{
    lpm_power_mode_callback_t callback;
    void *data;
    struct _lpm_power_mode_listener *next;
};

typedef struct _lpm_power_mode_listener lpm_power_mode_listener_t;
#endif

typedef void (*lpm_system_func_t)(uint32_t context);
typedef void (*freertos_tick_func_t)(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static lpm_power_mode_t s_curMode;
uint32_t g_savedPrimask;

#ifdef SDK_OS_FREE_RTOS
static SemaphoreHandle_t s_mutex;
#if configSUPPORT_STATIC_ALLOCATION
static StaticSemaphore_t s_staticMutex;
#endif
static lpm_power_mode_listener_t *s_listenerHead;
static lpm_power_mode_listener_t *s_listenerTail;
#if (configUSE_TICKLESS_IDLE == 2)
extern void vPortGptIsr(void);
GPT_Type *vPortGetGptBase(void);
IRQn_Type vPortGetGptIrqn(void);
#endif
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void EnableRegularLDO(void)
{
    /* Enable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_LINREG_MASK;
}

void DisableRegularLDO(void)
{
    /* Disable Regular LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_LINREG_MASK;
}

void EnableWeakLDO(void)
{
    /* Enable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_SET = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_SET = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;

    SDK_DelayAtLeastUs(40, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

void DisableWeakLDO(void)
{
    /* Disable Weak LDO 2P5 and 1P1 */
    PMU->REG_2P5_CLR = PMU_REG_2P5_ENABLE_WEAK_LINREG_MASK;
    PMU->REG_1P1_CLR = PMU_REG_1P1_ENABLE_WEAK_LINREG_MASK;
}

void BandgapOn(void)
{
    /* Turn on regular bandgap and wait for stable */
    PMU->MISC0_CLR = PMU_MISC0_REFTOP_PWD_MASK;
    while ((PMU->MISC0 & PMU_MISC0_REFTOP_VBGUP_MASK) == 0)
    {
    }
    /* Low power band gap disable */
    XTALOSC24M->LOWPWR_CTRL_CLR = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
}

void BandgapOff(void)
{
    XTALOSC24M->LOWPWR_CTRL_SET = XTALOSC24M_LOWPWR_CTRL_LPBG_SEL_MASK;
    PMU->MISC0_SET              = PMU_MISC0_REFTOP_PWD_MASK;
}

/*!
 * @brief Set CCM MUX node to certain value.
 *
 * @param mux   Which mux node to set, see \ref clock_mux_t.
 * @param value Clock mux value to set, different mux has different value range.
 */
void CLOCK_SET_MUX(clock_mux_t mux, uint32_t value)
{
    uint32_t busyShift;

    busyShift               = CCM_TUPLE_BUSY_SHIFT(mux);
    CCM_TUPLE_REG(CCM, mux) = (CCM_TUPLE_REG(CCM, mux) & (~CCM_TUPLE_MASK(mux))) |
                              (((uint32_t)((value) << CCM_TUPLE_SHIFT(mux))) & CCM_TUPLE_MASK(mux));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* Clock switch need Handshake? */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /* Wait until CCM internal handshake finish. */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}

/*!
 * @brief Set CCM DIV node to certain value.
 *
 * @param divider Which div node to set, see \ref clock_div_t.
 * @param value   Clock div value to set, different divider has different value range.
 */
void CLOCK_SET_DIV(clock_div_t divider, uint32_t value)
{
    uint32_t busyShift;

    busyShift                   = CCM_TUPLE_BUSY_SHIFT(divider);
    CCM_TUPLE_REG(CCM, divider) = (CCM_TUPLE_REG(CCM, divider) & (~CCM_TUPLE_MASK(divider))) |
                                  (((uint32_t)((value) << CCM_TUPLE_SHIFT(divider))) & CCM_TUPLE_MASK(divider));

    assert(busyShift <= CCM_NO_BUSY_WAIT);

    /* Clock switch need Handshake? */
    if (CCM_NO_BUSY_WAIT != busyShift)
    {
        /* Wait until CCM internal handshake finish. */
        while (CCM->CDHIPR & (1U << busyShift))
        {
        }
    }
}

void ClockSelectXtalOsc(void)
{
    /* Enable XTAL 24MHz clock source. */
    CLOCK_InitExternalClk(0);
    /* Wait for XTAL stable */
    SDK_DelayAtLeastUs(200, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    /* Switch clock source to external OSC. */
    CLOCK_SwitchOsc(kCLOCK_XtalOsc);
    /*
     * Some board will failed to wake up from suspend mode if rcosc is powered down when clock source switch from rcosc
     * to external osc. Root cause is not found. Workaround: keep rcosc on.
     */
#ifndef KEEP_RCOSC_ON
    /* Power Down internal RC. */
    CLOCK_DeinitRcOsc24M();
#endif
}

void ClockSelectRcOsc(void)
{
    /* Enable internal RC. */
    CLOCK_InitRcOsc24M();
    /* Switch clock source to internal RC. */
    CLOCK_SwitchOsc(kCLOCK_RcOsc);
    /* Disable XTAL 24MHz clock source. */
    CLOCK_DeinitExternalClk();
}

#ifdef SDK_OS_FREE_RTOS
#if (configUSE_TICKLESS_IDLE == 2)
void LPM_InitTicklessTimer(void)
{
    gpt_config_t gptConfig;

    /* Init GPT for wakeup as FreeRTOS tell us */
    GPT_GetDefaultConfig(&gptConfig);
    gptConfig.clockSource     = kGPT_ClockSource_LowFreq; /* 32K RTC OSC */
    gptConfig.enableRunInDoze = true;
    /* Initialize GPT module */
    GPT_Init(vPortGetGptBase(), &gptConfig);

    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(vPortGetGptBase(), kGPT_OutputCompare1InterruptEnable);
    NVIC_SetPriority(vPortGetGptIrqn(), configMAX_SYSCALL_INTERRUPT_PRIORITY + 2);
    EnableIRQ(vPortGetGptIrqn());
}
#endif
#endif

bool LPM_Init(lpm_power_mode_t run_mode)
{
    uint32_t i;
    uint32_t tmp_reg = 0;

#ifdef SDK_OS_FREE_RTOS
#if configSUPPORT_STATIC_ALLOCATION
    s_mutex = xSemaphoreCreateMutexStatic(&s_staticMutex);
#else
    s_mutex = xSemaphoreCreateMutex();
#endif

    if (s_mutex == NULL)
    {
        return false;
    }

    s_listenerHead = s_listenerTail = NULL;

#if (configUSE_TICKLESS_IDLE == 2)
    LPM_InitTicklessTimer();
#endif
#endif

    if (run_mode > LPM_PowerModeRunEnd)
    {
        return false;
    }
    s_curMode = run_mode;
    CLOCK_SetMode(kCLOCK_ModeRun);

    CCM->CGPR |= CCM_CGPR_INT_MEM_CLK_LPM_MASK;

    /* Enable RC OSC. It needs at least 4ms to be stable, so self tuning need to be enabled. */
    XTALOSC24M->LOWPWR_CTRL |= XTALOSC24M_LOWPWR_CTRL_RC_OSC_EN_MASK;
    /* Configure RC OSC */
    XTALOSC24M->OSC_CONFIG0 = XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG_CUR(0x4) | XTALOSC24M_OSC_CONFIG0_SET_HYST_MINUS(0x2) |
                              XTALOSC24M_OSC_CONFIG0_RC_OSC_PROG(0xA7) | XTALOSC24M_OSC_CONFIG0_START_MASK |
                              XTALOSC24M_OSC_CONFIG0_ENABLE_MASK;
    XTALOSC24M->OSC_CONFIG1 = XTALOSC24M_OSC_CONFIG1_COUNT_RC_CUR(0x40) | XTALOSC24M_OSC_CONFIG1_COUNT_RC_TRG(0x2DC);
    /* Take some delay */
    SDK_DelayAtLeastUs(4000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    /* Add some hysteresis */
    tmp_reg = XTALOSC24M->OSC_CONFIG0;
    tmp_reg &= ~(XTALOSC24M_OSC_CONFIG0_HYST_PLUS_MASK | XTALOSC24M_OSC_CONFIG0_HYST_MINUS_MASK);
    tmp_reg |= XTALOSC24M_OSC_CONFIG0_HYST_PLUS(3) | XTALOSC24M_OSC_CONFIG0_HYST_MINUS(3);
    XTALOSC24M->OSC_CONFIG0 = tmp_reg;
    /* Set COUNT_1M_TRG */
    tmp_reg = XTALOSC24M->OSC_CONFIG2;
    tmp_reg &= ~XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG_MASK;
    tmp_reg |= XTALOSC24M_OSC_CONFIG2_COUNT_1M_TRG(0x2d7);
    XTALOSC24M->OSC_CONFIG2 = tmp_reg;
    /* Hardware requires to read OSC_CONFIG0 or OSC_CONFIG1 to make OSC_CONFIG2 write work */
    tmp_reg                 = XTALOSC24M->OSC_CONFIG1;
    XTALOSC24M->OSC_CONFIG1 = tmp_reg;

    /* ERR050143 */
    IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_GINT_MASK;

    /* Initialize GPC to mask all IRQs */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
    GPC->IMR5 = 0xFFFFFFFFU;

    /* DCM Mode */
    DCDC_BootIntoDCM(DCDC);
    /* Adjust SOC voltage to 1.275V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
    /* Disconnect internal the load resistor */
    DCDC->REG1 &= ~DCDC_REG1_REG_RLOAD_SW_MASK;

    /* Enable FET ODRIVE */
    PMU->REG_CORE_SET = PMU_REG_CORE_FET_ODRIVE_MASK;
    /* Connect vdd_high_in and connect vdd_snvs_in */
    PMU->MISC0_CLR = PMU_MISC0_DISCON_HIGH_SNVS_MASK;

    return true;
}

void LPM_Deinit(void)
{
#ifdef SDK_OS_FREE_RTOS
    if (s_mutex != NULL)
    {
        vSemaphoreDelete(s_mutex);
        s_mutex = NULL;
    }
#endif

    /* ERR050143 */
    IOMUXC_GPR->GPR1 &= ~IOMUXC_GPR_GPR1_GINT_MASK;
}

void LPM_EnableWakeupSource(uint32_t irq)
{
    GPC_EnableIRQ(GPC, irq);
}

void LPM_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}

void LPM_SetRunModeConfig(void)
{
    CCM->CLPCR &= ~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK);
}

void LPM_SetWaitModeConfig(void)
{
    uint32_t clpcr;

    /*
     * ERR050143: CCM: When improper low-power sequence is used,
     * the SoC enters low power mode before the ARM core executes WFI.
     *
     * Software workaround:
     * 1) Software should trigger IRQ #41 (GPR_IRQ) to be always pending
     *      by setting IOMUXC_GPR_GPR1_GINT.
     * 2) Software should then unmask IRQ #41 in GPC before setting CCM
     *      Low-Power mode.
     * 3) Software should mask IRQ #41 right after CCM Low-Power mode
     *      is set (set bits 0-1 of CCM_CLPCR).
     */
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeWait) | CCM_CLPCR_MASK_SCU_IDLE_MASK | CCM_CLPCR_MASK_L2CC_IDLE_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_BYPASS_LPM_HS_BITS;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}

void LPM_SetStopModeConfig(void)
{
    uint32_t clpcr;

    /*
     * ERR050143: CCM: When improper low-power sequence is used,
     * the SoC enters low power mode before the ARM core executes WFI.
     *
     * Software workaround:
     * 1) Software should trigger IRQ #41 (GPR_IRQ) to be always pending
     *      by setting IOMUXC_GPR_GPR1_GINT.
     * 2) Software should then unmask IRQ #41 in GPC before setting CCM
     *      Low-Power mode.
     * 3) Software should mask IRQ #41 right after CCM Low-Power mode
     *      is set (set bits 0-1 of CCM_CLPCR).
     */
    GPC_EnableIRQ(GPC, GPR_IRQ_IRQn);
    clpcr      = CCM->CLPCR & (~(CCM_CLPCR_LPM_MASK | CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK));
    CCM->CLPCR = clpcr | CCM_CLPCR_LPM(kCLOCK_ModeStop) | CCM_CLPCR_MASK_L2CC_IDLE_MASK | CCM_CLPCR_MASK_SCU_IDLE_MASK |
                 CCM_CLPCR_VSTBY_MASK | CCM_CLPCR_STBY_COUNT_MASK | CCM_CLPCR_SBYOS_MASK |
                 CCM_CLPCR_ARM_CLK_DIS_ON_LPM_MASK | CCM_CLPCR_BYPASS_LPM_HS_BITS;
    GPC_DisableIRQ(GPC, GPR_IRQ_IRQn);
}

/* Adjust system settings according to current run mode and target mode */
void LPM_AdjustSystemSettings(lpm_power_mode_t curRunMode, lpm_power_mode_t targetMode)
{
    if (curRunMode == targetMode)
        return;

    switch (curRunMode)
    {
        case LPM_PowerModeOverRun:
            if (targetMode == LPM_PowerModeLowPowerRun)
            {
                ClockSelectRcOsc();
                /* Adjust SOC voltage to 0.95V */
                DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
                EnableWeakLDO();
                DisableRegularLDO();
                BandgapOff();
            }
            else
            {
                /* Adjust SOC voltage to 1.15V */
                DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
            }
            break;
        case LPM_PowerModeFullRun:
        case LPM_PowerModeLowSpeedRun:
            if (targetMode == LPM_PowerModeOverRun)
            {
                /* Adjust SOC voltage to 1.275V */
                DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
            }
            else if (targetMode == LPM_PowerModeLowPowerRun)
            {
                ClockSelectRcOsc();
                /* Adjust SOC voltage to 0.95V */
                DCDC_AdjustTargetVoltage(DCDC, 0x6, 0x1);
                EnableWeakLDO();
                DisableRegularLDO();
                BandgapOff();
            }
            break;
        case LPM_PowerModeLowPowerRun:
            if (targetMode == LPM_PowerModeOverRun)
            {
                /* Adjust SOC voltage to 1.275V */
                DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
            }
            else
            {
                /* Adjust SOC voltage to 1.15V */
                DCDC_AdjustTargetVoltage(DCDC, 0xe, 0x1);
            }
            BandgapOn();
            EnableRegularLDO();
            DisableWeakLDO();
            ClockSelectXtalOsc();
            break;
        default:
            assert(false);
            break;
    }
}

void LPM_OverDriveRun(lpm_power_mode_t curRunMode)
{
    /* Increase power supply before increasing core frequency */
    LPM_AdjustSystemSettings(curRunMode, LPM_PowerModeOverRun);
    ClockSetToOverDriveRun();
}

void LPM_FullSpeedRun(lpm_power_mode_t curRunMode)
{
    if (curRunMode == LPM_PowerModeOverRun)
    {
        /* Decrease core frequency before decreasing power supply */
        ClockSetToFullSpeedRun();
        LPM_AdjustSystemSettings(curRunMode, LPM_PowerModeFullRun);
    }
    else
    {
        /* Increase power supply before increasing core frequency */
        LPM_AdjustSystemSettings(curRunMode, LPM_PowerModeFullRun);
        ClockSetToFullSpeedRun();
    }
}

void LPM_LowSpeedRun(lpm_power_mode_t curRunMode)
{
    if (curRunMode == LPM_PowerModeLowPowerRun)
    {
        /* Increase power supply before increasing core frequency */
        LPM_AdjustSystemSettings(curRunMode, LPM_PowerModeLowSpeedRun);
        ClockSetToLowSpeedRun();
    }
    else
    {
        /* Decrease core frequency before decreasing power supply */
        ClockSetToLowSpeedRun();
        LPM_AdjustSystemSettings(curRunMode, LPM_PowerModeLowSpeedRun);
    }
}

void LPM_LowPowerRun(lpm_power_mode_t curRunMode)
{
    /* Decrease core frequency before decreasing power supply */
    ClockSetToLowPowerRun();
    LPM_AdjustSystemSettings(curRunMode, LPM_PowerModeLowPowerRun);
}

void LPM_EnterSystemIdle(lpm_power_mode_t curRunMode)
{
    /* SystemIdle is the idle state of LowSpeedRun. Enter LowSpeedRun mode first, then enter idle state. */
    if (curRunMode != LPM_PowerModeLowSpeedRun)
    {
        LPM_LowSpeedRun(curRunMode);
    }
    LPM_SetWaitModeConfig();
    PeripheralEnterDozeMode();
}

void LPM_ExitSystemIdle(lpm_power_mode_t curRunMode)
{
    PeripheralExitDozeMode();
    LPM_SetRunModeConfig();

    if (curRunMode != LPM_PowerModeLowSpeedRun)
    {
        /* Recover to previous run mode from LowSpeedRun mode */
        switch (curRunMode)
        {
            case LPM_PowerModeOverRun:
                LPM_OverDriveRun(LPM_PowerModeLowSpeedRun);
                break;
            case LPM_PowerModeFullRun:
                LPM_FullSpeedRun(LPM_PowerModeLowSpeedRun);
                break;
            case LPM_PowerModeLowPowerRun:
                LPM_LowPowerRun(LPM_PowerModeLowSpeedRun);
                break;
            default:
                break;
        }
    }
}

void LPM_EnterLowPowerIdle(lpm_power_mode_t curRunMode)
{
    /* LowPowerIdle is the idle state of LowPowerRun. Enter LowPowerRun mode first, then enter idle state. */
    if (curRunMode != LPM_PowerModeLowPowerRun)
    {
        LPM_LowPowerRun(curRunMode);
    }
    LPM_SetWaitModeConfig();
    PeripheralEnterDozeMode();
}

void LPM_ExitLowPowerIdle(lpm_power_mode_t curRunMode)
{
    PeripheralExitDozeMode();
    LPM_SetRunModeConfig();

    if (curRunMode != LPM_PowerModeLowPowerRun)
    {
        /* Recover to previous run mode from LowPowerRun mode */
        switch (curRunMode)
        {
            case LPM_PowerModeOverRun:
                LPM_OverDriveRun(LPM_PowerModeLowPowerRun);
                break;
            case LPM_PowerModeFullRun:
                LPM_FullSpeedRun(LPM_PowerModeLowPowerRun);
                break;
            case LPM_PowerModeLowSpeedRun:
                LPM_LowSpeedRun(LPM_PowerModeLowPowerRun);
                break;
            default:
                break;
        }
    }
}

void LPM_EnterSuspend(void)
{
    uint32_t i;
    uint32_t gpcIMR[LPM_GPC_IMR_NUM];
    uint32_t gpcIMR5;

    LPM_SetStopModeConfig();

    /* Connect internal the load resistor */
    DCDC->REG1 |= DCDC_REG1_REG_RLOAD_SW_MASK;

    /* Turn off FlexRAM0 */
    GPC->CNTR |= GPC_CNTR_PDRAM0_PGE_MASK;
    /* Turn off FlexRAM1 */
    PGC->MEGA_CTRL |= PGC_MEGA_CTRL_PCR_MASK;

    /* Clean data cache to make sure context is saved into RAM */
    SCB_CleanDCache();

    /* Adjust LP voltage to 0.925V */
    DCDC_AdjustTargetVoltage(DCDC, 0x13, 0x1);
    /* Switch DCDC to use DCDC internal OSC */
    DCDC_SetClockSource(DCDC, kDCDC_ClockInternalOsc);

    /* Power down CPU when requested */
    PGC->CPU_CTRL = PGC_CPU_CTRL_PCR_MASK;

    /* STOP_MODE config, turn off all analog except RTC in stop mode */
    PMU->MISC0_CLR = PMU_MISC0_STOP_MODE_CONFIG_MASK;

    /* Mask all GPC interrupts before enabling the RBC counters to
     * avoid the counter starting too early if an interupt is already
     * pending.
     */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        gpcIMR[i]   = GPC->IMR[i];
        GPC->IMR[i] = 0xFFFFFFFFU;
    }
    gpcIMR5   = GPC->IMR5;
    GPC->IMR5 = 0xFFFFFFFFU;

    /*
     * ERR006223: CCM: Failure to resuem from wait/stop mode with power gating
     *   Configure REG_BYPASS_COUNTER to 2
     *   Enable the RBC bypass counter here to hold off the interrupts. RBC counter
     *  needs to be no less than 2.
     */
    CCM->CCR = (CCM->CCR & ~CCM_CCR_REG_BYPASS_COUNT_MASK) | CCM_CCR_REG_BYPASS_COUNT(2);
    CCM->CCR |= (CCM_CCR_OSCNT(0xAF) | CCM_CCR_COSC_EN_MASK | CCM_CCR_RBC_EN_MASK);

    /* Now delay for a short while (3usec) at this point
     * so a short loop should be enough. This delay is required to ensure that
     * the RBC counter can start counting in case an interrupt is already pending
     * or in case an interrupt arrives just as ARM is about to assert DSM_request.
     */
    SDK_DelayAtLeastUs(3, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    /* Recover all the GPC interrupts. */
    for (i = 0; i < LPM_GPC_IMR_NUM; i++)
    {
        GPC->IMR[i] = gpcIMR[i];
    }
    GPC->IMR5 = gpcIMR5;

    PeripheralEnterStopMode();
}

void LPM_EnterSNVS(void)
{
    SNVS->LPCR |= SNVS_LPCR_TOP_MASK;
    while (1) /* Shutdown */
    {
    }
}

bool LPM_SetPowerMode(lpm_power_mode_t mode)
{
#ifdef SDK_OS_FREE_RTOS /* Only FreeRTOS supports listener notification */
    lpm_power_mode_listener_t *l1, *l2;
#endif
    bool ret = true;

    if (mode == s_curMode)
    {
        return ret;
    }

#ifdef SDK_OS_FREE_RTOS /* Only FreeRTOS supports listener notification */
                        /* Need to make sure the list of listeners is not changed
                         * when traversing the list.
                         */
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdFALSE)
    {
        assert(0);
    }

    for (l1 = s_listenerHead; l1 != NULL; l1 = l1->next)
    {
        if (l1->callback == NULL)
        {
            continue;
        }

        if (!l1->callback(s_curMode, mode, l1->data))
        {
            /* One stakeholder doesn't allow new mode */
            ret = false;
            break;
        }
    }

    if (ret)
    {
        s_curMode = mode;
    }
    else
    {
        /* roll back the state change of previous listeners */
        for (l2 = s_listenerHead; l2 != l1; l2 = l2->next)
        {
            if (l2->callback == NULL)
            {
                continue;
            }

            l2->callback(mode, s_curMode, l2->data);
        }
    }

    xSemaphoreGive(s_mutex);
#else
    s_curMode = mode;
#endif /* SDK_OS_FREE_RTOS */

    return ret;
}

#ifdef SDK_OS_FREE_RTOS /* Only FreeRTOS supports listener notification */
void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data)
{
    lpm_power_mode_listener_t *l = (lpm_power_mode_listener_t *)pvPortMalloc(sizeof(lpm_power_mode_listener_t));
    assert(l);

    l->callback = callback;
    l->data     = data;
    l->next     = NULL;

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdFALSE)
    {
        assert(0);
    }

    if (s_listenerHead)
    {
        s_listenerTail->next = l;
        s_listenerTail       = l;
    }
    else
    {
        s_listenerHead = s_listenerTail = l;
    }

    xSemaphoreGive(s_mutex);
}

void LPM_UnregisterPowerListener(lpm_power_mode_callback_t callback, void *data)
{
    lpm_power_mode_listener_t *l, *p = NULL;

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdFALSE)
    {
        assert(0);
    }

    for (l = s_listenerHead; l != NULL; l = l->next)
    {
        if (l->callback == callback && l->data == data)
        {
            if (p)
            {
                p->next = l->next;
            }
            else
            {
                s_listenerHead = l->next;
            }

            if (l->next == NULL)
            {
                s_listenerTail = p;
            }

            vPortFree(l);
            break;
        }
        p = l;
    }

    xSemaphoreGive(s_mutex);
}
#endif

/************ Internal public API start **************/
#ifdef SDK_OS_FREE_RTOS
#if (configUSE_TICKLESS_IDLE == 2)

/*!
 * @brief Interrupt service fuction of GPT timer.
 *
 * This function to call low power timer ISR
 */
void GPT1_IRQHandler(void)
{
    vPortGptIsr();
}

/*!
 * @brief Fuction of GPT timer.
 *
 * This function to return GPT timer base address
 */

GPT_Type *vPortGetGptBase(void)
{
    return GPT1;
}

/*!
 * @brief Fuction of GPT timer.
 *
 * This function to return GPT timer interrupt number
 */

IRQn_Type vPortGetGptIrqn(void)
{
    return GPT1_IRQn;
}

void LPM_EnterSleepMode(clock_mode_t mode)
{
    assert(mode != kCLOCK_ModeRun);

    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();

    if (mode == kCLOCK_ModeWait)
    {
        /* Clear the SLEEPDEEP bit to go into sleep mode (WAIT) */
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }
    else
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

void vPortPRE_SLEEP_PROCESSING(unsigned long timeoutMilliSec)
{
    clock_mode_t mode = kCLOCK_ModeRun;
    APP_PowerPreSwitchHook(APP_GetLPMPowerMode());
    LPM_EnableWakeupSource(vPortGetGptIrqn());

    switch (APP_GetLPMPowerMode())
    {
        case LPM_PowerModeSysIdle:
            mode = kCLOCK_ModeWait;
            LPM_EnterSystemIdle(APP_GetRunMode());
            break;
        case LPM_PowerModeLPIdle:
            mode = kCLOCK_ModeWait;
            LPM_EnterLowPowerIdle(APP_GetRunMode());
            break;
        case LPM_PowerModeSuspend:
            mode = kCLOCK_ModeStop;
            LPM_EnterSuspend();
            break;
        default:
            assert(false);
            break;
    }
    if (timeoutMilliSec == 0)
    {
        LPM_EnterSleepMode(mode);
    }
}

void vPortPOST_SLEEP_PROCESSING(unsigned long timeoutMilliSec)
{
    switch (APP_GetLPMPowerMode())
    {
        case LPM_PowerModeSysIdle:
            LPM_ExitSystemIdle(APP_GetRunMode());
            break;
        case LPM_PowerModeLPIdle:
            LPM_ExitLowPowerIdle(APP_GetRunMode());
            break;
        case LPM_PowerModeSuspend:
            break;
        default:
            assert(false);
            break;
    }

    LPM_DisableWakeupSource(vPortGetGptIrqn());
    APP_PowerPostSwitchHook(APP_GetLPMPowerMode());
}
#endif /* configUSE_TICKLESS_IDLE */
#endif
