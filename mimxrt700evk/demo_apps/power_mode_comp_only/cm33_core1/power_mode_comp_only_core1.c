/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_power.h"
#include "power_demo_config.h"
#include "fsl_lpuart.h"
#include "fsl_mu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_CORE_NAME "CPU1"
#define APP_WAIT_INPUT_TO_START \
    0U /* If non-zero, need set DEMO_POWER_CPU1_PRINT_ENABLE to 1 and need input to enter Deep sleep mode. */

#define APP_MU            MU1_MUB
#define APP_MU_IRQHandler MU1_B_IRQHandler

/* Enable the PMC interrupt for other domain entering DS. */
#define APP_USE_PMC_INT         (0U)
#define APP_PMC_INT_ENABLE_MASK PMC_INTRCTRL_DSCOMPIE_MASK
#define APP_PMC_INT_STATUS_MASK PMC_FLAGS_DSCOMPF_MASK

#define APP_USART_RX_ERROR (kLPUART_RxOverrunFlag | kLPUART_RxFifoUnderflowFlag)

/*!< Power down all unnecessary blocks and enable RBB during deep sleep. */
#define APP_DEEPSLEEP_SLEEPCFG    (0U)        /* SLEEPCON->SLEEPCFG */
#define APP_DEEPSLEEP_PDSLEEPCFG0 (0U)
#define APP_DSR_PDSLEEPCFG0       (0U)        /* PMC->PDSLEEPCFG0 */
#define APP_DEEPSLEEP_RAM_APD     0x3FFC0000U /* PMC->PDSLEEPCFG2, all keep powered */
#define APP_DEEPSLEEP_RAM_PPD     0x0U        /* PMC->PDSLEEPCFG3 */
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DEEPSLEEP_PDSLEEPCFG0, 0U, APP_DEEPSLEEP_RAM_APD, \
                         APP_DEEPSLEEP_RAM_PPD, 0U, 0U}))
#define APP_EXCLUDE_FROM_DSR                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DSR_PDSLEEPCFG0, 0U, APP_DEEPSLEEP_RAM_APD, \
                         APP_DEEPSLEEP_RAM_PPD, 0U, 0U}))
#define APP_EXCLUDE_FROM_DEEP_POWERDOWN      (((const uint32_t[]){0, 0, 0, 0, 0, 0, 0}))
#define APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN (((const uint32_t[]){0, 0, 0, 0, 0, 0, 0}))
#if (DEMO_POWER_CPU1_PRINT_ENABLE != 0U)
#define DEMO_LOG PRINTF
#else
#define DEMO_LOG(...)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_targetMode = 0U;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_NotifyBoot(void);
void BOARD_EnterDeepSleep(const uint32_t exclude_from_pd[7]);
void BOARD_RequestDPD(const uint32_t exclude_from_pd[7]);
void BOARD_RequestFDPD(const uint32_t exclude_from_pd[7]);
/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_NotifyBoot(void)
{
    RESET_ClearPeripheralReset(kMU1_RST_SHIFT_RSTn);
    MU_Init(MU1_MUB);
    MU_SetFlags(MU1_MUB, BOOT_FLAG);

    EnableDeepSleepIRQ(MU1_B_IRQn);
    MU_EnableInterrupts(APP_MU, kMU_Rx0FullInterruptEnable);
}

void BOARD_InitPowerConfig(void)
{
    /* Enable the used modules in sense side. */
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSEP_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_GATE_FRO2);
    POWER_DisablePD(kPDRUNCFG_PD_FRO2);
    POWER_DisablePD(kPDRUNCFG_SHUT_RAM1_CLK);

    PMC1->PDRUNCFG2 &= ~(0x3FFC0000U); /* Power up all the SRAM partitions in Sense domain. */
    PMC1->PDRUNCFG3 &= ~(0x3FFC0000U);

    /* Disable unused clock. */
    CLOCK_DisableClock(kCLOCK_Glikey1);
    CLOCK_DisableClock(kCLOCK_Glikey2);
    CLOCK_DisableClock(kCLOCK_Glikey4);
    CLOCK_DisableClock(kCLOCK_Glikey5);
    CLOCK_DisableClock(kCLOCK_SenseAccessRamArbiter0);
    CLOCK_DisableClock(kCLOCK_MediaAccessRamArbiter1);
    CLOCK_AttachClk(kNONE_to_SYSTICK);
    CLOCK_AttachClk(kNONE_to_MICFIL0);

#if ((DEMO_POWER_CPU1_PRINT_ENABLE == 0U))
    CLOCK_AttachClk(kNONE_to_FLEXCOMM19);
#endif

    /* Disable unused modules. */
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM0_CLK);
    POWER_EnablePD(kPDRUNCFG_SHUT_COMNN_MAINCLK);
    POWER_EnablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_EnablePD(kPDRUNCFG_PD_SYSXTAL);
    POWER_EnablePD(kPDRUNCFG_PD_PLLANA);
    POWER_EnablePD(kPDRUNCFG_PD_PLLLDO);
    POWER_EnablePD(kPDRUNCFG_PD_AUDPLLANA);
    POWER_EnablePD(kPDRUNCFG_PD_AUDPLLLDO);
    POWER_EnablePD(kPDRUNCFG_PD_ADC0);
    POWER_EnablePD(kPDRUNCFG_APD_SRAM1);

    POWER_EnablePD(kPDRUNCFG_PPD_OCOTP);
    POWER_EnablePD(kPDRUNCFG_APD_OCOTP);

    POWER_EnablePD(kPDRUNCFG_SHUT_RAM0_CLK); /* Compute access RAM arbiter1 clock. */
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM1_CLK); /* Compute access RAM arbiter1 clock. */

    /* Request the domains out of sense into RBB mode. */
    POWER_EnableRunAFBB(kPower_BodyBiasVdd1);
    POWER_EnableRunNBB(kPower_BodyBiasVdd1Sram);
    POWER_EnableRunRBB(kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd2Sram);
    POWER_EnableSleepRBB(kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd2Sram | kPower_BodyBiasVdd1 |
                         kPower_BodyBiasVdd1Sram);
    POWER_ApplyPD();

    SYSCON3->SENSE_AUTOGATE_EN = 0x3U;

    const power_regulator_voltage_t ldo = {
        .LDO.vsel0 = 630000,  /* 630mv, 0.45 V + 12.5 mV * x */
        .LDO.vsel1 = 800000,  /* 800mv*/
        .LDO.vsel2 = 900000,  /* 900mv */
        .LDO.vsel3 = 1000000, /* 1000mv */
    };

    const power_lvd_voltage_t lvd = {
        .VDD12.lvl0 = 500000, /* 500mv */
        .VDD12.lvl1 = 700000, /* 700mv */
        .VDD12.lvl2 = 800000, /* 800mv */
        .VDD12.lvl3 = 900000, /* 900mv */
    };
    POWER_ConfigRegulatorSetpoints(kRegulator_Vdd1LDO, &ldo, &lvd);

    POWER_SelectRunSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_SelectRunSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_ApplyPD();

    CLOCK_DisableClock(kCLOCK_Rtc);
    CLOCK_DisableClock(kCLOCK_Syscon1);
    CLOCK_DisableClock(kCLOCK_Syscon2);
    CLOCK_DisableClock(kCLOCK_Syscon3);
    CLOCK_DisableClock(kCLOCK_Syscon4);
    CLOCK_DisableClock(kCLOCK_Iopctl1);

#if (DEMO_POWER_ENABLE_DEBUG == 0U)
    CLOCK_DisableClock(kCLOCK_Dbg);
#endif
}


void BOARD_EnterDeepSleep(const uint32_t exclude_from_pd[7])
{
    POWER_EnableSleepRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd1Sram |
                         kPower_BodyBiasVdd2Sram);
    /* Change to a lower frequency to safly decrease the VDD1 voltage when CPU1 enter low power mode but CPU0 active and
     * requires sense shared main clock. */
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
    CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv6OutEn); /* Need Keep DIV6. */

    POWER_EnterDeepSleep(exclude_from_pd);
    CLOCK_EnableFroClkOutput(FRO2, kCLOCK_FroDiv1OutEn | kCLOCK_FroDiv3OutEn | kCLOCK_FroDiv6OutEn);
    CLOCK_AttachClk(kFRO2_DIV1_to_SENSE_MAIN);
    CLOCK_AttachClk(kFRO2_DIV3_to_SENSE_BASE);
}

void BOARD_RequestDPD(const uint32_t exclude_from_pd[7])
{

    POWER_EnableSleepRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd1Sram |
                         kPower_BodyBiasVdd2Sram);
     /* Need keep sense shared main clock in case CPU0 enters power down mode after CPU1. */
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
    CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
    POWER_RequestDeepPowerDown(exclude_from_pd);
}

void BOARD_RequestFDPD(const uint32_t exclude_from_pd[7])
{
    POWER_EnableSleepRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd2 | kPower_BodyBiasVddn | kPower_BodyBiasVdd1Sram |
                         kPower_BodyBiasVdd2Sram);
    CLOCK_AttachClk(kLPOSC_to_SENSE_BASE);
    CLOCK_AttachClk(kSENSE_BASE_to_SENSE_MAIN);
    POWER_RequestFullDeepPowerDown(exclude_from_pd);
}
#if (defined(APP_USE_PMC_INT) && (APP_USE_PMC_INT))
void PMC_IRQHandler(void)
{
    if ((POWER_GetEventFlags() & APP_PMC_INT_STATUS_MASK) == APP_PMC_INT_STATUS_MASK)
    {
        POWER_ClearEventFlags(APP_PMC_INT_STATUS_MASK);
        DEMO_LOG("The other CPU enterred Deep Sleep\r\n");
    }
}
#endif
void APP_MU_IRQHandler(void)
{
    if ((MU_GetStatusFlags(APP_MU) & kMU_Rx0FullFlag) == kMU_Rx0FullFlag)
    {
        g_targetMode = MU_ReceiveMsgNonBlocking(APP_MU, APP_MU_REG);
    }

    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
#if (DEMO_POWER_CPU1_PRINT_ENABLE != 0U)
    uint32_t freq = 0U;
#endif

    /* Init board hardware. */
    POWER_DisablePD(kPDRUNCFG_PD_FRO2); /* Sense uses FRO2. */
#if (DEMO_POWER_CPU1_PRINT_ENABLE != 0U)
    BOARD_InitPins();
#endif
    BOARD_BootClockRUN();
#if (DEMO_POWER_CPU1_PRINT_ENABLE != 0U)
    BOARD_InitDebugConsole();
#endif

    BOARD_InitPowerConfig();

    /* Determine the power mode before bring up. */
    if ((POWER_GetEventFlags() & PMC_FLAGS_DEEPPDF_MASK) != 0)
    {
        DEMO_LOG("Board wake up from deep or full deep power down mode.\r\n");
        POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
    }
    POWER_ClearEventFlags(0xFFFFFFFF);

    BOARD_NotifyBoot(); /* Set boot flag. */

#if APP_WAIT_INPUT_TO_START
    DEMO_LOG("Input any key to start the demo\r\n");
    GETCHAR();
#endif

    while (1)
    {
#if (DEMO_POWER_CPU1_PRINT_ENABLE != 0U)
        freq = CLOCK_GetCoreSysClkFreq();

        DEMO_LOG("\r\n####################  Power Mode Demo - %s ####################\r\n", APP_CORE_NAME);
        DEMO_LOG("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        DEMO_LOG("    Core Clock: %dHz \r\n", freq);
#endif

        switch (g_targetMode)
        {
            case DEMO_EVENT_ENTER_DPD:
                DEMO_LOG("Entering DPD...\r\n");
                BOARD_RequestDPD(APP_EXCLUDE_FROM_DEEP_POWERDOWN);
                break;

            case DEMO_EVENT_ENTER_FDPD:
                DEMO_LOG("Entering FDPD...\r\n");
                BOARD_RequestFDPD(APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN);
                break;

            default:
                DEMO_LOG("Entering Deep sleep...\r\n");
                g_targetMode = (uint32_t)kPower_DeepSleep;
                BOARD_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
                break;
        }

        DEMO_LOG("Wakeup.\r\n");
    }
}
