/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_pca9422.h"
#include "pmic_support.h"
#include "core1_support.h"
#include "fsl_cache.h"
#include "fsl_mu.h"
#include "power_demo_config.h"
#include "fsl_iopctl.h"
#include "fsl_irtc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_NORMAL_RUN_VOLT 900000U /* The VDD2 voltage during normal run. */
#define DEMO_LOW_POWER_RUN_VOLT                                                                            \
    700000U                          /* The VDD2 voltage during low power run. Used for CPU0 DS while CPU1 \
                                        active.*/
#define APP_POWER_NAME                                                                           \
    {                                                                                            \
        "Sleep", "Deep Sleep", "Deep Sleep Retention", "Deep Power Down", "Full Deep Power Down" \
    }
#define APP_TARGET_POWER_NUM (5U)

#define APP_SLEEP_CONSTRAINTS                                                                                       \
    17U, PM_RESC_COMP_MAINCLK_ON, PM_RESC_SENSES_MAINCLK_ON, PM_RESC_COMN_MAINCLK_ON, PM_RESC_VNCOM_ON,             \
        PM_RESC_FRO0_ON, PM_RESC_FRO0_EN, PM_RESC_LPOSC_ON, PM_RESC_SRAM8_256KB_ACTIVE, PM_RESC_SRAM9_256KB_ACTIVE, \
        PM_RESC_SRAM10_512KB_ACTIVE, PM_RESC_SRAM11_512KB_ACTIVE, PM_RESC_SRAM12_1MB_ACTIVE,                        \
        PM_RESC_SRAM13_1MB_ACTIVE, PM_RESC_SRAM_CPU0_ICACHE_ACTIVE, PM_RESC_SRAM_CPU0_DCACHE_ACTIVE,                \
        PM_RESC_SRAM_XSPI0_ACTIVE, PM_RESC_SRAM_OCOTP_RETENTION

#define APP_DEEP_SLEEP_CONSTRAINTS                                                                    \
    9U, PM_RESC_SRAM8_256KB_RETENTION, PM_RESC_SRAM9_256KB_RETENTION, PM_RESC_SRAM10_512KB_RETENTION, \
        PM_RESC_SRAM11_512KB_RETENTION, PM_RESC_SRAM12_1MB_RETENTION, PM_RESC_SRAM13_1MB_RETENTION,   \
        PM_RESC_SRAM_CPU0_ICACHE_RETENTION, PM_RESC_SRAM_CPU0_DCACHE_RETENTION, PM_RESC_SRAM_OCOTP_RETENTION

/* CPU0 cache is not retented during DSR. */
#define APP_DSR_CONSTRAINTS                                                                           \
    7U, PM_RESC_SRAM8_256KB_RETENTION, PM_RESC_SRAM9_256KB_RETENTION, PM_RESC_SRAM10_512KB_RETENTION, \
        PM_RESC_SRAM11_512KB_RETENTION, PM_RESC_SRAM12_1MB_RETENTION, PM_RESC_SRAM13_1MB_RETENTION,   \
        PM_RESC_SRAM_OCOTP_RETENTION

#define APP_DEEP_POWER_DOWN_CONSTRAINTS 0U

#define APP_FULL_DEEP_POWER_DOWN_CONSTRAINTS 0U

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
#define APP_RTC_IRQN        RTC0_IRQn
#define APP_RTC_IRQ_HANDLER RTC0_IRQHandler

#define APP_MU MU1_MUA

/************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_RestorePeripheralsAfterDSR(void);
void BOARD_DisablePll(void);
void BOARD_RestorePll(void);
void DEMO_InitDebugConsole(void);
void DEMO_DeinitDebugConsole(void);

status_t APP_ControlCallback_notify(pm_event_type_t eventType, uint8_t powerState, void *data);
void APP_InitWakeupSource(void);
void APP_RegisterNotify(void);
uint32_t APP_GetWakeupTimeout(void);
void APP_SetConstraints(uint8_t powerMode);
void APP_ReleaseConstraints(uint8_t powerMode);

static uint8_t APP_GetTargetPowerMode(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern pm_handle_t g_pmHandle;
extern uint8_t g_targetPowerMode;

AT_ALWAYS_ON_DATA(uint32_t mainDiv);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_UserkeyWakeupSource);
AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_timerWakeupSource);

AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify1) = {
    .notifyCallback = APP_ControlCallback_notify,
    .data           = NULL,
};
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHandle);
AT_ALWAYS_ON_DATA(uint8_t g_targetPowerMode);
AT_ALWAYS_ON_DATA(uint32_t g_irqMask);
static const char *const g_targetPowerNameArray[APP_TARGET_POWER_NUM] = APP_POWER_NAME;

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
            cfg[mode].sw1OutVolt  = 630000U;
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
        /* PMIC_MODE[1:0] = 11b */
        case kPCA9422_DPStandbyMode:
            cfg[mode].sw1OutVolt  = 500000U;
            cfg[mode].sw2OutVolt  = 1000000U;
            cfg[mode].sw3OutVolt  = 500000U;
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
    cfg.sw1OutEnMode = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.sw3OutEnMode = kPCA9422_EnmodeOnActiveSleepStandby;
#else
    cfg.sw1OutEnMode = kPCA9422_EnmodeOnActive;
    cfg.sw3OutEnMode = kPCA9422_EnmodeOnActive;
#endif
    cfg.sw2OutEnMode  = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.sw4OutEnMode  = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.ldo1OutEnMode = kPCA9422_EnmodeOnAll;
    cfg.ldo2OutEnMode = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.ldo3OutEnMode = kPCA9422_EnmodeOnActiveSleepStandby;
    cfg.ldo4OutEnMode = kPCA9422_EnmodeOnActiveSleepStandby;

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
    CLOCK_DisableClock(kCLOCK_CompAccessRamArbiter1);
    CLKCTL0->RAMCLKSEL = 0;          /* Sense access RAM arbiter0 clock. */
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

    POWER_EnablePD(kPDRUNCFG_SHUT_RAM0_CLK); /* Sense access RAM arbiter0 clock. */
    POWER_EnablePD(kPDRUNCFG_SHUT_RAM1_CLK); /* Compute access RAM arbiter1 clock. */

    POWER_EnableRunAFBB(kPower_BodyBiasVdd2 |
                        kPower_BodyBiasVddn); /* For run mode, AFBB for VDD2 and NBB for VDD2SRAM should be used. */
    POWER_EnableRunNBB(kPower_BodyBiasVdd2Sram);
    POWER_EnableRunRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd1Sram);
    POWER_EnableSleepRBB(kPower_BodyBiasVddn | kPower_BodyBiasVdd2Sram | kPower_BodyBiasVdd2 | kPower_BodyBiasVdd1 |
                         kPower_BodyBiasVdd1Sram);

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
    POWER_SelectSleepSetpoint(kRegulator_Vdd2LDO, 0U);
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
#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
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

    /* Low power handshake to for async interrupt. */
    if (POWER_ModuleEnterLPRequest(APP_SW_LP_REQ) != kStatus_Success)
    {
        assert(false);
    }

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

#if defined(DEMO_POWER_SUPPLY_OPTION) && (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
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

void APP_PrepareForSleep(uint8_t powerState)
{
    switch (powerState)
    {
        case PM_LP_STATE_SLEEP:
        {
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            mainDiv = BOARD_PrepareForDS();
            break;
        }
        case PM_LP_STATE_DSR:
        {
            mainDiv = BOARD_PrepareForDS();
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
#if DEMO_POWER_USE_PLL
            BOARD_DisablePll();
#endif
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
            BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
            break;
        }
        case PM_LP_STATE_FULL_DEEP_POWER_DOWN:
        {
#if DEMO_POWER_USE_PLL
            BOARD_DisablePll();
#endif
#if (DEMO_POWER_SUPPLY_OPTION == DEMO_POWER_SUPPLY_PMIC)
            BOARD_SetPmicVoltageBeforeDeepPowerDown();
#endif
            break;
        }
        default:
            /*No action need to be done. */
            break;
    }
}

void APP_RestoreAfterSleep(uint8_t powerState)
{
    switch (powerState)
    {
        case PM_LP_STATE_SLEEP:
        {
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            BOARD_RestoreAfterDS(mainDiv);
            break;
        }
        case PM_LP_STATE_DSR:
        {
            BOARD_RestoreAfterDS(mainDiv);
            BOARD_RestorePeripheralsAfterDSR();
            break;
        }

        default:
            /*No action need to be done. */
            break;
    }
}

status_t APP_ControlCallback_notify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        PRINTF("APP_ControlCallback ENTERING LP mode %d -- Notify.\r\n", powerState);
        APP_PrepareForSleep(powerState);
    }
    else
    {
        APP_RestoreAfterSleep(powerState);
        PRINTF("APP_ControlCallback EXITING LP mode %d -- Notify.\r\n", powerState);
    }

    return kStatus_Success;
}

void APP_SW_IRQ_HANDLER(void)
{
    POWER_ModuleExitLPRequest(APP_SW_LP_REQ); /* Clear the low power request before accessing the GPIO. */
    GPIO_GpioClearInterruptFlags(APP_USER_WAKEUP_KEY_GPIO, 1U << APP_USER_WAKEUP_KEY_PIN);
}

void APP_RTC_IRQ_HANDLER(void)
{
    uint32_t flags = IRTC_GetStatusFlags(APP_RTC);
    if (0U != flags)
    {
        /* Unlock to allow register write operation */
        IRTC_SetWriteProtection(APP_RTC, false);
        /*Clear all irtc flag */
        IRTC_ClearStatusFlags(APP_RTC, flags);
    }
}

void APP_StartTimer(uint64_t timeOutTickes)
{
    if (timeOutTickes != 0U)
    {
        IRTC_SetWriteProtection(APP_RTC, false);
        IRTC_ClearStatusFlags(APP_RTC, kIRTC_WakeTimerFlag);
        IRTC_EnableInterrupts(APP_RTC, kIRTC_WakeTimerInterruptEnable);
        IRTC_SetWakeupCount(APP_RTC, true, (timeOutTickes + 1023UL) / 1024UL);
    }
}

void APP_StopTimer(void)
{
    IRTC_SetWriteProtection(APP_RTC, false);
    IRTC_DisableInterrupts(APP_RTC, kIRTC_WakeTimerInterruptEnable);
}

void APP_InitWakeupSource(void)
{
    gpio_pin_config_t gpioPinConfigStruct;
    irtc_config_t irtcConfig;

    PM_InitWakeupSource(&g_UserkeyWakeupSource, (uint32_t)APP_SW_IRQN, NULL, true);
    PM_InitWakeupSource(&g_timerWakeupSource, (uint32_t)APP_RTC_IRQN, NULL, true);
    PM_RegisterTimerController(&g_pmHandle, APP_StartTimer, APP_StopTimer, NULL, NULL);

    /* Set SW pin as GPIO input. */
    gpioPinConfigStruct.pinDirection = kGPIO_DigitalInput;
    gpioPinConfigStruct.outputLogic  = 0U;

    RESET_ClearPeripheralReset(APP_SW_RESET_RSTn);
    CLOCK_EnableClock(APP_SW_CLOCK_EN);

    GPIO_SetPinInterruptConfig(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, kGPIO_InterruptFallingEdge);
    GPIO_PinInit(APP_USER_WAKEUP_KEY_GPIO, APP_USER_WAKEUP_KEY_PIN, &gpioPinConfigStruct);

    IRTC_GetDefaultConfig(&irtcConfig);
    if (IRTC_Init(APP_RTC, &irtcConfig) != kStatus_Success)
    {
        PRINTF("RTC Init Failed.\r\n");
    }
}

uint32_t APP_GetWakeupTimeout(void)
{
    uint8_t timeout;
    uint32_t timeoutTicks = 0U;

    if (g_targetPowerMode < PM_LP_STATE_DSR)
    {
        PRINTF("\r\n Press User button(SW5) to wakeup from Sleep or DeepSleep mode.\r\n");
        timeoutTicks = 0U;
    }
    else if (g_targetPowerMode == PM_LP_STATE_DSR)
    {
        while (1)
        {
            PRINTF("\r\n Select the wake up timeout in seconds.");
            PRINTF("\r\n The allowed range is 0s - 9s.");
            PRINTF("\r\n Eg. enter 0 means no timeout and need reset to wakeup.");
            PRINTF("\r\n Eg. enter 5 to wake up in 5 seconds.");
            PRINTF("\r\n Waiting for input timeout value...");

            timeout = GETCHAR();
            if ((timeout >= '0') && (timeout <= '9'))
            {
                timeout -= '0';
                if (timeout != 0U)
                {
                    PRINTF("\r\n Will wakeup in %d seconds.", timeout);
                    timeoutTicks = (timeout * 1000000UL);
                }
                else
                {
                    timeoutTicks = 0U;
                }
                return timeoutTicks;
            }
            PRINTF("\r\n Wrong value!");
        }
    }
    else
    {
        PRINTF("\r\n The chip enters Deep Power Down or Full Deep Power Down mode only when both cores requested the mode.\r\n");
        PRINTF(" Use reset button to wakeup from Deep Power Down or Full Deep Power Down.\r\n\r\n");
    }

    return timeoutTicks;
}

void APP_RegisterNotify(void)
{
    PM_RegisterNotify(kPM_NotifyGroup2, &g_notify1);
}

void APP_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_SLEEP:
        {
            PM_SetConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DSR:
        {
            PM_SetConstraints(PM_LP_STATE_DSR, APP_DSR_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            PM_SetConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_FULL_DEEP_POWER_DOWN:
        {
            PM_SetConstraints(PM_LP_STATE_FULL_DEEP_POWER_DOWN, APP_FULL_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        default:
        {
            /* This branch will never be hit. */
            break;
        }
    }
}

void APP_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_SLEEP:
        {
            PM_ReleaseConstraints(PM_LP_STATE_SLEEP, APP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_SLEEP:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_SLEEP, APP_DEEP_SLEEP_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DSR:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DSR, APP_DSR_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_DEEP_POWER_DOWN:
        {
            PM_ReleaseConstraints(PM_LP_STATE_DEEP_POWER_DOWN, APP_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_FULL_DEEP_POWER_DOWN:
        {
            PM_ReleaseConstraints(PM_LP_STATE_FULL_DEEP_POWER_DOWN, APP_FULL_DEEP_POWER_DOWN_CONSTRAINTS);
            break;
        }
        default:
        {
            /* This branch should never be hit. */
            break;
        }
    }
}


int main(void)
{
    uint32_t timeoutUs = 0UL;

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

    if ((POWER_GetEventFlags() & PMC_FLAGS_DEEPPDF_MASK) != 0)
    {
        PRINTF("Board wake up from deep or full deep power down mode.\r\n");
        POWER_ClearEventFlags(PMC_FLAGS_DEEPPDF_MASK);
    }
    POWER_ClearEventFlags(0xFFFFFFFF);
    PRINTF("\r\nPower Manager Test.\r\n");
    PRINTF("\r\nNormal Boot.\r\n");
    PM_CreateHandle(&g_pmHandle);

    APP_RegisterNotify();
    APP_InitWakeupSource();
    while (1)
    {
        g_targetPowerMode = APP_GetTargetPowerMode();
        if (g_targetPowerMode >= APP_TARGET_POWER_NUM)
        {
            PRINTF("\r\nWrong Input! Please reselect.\r\n");
            continue;
        }
        PRINTF("Selected to enter %s.\r\n", g_targetPowerNameArray[(uint8_t)g_targetPowerMode]);
        timeoutUs = APP_GetWakeupTimeout();
        APP_SetConstraints(g_targetPowerMode);
        g_irqMask = DisableGlobalIRQ();
        PM_EnablePowerManager(true);
        PM_EnterLowPower(timeoutUs);
        PM_EnablePowerManager(false);
        EnableGlobalIRQ(g_irqMask);
        APP_ReleaseConstraints(g_targetPowerMode);
        PRINTF("\r\nNext Loop\r\n");
    }
}

static uint8_t APP_GetTargetPowerMode(void)
{
    uint32_t i;
    uint8_t ch;
    uint8_t g_targetPowerModeIndex;

    PRINTF("\r\nPlease select the desired power mode:\r\n");
    for (i = 0UL; i < APP_TARGET_POWER_NUM; i++)
    {
        PRINTF("\tPress %c to enter: %s\r\n", ('A' + i), g_targetPowerNameArray[i]);
    }

    PRINTF("\r\nWaiting for power mode select...\r\n\r\n");

    ch = GETCHAR();

    if ((ch >= 'a') && (ch <= 'z'))
    {
        ch -= 'a' - 'A';
    }

    g_targetPowerModeIndex = ch - 'A';

    return g_targetPowerModeIndex;
}
