/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_mu.h"
#include "fsl_msmc.h"
#include "fsl_lptmr.h"
#include "fsl_llwu.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_spm.h"
#include "fsl_lpspi.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_SMC SMC0

#define APP_MU                        MUA
#define APP_CORE1_BOOT_MODE           kMU_CoreBootFromDflashBase
#define APP_MU_IRQHandler             MUA_IRQHandler
#define APP_MU_XFER_CMD_CHANNEL_INDEX 0U
#define APP_MU_SHAKE_HAND_VALUE       0xAAAAAAAA

#define APP_LLWU LLWU0
#define APP_LLWU_LPTMR_IDX                                                               \
    0U /* According to Table 4-11. Wakeup Sources for LLWU0 inputs, LLWU_M4DR - LPTMR0 \ \
          Trigger*/
#define APP_LLWU_IRQHandler LLWU0_IRQHandler

#define APP_LPTMR            LPTMR0
#define APP_LPTMR_IRQn       LPTMR0_IRQn
#define APP_LPTMR_IRQHandler LPTMR0_IRQHandler

#define FLASH_SPI_MASTER          LPSPI1
#define FLASH_SPI_MASTER_CLK_FREQ CLOCK_GetIpFreq(kCLOCK_Lpspi1)

#define DPCommand 0xB9

/* Power mode definition used in application. */
typedef enum _app_power_mode
{
    kAPP_PowerModeMin = 'A' - 1,
    kAPP_PowerModeRun,   /* Normal RUN mode */
    kAPP_PowerModeWait,  /* WAIT mode. */
    kAPP_PowerModeStop,  /* STOP mode. */
    kAPP_PowerModeVlpr,  /* VLPR mode. */
    kAPP_PowerModeVlpw,  /* VLPW mode. */
    kAPP_PowerModeVlps,  /* VLPS mode. */
    kAPP_PowerModeLls,   /* LLS mode. */
    kAPP_PowerModeVlls2, /* VLLS2 mode. */
    kAPP_PowerModeVlls0, /* VLLS0 mode. */
    kAPP_PowerModeHsrun, /* HSRUN mode. */
    kAPP_PowerModeMax
} app_power_mode_t;

/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/*******************************************************************************
 * Variables
 ******************************************************************************/
app_power_mode_t targetPowerMode;
smc_power_state_t curPowerState;
static uint8_t g_WakeupTimeoutSecond; /* Wakeup timeout. (Unit: Second) */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static bool APP_CheckPowerModeCanTransfer(smc_power_state_t curPowerState, app_power_mode_t targetPowerMode);
static bool APP_CheckPowerModeNeedWakeup(app_power_mode_t targetPowerMode);
static void APP_ShowPowerMode(smc_power_state_t powerMode);
static void LPTMR_Configuration(void);
static uint8_t APP_GetWakeupTimeout(void);
void APP_PowerModeSwitch(smc_power_state_t curPowerState, app_power_mode_t targetPowerMode);

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_InitDebugConsoleWithSirc(void)
{
    scg_sirc_config_t scgSircConfigStruct = {
        .enableMode =
            kSCG_SircEnable | kSCG_SircEnableInLowPower, /* Enable SIRC clock, Enable SIRC in low power mode */
        .div1  = kSCG_AsyncClkDisable,                   /* Slow IRC Clock Divider 1: Clock output is disabled */
        .div2  = kSCG_AsyncClkDivBy1,                    /* Slow IRC Clock Divider 2: Clock output is disabled */
        .div3  = kSCG_AsyncClkDivBy1,                    /* Slow IRC Clock Divider 3: divided by 1 */
        .range = kSCG_SircRangeHigh,                     /* Slow IRC high range clock (8 MHz) */
    };
    uint32_t uartClkSrcFreq;

    /* Init SIRC with DIV2 enabled for LPUART in low power mode. */
    CLOCK_InitSirc(&scgSircConfigStruct);

    /* Set PCC LPUART0 selection */
    CLOCK_SetIpSrc(kCLOCK_Lpuart0, kCLOCK_IpSrcSircAsync);
    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

static void SetExternalFlashDPMode(void)
{
    uint32_t srcClock_Hz;
    lpspi_master_config_t masterConfig = {0};

    CLOCK_SetIpSrc(kCLOCK_Lpspi1, kCLOCK_IpSrcFircAsync);
    CLOCK_EnableClock(kCLOCK_Lpspi1);

    /*LPSPI config*/
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    srcClock_Hz = FLASH_SPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(FLASH_SPI_MASTER, &masterConfig, srcClock_Hz);

    /*Send command of entering deep power-down*/
    LPSPI_WriteData(FLASH_SPI_MASTER, DPCommand);

    CLOCK_DisableClock(kCLOCK_Lpspi1);
}

/*!
 * @brief Application entry.
 */
int main()
{
    uint32_t power_mode_cmd;

    /* Init board hardware.*/
    /* Clear the IO lock when reset from VLLSx low power mode. */
    if (SPM_GetPeriphIOIsolationFlag(SPM))
    {
        SPM_ClearPeriphIOIsolationFlag(SPM);
    }
    BOARD_InitPins_Core0();
    BOARD_InitBootClocks();
    /* BOARD_InitDebugConsole(); */
    BOARD_InitDebugConsoleWithSirc();
    /* Set external flash enter deep power-down */
    SetExternalFlashDPMode();
    LPTMR_Configuration();

    /* Unlock the Very Low Power protection. */
    SMC_SetPowerModeProtection(APP_SMC, kSMC_AllowPowerModeAll);

    PRINTF("\r\nPower Mode Switch demo - dual core, core 0.\r\n");

    /* For core 1.
     * The whole system's lowest power mode depends on the highest power mode of both core.
     * To meet the lowest system power mode, here we would set the other core into the lowest power mode.
     */
    MU_Init(APP_MU);
    PRINTF("Boot up another core, core 1 ...\r\n");
    /* Wake up another core, core 1. */
    MU_BootCoreB(APP_MU, APP_CORE1_BOOT_MODE);

    /* Wait Core 1 is Boot Up */
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
    {
    }

    PRINTF("Core 1 is booted up now.\r\n");

    /* Shake hands with the other core.
     *  The other core would fall into the lowest power mode after send out the ACK.
     */
    MU_SendMsg(APP_MU, APP_MU_XFER_CMD_CHANNEL_INDEX, APP_MU_SHAKE_HAND_VALUE);
    if (APP_MU_SHAKE_HAND_VALUE == MU_ReceiveMsg(APP_MU, APP_MU_XFER_CMD_CHANNEL_INDEX))
    {
        PRINTF("Shake hands done.\r\n");
    }
    else
    {
        PRINTF("Bad response from the other core.\r\n");
    }

    while (1)
    {
        curPowerState = SMC_GetPowerModeState(APP_SMC);
        APP_ShowPowerMode(curPowerState);
        do
        {
            PRINTF("--------------------------------\r\n");

            PRINTF(" - %c: kAPP_PowerModeRun\r\n", kAPP_PowerModeRun);
            PRINTF(" - %c: kAPP_PowerModeWait\r\n", kAPP_PowerModeWait);
            PRINTF(" - %c: kAPP_PowerModeStop\r\n", kAPP_PowerModeStop);
            PRINTF(" - %c: kAPP_PowerModeVlpr\r\n", kAPP_PowerModeVlpr);
            PRINTF(" - %c: kAPP_PowerModeVlpw\r\n", kAPP_PowerModeVlpw);
            PRINTF(" - %c: kAPP_PowerModeVlps\r\n", kAPP_PowerModeVlps);
            PRINTF(" - %c: kAPP_PowerModeLls\r\n", kAPP_PowerModeLls);
            PRINTF(" - %c: kAPP_PowerModeVlls2\r\n", kAPP_PowerModeVlls2);
            PRINTF(" - %c: kAPP_PowerModeVlls0\r\n", kAPP_PowerModeVlls0);
            PRINTF(" - %c: kAPP_PowerModeHsrun\r\n", kAPP_PowerModeHsrun);

            PRINTF("Input %c to %c: \r\n", kAPP_PowerModeMin + 1U, kAPP_PowerModeMax - 1U);

            power_mode_cmd = GETCHAR();
            PRINTF("%c\r\n", power_mode_cmd);
        } while ((power_mode_cmd <= kAPP_PowerModeMin) || (power_mode_cmd >= kAPP_PowerModeMax));
        targetPowerMode = (app_power_mode_t)power_mode_cmd;

        /* Check if the power mode switch is not available or not necessary. */
        if (!APP_CheckPowerModeCanTransfer(curPowerState, targetPowerMode))
        {
            continue;
        }

        PRINTF("Available transfer.\r\n");

        /* Check if a wakeup source is needed to return the run mode. */
        if (APP_CheckPowerModeNeedWakeup(targetPowerMode))
        {
            PRINTF("Prepare the wakeup source...\r\n");

            /* Setup LLWU if necessary. */
            if ((kAPP_PowerModeWait != targetPowerMode) && (kAPP_PowerModeVlpw != targetPowerMode) &&
                (kAPP_PowerModeVlps != targetPowerMode) && (kAPP_PowerModeStop != targetPowerMode))
            {
                PRINTF("LLWU_EnableInternalModuleInterruptWakup()\r\n");
                LLWU_EnableInternalModuleInterruptWakup(APP_LLWU, APP_LLWU_LPTMR_IDX, true);
            } /* Else with NVIC or AWIC as default. */

            /* Setup the LPTMR. */
            g_WakeupTimeoutSecond = APP_GetWakeupTimeout();
            LPTMR_SetTimerPeriod(APP_LPTMR, (1000U * g_WakeupTimeoutSecond) - 1U);
            LPTMR_StartTimer(APP_LPTMR);
        }

        if (kAPP_PowerModeVlls0 == targetPowerMode)
        {
            PRINTF("Reset to wakeup the chip.\r\n");
        }

        /* Switch the power mode. */
        APP_PowerModeSwitch(curPowerState, targetPowerMode);
    }
}

/*
 * Check whether could switch to target power mode from current mode.
 * Return true if could switch, return false if could not switch.
 */
static bool APP_CheckPowerModeCanTransfer(smc_power_state_t curPowerState, app_power_mode_t targetPowerMode)
{
    bool modeValid = true;

    /*
     * Check wether the mode change is allowed.
     *
     * 1. If current mode is HSRUN mode, the target mode must be RUN mode.
     * 2. If current mode is RUN mode, the target mode must not be VLPW mode.
     * 3. If current mode is VLPR mode, the target mode must not be HSRUN/WAIT/STOP mode.
     * 4. If already in the target mode.
     */
    switch (curPowerState)
    {
        case kSMC_PowerStateHsrun:
            if (kAPP_PowerModeRun != targetPowerMode)
            {
                PRINTF("Current mode is HSRUN, please choose RUN mode as target mode.\r\n");
                modeValid = false;
            }
            break;

        case kSMC_PowerStateRun:
            if (kAPP_PowerModeVlpw == targetPowerMode)
            {
                PRINTF("Could not enter VLPW mode from RUN mode.\r\n");
                modeValid = false;
            }
            break;

        case kSMC_PowerStateVlpr:
            if ((kAPP_PowerModeWait == targetPowerMode) || (kAPP_PowerModeHsrun == targetPowerMode) ||
                (kAPP_PowerModeStop == targetPowerMode))
            {
                PRINTF("Could not enter HSRUN/STOP/WAIT modes from VLPR mode.\r\n");
                modeValid = false;
            }
            break;
        default:
            PRINTF("Wrong power state.\r\n");
            modeValid = false;
            break;
    }

    if (!modeValid)
    {
        return false;
    }

    /* Don't need to change power mode if current mode is already the target mode. */
    if (((kAPP_PowerModeRun == targetPowerMode) && (kSMC_PowerStateRun == curPowerState)) ||
        ((kAPP_PowerModeHsrun == targetPowerMode) && (kSMC_PowerStateHsrun == curPowerState)) ||
        ((kAPP_PowerModeVlpr == targetPowerMode) && (kSMC_PowerStateVlpr == curPowerState)))
    {
        PRINTF("Already in the target power mode.\r\n");
        return false;
    }

    return true;
}

static bool APP_CheckPowerModeNeedWakeup(app_power_mode_t targetPowerMode)
{
    bool needSetWakeup;

    /* If target mode is RUN/VLPR/HSRUN, don't need to set wakeup source. */
    if ((kAPP_PowerModeRun == targetPowerMode) || (kAPP_PowerModeHsrun == targetPowerMode) ||
        (kAPP_PowerModeVlpr == targetPowerMode))
    {
        needSetWakeup = false;
    }
    /* If target mode is the lowest power mode (kAPP_PowerModeVlls0), LPTMR could not wakeup it. */
    else if (kAPP_PowerModeVlls0 == targetPowerMode)
    {
        needSetWakeup = false;
    }
    else
    {
        needSetWakeup = true;
    }

    return needSetWakeup;
}

static void APP_ShowPowerMode(smc_power_state_t powerMode)
{
    PRINTF("\r\n**************************************************************************\r\n");
    switch (powerMode)
    {
        case kSMC_PowerStateRun:
            PRINTF("    Power mode: RUN\r\n");
            break;
        case kSMC_PowerStateVlpr:
            PRINTF("    Power mode: VLPR\r\n");
            break;
        case kSMC_PowerStateHsrun:
            PRINTF("    Power mode: HSRUN\r\n");
            break;
        default:
            PRINTF("    Power mode wrong\r\n");
            break;
    }
}

static uint8_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;

    while (1)
    {
        PRINTF("Select the wake up timeout in seconds...\r\n");
        PRINTF("The allowed range is 1s ~ 9s.\r\n");
        PRINTF("Eg. enter 5 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        timeout = GETCHAR();
        PRINTF("%c\r\n", timeout);
        if ((timeout > '0') && (timeout <= '9'))
        {
            return timeout - '0';
        }
        PRINTF("Wrong value!\r\n");
    }
}

static void LPTMR_Configuration(void)
{
    lptmr_config_t lptmrConfig;

    /* Setup LPTMR. */
    LPTMR_GetDefaultConfig(&lptmrConfig); /* LPO as clock source. */
    LPTMR_Init(APP_LPTMR, &lptmrConfig);

    /* Enable the interrupt. */
    LPTMR_EnableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable);
    EnableIRQ(APP_LPTMR_IRQn);
}

void APP_LPTMR_IRQHandler(void)
{
    LPTMR_ClearStatusFlags(APP_LPTMR, kLPTMR_TimerCompareFlag);
    LPTMR_StopTimer(APP_LPTMR);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

/* Even with the defination, this ISR would be executed only when wakup from LLS/VLLSx mode. */
void APP_LLWU_IRQHandler(void)
{
    /* LPTMR_DisableInterrupts(APP_LPTMR, kLPTMR_TimerInterruptEnable); */
    LPTMR_ClearStatusFlags(APP_LPTMR, kLPTMR_TimerCompareFlag);
    LPTMR_StopTimer(APP_LPTMR);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

/*
 * Power mode switch.
 */
void APP_PowerModeSwitch(smc_power_state_t curPowerState, app_power_mode_t targetPowerMode)
{
    switch (targetPowerMode)
    {
        case kAPP_PowerModeRun:

            /* Power mode change. */
            SMC_SetPowerModeRun(APP_SMC);
            while (kSMC_PowerStateRun != SMC_GetPowerModeState(APP_SMC))
            {
            }
            break;
        case kAPP_PowerModeWait:
            SMC_SetPowerModeWait(APP_SMC);
            break;

        case kAPP_PowerModeStop:
            SMC_SetPowerModeStop(APP_SMC, kSMC_PartialStop);
            break;
        case kAPP_PowerModeVlpr:
            SMC_SetPowerModeVlpr(APP_SMC);
            while (kSMC_PowerStateVlpr != SMC_GetPowerModeState(APP_SMC))
            {
            }
            break;
        case kAPP_PowerModeVlpw:
            SMC_SetPowerModeVlpw(APP_SMC);
            break;
        case kAPP_PowerModeVlps:
            SMC_SetPowerModeVlps(APP_SMC);
            break;
        case kAPP_PowerModeLls:
            SMC_SetPowerModeLls(APP_SMC);
            break;
        case kAPP_PowerModeVlls0:
            SMC_SetPowerModeVlls0(APP_SMC);
            break;
        case kAPP_PowerModeVlls2:
            SMC_SetPowerModeVlls2(APP_SMC);
            break;
        case kAPP_PowerModeHsrun:
            SMC_SetPowerModeHsrun(APP_SMC);
            while (kSMC_PowerStateHsrun != SMC_GetPowerModeState(APP_SMC))
            {
            }
            break;
        default:
            PRINTF("Wrong value");
            break;
    }
}
