/*
 * Copyright 2024 NXP
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
#include "pmic_support.h"
#include "power_demo_config.h"
#include "fsl_lpuart.h"
#include "fsl_gpio.h"
#include "fsl_irtc.h"
#include "fsl_iopctl.h"

#include "fsl_clock.h"
#include "fsl_pca9422.h"
#include "core1_support.h"
#include "fsl_cache.h"
#include "fsl_mu.h"
#include "dsp_support.h"
#include "fsl_dsp.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_NORMAL_RUN_VOLT 900000U /* The VDD2 voltage during normal run. */
#define DEMO_LOW_POWER_RUN_VOLT                                                                            \
    700000U                          /* The VDD2 voltage during low power run. Used for CPU0 DS while CPU1 \
                                        active.*/
#ifndef DEMO_POWER_CPU0_PRINT_ENABLE
#define DEMO_POWER_CPU0_PRINT_ENABLE 1U
#endif
/* Wheter HiFi uses LPUART for PRINTF or not */
#ifndef DEMO_POWER_HIFI_PRINT_ENABLE
#define DEMO_POWER_HIFI_PRINT_ENABLE 0U
#endif
#if DEMO_POWER_CPU0_PRINT_ENABLE
#define DEMO_LOG PRINTF
#else
#define DEMO_LOG(...)
#endif

#define APP_CORE_NAME            "CPU0"
#define BOOT_FLAG                0x1U /* Flag indicates Core1 Boot Up*/
#define APP_USER_WAKEUP_KEY_GPIO BOARD_SW5_GPIO
#define APP_USER_WAKEUP_KEY_PIN  BOARD_SW5_GPIO_PIN
#define APP_USER_WAKEUP_KEY_NAME "SW5"
#define APP_SW_IRQ_HANDLER       GPIO00_IRQHandler
#define APP_SW_IRQN              GPIO00_IRQn
#define APP_SW_RESET_RSTn        kGPIO0_RST_SHIFT_RSTn
#define APP_SW_CLOCK_EN          kCLOCK_Gpio0
#define APP_SW_LP_REQ            kPower_GPIO0_LPREQ

#define APP_RTC             RTC0
#define APP_RTC_IRQN        RTC0_ALARM_IRQn
#define APP_RTC_IRQ_HANDLER RTC0_ALARM_IRQHandler

#define APP_MU MU1_MUA

#define APP_USART_RX_ERROR (kLPUART_RxOverrunFlag | kLPUART_RxFifoUnderflowFlag)

/***********************Example configuration****************************/
/*!< Power down all unnecessary blocks and enable RBB during deep sleep. */
#if (DEMO_POWER_HIFI4_USED != 0U)
#if (DEMO_POWER_HIFI_PRINT_ENABLE != 0U)
#define APP_DEEPSLEEP_SLEEPCFG                                                                                        \
    (SLEEPCON0_SLEEPCFG_FRO0_PD_MASK | SLEEPCON0_SLEEPCFG_FRO0_GATE_MASK | SLEEPCON0_SLEEPCFG_RAM0_CLK_SHUTOFF_MASK | \
     SLEEPCON0_SLEEPCFG_COMP_MAINCLK_SHUTOFF_MASK) /* SLEEPCON->SLEEPCFG */
#else
#define APP_DEEPSLEEP_SLEEPCFG                                             \
    (SLEEPCON0_SLEEPCFG_FRO0_PD_MASK | SLEEPCON0_SLEEPCFG_FRO0_GATE_MASK | \
     SLEEPCON0_SLEEPCFG_RAM0_CLK_SHUTOFF_MASK)                    /* SLEEPCON->SLEEPCFG */
#endif                                                            /* DEMO_POWER_HIFI_PRINT_ENABLE */
#define APP_DEEPSLEEP_PDSLEEPCFG0 (PMC_PDSLEEPCFG0_V2DSP_PD_MASK) /* PMC->PDSLEEPCFG0 */
#define APP_DEEPSLEEP_PDSLEEPCFG1 (0U)                            /* PMC->PDSLEEPCFG1 */
#define APP_DSR_PDSLEEPCFG0       (0U)                            /* PMC->PDSLEEPCFG0 */
#define APP_DEEPSLEEP_RAM_APD     (0x3FFFFU) /* PMC->PDSLEEPCFG2, all keep powered, the unused PT can be power off. */
#define APP_DEEPSLEEP_RAM_PPD     (0x3C000U) /* PMC->PDSLEEPCFG3, keep PT14-17 which used by HIFI. */
#define APP_DEEPSLEEP_PDSLEEPCFG4                                                                        \
    (PMC_PDSLEEPCFG4_CPU0_CCACHE_MASK | PMC_PDSLEEPCFG4_CPU0_SCACHE_MASK | PMC_PDSLEEPCFG4_OCOTP_MASK |  \
     PMC_PDSLEEPCFG4_DSP_ICACHE_MASK | PMC_PDSLEEPCFG4_DSP_DCACHE_MASK | PMC_PDSLEEPCFG4_DSP_ITCM_MASK | \
     PMC_PDSLEEPCFG4_DSP_DTCM_MASK)

#define APP_DEEPSLEEP_PDSLEEPCFG5                                                                        \
    (PMC_PDSLEEPCFG5_DSP_ICACHE_MASK | PMC_PDSLEEPCFG5_DSP_DCACHE_MASK | PMC_PDSLEEPCFG5_DSP_ITCM_MASK | \
     PMC_PDSLEEPCFG5_DSP_DTCM_MASK)
#else
#define APP_DEEPSLEEP_SLEEPCFG    (0U)     /* SLEEPCON->SLEEPCFG */
#define APP_DEEPSLEEP_PDSLEEPCFG0 (0U)     /* PMC->PDSLEEPCFG0 */
#define APP_DEEPSLEEP_PDSLEEPCFG1 (0U)     /* PMC->PDSLEEPCFG1 */
#define APP_DSR_PDSLEEPCFG0       (0U)     /* PMC->PDSLEEPCFG0 */
#define APP_DEEPSLEEP_RAM_APD     0x3FFFFU /* PMC->PDSLEEPCFG2, all keep powered, the unused PT can be power off. */
#define APP_DEEPSLEEP_RAM_PPD     (0U)     /* PMC->PDSLEEPCFG3 */
#define APP_DEEPSLEEP_PDSLEEPCFG4 \
    (PMC_PDSLEEPCFG4_CPU0_CCACHE_MASK | PMC_PDSLEEPCFG4_CPU0_SCACHE_MASK | PMC_PDSLEEPCFG4_OCOTP_MASK)
#define APP_DEEPSLEEP_PDSLEEPCFG5 (0U)
#endif                               /* DEMO_POWER_HIFI4_USED */

#define APP_DSR_SLEEPCFG    (0U)     /* SLEEPCON->SLEEPCFG */
#define APP_DSR_PDSLEEPCFG0 (0U)     /* PMC->PDSLEEPCFG0 */
#define APP_DSR_PDSLEEPCFG1 (0U)     /* PMC->PDSLEEPCFG1 */
#define APP_DSR_RAM_APD     0x3FFFFU /* PMC->PDSLEEPCFG2, all keep powered, the unused PT can be power off. */
#define APP_DSR_RAM_PPD     (0U)     /* PMC->PDSLEEPCFG3 */
#define APP_DSR_PDSLEEPCFG4 (PMC_PDSLEEPCFG4_OCOTP_MASK) /* Cache is not retented in DSR. */
#define APP_DSR_PDSLEEPCFG5 (0U)

#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DEEPSLEEP_PDSLEEPCFG0, APP_DEEPSLEEP_PDSLEEPCFG1, \
                         APP_DEEPSLEEP_RAM_APD, APP_DEEPSLEEP_RAM_PPD, APP_DEEPSLEEP_PDSLEEPCFG4,      \
                         APP_DEEPSLEEP_PDSLEEPCFG5}))
#define APP_EXCLUDE_FROM_DSR                                                                          \
    (((const uint32_t[]){APP_DSR_SLEEPCFG, APP_DSR_PDSLEEPCFG0, 0U, APP_DSR_RAM_APD, APP_DSR_RAM_PPD, \
                         APP_DSR_PDSLEEPCFG4, APP_DSR_PDSLEEPCFG5}))
#define APP_EXCLUDE_FROM_DEEP_POWERDOWN      (((const uint32_t[]){0, 0, 0, 0, 0, 0, 0}))
#define APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN (((const uint32_t[]){0, 0, 0, 0, 0, 0, 0}))
/************************************************************************/
typedef enum _app_wakeup_source
{
    kAPP_WakeupSourcePin,   /*!< Wakeup by external pin. */
    kAPP_WakeupSourceRtc,   /*!< Wakeup by RTC.        */
    kAPP_WakeupSourceReset, /*!< Wakeup by Reset. */
} app_wakeup_source_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
const char *gWakeupInfoStr[] = {"Sleep", "Deep Sleep", "Deep Sleep Retention", "Deep Powerdown [Reset to wakeup]",
                                "Full Deep Powerdown [Reset to wakeup]"};
uint32_t gCurrentPowerMode;

static uint32_t s_wakeupTimeout;           /* Wakeup timeout. (Unit: seconds) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_RestorePeripheralsAfterDSR(void);
void BOARD_DisablePll(void);
void BOARD_RestorePll(void);
void DEMO_InitDebugConsole(void);
void DEMO_DeinitDebugConsole(void);
void BOARD_RunActiveTest(void);
void BOARD_EnterSleep(void);
void BOARD_EnterDeepSleep(const uint32_t exclude_from_pd[7]);
void BOARD_RequestDSR(const uint32_t exclude_from_pd[7]);
void BOARD_RequestDPD(const uint32_t exclude_from_pd[7]);
void BOARD_RequestFDPD(const uint32_t exclude_from_pd[7]);
void BOARD_BootDSP(void);
void BOARD_StopDSP(void);
static uint32_t APP_GetUserSelection(void);
static void APP_InitWakeupPin(void);
/*******************************************************************************
 * Code
 ******************************************************************************/
extern const clock_main_pll_config_t g_mainPllConfig_BOARD_BootClockRUN;
extern const clock_audio_pll_config_t g_audioPllConfig_BOARD_BootClockRUN;

void BOARD_ConfigPMICModes(pca9422_modecfg_t *cfg, pca9422_power_mode_t mode)
{
    assert(cfg);

    switch (mode)
    {
        case kPCA9422_ActiveModeDVS0:
        case kPCA9422_ActiveModeDVS1:
        case kPCA9422_ActiveModeDVS2:
        case kPCA9422_ActiveModeDVS3:
        case kPCA9422_ActiveModeDVS4:
        case kPCA9422_ActiveModeDVS5:
        case kPCA9422_ActiveModeDVS6:
        case kPCA9422_ActiveModeDVS7:
            cfg[mode].sw1OutVolt  = 1000000U; /* VDD2 */
            cfg[mode].sw2OutVolt  = 1100000U; /* VDDN */
            cfg[mode].sw3OutVolt  = 1000000U; /* VDD1 */
            cfg[mode].sw4OutVolt  = 1800000U;
            cfg[mode].ldo1OutVolt = 1800000U; /* 1V8 AO */
            cfg[mode].ldo2OutVolt = 1800000U;
            cfg[mode].ldo3OutVolt = 1200000U;
            cfg[mode].ldo4OutVolt = 3300000U;
            break;

        /* PMIC_MODE[1:0] = 01b */
        case kPCA9422_SleepMode:
#if (DEMO_POWER_HIFI4_USED != 0U)
            cfg[mode].sw1OutVolt = 900000U; /* DSP keep running. */
#else
            cfg[mode].sw1OutVolt = 630000U;
#endif
            cfg[mode].sw2OutVolt  = 1000000U;
            cfg[mode].sw3OutVolt  = 630000U;
            cfg[mode].sw4OutVolt  = 1800000U;
            cfg[mode].ldo1OutVolt = 1800000U;
            cfg[mode].ldo2OutVolt = 1800000U;
            cfg[mode].ldo3OutVolt = 1200000U;
            cfg[mode].ldo4OutVolt = 3300000U;
            break;

            /* PMIC_MODE[1:0] = 10b */
        case kPCA9422_StandbyMode:
            cfg[mode].sw1OutVolt  = 630000U;
            cfg[mode].sw2OutVolt  = 1000000U;
            cfg[mode].sw3OutVolt  = 630000U;
            cfg[mode].sw4OutVolt  = 1800000U;
            cfg[mode].ldo1OutVolt = 1800000U;
            cfg[mode].ldo2OutVolt = 1800000U;
            cfg[mode].ldo3OutVolt = 1200000U;
            cfg[mode].ldo4OutVolt = 3300000U;
            break;
            /* PMIC_MODE[1:0] = 11b */
        case kPCA9422_DPStandbyMode:
            cfg[mode].sw1OutVolt  = 630000U;
            cfg[mode].sw2OutVolt  = 1000000U;
            cfg[mode].sw3OutVolt  = 630000U;
            cfg[mode].sw4OutVolt  = 1800000U;
            cfg[mode].ldo1OutVolt = 1800000U;
            cfg[mode].ldo2OutVolt = 1800000U;
            cfg[mode].ldo3OutVolt = 1200000U;
            cfg[mode].ldo4OutVolt = 3300000U;
            break;

        default:
            break;
    }
}

/* Configure regulator output enable in Run mode. */
void BOARD_ConfigPMICRegEnable(pca9422_handle_t *handle)
{
    pca9422_regulatoren_t cfg;

    /* Configure Regulator Enable */
    PCA9422_GetDefaultRegEnableConfig(&cfg);

    /* All regulators enable in RUN state. */
    cfg.sw2Enable = true;
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    cfg.sw1Enable = true;
    cfg.sw3Enable = true;
#else /* VDD1, VDD2 are supplied by internal LDO. */
    cfg.sw1Enable    = false;
    cfg.sw3Enable    = false;
#endif
    cfg.sw4Enable  = true;
    cfg.ldo1Enable = true;
    cfg.ldo2Enable = true;
    cfg.ldo3Enable = true;
    cfg.ldo4Enable = true;

    PCA9422_WriteRegEnableConfig(handle, cfg);
}

void BOARD_ConfigPMICEnMode(pca9422_handle_t *handle)
{
    pca9422_enmodecfg_t cfg;
    /* Configure ENMODE */
    PCA9422_GetDefaultEnModeConfig(&cfg);

#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    cfg.sw1OutEnMode = kPCA9422_EnmodeOnActiveSleep;
    cfg.sw3OutEnMode = kPCA9422_EnmodeOnActiveSleep;
#else
    cfg.sw1OutEnMode = kPCA9422_EnmodeOnActive;
    cfg.sw3OutEnMode = kPCA9422_EnmodeOnActive;
#endif
    cfg.sw2OutEnMode  = kPCA9422_EnmodeOnActiveSleep;
    cfg.sw4OutEnMode  = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.ldo1OutEnMode = kPCA9422_EnmodeOnAll;
    cfg.ldo2OutEnMode = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.ldo3OutEnMode = kPCA9422_EnmodeOnActiveSleep;
    cfg.ldo4OutEnMode = kPCA9422_EnmodeOnActiveSleep;

    PCA9422_WriteEnModeConfig(handle, cfg);
}

void BOARD_RestorePeripheralsAfterDSR(void)
{
    DEMO_InitDebugConsole();
}

void BOARD_WaitCPU1Booted(void)
{
    RESET_ClearPeripheralReset(kMU1_RST_SHIFT_RSTn);
    MU_Init(APP_MU);

    /* Wait Core 1 is Boot Up */
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
    {
    }

    MU_Deinit(APP_MU);
}

void APP_BootCore1(void)
{
    BOARD_ReleaseCore1Power();
    BOARD_InitAHBSC();
#ifdef CORE1_IMAGE_COPY_TO_RAM
    BOARD_CopyCore1Image(CORE1_BOOT_ADDRESS);
#endif
    BOARD_BootCore1(CORE1_BOOT_ADDRESS, CORE1_BOOT_ADDRESS);
    BOARD_WaitCPU1Booted();
}

void BOARD_InitPowerConfig(void)
{
    pca9422_modecfg_t pca9422ModeCfg[12];
    uint32_t i;
    clock_osc32k_config_t config = {
        .bypass = false, .monitorEnable = false, .lowPowerMode = false, .cap = kCLOCK_Osc32kCapPf0};
    /* If OSC32K not enabled, enable OSC32K. */
    if (CLOCK_GetOsc32KFreq() == 0U)
    {
        CLOCK_EnableOsc32K(&config);
    }

#if defined(DEMO_POWER_USE_PLL) && (DEMO_POWER_USE_PLL == 0U)
    CLKCTL2->MAINPLL0PFDDOMAINEN  = 0;
    CLKCTL2->AUDIOPLL0PFDDOMAINEN = 0;
    /* Disable PLL. */
    CLOCK_DeinitMainPll();
    CLOCK_DeinitAudioPll();
#endif

    /* Disable the clock for unused modules. */
    CLOCK_DisableClock(kCLOCK_Mmu0);
    CLOCK_DisableClock(kCLOCK_Mmu1);
    CLOCK_DisableClock(kCLOCK_Pkc);
    CLOCK_DisableClock(kCLOCK_PkcRam);
    CLOCK_DisableClock(kCLOCK_Syspm0);
    CLOCK_DisableClock(kCLOCK_Syspm1);
    CLOCK_DisableClock(kCLOCK_PrinceExe);
    CLOCK_DisableClock(kCLOCK_Prince0);
    CLOCK_DisableClock(kCLOCK_Prince1);
    CLOCK_DisableClock(kCLOCK_Iopctl0);
    CLOCK_DisableClock(kCLOCK_Ocotp0);
    CLOCK_DisableClock(kCLOCK_Glikey3);
    CLOCK_DisableClock(kCLOCK_Glikey4);
    CLOCK_DisableClock(kCLOCK_Glikey5);
    CLOCK_DisableClock(kCLOCK_Hifi4AccessRamArbiter1);
    CLOCK_DisableClock(kCLOCK_MediaAccessRamArbiter0);
    CLOCK_DisableClock(kCLOCK_MediaAccessRamArbiter1);
    CLOCK_DisableClock(kCLOCK_Hifi4);
    CLOCK_DisableClock(kCLOCK_Romcp);

    CLOCK_AttachClk(kNONE_to_DSP);
    CLOCK_AttachClk(kNONE_to_TPIU);
    CLOCK_AttachClk(kNONE_to_SYSTICK);
    CLOCK_AttachClk(kNONE_to_FCCLK1);
    CLOCK_AttachClk(kNONE_to_FCCLK2);
    CLOCK_AttachClk(kNONE_to_FCCLK3);
    CLOCK_AttachClk(kNONE_to_TRNG);
    CLOCK_AttachClk(kNONE_to_SDIO0);
    CLOCK_AttachClk(kNONE_to_SDIO1);

    BOARD_InitPmic();
    for (i = 0; i < ARRAY_SIZE(pca9422ModeCfg); i++)
    {
        PCA9422_GetDefaultPowerModeConfig(&pca9422ModeCfg[i]);
    }

    for (i = 0; i < ARRAY_SIZE(pca9422ModeCfg); i++)
    {
        BOARD_ConfigPMICModes(pca9422ModeCfg, (pca9422_power_mode_t)i);
        PCA9422_WritePowerModeConfigs(&pca9422Handle, (pca9422_power_mode_t)i, pca9422ModeCfg[i]);
    }
    BOARD_ConfigPMICRegEnable(&pca9422Handle);
    BOARD_ConfigPMICEnMode(&pca9422Handle);
#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    /* Switch to a new DVS mode before re-configuring the VDD1/VDD2 per CPU frequency. */
    BOARD_SetPmicDVSPinStatus(0x1);
    /* PMIC is used. When using On-Chip regulator, need to be changed to kVddSrc_PMC. */
    POWER_SetVddnSupplySrc(kVddSrc_PMIC);
    POWER_SetVdd1SupplySrc(kVddSrc_PMIC);
    POWER_SetVdd2SupplySrc(kVddSrc_PMIC);
    POWER_DisableRegulators(kPower_SCPC);

    POWER_SelectRunSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd1LDO, 0U);
#endif

    /* Keep the used resources on. */
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK); /* Keep Sense shared parts clock on. */

    POWER_DisablePD(kPDRUNCFG_GATE_FRO0);           /* Just use PD bit to control FRO. */
    POWER_DisablePD(kPDRUNCFG_SHUT_RAM1_CLK);
    POWER_ApplyPD();
    POWER_DisableLPRequestMask(kPower_MaskAll); /* Let's compute control all the shared resources. */
}

static void BOARD_DisableCache(CACHE64_CTRL_Type *base)
{
    if ((base->CCR & CACHE64_CTRL_CCR_ENCACHE_MASK) == CACHE64_CTRL_CCR_ENCACHE_MASK)
    {
        /* First, push any modified contents. */
        base->CCR |= CACHE64_CTRL_CCR_PUSHW0_MASK | CACHE64_CTRL_CCR_PUSHW1_MASK | CACHE64_CTRL_CCR_GO_MASK;

        /* Wait until the cache command completes. */
        while ((base->CCR & CACHE64_CTRL_CCR_GO_MASK) != 0x00U)
        {
        }

        /* As a precaution clear the bits to avoid inadvertently re-running this command. */
        base->CCR &= ~(CACHE64_CTRL_CCR_PUSHW0_MASK | CACHE64_CTRL_CCR_PUSHW1_MASK);

        /* Now disable the cache. */
        base->CCR &= ~CACHE64_CTRL_CCR_ENCACHE_MASK;
    }
}

void BOARD_PowerConfigAfterCPU1Booted(void)
{
    /* Turn off unused resources. */
    CLOCK_DisableClock(kCLOCK_Glikey0);
    CLOCK_DisableClock(kCLOCK_Glikey1);
    CLOCK_DisableClock(kCLOCK_Glikey2);
    CLKCTL0->RAMCLKSEL = 0;          /* Compute access RAM arbiter1 clock. */
    CLOCK_DisableClock(kCLOCK_Cpu1); /*Let CPU1 control it's clock. */

#if (DEMO_POWER_ENABLE_DEBUG == 0U)
    CLOCK_DisableClock(kCLOCK_Dbg);
#endif

    if (!IS_XIP_XSPI0())
    {
        BOARD_DisableCache(CACHE64_CTRL0);
        CLOCK_DisableClock(kCLOCK_Cache64ctrl0);
        CLOCK_DisableClock(kCLOCK_Xspi0);
        CLOCK_AttachClk(kNONE_to_XSPI0);
        POWER_EnablePD(kPDRUNCFG_APD_XSPI0);
        POWER_EnablePD(kPDRUNCFG_PPD_XSPI0);
        POWER_ApplyPD();
    }

    if (!IS_XIP_XSPI1())
    {
        BOARD_DisableCache(CACHE64_CTRL1);
        CLOCK_DisableClock(kCLOCK_Cache64ctrl1);
        CLOCK_DisableClock(kCLOCK_Xspi1);
        CLOCK_AttachClk(kNONE_to_XSPI1);
        POWER_EnablePD(kPDRUNCFG_APD_XSPI1);
        POWER_EnablePD(kPDRUNCFG_PPD_XSPI1);
        POWER_ApplyPD();
    }
#if defined(DEMO_POWER_USE_PLL) && (DEMO_POWER_USE_PLL == 0U)
    POWER_EnablePD(kPDRUNCFG_PD_SYSXTAL);
#endif
    POWER_EnablePD(kPDRUNCFG_PD_ADC0);
    POWER_EnablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM1_CLK);

    POWER_EnablePD(kPDRUNCFG_LP_DCDC);
    POWER_EnablePD(kPDRUNCFG_APD_XSPI2);
    POWER_EnablePD(kPDRUNCFG_PPD_XSPI2);
    POWER_EnablePD(kPDRUNCFG_APD_DMA0_1_PKC_ETF);
    POWER_EnablePD(kPDRUNCFG_PPD_DMA0_1_PKC_ETF);
    POWER_EnablePD(kPDRUNCFG_APD_USB0_SRAM);
    POWER_EnablePD(kPDRUNCFG_PPD_USB0_SRAM);
    POWER_EnablePD(kPDRUNCFG_APD_SDHC0_SRAM);
    POWER_EnablePD(kPDRUNCFG_PPD_SDHC0_SRAM);
    POWER_EnablePD(kPDRUNCFG_APD_SDHC1_SRAM);
    POWER_EnablePD(kPDRUNCFG_PPD_SDHC1_SRAM);
    POWER_EnablePD(kPDRUNCFG_PPD_OCOTP);

    SYSCON0->COMP_AUTOGATE_EN = 0x7U; /* MBUS_EN bit disabled to allow other master accessing RAM0. */

    POWER_EnablePD(kPDRUNCFG_DSR_VDD2N_MEDIA);
    POWER_ApplyPD();

    PMC0->PDRUNCFG1 = 0x7FFFFFFFU; /* Power down ROM, Power down or set low-power mode for HVD, LVD, GDET. 0x4020F0A4 */

    POWER_EnablePD(kPDRUNCFG_SHUT_SENSEP_MAINCLK); /* Let Sense control private parts clock. */
    POWER_EnablePD(kPDRUNCFG_PD_FRO1); /* Note: Sense boots using FRO1 and switchs to FRO2(Sense can't control FRO1). */
    POWER_EnablePD(kPDRUNCFG_PD_FRO2);

    CLOCK_DisableClock(kCLOCK_CompAccessRamArbiter1);
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM0_CLK);                        /* Sense access RAM arbiter0 clock. */
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM1_CLK);                        /* Compute access RAM arbiter1 clock. */

    POWER_EnableRunAFBB(kPower_BodyBiasVdd2 | kPower_BodyBiasVddn); /* Configure VDD2 AFBB mode during active.*/
    POWER_EnableRunNBB(kPower_BodyBiasVdd2Sram);
    POWER_EnableRunRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd1Sram);
#if (DEMO_POWER_HIFI4_USED != 0U)
    POWER_EnableSleepNBB(kPower_BodyBiasVdd2Sram | kPower_BodyBiasVdd2);
    POWER_EnableSleepRBB(kPower_BodyBiasVddn | kPower_BodyBiasVdd1 | kPower_BodyBiasVdd1Sram);
#else
    POWER_EnableSleepRBB(kPower_BodyBiasVddn | kPower_BodyBiasVdd2Sram | kPower_BodyBiasVdd2 | kPower_BodyBiasVdd1 |
                         kPower_BodyBiasVdd1Sram);
#endif

    POWER_ApplyPD();

    power_regulator_voltage_t ldo = {
        .LDO.vsel0 = 700000U,  /* 700mv, 0.45 V + 12.5 mV * x */
        .LDO.vsel1 = 800000U,  /* 800mv*/
        .LDO.vsel2 = 900000U,  /* 900mv */
        .LDO.vsel3 = 1000000U, /* 1000mv */
    };

    power_lvd_voltage_t lvd = {
        .VDD12.lvl0 = 500000U, /* 500mv */
        .VDD12.lvl1 = 700000U, /* 700mv */
        .VDD12.lvl2 = 800000U, /* 800mv */
        .VDD12.lvl3 = 900000U, /* 900mv */
    };

    POWER_ConfigRegulatorSetpoints(kRegulator_Vdd2LDO, &ldo, &lvd);

#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_MIXED)
    /* VDDN use external PMIC supply, VDD1&VDD2 use internal LDO. */
    POWER_SetVddnSupplySrc(kVddSrc_PMIC);
    POWER_SetVdd1SupplySrc(kVddSrc_PMC);
    POWER_SetVdd2SupplySrc(kVddSrc_PMC);

    POWER_SelectRunSetpoint(kRegulator_Vdd2LDO, 2U);
#if defined(DEMO_POWER_HIFI4_USED) && (DEMO_POWER_HIFI4_USED != 0U)
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 2U); /* HIFI4 can keep running in deep sleep mode. */
#else
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U);
#endif                                                 /* DEMO_POWER_HIFI4_USED */
    POWER_SelectRunSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd1LDO, 0U);
    POWER_ApplyPD();
#elif defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
    POWER_SelectRunSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U);
    POWER_ApplyPD();

    BOARD_SetPmicVdd2Voltage(900000U); /* 0.9V. */
#endif
}

void DEMO_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    CLOCK_AttachClk(kFRO0_DIV1_to_FCCLK0);
    CLOCK_SetClkDiv(kCLOCK_DivFcclk0Clk, 10U);

    /* Attach FC0 clock to LP_FLEXCOMM (debug console) */
    CLOCK_AttachClk(kFCCLK0_to_FLEXCOMM0);

    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void DEMO_DeinitDebugConsole(void)
{
    DbgConsole_Deinit();
    CLOCK_AttachClk(kNONE_to_FCCLK0);
}

void BOARD_DisableIoPads()
{
    uint8_t port, pin;

    IOPCTL_PinMuxSet(4U, 11U, 0U);

    if (IS_XIP_XSPI0() == 0U)
    {
        port = 6U;
        for (pin = 0U; pin <= 12U; pin++)
        {
            IOPCTL_PinMuxSet(port, pin, 0U);
        }
    }

    if (IS_XIP_XSPI1() == 0U)
    {
        port = 5U;
        for (pin = 0U; pin <= 20U; pin++)
        {
            IOPCTL_PinMuxSet(port, pin, 0U);
        }
    }
}

/*!
 * @brief API to perform a dummy read to the selected SRAM partitions. SRAM auto clock gating can save power if
 * partitions are parked on the core for their domain (CPU0 for RAM arbiter0 and CPU1 for RAM
 * arbiter1). This function will perform a dummy read to the specified RAM partitions to force the clock to
 * park on that core until another master accesses each partition.
 *
 * NOTE, make sure the caller is allowed to access the given SRAM partition, otherwise the system may hang!
 *
 * @param pt SRAM Partition, 0-17 for RAM arbiter0, 18-29 for RAM arbiter1.
 *
 */
void POWER_SramDummyRead(uint32_t pt)
{
    uint32_t dummy;
    const uint32_t sram_addr[30] = {0x20000000, 0x20008000, 0x20010000, 0x20018000, 0x20020000, 0x20030000,
                                    0x20040000, 0x20060000, 0x20080000, 0x200C0000, 0x20100000, 0x20180000,
                                    0x20200000, 0x20300000, 0x20400000, 0x20480000, 0x20500000, 0x20540000,
                                    0x20580000, 0x20588000, 0x20590000, 0x20598000, 0x205A0000, 0x205B0000,
                                    0x205C0000, 0x205E0000, 0x20600000, 0x20680000, 0x20700000, 0x20740000};

#if defined(XCACHE0) /* Only CPU0 has cache. */
    bool enabled = false;

    /*Disable cache, only disable if it's enabled. */
    if (XCACHE_CCR_ENCACHE_MASK == (XCACHE_CCR_ENCACHE_MASK & XCACHE0->CCR))
    {
        enabled = true;
        XCACHE_DisableCache(XCACHE0);
    }
#endif
    dummy = *((volatile uint32_t *)(sram_addr[pt]));
    dummy++;         /* suppress warning. */
#if defined(XCACHE0) /* Only CPU0 has cache. */
    /*Disable cache, only disable if it's enabled. */
    if (enabled)
    {
        XCACHE_EnableCache(XCACHE0);
    }
#endif
}

void BOARD_BootDSP(void)
{
    /* Boot HIFI4. */
    CLKCTL0->FRO0MAXDOMAINEN |= CLKCTL0_FRO0MAXDOMAINEN_FRO0MAX_OF_VDD2_DSP_MASK;
    BOARD_DSP_Init(2U, 1U, true); /* Select FRO0_MAX divider by 1 for HIFI clock. */
}

void BOARD_StopDSP(void)
{
    uint32_t i;
    DSP_Stop();

    /* Dummy read the SRAM partition to let SRAM partition's clock park on CM33. */
    for (i = DEMO_HIFI4_SRAM_PT_START; i <= DEMO_HIFI4_SRAM_PT_END; i++)
    {
        POWER_SramDummyRead(i);
    }

    DSP_Deinit();
    CLOCK_AttachClk(kNONE_to_DSP);
    CLKCTL0->FRO0MAXDOMAINEN &= ~CLKCTL0_FRO0MAXDOMAINEN_FRO0MAX_OF_VDD2_DSP_MASK;
    POWER_EnablePD(kPDRUNCFG_PD_VDD2_DSP);
}


#if DEMO_POWER_USE_PLL
void BOARD_DisablePll(void)
{
    /* Disable the PFD clock output first. */
    CLOCK_DeinitMainPfd(kCLOCK_Pfd0);
    CLOCK_DeinitAudioPfd(kCLOCK_Pfd3);
    /* Disable PLL. */
    CLOCK_DeinitMainPll();
    CLOCK_DeinitAudioPll();
}

void BOARD_RestorePll(void)
{
    /*Restore PLL*/
    CLOCK_InitMainPll(&g_mainPllConfig_BOARD_BootClockRUN);
    CLOCK_InitAudioPll(&g_audioPllConfig_BOARD_BootClockRUN);
    /*Restore PFD*/
    CLOCK_InitMainPfd(kCLOCK_Pfd0, 20U); /* Enable main PLL clock 475MHz. */
    CLOCK_InitAudioPfd(kCLOCK_Pfd3, 26); /* Configure audio_pll_clk to 368.64MHz */
}
#endif

/*! Disable clock for modules for cpu run only or sleep. */
static inline void BOARD_DisableClocks(void)
{
    CLOCK_DisableClock(kCLOCK_LPI2c15);

    CLOCK_DisableClock(kCLOCK_Gpio7);
    CLOCK_DisableClock(kCLOCK_Mu1);
    CLOCK_DisableClock(kCLOCK_Syscon0);
    CLOCK_DisableClock(kCLOCK_Syscon3);
    CLOCK_DisableClock(kCLOCK_Iopctl0);
    CLOCK_DisableClock(kCLOCK_Iopctl1);
    CLOCK_DisableClock(kCLOCK_Sema420);

#if (DEMO_POWER_USE_PLL == 0U) /* PLL located in VDDN. */
    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
#if (DEMO_POWER_HIFI4_USED != 0U)
        CLOCK_EnableFro0ClkForDomain(kCLOCK_Vdd2CompDomainEnable | kCLOCK_Vdd2DspDomainEnable);
#else
        CLOCK_EnableFro0ClkForDomain(kCLOCK_Vdd2CompDomainEnable);
#endif /* DEMO_POWER_HIFI4_USED */
    }
#endif
    CLOCK_EnableFroClkOutput(FRO0, kCLOCK_FroDiv1OutEn);
}

static inline void BOARD_RestoreClocks(void)
{
    /* Restore clock, power for used modules. */
    CLOCK_EnableFroClkOutput(FRO0, kCLOCK_FroDiv1OutEn | kCLOCK_FroDiv3OutEn | kCLOCK_FroDiv6OutEn);
#if (DEMO_POWER_HIFI4_USED != 0U)
    CLOCK_EnableFro0ClkForDomain(kCLOCK_Vdd2CompDomainEnable | kCLOCK_VddnComDomainEnable | kCLOCK_Vdd2DspDomainEnable);
#else
    CLOCK_EnableFro0ClkForDomain(kCLOCK_Vdd2CompDomainEnable | kCLOCK_VddnComDomainEnable);
#endif

    CLOCK_EnableClock(kCLOCK_Syscon0);
    CLOCK_EnableClock(kCLOCK_Syscon3);
    CLOCK_EnableClock(kCLOCK_Iopctl0);
    CLOCK_EnableClock(kCLOCK_Iopctl1);
    CLOCK_EnableClock(kCLOCK_Mu1);
    CLOCK_EnableClock(kCLOCK_Sema420);
    CLOCK_EnableClock(kCLOCK_Gpio7);
    CLOCK_EnableClock(kCLOCK_LPI2c15);
}

void BOARD_RunActiveTest(void)
{
#if (DEMO_POWER_HIFI4_USED == 0U) || (DEMO_POWER_HIFI_PRINT_ENABLE == 0U)
    uint32_t pinCfg[3] = {0}; /* Used for IOPCTL configuration backup. */
#endif

    DEMO_LOG("\r\nThis test mode will keep CPU in run mode but close all other unused modules for a while.\n");
    DEMO_LOG("\r\nPlease don't input any charator until the mode finished.\n");

    /* Deinit unused modules. */
    BOARD_PMIC_I2C_Deinit();
    CLOCK_AttachClk(kNONE_to_LPI2C15);
#if (DEMO_POWER_HIFI4_USED == 0U) || (DEMO_POWER_HIFI_PRINT_ENABLE == 0U)
    DEMO_DeinitDebugConsole();
    CLOCK_AttachClk(kNONE_to_FCCLK0);

    pinCfg[0] = IOPCTL0->PIO[0][31];
    pinCfg[1] = IOPCTL0->PIO[1][0];
    pinCfg[2] = IOPCTL0->PIO[0][9];
#endif

    CLOCK_DisableClock(kCLOCK_Rtc);
    /* Power down unused modules. */
#if (DEMO_POWER_USE_PLL == 0U) /* PLL located in VDDN. */
    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /* XSPI0 and XSPI1 memory interface located in VDDN_COM. */
        POWER_EnablePD(kPDRUNCFG_DSR_VDDN_COM);
        POWER_EnableRunRBB(kPower_BodyBiasVddn);
        POWER_EnablePD(kPDRUNCFG_SHUT_COMNN_MAINCLK);
        POWER_ApplyPD();
    }
#endif
    POWER_EnablePD(kPDRUNCFG_APD_OCOTP);
    POWER_ApplyPD();

    BOARD_DisableClocks();

    /* Note, the debug will not work anymore when the sense shared mainclk is disabled. */
    POWER_EnablePD(kPDRUNCFG_PD_LPOSC);
    POWER_EnablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);

    CLOCK_DisableClock(kCLOCK_Sleepcon0);

    /* Simulate a task. */
    uint32_t i;
    for (i = 0U; i < 500U; i++)
    {
        SDK_DelayAtLeastUs(10000U, CLOCK_GetCoreSysClkFreq());
    }

    /* Restore clock, power for used modules. */
    CLOCK_EnableClock(kCLOCK_Sleepcon0);

    POWER_DisablePD(kPDRUNCFG_PD_LPOSC);
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);

    BOARD_RestoreClocks();
    CLOCK_EnableClock(kCLOCK_Rtc);
#if (DEMO_POWER_HIFI4_USED == 0U) || (DEMO_POWER_HIFI_PRINT_ENABLE == 0U)
    IOPCTL0->PIO[0][31] = pinCfg[0];
    IOPCTL0->PIO[1][0]  = pinCfg[1];
    IOPCTL0->PIO[0][9]  = pinCfg[2];
    DEMO_InitDebugConsole();
#endif
#if (DEMO_POWER_USE_PLL == 0U) /* PLL located in VDDN. */
    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /* XSPI0 and XSPI1 memory interface located in VDDN_COM. */
        POWER_DisablePD(kPDRUNCFG_DSR_VDDN_COM);
        POWER_EnableRunAFBB(kPower_BodyBiasVddn);
        POWER_DisablePD(kPDRUNCFG_SHUT_COMNN_MAINCLK);
        POWER_ApplyPD();
    }
#endif
    POWER_DisablePD(kPDRUNCFG_APD_OCOTP); /* Need keep OCOTP for warm reset boot. */
    POWER_ApplyPD();

    CLOCK_AttachClk(kSENSE_BASE_to_LPI2C15);
    BOARD_PMIC_I2C_Init();
}

void BOARD_EnterSleep(void)
{
    uint32_t irqMask;
#if (DEMO_POWER_HIFI4_USED == 0U) || (DEMO_POWER_HIFI_PRINT_ENABLE == 0U)
    DEMO_DeinitDebugConsole();
    CLOCK_AttachClk(kNONE_to_FCCLK0);
#endif
    BOARD_DisableClocks();

#if (DEMO_POWER_USE_PLL == 0U) /* PLL located in VDDN. */
    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /* XSPI0 and XSPI1 memory interface located in VDDN_COM. */
        POWER_EnablePD(kPDRUNCFG_DSR_VDDN_COM);
        POWER_EnableRunRBB(kPower_BodyBiasVddn);
        POWER_EnablePD(kPDRUNCFG_SHUT_COMNN_MAINCLK);
        POWER_ApplyPD();
    }
#endif

    /*LPOSC and Sense shared main clock are needed for RTC. */
    /* NOTE, debug and PMC registers access require sense shared main clock. */
    irqMask = DisableGlobalIRQ();

    /* POWER_EnablePD(kPDRUNCFG_PD_LPOSC); */
    POWER_EnablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);

    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /* Use 1MHz LPOSC for CPU0 when sleep. */
        CLOCK_AttachClk(kCOMPUTE_BASE_to_COMPUTE_MAIN); 
    }

    POWER_EnterSleep();

    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /*Restore main clock. */
        CLOCK_AttachClk(kFRO0_DIV1_to_COMPUTE_MAIN); 
    }

    /* POWER_DisablePD(kPDRUNCFG_PD_LPOSC); */
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);

    EnableGlobalIRQ(irqMask);
    __ISB();

#if (DEMO_POWER_USE_PLL == 0U) /* PLL located in VDDN. */
    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /* XSPI0 and XSPI1 memory interface located in VDDN_COM. */
        POWER_DisablePD(kPDRUNCFG_DSR_VDDN_COM);
        POWER_EnableRunAFBB(kPower_BodyBiasVddn);
        POWER_DisablePD(kPDRUNCFG_SHUT_COMNN_MAINCLK);
        POWER_ApplyPD();
    }
#endif

    BOARD_RestoreClocks();
#if (DEMO_POWER_HIFI4_USED == 0U) || (DEMO_POWER_HIFI_PRINT_ENABLE == 0U)
    DEMO_InitDebugConsole();
#endif
}

/*! @brief Decrease the CPU frequency and supply voltage for lower power consumption.
 * @return the origin mainclk divider.
 */
static inline uint32_t BOARD_PrepareForDS(void)
{
    uint32_t mainDiv = 0U;
#if DEMO_POWER_USE_PLL
    /*
     * Special sequence is needed for the PLL power up/initialization. The application should manually handle the states
     * changes for the PLL if the PLL power states configuration are different in Active mode and Deep Sleep mode. To
     * save power and to be simple, keep the PLL on only when Compute domain is active and sense domain will not use the
     * PLL.
     */
    /* Disable Pll before enter deep sleep mode */
    BOARD_DisablePll();
#endif
#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC) && \
    (DEMO_POWER_HIFI4_USED == 0U)
    /* Decrease CPU clock to decrease VDD2 supply in case sense is active. */
    mainDiv = (CLKCTL0->MAINCLKDIV & CLKCTL0_MAINCLKDIV_DIV_MASK) + 1U;

    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        CLOCK_SetClkDiv(kCLOCK_DivCmptMainClk, (SystemCoreClock + 31999999U) / 32000000U);
        BOARD_SetPmicVdd2Voltage(DEMO_LOW_POWER_RUN_VOLT);
    }
#endif /* DEMO_POWER_SUPPLY_OPTION */

#if (defined(BOARD_PMIC_CONFIG_USE_SEMA4) && (BOARD_PMIC_CONFIG_USE_SEMA4 != 0U))
    CLOCK_DisableClock(kCLOCK_Sema420);
#endif
    CLOCK_DisableClock(kCLOCK_LPI2c15);

    return mainDiv;
}

/*! @brief Decrease the CPU frequency and supply voltage for lower power consumption.
 * @param mainDiv the mainclk divider value.
 */
static inline void BOARD_RestoreAfterDS(uint32_t mainDiv)
{
#if (defined(BOARD_PMIC_CONFIG_USE_SEMA4) && (BOARD_PMIC_CONFIG_USE_SEMA4 != 0U))
    CLOCK_EnableClock(kCLOCK_Sema420);
#endif
    CLOCK_EnableClock(kCLOCK_LPI2c15);

#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC) && \
    (DEMO_POWER_HIFI4_USED == 0U)
    if (!IS_XIP_XSPI0() && !IS_XIP_XSPI1())
    {
        /* Restore VDD2 supply and CPU clock. */
        BOARD_SetPmicVdd2Voltage(DEMO_NORMAL_RUN_VOLT); /* Restore VDD2 supply. */
        CLOCK_SetClkDiv(kCLOCK_DivCmptMainClk, mainDiv);
    }

#endif /* DEMO_POWER_SUPPLY_OPTION */
#if DEMO_POWER_USE_PLL
    /* Restore Pll before enter deep sleep mode */
    BOARD_RestorePll();
#endif
}

void BOARD_EnterDeepSleep(const uint32_t exclude_from_pd[7])
{
    uint32_t mainDiv;
    mainDiv = BOARD_PrepareForDS();
    POWER_EnterDeepSleep(exclude_from_pd);
    BOARD_RestoreAfterDS(mainDiv);
}

void BOARD_RequestDSR(const uint32_t exclude_from_pd[7])
{
    uint32_t mainDiv;
    BOARD_StopDSP();
    mainDiv = BOARD_PrepareForDS();
    POWER_EnterDSR(exclude_from_pd);
    BOARD_RestoreAfterDS(mainDiv);
    BOARD_BootDSP(); /* Restart DSP. */
}

void BOARD_RequestDPD(const uint32_t exclude_from_pd[7])
{
#if DEMO_POWER_USE_PLL
    /* Disable Pll. */
    BOARD_DisablePll();
#endif
    POWER_RequestDeepPowerDown(exclude_from_pd);
#if DEMO_POWER_USE_PLL
    /* Restore Pll. The code will not executed if the chip goes into DPD. */
    BOARD_RestorePll();
#endif
}

void BOARD_RequestFDPD(const uint32_t exclude_from_pd[7])
{
#if DEMO_POWER_USE_PLL
    BOARD_DisablePll();
#endif
    POWER_RequestFullDeepPowerDown(exclude_from_pd);
    /* The code will not executed if the chip goes into FDPD. */
#if DEMO_POWER_USE_PLL
    BOARD_RestorePll();
#endif
}
void APP_SW_IRQ_HANDLER(void)
{
    POWER_ModuleExitLPRequest(APP_SW_LP_REQ); /* Clear the low power request before accessing the GPIO. */
    GPIO_GpioClearInterruptFlags(APP_USER_WAKEUP_KEY_GPIO, 1U << APP_USER_WAKEUP_KEY_PIN);
    DisableDeepSleepIRQ(APP_SW_IRQN);

    CLOCK_DisableClock(APP_SW_CLOCK_EN);
}

void APP_RTC_IRQ_HANDLER(void)
{
    uint32_t flags = IRTC_GetStatusFlags(APP_RTC);
    if ((flags & kIRTC_AlarmFlag) != 0U)
    {
        /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(APP_RTC, false);
        /*Clear all irtc flag */
        IRTC_ClearStatusFlags(APP_RTC, flags);
    }

    DisableDeepSleepIRQ(APP_RTC_IRQN);
}

/*
 * Get user selection from UART.
 */
static uint32_t APP_GetUserSelection(void)
{
    uint32_t ch;

    /* Clear rx overflow error once it happens during low power mode. */
    if (APP_USART_RX_ERROR == (APP_USART_RX_ERROR & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
    {
        LPUART_ClearStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR, APP_USART_RX_ERROR);
    }

    DEMO_LOG(
        "Select an option\r\n"
        "\t1. Sleep mode\r\n"
        "\t2. Deep Sleep mode\r\n"
        "\t3. Deep Sleep Retention mode\r\n"
        "\t4. Deep power down mode\r\n"
        "\t5. Full deep power down mode\r\n"
        "\t6. Active test mode\r\n");
    while (1)
    {
        ch = GETCHAR();
        if ((ch < '1') || (ch > '6'))
        {
            continue;
        }
        else
        {
            ch = ch - '1';
            break;
        }
    }
    switch (ch)
    {
        case 0:
            ch = kPower_Sleep;
            break;
        case 1:
            ch = kPower_DeepSleep;
            break;
        case 2:
            ch = kPower_DeepSleepRetention;
            break;
        case 3:
            ch = kPower_DeepPowerDown;
            break;
        case 4:
            ch = kPower_FullDeepPowerDown;
            break;
        default:
            break;
    }
    return ch;
}

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(uint32_t mode)
{
    uint8_t ch;

    while (true)
    {
        DEMO_LOG("Select the wake up source:\r\n");
        if ((mode != kPower_DeepPowerDown) && (mode != kPower_FullDeepPowerDown))
        {
            DEMO_LOG("Press T for RTC.\r\n");
            if (mode != kPower_DeepSleepRetention)
            {
                DEMO_LOG("Press S for wakeup pin(%s).\r\n", APP_USER_WAKEUP_KEY_NAME);
            }
            DEMO_LOG("Press N for Reset.\r\n");
            DEMO_LOG("\r\nWaiting for key press..\r\n\r\n");
            ch = GETCHAR();
            DEMO_LOG("%c\r\n", ch);

            if ((ch >= 'a') && (ch <= 'z'))
            {
                ch -= 'a' - 'A';
            }

            if (ch == 'T')
            {
                return kAPP_WakeupSourceRtc;
            }
            else if ((ch == 'S') && (mode != kPower_DeepSleepRetention))
            {
                return kAPP_WakeupSourcePin;
            }
            else if (ch == 'N')
            {
                return kAPP_WakeupSourceReset;
            }
            else
            {
                DEMO_LOG("Wrong value!\r\n");
            }
        }
        else
        {
            DEMO_LOG("Reset to wakeup.\r\n");
            return kAPP_WakeupSourceReset;
        }
    }
}

/*!
 * @brief Get input from user about wakeup timeout
 */
static uint8_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;

    while (1)
    {
        DEMO_LOG("Select the wake up timeout in seconds.\r\n");
        DEMO_LOG("The allowed range is 1s ~ 9s.\r\n");
        DEMO_LOG("Eg. enter 5 to wake up in 5 seconds.\r\n");
        DEMO_LOG("\r\nWaiting for input timeout value...\r\n\r\n");

        timeout = GETCHAR();
        DEMO_LOG("%c\r\n", timeout);
        if ((timeout > '0') && (timeout <= '9'))
        {
            return timeout - '0';
        }
        DEMO_LOG("Wrong value!\r\n");
    }
}

void APP_GetWakeupConfig(uint32_t targetMode)
{
    /* Get wakeup source by user input. */
    s_wakeupSource = APP_GetWakeupSource(targetMode);

    if (kAPP_WakeupSourceRtc == s_wakeupSource)
    {
        /* Wakeup source is Timer, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
    }
    else
    {
        s_wakeupTimeout = 0U;
    }
}

/*
 * Setup a GPIO input pin as wakeup source.
 */
static void APP_InitWakeupPin(void)
{
    gpio_pin_config_t gpioPinConfigStruct;

    /* Set SW pin as GPIO input. */
    gpioPinConfigStruct.pinDirection = kGPIO_DigitalInput;
    gpioPinConfigStruct.outputLogic  = 0U;

    RESET_ClearPeripheralReset(APP_SW_RESET_RSTn);
    CLOCK_EnableClock(APP_SW_CLOCK_EN);

    GPIO_SetPinInterruptConfig(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, kGPIO_InterruptFallingEdge);
    GPIO_PinInit(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);
}

static void APP_InitWakeupTimer(void)
{
    irtc_config_t irtcConfig;

    IRTC_GetDefaultConfig(&irtcConfig);
    if (IRTC_Init(APP_RTC, &irtcConfig) != kStatus_Success)
    {
        DEMO_LOG("RTC Init Failed.\r\n");
    }
}

static void APP_SetWakeupConfig(void)
{
    irtc_datetime_t datetime;

    if (s_wakeupSource == kAPP_WakeupSourceRtc)
    {
        /* Read the RTC seconds register to get current time in seconds */
        IRTC_GetDatetime(APP_RTC, &datetime);
        /* Add alarm seconds to current time */
        datetime.second += s_wakeupTimeout;
        if (datetime.second > 59U)
        {
            datetime.minute++;
            datetime.second -= 60U;
        }
        /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(APP_RTC, false);
        /* Set alarm time in seconds */
        IRTC_SetAlarm(APP_RTC, &datetime);
        /* Enable RTC alarm interrupt */
        IRTC_EnableInterrupts(APP_RTC, kIRTC_AlarmInterruptEnable);
        EnableDeepSleepIRQ(APP_RTC_IRQN);
        DEMO_LOG("RTC wake up after %d seconds.\r\n", s_wakeupTimeout);
    }
    else if (s_wakeupSource == kAPP_WakeupSourcePin)
    {
        APP_InitWakeupPin();
        EnableDeepSleepIRQ(APP_SW_IRQN);
        /* Low power handshake to for async interrupt. */
        if (POWER_ModuleEnterLPRequest(APP_SW_LP_REQ) != kStatus_Success)
        {
            assert(false);
        }
        DEMO_LOG("Push wakeup PIN to wake up.\r\n");
    }
    else
    {
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t freq = 0U;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_DisableIoPads();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    DEMO_InitDebugConsole();

    /* BE CAUTIOUS TO SET CORRECT VOLTAGE RANGE ACCORDING TO YOUR BOARD/APPLICATION. PAD SUPPLY BEYOND THE RANGE DO
       HARM TO THE SILICON. */
    POWER_SetPio2VoltRange(kPadVol_300_360);

    /* Initialze power/clock configuration. */
    BOARD_InitPowerConfig();

    /* Boot and wait CPU1 booted. */
    APP_BootCore1();

    /* After the CPU1 booted, CPU0 relinquish the domain's control over the modules, and give the other domain exclusive
     * control. */
    BOARD_PowerConfigAfterCPU1Booted();

    BOARD_BootDSP();

    /* Determine the power mode before bring up. */
    if ((POWER_GetEventFlags() & PMC_FLAGS_DEEPPDF_MASK) != 0)
    {
        DEMO_LOG("Board wake up from deep or full deep power down mode.\r\n");
        POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
    }
    POWER_ClearEventFlags(0xFFFFFFFF);

    APP_InitWakeupTimer();

    while (1)
    {
        freq = CLOCK_GetCoreSysClkFreq();

        DEMO_LOG("\r\n####################  Power Mode Switch - %s ####################\n\r\n", APP_CORE_NAME);
        DEMO_LOG("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        DEMO_LOG("    Core Clock: %dHz \r\n", freq);

        gCurrentPowerMode = APP_GetUserSelection();

        if (gCurrentPowerMode == 5U)
        {
            BOARD_RunActiveTest();
            continue;
        }

        APP_GetWakeupConfig(gCurrentPowerMode);
        APP_SetWakeupConfig();

        DEMO_LOG("Entering %s ...\r\n", gWakeupInfoStr[gCurrentPowerMode]);
        /* Enter the low power mode. */
        switch (gCurrentPowerMode)
        {
            case kPower_Sleep: /* Enter sleep mode. */
                BOARD_EnterSleep();
                break;
            case kPower_DeepSleep: /* Enter deep sleep mode. */
                BOARD_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
                break;
            case kPower_DeepSleepRetention:
                BOARD_RequestDSR(APP_EXCLUDE_FROM_DSR);
                BOARD_RestorePeripheralsAfterDSR(); /* Restore the peripherals whose states are not retained in DSR. */
                break;
            case kPower_DeepPowerDown: /* Request deep power down(DPD) mode. The SOC enters DPD only when both domains
                                          requested DPD mode. */
                DEMO_LOG(
                    "The chip enters DPD only when both domains request entering DPD mode. Wakeup the device by "
                    "reset\r\n\r\n");
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
                BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
                BOARD_RequestDPD(APP_EXCLUDE_FROM_DEEP_POWERDOWN);
                /* After deep power down wakeup, the code will restart and cannot reach here. */
                break;
            case kPower_FullDeepPowerDown: /* Request full deep power down(FDPD) mode. The SOC enters FDPD only when
                                              both domains requested FDPD mode. */
                DEMO_LOG(
                    "The chip enters FDPD only when both domains request entering FDPD mode. Wakeup the device by "
                    "reset\r\n\r\n");
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
                BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
                BOARD_RequestFDPD(APP_EXCLUDE_FROM_FULL_DEEP_POWERDOWN);
                /* After full deep power down wakeup, the code will restart and cannot reach here. */
                break;
            default:
                break;
        }

        DEMO_LOG("Wakeup.\r\n");
    }
}
