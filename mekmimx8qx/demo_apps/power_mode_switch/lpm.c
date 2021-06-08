/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "lpm.h"
#include "fsl_asmc.h"
#include "fsl_cache.h"
#include "fsl_irqsteer.h"
#include "fsl_memory.h"
#include "fsl_mu.h"

#ifdef FSL_RTOS_FREE_RTOS
#include "fsl_gpt.h"
#include "fsl_tstmr.h"
#include "svc/irq/irq_api.h"
#include "svc/timer/timer_api.h"
/* FreeRTOS header */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#endif

#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef FSL_RTOS_FREE_RTOS
/* Systick counter source clock frequency. Need config the Systick counter's(GPT) clock
 * source to this frequency in application.
 */
#define SYSTICK_COUNTER_SOURCE_CLK_FREQ (SC_24MHZ)
/* Define the counter clock of the systick (GPT). The SYSCTR used as the timer in tickless idle counting at 8MHZ,
 * To make it simple set the systick ounter to 8MHZ. For accuracy purpose,
 * please make LPM_SYSTICK_COUNTER_FREQ divisible by 8000000, and times of
 * configTICK_RATE_HZ.
 */
#define SYSTICK_COUNTER_FREQ          (SC_8MHZ)
#define SYSTICK_TICKLESS_COUNTER_FREQ (SC_8MHZ)
/* Define the count per tick of the systick in run mode. For accuracy purpose,
 * please make SYSTICK_SOURCE_CLOCK times of configTICK_RATE_HZ.
 */
#define SYSTICK_COUNT_PER_TICK (SYSTICK_COUNTER_FREQ / configTICK_RATE_HZ)
/* The approximate resume time(since tickless idle enter to tickless idle exit) for LLS is 2.7ms */
#define RESUME_TIME_MS_LLS 3
/* The approximate resume time(since tickless idle enter to tickless idle exit) for VLLS is 5ms */
#define RESUME_TIME_MS_VLLS 5
/* Get the NVIC IRQn of given IRQSTEER IRQn */
#define GET_IRQSTEER_MASTER_IRQn(IRQn) \
    (IRQn_Type)(IRQSTEER_0_IRQn + (IRQn - FSL_FEATURE_IRQSTEER_IRQ_START_INDEX) / 64U)
#endif /* FSL_RTOS_FREE_RTOS */

#define TSTMR_BASE           CM4__TSTMR
#define M4_LANDING_ZONE_SIZE (32U)
#define SC_RPC_MU_GIPn       0x1U /* The GIPn used for SCU MU broadcast IRQ */
/* The IPC MU interrupt priority. Once waken up by SCU, the MU interrtup will be handled.
And the FreeRTOS software timer runs, users can handle SCU MU broadcast in the timer callback function. */
#define SC_RPC_MU_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY - 1U)

#ifdef FSL_RTOS_FREE_RTOS
struct _lpm_power_mode_listener
{
    lpm_power_mode_callback_t callback;
    void *data;
    struct _lpm_power_mode_listener *next;
};

typedef struct _lpm_power_mode_listener lpm_power_mode_listener_t;

static SemaphoreHandle_t s_mutex;
static lpm_power_mode_listener_t *s_listenerHead;
static lpm_power_mode_listener_t *s_listenerTail;
static uint32_t s_resumeTicks; /* Approximate ticks expired during LPM enter and resume. */
static volatile uint64_t s_lastTimeStamp;

/* FreeRTOS implemented Systick handler. */
extern void xPortSysTickHandler(void);
#endif

extern uint32_t __BACKUP_REGION_START[];
/* M4 Resume landing zone. The SCU will copy 32 bytes from resume address to it's TCM and start M4 core. The DataLpm
 * section need to be retained in all power mode.*/
#if (defined(__ICCARM__))
SDK_ALIGN(static uint32_t s_suspendMem[0x8], 4) @"DataLpm";
#elif (defined(__ARMCC_VERSION))
SDK_ALIGN(static uint32_t s_suspendMem[0x8], 4) __attribute__((section("DataLpm"), zero_init));
#elif (defined(__GNUC__))
SDK_ALIGN(static uint32_t s_suspendMem[0x8], 4) __attribute__((section("DataLpm,\"aw\",%nobits @")));
#else
#error Toolchain not supported.
#endif

typedef struct _lpm_nvic_context
{
    uint32_t PriorityGroup;
    uint32_t ISER[8];
    uint8_t IP[240]; /* 8QX CM4 max IRQn is 50 */
    uint8_t SHP[12];
} lpm_nvic_context_t;

static lpm_power_mode_t s_curMode;
static lpm_nvic_context_t s_nvicContext;
static volatile uint32_t s_psp;                               /* The MSP is used in bare metal application. */
static volatile uint32_t *lmpecrReg = (uint32_t *)0xE0080480; /* LMEM Parity & ECC Control Register in MCM */
static volatile uint32_t s_lmpecr;

/* Functions implemented in lpm_asm.s */
extern bool LPM_Suspend(void);
extern void LPM_Resume(void);
extern void LPM_ResumeWithBackup(void);

/* Exception handler defined in startup_xxx.s */
extern void NMI_Handler(void);
extern void HardFault_Handler(void);
extern void MemManage_Handler(void);
extern void BusFault_Handler(void);
extern void UsageFault_Handler(void);
/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef FSL_RTOS_FREE_RTOS

static void LPM_GptInit(void)
{
    gpt_config_t gptConfig;

    /* Power on GPT and set GPT clock to 24MHZ. */
    RTN_ERR(sc_pm_set_resource_power_mode((sc_ipc_t)IPC_MU, SYSTICK_RSRC, SC_PM_PW_MODE_ON));
    CLOCK_SetIpFreq(SYSTICK_CLOCK, SC_24MHZ);
    /*
     * gptConfig.clockSource = kGPT_ClockSource_Periph;
     * gptConfig.divider = 1U;
     * gptConfig.enableRunInStop = true;
     * gptConfig.enableRunInWait = true;
     * gptConfig.enableRunInDoze = false;
     * gptConfig.enableRunInDbg = false;
     * gptConfig.enableFreeRun = false;
     * gptConfig.enableMode  = true;
     */
    GPT_GetDefaultConfig(&gptConfig);
    gptConfig.clockSource = kGPT_ClockSource_Periph;
    /* Initialize timer */
    GPT_Init(SYSTICK_BASE, &gptConfig);
    /* Divide GPT clock source frequency to SYSTICK_COUNTER_FREQ */
    GPT_SetClockDivider(SYSTICK_BASE, SYSTICK_COUNTER_SOURCE_CLK_FREQ / SYSTICK_COUNTER_FREQ);

    /* Enable timer interrupt, NIVC configuration is saved/restored during DSM/Wake */
    GPT_EnableInterrupts(SYSTICK_BASE, kGPT_OutputCompare1InterruptEnable);
    IRQSTEER_EnableInterrupt(IRQSTEER, SYSTICK_IRQn);
}

/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
 * defined in the FreeRTOS Cortex-M4F port layer with a version that configures
 * GPT to generate the tick interrupt. */
void vPortSetupTimerInterrupt(void)
{
    LPM_GptInit();

    /* Set timer period */
    GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, SYSTICK_COUNT_PER_TICK - 1U);
    /* Enable timer interrupt */
    NVIC_SetPriority(GET_IRQSTEER_MASTER_IRQn(SYSTICK_IRQn), configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(GET_IRQSTEER_MASTER_IRQn(SYSTICK_IRQn));

    /* Start counting */
    GPT_StartTimer(SYSTICK_BASE);
}

static uint32_t LPM_EnterTicklessIdle(uint32_t timeoutMilliSec, uint64_t *pCounter)
{
    uint64_t counter;
    uint32_t flag, timeoutTicks, expired = 0;
    uint32_t countPerTick = SYSTICK_COUNT_PER_TICK;

    /* Enter and resume from LLS/VLLS need several ticks if RTOS ticks rate is high.
     * Calculate the ticks needed for resume and minus from the FreeRTOS timeoutMilliSec
     * to make sure the ready task execuated on time. The minused ticks will be added to
     * FreeRTOS xTickCount in LPM_ExitTicklessIdle.
     */
    if (s_curMode == LPM_PowerModeVlls)
    {
        s_resumeTicks = RESUME_TIME_MS_VLLS * configTICK_RATE_HZ / 1000;
    }
    else if (s_curMode == LPM_PowerModeLls)
    {
        s_resumeTicks = RESUME_TIME_MS_LLS * configTICK_RATE_HZ / 1000;
    }
    else
    {
        s_resumeTicks = 0;
    }

    /* If (timeout - resumeTime ) < 2 ticks, don't do tickless idle. */
    if ((uint64_t)timeoutMilliSec * configTICK_RATE_HZ < (2 + s_resumeTicks) * 1000)
    {
        return 0;
    }

    /* Calculate the timer counter needed for timeout. */
    timeoutTicks = (uint64_t)timeoutMilliSec * configTICK_RATE_HZ / 1000 - s_resumeTicks;
    counter      = (uint64_t)timeoutTicks * countPerTick;

    GPT_StopTimer(SYSTICK_BASE); /* Timer stopped. */
    flag = GPT_GetStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);
    if (flag)
    {
        GPT_ClearStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);
        NVIC_ClearPendingIRQ(GET_IRQSTEER_MASTER_IRQn(SYSTICK_IRQn));
        expired = countPerTick;
    }
    expired += GPT_GetCurrentTimerCount(SYSTICK_BASE);
    /* Minus those already expired to get accurate waiting counter. */
    counter -= expired;

    /* Switch to SYSCTR and power off SYSTICK timer for power saving.
     * The TSTMR's counter value in M4 Subsystem alwasy sync to the SYSCTR(System Counter).
     * The SYSCTR in System Controller Subsystem is a 56-bit wide always running counter
     * equivalently increase at 8MHZ.
     */
    /*
     * NOTE : When lpm driver is used, the BOARD_Enable_SCIRQ must be invoked
     * It is done in LPM_Init
     */
    BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_SysCtr), true);
    /* The tickless timer and systick timer ticks at same freq. */
    sc_timer_set_sysctr_periodic_alarm((sc_ipc_t)IPC_MU, counter - 1U);
    s_lastTimeStamp = TSTMR_ReadTimeStamp(TSTMR_BASE);

    /* Power off the GPT tiemr. */
    RTN_ERR(sc_pm_set_resource_power_mode((sc_ipc_t)IPC_MU, SYSTICK_RSRC, SC_PM_PW_MODE_OFF));

    /* return waiting counter */
    *pCounter = counter;

    return timeoutTicks;
}

static void LPM_ExitTicklessIdle(uint32_t timeoutTicks, uint64_t timeoutCounter)
{
    uint64_t counter, expired;
    uint32_t expiredTicks, completeTicks;
    uint32_t countPerTick = SYSTICK_COUNT_PER_TICK;
    uint32_t status;

    BOARD_EnableSCEvent(SC_EVENT_MASK(kSCEvent_SysCtr), false);
    /*
     * Cancle the SYSCTR and clear pending flag
     */
    sc_timer_cancel_sysctr_alarm((sc_ipc_t)IPC_MU);
    sc_irq_status((sc_ipc_t)IPC_MU, IPC_MU_RSRC, SC_IRQ_GROUP_SYSCTR, &status);

    expired = TSTMR_ReadTimeStamp(TSTMR_BASE) - s_lastTimeStamp;

    /* Remaining counter. */
    if (expired < timeoutCounter)
    {
        counter       = timeoutCounter - expired;
        completeTicks = timeoutTicks - (counter - 1) / countPerTick - 1;
        completeTicks += s_resumeTicks;             /* Adding recovery consumed ticks. */
        counter = (counter - 1) % countPerTick + 1; /* The SYSCTR and GPT counter frequency is same. */
    }
    else
    {
        /* The counter already exceeds expected ticks, it means wakeup takes too much time,
           and we have already lost some ticks.  Restart the SYSTICK. */
        expiredTicks  = (expired - timeoutCounter) / countPerTick;
        expiredTicks  = expiredTicks > s_resumeTicks ? s_resumeTicks : expiredTicks;
        completeTicks = timeoutTicks + expiredTicks;

        counter = SYSTICK_COUNT_PER_TICK;
    }

    /* Reconfig GPT as SYSTICK timer. */
    LPM_GptInit();
    GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, counter - 1U);
    GPT_StartTimer(SYSTICK_BASE);

    vTaskStepTick(completeTicks);
}

/* The systick interrupt handler. */
void SYSTICK_HANDLER(void)
{
    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);

    /* This is the first tick since the MCU left a low power mode the
     * compare value need to be reset to its default.
     */
    if (GPT_GetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1) != SYSTICK_COUNT_PER_TICK - 1)
    {
        /* Counter will be reset and cause minor accuracy loss */
        GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, SYSTICK_COUNT_PER_TICK - 1);
    }

    /* Call FreeRTOS tick handler. */
    xPortSysTickHandler();

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}
#endif /* FSL_RTOS_FREE_RTOS */

bool LPM_Init(void)
{
    asmc_power_state_t mode;

#ifdef FSL_RTOS_FREE_RTOS
    s_mutex = xSemaphoreCreateMutex();

    if (s_mutex == NULL)
    {
        return false;
    }

    s_listenerHead = s_listenerTail = NULL;
#endif
    mode = ASMC_GetPowerModeState(CM4__ASMC);
    switch (mode)
    {
        case kASMC_PowerStateRun:
            s_curMode = LPM_PowerModeRun;
            break;
        case kASMC_PowerStateVlpr:
            s_curMode = LPM_PowerModeVlpr;
            break;
        default:
            return false;
    }

    /*
     * MU IRQ must be enabled, otherwise the M4 cannot actually go into WAIT/STOP
     */
    BOARD_Enable_SCIRQ(true);

    return true;
}

void LPM_Deinit(void)
{
#ifdef FSL_RTOS_FREE_RTOS
    if (s_mutex != NULL)
    {
        vSemaphoreDelete(s_mutex);
        s_mutex = NULL;
    }
#endif
    BOARD_Enable_SCIRQ(false);
}

bool LPM_IsTargetModeValid(lpm_power_mode_t targetPowerMode, const char **pErrorMsg)
{
    bool modeValid = true;
    asmc_power_state_t curPowerState;
    const char *errorMsg = NULL;

    curPowerState = ASMC_GetPowerModeState(CM4__ASMC);
    /*
     * Check whether the mode change is allowed.
     *
     * 1. If current mode is RUN mode, the target mode must not be VLPW mode.
     * 2. If current mode is VLPR mode, the target mode must not be WAIT/STOP mode.
     * 3. If already in the target mode.
     */
    switch (curPowerState)
    {
        case kASMC_PowerStateRun:
            if (LPM_PowerModeVlpw == targetPowerMode)
            {
                errorMsg  = "Could not enter VLPW mode from RUN mode.\r\n";
                modeValid = false;
            }
            break;

        case kASMC_PowerStateVlpr:
            if ((LPM_PowerModeWait == targetPowerMode) || (LPM_PowerModeStop == targetPowerMode))
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
        /* Don't need to change power mode if current mode is already the target mode. */
        if (((LPM_PowerModeRun == targetPowerMode) && (kASMC_PowerStateRun == curPowerState)) ||
            ((LPM_PowerModeVlpr == targetPowerMode) && (kASMC_PowerStateVlpr == curPowerState)))
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
#ifdef FSL_RTOS_FREE_RTOS
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
#else
    s_curMode = mode;
    return true;
#endif
}

lpm_power_mode_t LPM_GetPowerMode(void)
{
    return s_curMode;
}

bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec)
{
    status_t status = kStatus_Success;
#ifdef FSL_RTOS_FREE_RTOS
    uint64_t counter = 0;
    uint32_t timeoutTicks;
#endif

    if (!LPM_IsTargetModeValid(s_curMode, NULL))
    {
        return false;
    }

#ifdef FSL_RTOS_FREE_RTOS
    timeoutTicks = LPM_EnterTicklessIdle(timeoutMilliSec, &counter);
#endif

    switch (s_curMode)
    {
        case LPM_PowerModeWait:
            status = ASMC_SetPowerModeWait(CM4__ASMC);
            break;
        case LPM_PowerModeStop:
            status = ASMC_SetPowerModeStop(CM4__ASMC, kASMC_PartialStop);
            break;
        case LPM_PowerModeVlpw:
            status = ASMC_SetPowerModeVlpw(CM4__ASMC);
            break;
        case LPM_PowerModeVlps:
            status = ASMC_SetPowerModeVlps(CM4__ASMC);
            break;
        case LPM_PowerModeLls:
        case LPM_PowerModeVlls:
            if (!LPM_Suspend())
            {
                status = kStatus_Fail;
            }
            break;
        default:
            break;
    }

    /* Power up IRQSTEER to allow CPU access IRQSTEER registers in case the IRQSTEER IRQ Handler execute after wake
     * up.*/
    RTN_ERR(sc_pm_set_resource_power_mode((sc_ipc_t)IPC_MU, IRQSTEER_RSRC, SC_PM_PW_MODE_ON));

#ifdef FSL_RTOS_FREE_RTOS
    if (timeoutTicks > 0)
    {
        LPM_ExitTicklessIdle(timeoutTicks, counter);
    }
#endif

    return status == kStatus_Success;
}

#ifdef FSL_RTOS_FREE_RTOS
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
#endif

/************ Internal public API start **************/
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
    uint32_t resumeAddr;
    s_psp    = psp;        /* Save PSP for resume context */
    s_lmpecr = *lmpecrReg; /* Save ECC/Parity config */

    s_suspendMem[0] = psp;
    s_suspendMem[2] = (uint32_t)NMI_Handler;
    s_suspendMem[3] = (uint32_t)HardFault_Handler;
    s_suspendMem[4] = (uint32_t)MemManage_Handler;
    s_suspendMem[5] = (uint32_t)BusFault_Handler;
    s_suspendMem[6] = (uint32_t)UsageFault_Handler;
    s_suspendMem[7] = 0; /* Reserved */

    resumeAddr = MEMORY_ConvertMemoryMapAddress((uint32_t)(&s_suspendMem), kMEMORY_Local2DMA);
    /* Setup Resume Entry. */
    sc_pm_set_cpu_resume_addr((sc_ipc_t)IPC_MU, CPU_RSRC, resumeAddr);

    switch (s_curMode)
    {
        case LPM_PowerModeLls:
            s_suspendMem[1] = (uint32_t)LPM_Resume;
            /*The M4 always boot from address 0x0 which mapped to start of TCML, to support boot from other address,SCFW
             * will copy the first 32 Bytes excption vector from M4 image to TCML. The suspend/resume of VLLS/LLS mode
             * uses this feature, so need backup the 32 Bytes in TCML before suspend and recover after resume.*/
            memcpy((uint32_t *)__BACKUP_REGION_START, (uint32_t *)FSL_MEM_M4_TCM_BEGIN, M4_LANDING_ZONE_SIZE);
            /* Clean System cache to make sure the data writen to memory. */
            L1CACHE_CleanSystemCache();
            ASMC_SetPowerModeLls(CM4__ASMC);
            break;
        case LPM_PowerModeVlls:
            s_suspendMem[1] = (uint32_t)LPM_ResumeWithBackup;
            /* The TCM is not retained in VLLS mode, backup TCM */
            *lmpecrReg = 0x0; /* Disable ECC/Parity, will be recovered after wakeup. */
            memcpy((uint32_t *)__BACKUP_REGION_START, (uint32_t *)FSL_MEM_M4_TCM_BEGIN,
                   (FSL_MEM_M4_TCM_END - FSL_MEM_M4_TCM_BEGIN + 1));
            L1CACHE_CleanSystemCache();
            ASMC_SetPowerModeVlls(CM4__ASMC);
            break;
        default:
            break;
    }
}

uint32_t LPM_SystemResume(bool resume)
{
    /* Recover ECC/Parity config */
    *lmpecrReg = s_lmpecr;

    /* Recover resume address */
    sc_pm_set_cpu_resume_addr((sc_ipc_t)IPC_MU, CPU_RSRC, 0ULL);

    ASMC_SetPowerModeProtection(CM4__ASMC, kASMC_AllowPowerModeAll);

    /*
     * In this demo, always enable the MU interrupt
     */
    MU_EnableInterrupts(IPC_MU, MU_SR_GIPn_MASK);
    return s_psp;
}
