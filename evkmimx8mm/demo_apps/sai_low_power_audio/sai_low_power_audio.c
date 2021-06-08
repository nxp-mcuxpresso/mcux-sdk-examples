/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "app_srtm.h"
#include "fsl_gpc.h"
#include "sai_low_power_audio.h"
#include "lpm.h"
#include "fsl_debug_console.h"
#include "fsl_rdc.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RDC_DISABLE_A53_ACCESS 0xFC
#define RDC_DISABLE_M4_ACCESS  0xF3
static LPM_POWER_STATUS_M4 m4_lpm_state = LPM_M4_STATE_RUN;
/* Using SRC_GPR9 register to sync the tasks status with A core */
#define ServiceFlagAddr SRC->GPR9
/* The flags,ServiceBusy and ServiceIdle, shows if the service task is running or not.
 * If the task is runing, A core should not put DDR in self-refresh mode after A core enters supsend.
 */
#define ServiceBusy (0x5555U)
#define ServiceIdle (0x0U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern volatile app_srtm_state_t srtmState;

/*******************************************************************************
 * Code
 ******************************************************************************/
void Peripheral_RdcSetting(void)
{
    rdc_domain_assignment_t assignment = {0};
    rdc_periph_access_config_t periphConfig;

    assignment.domainId = BOARD_DOMAIN_ID;
    RDC_SetMasterDomainAssignment(RDC, kRDC_Master_SDMA3_PERIPH, &assignment);
    RDC_SetMasterDomainAssignment(RDC, kRDC_Master_SDMA3_BURST, &assignment);
    RDC_SetMasterDomainAssignment(RDC, kRDC_Master_SDMA3_SPBA2, &assignment);

    RDC_GetDefaultPeriphAccessConfig(&periphConfig);
    /* Do not allow the A53 domain(domain0) to access the following peripherals. */
    periphConfig.policy = RDC_DISABLE_A53_ACCESS;
    periphConfig.periph = kRDC_Periph_SAI1;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    periphConfig.periph = kRDC_Periph_UART4;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    periphConfig.periph = kRDC_Periph_I2C3;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    periphConfig.periph = kRDC_Periph_GPT1;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    /* For SAI3, both kRDC_Periph_SAI3_ACCESS and kRDC_Periph_SAI3_LPM registers need set.*/
    periphConfig.periph = kRDC_Periph_SAI3_ACCESS;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    periphConfig.periph = kRDC_Periph_SAI3_LPM;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
    /* Remove the SAI2 power control in RDC from M4 to avoid A53 hang when it touches SAI2 under M core enters STOP
     * mode.*/
    periphConfig.policy = RDC_DISABLE_M4_ACCESS;
    periphConfig.periph = kRDC_Periph_SAI2_LPM;
    RDC_SetPeriphAccessConfig(RDC, &periphConfig);
}
void LPM_MCORE_ChangeM4Clock(LPM_M4_CLOCK_SPEED target)
{
    /* Change CCM Root to change M4 clock*/
    switch (target)
    {
        case LPM_M4_LOW_FREQ:
            if (CLOCK_GetRootMux(kCLOCK_RootM4) != kCLOCK_M4RootmuxOsc24M)
            {
                CLOCK_SetRootMux(kCLOCK_RootM4, kCLOCK_M4RootmuxOsc24M);
                CLOCK_SetRootDivider(kCLOCK_RootM4, 1U, 1U);
            }
            break;
        case LPM_M4_HIGH_FREQ:
            if (CLOCK_GetRootMux(kCLOCK_RootM4) != kCLOCK_M4RootmuxSysPll1)
            {
                CLOCK_SetRootDivider(kCLOCK_RootM4, 1U, 2U);
                CLOCK_SetRootMux(kCLOCK_RootM4, kCLOCK_M4RootmuxSysPll1); /* switch cortex-m4 to SYSTEM PLL1 */
            }
            break;
        default:
            break;
    }
}

void LPM_MCORE_SetPowerStatus(GPC_Type *base, LPM_POWER_STATUS_M4 targetPowerMode)
{
    gpc_lpm_config_t config;
    config.enCpuClk              = false;
    config.enFastWakeUp          = false;
    config.enDsmMask             = false;
    config.enWfiMask             = false;
    config.enVirtualPGCPowerdown = true;
    config.enVirtualPGCPowerup   = true;
    switch (targetPowerMode)
    {
        case LPM_M4_STATE_RUN:
            GPC->LPCR_M4 = GPC->LPCR_M4 & (~GPC_LPCR_M4_LPM0_MASK);
            break;
        case LPM_M4_STATE_WAIT:
            GPC_EnterWaitMode(GPC, &config);
            break;
        case LPM_M4_STATE_STOP:
            GPC_EnterStopMode(GPC, &config);
            break;
        default:
            break;
    }
}
void PreSleepProcessing(void)
{
    APP_SRTM_Suspend();
    DbgConsole_Deinit();
}

void PostSleepProcessing(void)
{
    APP_SRTM_Resume();
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE,
                    BOARD_DEBUG_UART_CLK_FREQ);
}

void ShowMCoreStatus(void)
{
    if (m4_lpm_state == LPM_M4_STATE_STOP)
    {
        PRINTF("\r\nNo audio playback, M core enters STOP mode!\r\n");
    }
    else if (m4_lpm_state == LPM_M4_STATE_RUN)
    {
        PRINTF("\r\nPlayback is running, M core enters RUN mode!\r\n");
    }
    else
    {
        ; /* For MISRA C-2012 rule 15.7. */
    }
}

void UpdateTargetPowerStatus(void)
{
    /*
     * The m4_lpm_state merely indicates what the power state the M core finally should be.
     * In this demo, if there is no audio playback, M core will be set to STOP mode finally.
     */
    LPM_POWER_STATUS_M4 m4_target_lpm;

    if (APP_SRTM_ServiceIdle())
    {
        m4_target_lpm = LPM_M4_STATE_STOP;
    }
    else
    {
        m4_target_lpm = LPM_M4_STATE_RUN;
    }

    if (m4_target_lpm != m4_lpm_state)
    {
        m4_lpm_state = m4_target_lpm;
        ShowMCoreStatus();
    }
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    uint32_t irqMask;
    uint64_t counter = 0;
    uint32_t timeoutTicks;
    uint32_t timeoutMilliSec = (uint32_t)((uint64_t)1000 * xExpectedIdleTime / configTICK_RATE_HZ);

    irqMask = DisableGlobalIRQ();

    UpdateTargetPowerStatus();

    /* Only when no context switch is pending and no task is waiting for the scheduler
     * to be unsuspended then enter low power entry.
     */
    if (eTaskConfirmSleepModeStatus() != eAbortSleep)
    {
        timeoutTicks = LPM_EnterTicklessIdle(timeoutMilliSec, &counter);
        if (timeoutTicks)
        {
            if (APP_SRTM_ServiceIdle() && LPM_AllowSleep())
            {
                LPM_MCORE_ChangeM4Clock(LPM_M4_LOW_FREQ);
                LPM_MCORE_SetPowerStatus(BOARD_GPC_BASEADDR, LPM_M4_STATE_STOP);
                PreSleepProcessing();
                ServiceFlagAddr = ServiceIdle;
                __DSB();
                __ISB();
                __WFI();
                ServiceFlagAddr = ServiceBusy;
                PostSleepProcessing();
                LPM_MCORE_ChangeM4Clock(LPM_M4_HIGH_FREQ);
                LPM_MCORE_SetPowerStatus(BOARD_GPC_BASEADDR, LPM_M4_STATE_RUN);
            }
            else
            {
                __DSB();
                __ISB();
                __WFI();
            }
        }
        LPM_ExitTicklessIdle(timeoutTicks, counter);
    }

    EnableGlobalIRQ(irqMask);
}
void MainTask(void *pvParameters)
{
    /* Treat M core as busy status by default.*/
    ServiceFlagAddr = ServiceBusy;

    /*
     * Wait For A core Side Become Ready
     */
    PRINTF("********************************\r\n");
    PRINTF(" Wait the Linux kernel boot up to create the link between M core and A core.\r\n");
    PRINTF("\r\n");
    PRINTF("********************************\r\n");
    while (srtmState != APP_SRTM_StateLinkedUp)
        ;
    PRINTF("The rpmsg channel between M core and A core created!\r\n");
    PRINTF("********************************\r\n");
    PRINTF("\r\n");

    /* Configure GPC */
    GPC_Init(BOARD_GPC_BASEADDR, APP_PowerUpSlot, APP_PowerDnSlot);
    GPC_EnableIRQ(BOARD_GPC_BASEADDR, BOARD_MU_IRQ_NUM);
    GPC_EnableIRQ(BOARD_GPC_BASEADDR, SYSTICK_IRQn);
    GPC_EnableIRQ(BOARD_GPC_BASEADDR, I2C3_IRQn);
    while (true)
    {
        /* Use App task logic to replace vTaskDelay */
        PRINTF("\r\nTask %s is working now.\r\n", (char *)pvParameters);
        vTaskDelay(portMAX_DELAY);
    }
}
/*!
 * @brief Main function
 */
int main(void)
{
    char *taskID = "A";
    uint32_t i = 0;

    BOARD_RdcInit();
    Peripheral_RdcSetting();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMemory();
    /*
     * In order to wakeup M4 from LPM, all PLLCTRLs need to be set to "NeededRun"
     */
    for (i = 0; i != 39; i++)
    {
        CCM->PLL_CTRL[i].PLL_CTRL = kCLOCK_ClockNeededRun;
    }
    CLOCK_SetRootMux(kCLOCK_RootSai1, kCLOCK_SaiRootmuxAudioPll1); /* Set SAI source to Audio PLL1 393216000HZ */
    CLOCK_SetRootDivider(kCLOCK_RootSai1, 1U, 8U);                 /* Set root clock to 393216000HZ / 8 = 49152000HZ */
    CLOCK_SetRootMux(kCLOCK_RootSai3, kCLOCK_SaiRootmuxAudioPll1); /* Set SAI source to Audio PLL1 393215996HZ */
    CLOCK_SetRootDivider(kCLOCK_RootSai3, 1U, 16U);                /* Set root clock to 393216000HZ / 16 = 27576000HZ */
    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */
    CLOCK_SetRootMux(kCLOCK_RootGpt1, kCLOCK_GptRootmuxOsc24M);      /* Set GPT source to Osc24 MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootGpt1, 1U, 1U);
#if APP_SRTM_CODEC_AK4497_USED
    APP_SRTM_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
#endif
    PRINTF("\r\n####################  LOW POWER AUDIO TASK ####################\n\r\n");
    PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);

    APP_SRTM_Init();

    if (xTaskCreate(MainTask, "Main Task", 256U, (void *)taskID, tskIDLE_PRIORITY + 1U, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
void vApplicationMallocFailedHook(void)
{
    PRINTF("Malloc Failed!!!\r\n");
}
