/*
 * Copyright 2017-2019, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "fsl_lptmr.h"
#include "fsl_pmc0.h"
#include "fsl_mu.h"

#include "lpm.h"
#include "fsl_msmc.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ROM_RESERVED_MEM ((uint8_t *)0x20008000)
#define ROM_HEADER_MEM   ((uint8_t *)0x1FFD0000)
#define ROM_HEADER_SIZE  (0x1C00)

#define SYSTICK_SOURCE_CLOCK   CLOCK_GetFreq(kCLOCK_ScgSircClk)
#define SYSTICK_TICKLESS_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)
/* Define the count per tick of the systick(LPTMR) in run mode. For accuracy purpose,
 * please make SYSTICK_SOURCE_CLOCK times of configTICK_RATE_HZ.
 */
#define SYSTICK_COUNT_PER_TICK (SYSTICK_SOURCE_CLOCK / configTICK_RATE_HZ)

#define PFD_GATES_MASK                                                                               \
    (SCG_SPLLPFD_PFD0_CLKGATE_MASK | SCG_SPLLPFD_PFD1_CLKGATE_MASK | SCG_SPLLPFD_PFD2_CLKGATE_MASK | \
     SCG_SPLLPFD_PFD3_CLKGATE_MASK)
#define PFD_VALID_MASK                                                                         \
    (SCG_SPLLPFD_PFD0_VALID_MASK | SCG_SPLLPFD_PFD1_VALID_MASK | SCG_SPLLPFD_PFD2_VALID_MASK | \
     SCG_SPLLPFD_PFD3_VALID_MASK)

struct _lpm_power_mode_listener
{
    lpm_power_mode_callback_t callback;
    void *data;
    struct _lpm_power_mode_listener *next;
};

typedef struct _lpm_power_mode_listener lpm_power_mode_listener_t;

typedef struct _lpm_nvic_context
{
    uint32_t PriorityGroup;
    uint32_t ISER[8];
    uint8_t IP[240]; /* ULP CM4 max IRQn is 76 */
    uint8_t SHP[12];
} lpm_nvic_context_t;

static lpm_power_mode_t s_curMode;
static lpm_nvic_context_t s_nvicContext;
static volatile uint32_t s_psp;
static uint32_t s_usbWakeup;
static uint32_t s_rccr;
static uint32_t s_qspipcc;
static uint32_t s_sosccsr;
static uint32_t s_firccsr;
static uint32_t s_sirccsr;
static uint32_t s_spllcsr;
static uint32_t s_apllcsr;

static SemaphoreHandle_t s_mutex;
static lpm_power_mode_listener_t *s_listenerHead;
static lpm_power_mode_listener_t *s_listenerTail;

#if (defined(__ICCARM__))
static uint8_t s_suspendMem[0x4000] @"M4SuspendRam";
#elif (defined(__ARMCC_VERSION))
static uint8_t s_suspendMem[0x4000] __attribute__((section("M4SuspendRam"), zero_init));
#elif (defined(__GNUC__))
static uint8_t s_suspendMem[0x4000] __attribute__((section("M4SuspendRam,\"aw\",%nobits @")));
#else
#error Toolchain not supported.
#endif

/* Functions implemented in lpm_asm.s */
extern bool LPM_Suspend(void);
extern void LPM_Resume(void);

AT_QUICKACCESS_SECTION_CODE(extern void BOARD_SetRunMode(
    SCG_Type *scg, uint32_t scgRunConfig, QuadSPI_Type *qspi, clock_ip_name_t qspiClock, uint32_t qspiClockConfig));
extern bool BOARD_IsRunOnQSPI(void);

/* FreeRTOS implemented Systick handler. */
extern void xPortSysTickHandler(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
static void LPM_DisableSOSC(void)
{
    QuadSPI_Type *qspi = BOARD_IsRunOnQSPI() ? QuadSPI0 : NULL;

    /* SOSC can only be disabled when A7 is in VLLS */
    if (MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm)
    {
        return;
    }

    /* Power down OCOTP. */
    OCOTP_CTRL->HW_OCOTP_PDN = 1;

    s_rccr = 0;
    /* Only RUN/VLPR may enter VLLS/LLS/VLPS, and VLPR doesn't use SOSC */
    if (SMC_GetPowerModeState(MSMC0) == kSMC_PowerStateRun)
    {
        s_rccr    = SCG0->RCCR;
        s_qspipcc = *((volatile uint32_t *)kCLOCK_Qspi);
        /* Switch CPU and QSPI to FIRC */
        BOARD_SetRunMode(SCG0, (kSCG_SysClkSrcFirc << SCG_RCCR_SCS_SHIFT) | SCG_RCCR_DIVSLOW(1), qspi, kCLOCK_Qspi,
                         PCC1_PCC_QSPI_OTFAD_CGC_MASK | PCC1_PCC_QSPI_OTFAD_PCS(3));
        s_spllcsr = SCG0->SPLLCSR & SCG_SPLLCSR_SPLLEN_MASK;
        s_apllcsr = SCG0->APLLCSR & SCG_APLLCSR_APLLEN_MASK;
        /* Disable SPLL. */
        if (s_spllcsr)
        {
            SCG0->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;
        }
        /* Disable APLL. */
        if (s_apllcsr)
        {
            SCG0->APLLCSR &= ~SCG_APLLCSR_APLLEN_MASK;
        }
    }
    s_sosccsr = SCG0->SOSCCSR & SCG_SOSCCSR_SOSCEN_MASK;
    /* Disable SOSC. */
    if (s_sosccsr)
    {
        SCG0->SOSCCSR &= ~SCG_SOSCCSR_SOSCEN_MASK;
    }
    /* Disable FIRC in VLPS. */
    s_firccsr = SCG0->FIRCCSR & SCG_FIRCCSR_FIRCLPEN_MASK;
    if (s_firccsr)
    {
        SCG0->FIRCCSR &= ~SCG_FIRCCSR_FIRCLPEN_MASK;
    }
    /* Disable SIRC in VLPS. */
    s_sirccsr = SCG0->SIRCCSR & SCG_SIRCCSR_SIRCLPEN_MASK;
    if (s_sirccsr)
    {
        SCG0->SIRCCSR &= ~SCG_SIRCCSR_SIRCLPEN_MASK;
    }
}

static void LPM_EnableSOSC(void)
{
    QuadSPI_Type *qspi = BOARD_IsRunOnQSPI() ? QuadSPI0 : NULL;

    /* SOSC can only be disabled when A7 is in VLLS */
    if (MU_GetOtherCorePowerMode(MUA) != kMU_PowerModeDsm)
    {
        return;
    }

    /* Restore FIRC VLPS Enable */
    if (s_firccsr)
    {
        SCG->FIRCCSR |= SCG_FIRCCSR_FIRCLPEN_MASK;
    }

    /* Restore SIRC VLPS Enable */
    if (s_sirccsr)
    {
        SCG->SIRCCSR |= SCG_SIRCCSR_SIRCLPEN_MASK;
    }

    /* Restore SOSC clock */
    if (s_sosccsr)
    {
        SCG->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
        while (!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK))
        {
        }
    }

    if (s_rccr)
    {
        /* Restore SPLL */
        if (s_spllcsr)
        {
            SCG0->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;
            while (!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK))
            {
            }
        }
        /* Restore APLL */
        if (s_apllcsr)
        {
            SCG0->APLLCSR |= SCG_APLLCSR_APLLEN_MASK;
            while (!(SCG->APLLCSR & SCG_APLLCSR_APLLVLD_MASK))
            {
            }
        }

        /* Switching CPU and QSPI clock back */
        BOARD_SetRunMode(SCG0, s_rccr, qspi, kCLOCK_Qspi, s_qspipcc);
    }
}

static uint32_t LPM_EnterTicklessIdle(uint32_t timeoutMilliSec, uint64_t *pCounter)
{
    uint64_t counter;
    uint32_t expired;
    uint32_t ms, maxMS;
    uint32_t flag;
    uint32_t timeoutTicks;
    uint32_t countPerTick = SYSTICK_COUNT_PER_TICK;
    uint32_t sourceClock  = SYSTICK_SOURCE_CLOCK;
    lptmr_config_t lptmrConfig;

    assert(sourceClock != 0U);

    if ((uint64_t)timeoutMilliSec * configTICK_RATE_HZ < 2 * 1000)
    {
        /* If timeout < 2 ticks, don't do tickless idle. */
        return 0;
    }

    maxMS = 0xFFFFU / SYSTICK_TICKLESS_CLOCK * 1000;
    ms    = timeoutMilliSec > maxMS ? maxMS : timeoutMilliSec;

    /* Calculate the LPTMR counter needed for timeout */
    timeoutTicks = (uint64_t)ms * configTICK_RATE_HZ / 1000;
    counter      = (uint64_t)timeoutTicks * countPerTick;

    expired = LPTMR_GetCurrentTimerCount(SYSTICK_BASE);
    flag    = LPTMR_GetStatusFlags(SYSTICK_BASE);
    LPTMR_Deinit(SYSTICK_BASE); /* Flag cleared and timer stopped. */
    NVIC_ClearPendingIRQ(SYSTICK_IRQn);
    if (flag)
    { /* tick already pending, just add 1 tick counter */
        /* A simple judgement to decide if the counter need to add in */
        if (expired <= countPerTick / 2)
        {
            /* Counter is accumulated after expired */
            expired += countPerTick;
        }
        else
        {
            /* Just expired before counter did reset */
            expired = countPerTick;
        }
    }

    /* Minus those already expired to get accurate waiting counter */
    assert(counter > expired);
    counter -= expired;

    /* Now reinit Systick with tickless clock source. */
    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* Select LPO 1kHz as tick timer clock source */
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
    /* Set counter to be free running. */
    lptmrConfig.enableFreeRunning = true;
    /* Initialize the LPTMR */
    LPTMR_Init(SYSTICK_BASE, &lptmrConfig);
    /* Convert count in systick freq to tickless clock count */
    LPTMR_SetTimerPeriod(SYSTICK_BASE, counter * SYSTICK_TICKLESS_CLOCK / sourceClock);
    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(SYSTICK_BASE, kLPTMR_TimerInterruptEnable);
    /* Restart timer */
    LPTMR_StartTimer(SYSTICK_BASE);

    /* return waiting counter */
    *pCounter = counter;

    return timeoutTicks;
}

static void LPM_ExitTicklessIdle(uint32_t timeoutTicks, uint64_t timeoutCounter)
{
    uint32_t expireTicks, counter;
    uint64_t expired;
    uint32_t completeTicks;
    uint32_t countPerTick = SYSTICK_COUNT_PER_TICK;
    lptmr_config_t lptmrConfig;

    assert(countPerTick != 0U);

    /* Convert tickless count to systick count. */
    expired = (uint64_t)(LPTMR_GetCurrentTimerCount(SYSTICK_BASE)) * SYSTICK_SOURCE_CLOCK / SYSTICK_TICKLESS_CLOCK;
    LPTMR_Deinit(SYSTICK_BASE);
    if (expired >= timeoutCounter)
    {
        expireTicks = (expired - timeoutCounter) / countPerTick;
        /* Tick already pending, indicates timeout. */
        completeTicks = timeoutTicks + expireTicks;
        /* Now calculate counter for next LPTMR interrupt. */
        counter = countPerTick - (expired - timeoutCounter) % countPerTick;
    }
    else
    {
        /* remaining counter */
        counter       = timeoutCounter - expired;
        completeTicks = timeoutTicks - (counter - 1) / countPerTick - 1;
        counter       = (counter - 1) % countPerTick + 1;
    }

    /* Now reinit Systick with systick clock source. */
    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* Select SIRC as tick timer clock source */
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_0;
    /* Set counter to be free running. */
    lptmrConfig.enableFreeRunning = true;
    /* Initialize the LPTMR */
    LPTMR_Init(SYSTICK_BASE, &lptmrConfig);
    /* Convert count in systick freq to tickless clock count */
    LPTMR_SetTimerPeriod(SYSTICK_BASE, counter);
    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(SYSTICK_BASE, kLPTMR_TimerInterruptEnable);
    /* Restart timer */
    LPTMR_StartTimer(SYSTICK_BASE);

    vTaskStepTick(completeTicks);
}

bool LPM_Init(void)
{
    smc_power_state_t mode;
    s_mutex = xSemaphoreCreateMutex();

    if (s_mutex == NULL)
    {
        return false;
    }

    s_listenerHead = s_listenerTail = NULL;

    mode = SMC_GetPowerModeState(MSMC0);
    switch (mode)
    {
        case kSMC_PowerStateRun:
            s_curMode = LPM_PowerModeRun;
            break;
        case kSMC_PowerStateVlpr:
            s_curMode = LPM_PowerModeVlpr;
            break;
        case kSMC_PowerStateHsrun:
            s_curMode = LPM_PowerModeHsrun;
            break;
        default:
            return false;
    }

    return true;
}

void LPM_Deinit(void)
{
    if (s_mutex != NULL)
    {
        vSemaphoreDelete(s_mutex);
        s_mutex = NULL;
    }
}

bool LPM_IsTargetModeValid(lpm_power_mode_t targetPowerMode, const char **pErrorMsg)
{
    bool modeValid = true;
    mu_power_mode_t powerMode;
    smc_power_state_t curPowerState;
    const char *errorMsg = NULL;

    curPowerState = SMC_GetPowerModeState(MSMC0);
    /*
     * Check whether the mode change is allowed.
     *
     * 1. If current mode is HSRUN mode, the target mode must be RUN mode.
     * 2. If current mode is RUN mode, the target mode must not be VLPW mode.
     * 3. If current mode is VLPR mode, the target mode must not be HSRUN/WAIT/STOP mode.
     * 4. If already in the target mode.
     */
    switch (curPowerState)
    {
        case kSMC_PowerStateHsrun:
            if (LPM_PowerModeRun != targetPowerMode)
            {
                errorMsg  = "Current mode is HSRUN, please choose RUN mode as target mode.\r\n";
                modeValid = false;
            }
            break;

        case kSMC_PowerStateRun:
            if (LPM_PowerModeVlpw == targetPowerMode)
            {
                errorMsg  = "Could not enter VLPW mode from RUN mode.\r\n";
                modeValid = false;
            }
            break;

        case kSMC_PowerStateVlpr:
            if ((LPM_PowerModeWait == targetPowerMode) || (LPM_PowerModeHsrun == targetPowerMode) ||
                (LPM_PowerModeStop == targetPowerMode))
            {
                errorMsg  = "Could not enter HSRUN/STOP/WAIT modes from VLPR mode.\r\n";
                modeValid = false;
            }
            break;

        default:
            errorMsg  = "Wrong power state.\r\n";
            modeValid = false;
            break;
    }

    if (modeValid)
    {
        powerMode = MU_GetOtherCorePowerMode(MUA);
        switch (targetPowerMode)
        {
            case LPM_PowerModeLls:
                if (powerMode != kMU_PowerModeDsm)
                {
                    errorMsg  = "M4 can enter LLS Mode only when A7 in LLS Mode or VLLS Mode!!!\r\n";
                    modeValid = false;
                }
                break;

            case LPM_PowerModeVlls:
                if (powerMode != kMU_PowerModeDsm)
                {
                    errorMsg  = "M4 can enter VLLS Mode only when A7 in VLLS Mode!!!\r\n";
                    modeValid = false;
                }
                break;

            default:
                break;
        }
    }

    if (modeValid)
    {
        /* Don't need to change power mode if current mode is already the target mode. */
        if (((LPM_PowerModeRun == targetPowerMode) && (kSMC_PowerStateRun == curPowerState)) ||
            ((LPM_PowerModeHsrun == targetPowerMode) && (kSMC_PowerStateHsrun == curPowerState)) ||
            ((LPM_PowerModeVlpr == targetPowerMode) && (kSMC_PowerStateVlpr == curPowerState)))
        {
            errorMsg  = "Already in the target power mode.\r\n";
            modeValid = false;
        }
    }

    if (pErrorMsg)
    {
        *pErrorMsg = errorMsg;
    }

    return modeValid;
}

bool LPM_SetPowerMode(lpm_power_mode_t mode)
{
    lpm_power_mode_listener_t *l1, *l2;
    bool ret = true;

    if (mode == s_curMode)
    {
        return ret;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);

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

    return ret;
}

lpm_power_mode_t LPM_GetPowerMode(void)
{
    return s_curMode;
}

bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec)
{
    uint32_t irqMask;
    uint64_t counter = 0;
    uint32_t timeoutTicks;
    status_t status = kStatus_Success;

    irqMask = DisableGlobalIRQ();

    if (!LPM_IsTargetModeValid(s_curMode, NULL))
    {
        EnableGlobalIRQ(irqMask);
        return false;
    }

    timeoutTicks = LPM_EnterTicklessIdle(timeoutMilliSec, &counter);

    switch (s_curMode)
    {
        case LPM_PowerModeWait:
            status = SMC_SetPowerModeWait(MSMC0);
            break;
        case LPM_PowerModeStop:
            status = SMC_SetPowerModeStop(MSMC0, kSMC_PartialStop);
            break;
        case LPM_PowerModeVlpw:
            status = SMC_SetPowerModeWait(MSMC0);
            break;
        case LPM_PowerModeVlps:
            LPM_DisableSOSC();
            status = SMC_SetPowerModeVlps(MSMC0);
            LPM_EnableSOSC();
            break;
        case LPM_PowerModeLls:
            LPM_DisableSOSC();
            status = SMC_SetPowerModeLls(MSMC0);
            LPM_EnableSOSC();
            break;
        case LPM_PowerModeVlls:
            if (!LPM_Suspend())
            {
                status = kStatus_Fail;
            }
            else
            {
                CLOCK_EnableClock(SYSTICK_CLOCK_NAME); /* Make sure Systick clock is recovered from VLLS. */
            }
            break;
        default:
            break;
    }

    if (timeoutTicks > 0)
    {
        LPM_ExitTicklessIdle(timeoutTicks, counter);
    }

    EnableGlobalIRQ(irqMask);

    return status == kStatus_Success;
}

void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data)
{
    lpm_power_mode_listener_t *l = (lpm_power_mode_listener_t *)pvPortMalloc(sizeof(lpm_power_mode_listener_t));
    assert(l);

    l->callback = callback;
    l->data     = data;
    l->next     = NULL;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

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

    xSemaphoreTake(s_mutex, portMAX_DELAY);

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

/************ Internal public API start **************/
/* The systick interrupt handler. */
void SYSTICK_HANDLER(void)
{
    /* Clear the interrupt. */
    LPTMR_ClearStatusFlags(SYSTICK_BASE, kLPTMR_TimerCompareFlag);

    if (SYSTICK_BASE->CSR & LPTMR_CSR_TFC_MASK)
    {
        /* Freerun timer means this is the first tick after tickless exit. */
        LPTMR_StopTimer(SYSTICK_BASE);
        LPTMR_SetTimerPeriod(LPTMR0, SYSTICK_SOURCE_CLOCK / configTICK_RATE_HZ);
        SYSTICK_BASE->CSR &= ~LPTMR_CSR_TFC_MASK;
        LPTMR_StartTimer(SYSTICK_BASE);
    }

    /* Call FreeRTOS tick handler. */
    xPortSysTickHandler();

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
 * defined in the FreeRTOS Cortex-M4F port layer with a version that configures LPTMR0
 * to generate the tick interrupt. */
void vPortSetupTimerInterrupt(void)
{
    lptmr_config_t lptmrConfig;

    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* Select SIRC as tick timer clock source */
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_0;
    /* Initialize the LPTMR */
    LPTMR_Init(SYSTICK_BASE, &lptmrConfig);

    /* Set timer period */
    LPTMR_SetTimerPeriod(SYSTICK_BASE, SYSTICK_SOURCE_CLOCK / configTICK_RATE_HZ);

    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(SYSTICK_BASE, kLPTMR_TimerInterruptEnable);
    NVIC_SetPriority(SYSTICK_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(SYSTICK_IRQn);

    /* Start counting */
    LPTMR_StartTimer(SYSTICK_BASE);
}

void LPM_NvicStateSave(void)
{
    uint32_t i;
    uint32_t irqRegs;
    uint32_t irqNum;

    irqRegs = (SCnSCB->ICTR & SCnSCB_ICTR_INTLINESNUM_Msk) + 1;
    irqNum  = irqRegs * 32;

    s_nvicContext.PriorityGroup = NVIC_GetPriorityGrouping();

    for (i = 0; i < irqRegs; i++)
    {
        s_nvicContext.ISER[i] = NVIC->ISER[i];
    }

    for (i = 0; i < irqNum; i++)
    {
        s_nvicContext.IP[i] = NVIC->IP[i];
    }

    s_nvicContext.SHP[0]  = SCB->SHP[0];  /* MemManage */
    s_nvicContext.SHP[1]  = SCB->SHP[1];  /* BusFault */
    s_nvicContext.SHP[2]  = SCB->SHP[2];  /* UsageFault */
    s_nvicContext.SHP[7]  = SCB->SHP[7];  /* SVCall */
    s_nvicContext.SHP[8]  = SCB->SHP[8];  /* DebugMonitor */
    s_nvicContext.SHP[10] = SCB->SHP[10]; /* PendSV */
    s_nvicContext.SHP[11] = SCB->SHP[11]; /* SysTick */
}

void LPM_NvicStateRestore(void)
{
    uint32_t i;
    uint32_t irqRegs;
    uint32_t irqNum;

    irqRegs = (SCnSCB->ICTR & SCnSCB_ICTR_INTLINESNUM_Msk) + 1;
    irqNum  = irqRegs * 32;

    NVIC_SetPriorityGrouping(s_nvicContext.PriorityGroup);

    for (i = 0; i < irqRegs; i++)
    {
        NVIC->ISER[i] = s_nvicContext.ISER[i];
    }

    for (i = 0; i < irqNum; i++)
    {
        NVIC->IP[i] = s_nvicContext.IP[i];
    }

    SCB->SHP[0]  = s_nvicContext.SHP[0];  /* MemManage */
    SCB->SHP[1]  = s_nvicContext.SHP[1];  /* BusFault */
    SCB->SHP[2]  = s_nvicContext.SHP[2];  /* UsageFault */
    SCB->SHP[7]  = s_nvicContext.SHP[7];  /* SVCall */
    SCB->SHP[8]  = s_nvicContext.SHP[8];  /* DebugMonitor */
    SCB->SHP[10] = s_nvicContext.SHP[10]; /* PendSV */
    SCB->SHP[11] = s_nvicContext.SHP[11]; /* SysTick */
}

void LPM_SystemSuspend(uint32_t psp)
{
    uint32_t mask0;

    /* Save USB wakeup setting. */
    s_usbWakeup = SIM->GPR1 & SIM_GPR1_USB_PHY_WAKEUP_ISO_DISABLE_MASK;

    mask0 = SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP6_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP5_MASK |
            SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP4_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP3_MASK |
            SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP2_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK;

    s_psp = psp; /* Save PSP for resume context */

    /* Setup VLLS Resume Entry. */
    SIM->SIM_DGO_GP1   = (uint32_t)LPM_Resume;
    SIM->SIM_DGO_CTRL0 = (SIM->SIM_DGO_CTRL0 & ~mask0) | SIM_SIM_DGO_CTRL0_UPDATE_DGO_GP1_MASK;
    /* Wait DGO GP1 updated */
    while ((SIM->SIM_DGO_CTRL0 & SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK) == 0)
    {
    }
    /* Clear DGO GP1 ACK and UPDATE bits */
    SIM->SIM_DGO_CTRL0 =
        (SIM->SIM_DGO_CTRL0 & ~(SIM_SIM_DGO_CTRL0_UPDATE_DGO_GP1_MASK | mask0)) | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK;

    LPM_DisableSOSC();
    /* Save data which will be updated by ROM resume. */
    memcpy(s_suspendMem, ROM_RESERVED_MEM, sizeof(s_suspendMem));
    memcpy(ROM_HEADER_MEM, ROM_RESERVED_MEM + sizeof(s_suspendMem), ROM_HEADER_SIZE);

    /* Request PMIC to standby mode */
    SIM->SOPT1 |= SIM_SOPT1_PMIC_STBY_REQ_MASK;
    SMC_SetPowerModeVlls(MSMC0);
    LPM_EnableSOSC();
}

uint32_t LPM_SystemResume(bool resume)
{
    uint32_t mask0;

    /* Clear PMIC standby request */
    SIM->SOPT1 &= ~SIM_SOPT1_PMIC_STBY_REQ_MASK;

    mask0 = SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP6_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP5_MASK |
            SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP4_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP3_MASK |
            SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP2_MASK | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK;

    SMC_SetPowerModeProtection(MSMC0, kSMC_AllowPowerModeAll);

    /* Clear VLLS Resume Entry. */
    SIM->SIM_DGO_GP1   = 0U;
    SIM->SIM_DGO_CTRL0 = (SIM->SIM_DGO_CTRL0 & ~mask0) | SIM_SIM_DGO_CTRL0_UPDATE_DGO_GP1_MASK;
    /* Wait DGO GP1 updated */
    while ((SIM->SIM_DGO_CTRL0 & SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK) == 0)
    {
    }
    /* Clear DGO GP1 ACK and UPDATE bits */
    SIM->SIM_DGO_CTRL0 =
        (SIM->SIM_DGO_CTRL0 & ~(SIM_SIM_DGO_CTRL0_UPDATE_DGO_GP1_MASK | mask0)) | SIM_SIM_DGO_CTRL0_WR_ACK_DGO_GP1_MASK;

    /* Restore data. */
    memcpy(ROM_RESERVED_MEM, s_suspendMem, sizeof(s_suspendMem));
    memcpy(ROM_RESERVED_MEM + sizeof(s_suspendMem), ROM_HEADER_MEM, ROM_HEADER_SIZE);

    SIM->GPR1 = (SIM->GPR1 & ~SIM_GPR1_USB_PHY_WAKEUP_ISO_DISABLE_MASK) | s_usbWakeup;

    return s_psp;
}
