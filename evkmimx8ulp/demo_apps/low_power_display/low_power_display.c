/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "fsl_rgpio.h"
#include "fsl_lptmr.h"
#include "fsl_upower.h"
#include "fsl_lcdif.h"
#include "fsl_reset.h"
#include "fsl_cache.h"
#include "fsl_mu.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
#include "app_srtm.h"
#include "lpm.h"
#include "low_power_display.h"
#include "fsl_rtd_cmc.h"
#include "fsl_sentinel.h"
#include "fsl_rgpio.h"
#include "fsl_wuu.h"
#include "lcdif_support.h"

#include "fsl_iomuxc.h"
#include "fsl_lpuart.h"
#include "fsl_trdc.h"
#include "fsl_flexspi.h"
#include "fsl_fusion.h"
/*******************************************************************************
 * Struct Definitions
 ******************************************************************************/
#define APP_DEBUG_UART_BAUDRATE       (115200U)             /* Debug console baud rate. */
#define APP_DEBUG_UART_DEFAULT_CLKSRC kCLOCK_IpSrcSircAsync /* SCG SIRC clock. */

/* LPTMR0 is WUU internal module 0. */
#define WUU_MODULE_SYSTICK WUU_MODULE_LPTMR0
/* Allow systick to be a wakeup source in Power Down mode. */
#define SYSTICK_WUU_WAKEUP_EVENT (kWUU_InternalModuleInterrupt)

#define APP_LPTMR1_IRQ_PRIO (5U)
#define WUU_WAKEUP_PIN_IDX     (24U) /* WUU0_P24 used for RTD Button2 (SW8) */
#define WUU_WAKEUP_PIN_TYPE    kWUU_ExternalPinFallingEdge
#define APP_WAKEUP_BUTTON_NAME "RTD BUTTON2 (SW8)"

#define DEMO_IMG_HEIGHT     DEMO_PANEL_HEIGHT
#define DEMO_IMG_WIDTH      DEMO_PANEL_WIDTH
#define DEMO_BYTE_PER_PIXEL 2

#define DEMO_MAKE_COLOR(red, green, blue) \
    ((((uint16_t)(red)&0xF8U) << 8U) | (((uint16_t)(green)&0xFCU) << 3U) | (((uint16_t)(blue)&0xF8U) >> 3U))

#define DEMO_COLOR_BLACK DEMO_MAKE_COLOR(0, 0, 0)
#define DEMO_COLOR_RED   DEMO_MAKE_COLOR(255, 0, 0)
#define DEMO_COLOR_GREEN DEMO_MAKE_COLOR(0, 255, 0)
#define DEMO_COLOR_BLUE  DEMO_MAKE_COLOR(0, 0, 255)

#define EXAMPLE_FLEXSPI            BOARD_FLEXSPI_PSRAM
#define EXAMPLE_FLEXSPI_AMBA_BASE0 FlexSPI1_AMBA_BASE

typedef enum _app_wakeup_source
{
    kAPP_WakeupSourceLptmr, /*!< Wakeup by LPTMR.        */
    kAPP_WakeupSourcePin    /*!< Wakeup by external pin. */
} app_wakeup_source_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
status_t BOARD_InitPsRam_Fro(void);
void BOARD_SwitchToFRO_ForSleep(void);
extern void APP_PowerPreSwitchHook(lpm_rtd_power_mode_e targetMode);
extern void APP_PowerPostSwitchHook(lpm_rtd_power_mode_e targetMode, bool result);
extern void APP_SRTM_WakeupCA35(void);
extern void APP_RebootCA35(void);
extern void APP_ShutdownCA35(void);
extern void APP_BootCA35(void);
extern void UPOWER_InitBuck2Buck3Table(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_wakeupTimeout;           /* Wakeup timeout. (Unit: Second) */
static app_wakeup_source_t s_wakeupSource; /* Wakeup source.                 */
static SemaphoreHandle_t s_wakeupSig;
static const char *s_modeNames[] = {"ACTIVE", "WAIT", "STOP", "Sleep", "Deep Sleep", "Power Down", "Deep Power Down"};
extern lpm_ad_power_mode_e AD_CurrentMode;
extern bool option_v_boot_flag;
extern lpm_rtd_power_mode_e s_curMode;
extern lpm_rtd_power_mode_e s_lastMode;
extern pca9460_buck3ctrl_t buck3_ctrl;
extern pca9460_ldo1_cfg_t ldo1_cfg;
extern bool wake_acore_flag;
rtd_mode_and_irq_allow_t current_state = {LPM_PowerModeActive, LPM_PowerModeActive, NotAvail_IRQn, RTD_GIVE_SIG_YES};
static uint32_t s_frameBufferAddr[3]   = {EXAMPLE_FLEXSPI_AMBA_BASE0, EXAMPLE_FLEXSPI_AMBA_BASE0 + 0x200000U,
                                          EXAMPLE_FLEXSPI_AMBA_BASE0 + 0x400000U};
// clang-format off
/*
 * For some system low power combine, such as APD->PD, RTD->PD, use GPIO as RTD wakeup source, it will trigger WUU + GPIO irq handler in RTD side, release twice Semaphore Sig.
 * In below mode_combi_array_for_give_sig table, use current_rtd_mode and irq_num to judge whether RTD give semaphore sig.
 * Use current_rtd_mode and last_rtd_mode to judge whether RTD can wakeup APD.
 */
rtd_mode_and_irq_allow_t mode_combi_array_for_give_sig[] = {
  {LPM_PowerModeActive, LPM_PowerModeActive, NotAvail_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeDeepSleep, LPM_PowerModeActive, WUU0_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModePowerDown, LPM_PowerModeActive, WUU0_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeDeepPowerDown, LPM_PowerModeActive, WUU0_IRQn, RTD_GIVE_SIG_YES},

  {LPM_PowerModeWait, LPM_PowerModeActive, LPTMR1_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeStop, LPM_PowerModeActive, LPTMR1_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeSleep, LPM_PowerModeActive, LPTMR1_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeDeepSleep, LPM_PowerModeActive, LPTMR1_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModePowerDown, LPM_PowerModeActive, LPTMR1_IRQn, RTD_GIVE_SIG_YES},

  {LPM_PowerModeWait, LPM_PowerModeActive, GPIOB_INT0_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeStop, LPM_PowerModeActive, GPIOB_INT0_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeSleep, LPM_PowerModeActive, GPIOB_INT0_IRQn, RTD_GIVE_SIG_YES},
  {LPM_PowerModeDeepSleep, LPM_PowerModeActive, GPIOB_INT0_IRQn, RTD_GIVE_SIG_YES},
};

mode_combi_t mode_combi_array_for_single_boot[] = {
    {LPM_PowerModeActive, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeWait, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeStop, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeSleep, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeDeepSleep, AD_ACT, MODE_COMBI_NO},
    {LPM_PowerModePowerDown, AD_ACT, MODE_COMBI_NO},
    {LPM_PowerModeDeepPowerDown, AD_ACT, MODE_COMBI_NO},

    {LPM_PowerModeActive, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeWait, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeStop, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeSleep, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeDeepSleep, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModePowerDown, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeDeepPowerDown, AD_PD, MODE_COMBI_NO},

    {LPM_PowerModeActive, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeWait, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeStop, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeSleep, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeDeepSleep, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModePowerDown, AD_DPD, MODE_COMBI_YES},
    /* RTD not support Deep Power Down Mode when boot type is SINGLE BOOT TYPE */
    {LPM_PowerModeDeepPowerDown, AD_DPD, MODE_COMBI_NO},
};

mode_combi_t mode_combi_array_for_dual_or_lp_boot[] = {
    {LPM_PowerModeActive, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeWait, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeStop, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeSleep, AD_ACT, MODE_COMBI_YES},
    {LPM_PowerModeDeepSleep, AD_ACT, MODE_COMBI_NO},
    {LPM_PowerModePowerDown, AD_ACT, MODE_COMBI_NO},
    {LPM_PowerModeDeepPowerDown, AD_ACT, MODE_COMBI_NO},

    {LPM_PowerModeActive, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeWait, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeStop, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeSleep, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeDeepSleep, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModePowerDown, AD_PD, MODE_COMBI_YES},
    {LPM_PowerModeDeepPowerDown, AD_PD, MODE_COMBI_NO},

    {LPM_PowerModeActive, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeWait, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeStop, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeSleep, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeDeepSleep, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModePowerDown, AD_DPD, MODE_COMBI_YES},
    {LPM_PowerModeDeepPowerDown, AD_DPD, MODE_COMBI_YES},
};
// clang-format on

/*******************************************************************************
 * Function Code
 ******************************************************************************/
extern lpm_ad_power_mode_e AD_CurrentMode;
extern pca9460_buck3ctrl_t buck3_ctrl;
extern pca9460_ldo1_cfg_t ldo1_cfg;
extern cgc_rtd_sys_clk_config_t g_sysClkConfigFroSource;
extern cgc_sosc_config_t g_cgcSysOscConfig;
static uint32_t iomuxBackup[25 + 16 + 24]; /* Backup 25 PTA, 16 PTB and 24 PTC IOMUX registers */
static uint32_t gpioICRBackup[25 + 16 + 24];


static void APP_Suspend(void)
{
    uint32_t i;
    uint32_t setting;
    uint32_t backupIndex;
    lpm_rtd_power_mode_e targetPowerMode = LPM_GetPowerMode();

    backupIndex = 0;

    /* Backup PTA IOMUXC and GPIOA ICR registers then disable */
    for (i = 0; i <= 24; i++)
    {
        iomuxBackup[backupIndex] = IOMUXC0->PCR0_IOMUXCARRAY0[i];

        gpioICRBackup[backupIndex] = GPIOA->ICR[i];

        GPIOA->ICR[i] = 0; /* Disable interrupts */

                           /* Skip PTA3 for MIPI DSI backlight */
        if (3 != i)
        {
            IOMUXC0->PCR0_IOMUXCARRAY0[i] = 0;
        }
        backupIndex++;
    }

    /* Backup PTB IOMUXC and GPIOB ICR registers then disable */
    for (i = 0; i <= 15; i++)
    {
        iomuxBackup[backupIndex] = IOMUXC0->PCR0_IOMUXCARRAY1[i];

        gpioICRBackup[backupIndex] = GPIOB->ICR[i];

        GPIOB->ICR[i] = 0; /* disable interrupts */

        /*
         * If it's wakeup source, need to set as WUU0_P24
         * Power Down/Deep Power Down wakeup through WUU and NMI pin
         * Sleep/Deep Sleep wakeup via interrupt from M33 peripherals or external GPIO pins. WIC detects wakeup source.
         */
        if ((i == 12) && (WUU0->PE2 & WUU_PE2_WUPE24_MASK))
        {
            if (targetPowerMode == LPM_PowerModeDeepSleep)
            {
                /*
                 * Deep Sleep wakeup via interrupt not WUU,
                 * so do nothing in here for Deep Sleep Mode
                 */
                /* enable interrupts for PTB12 */
                GPIOB->ICR[i] = gpioICRBackup[backupIndex];
            }
            else
            {
                /*
                 * Disable interrupt temperarily to prevent glitch
                 * interrupt during switching IOMUXC pin selection
                 */
                setting = WUU0->PE2 & WUU_PE2_WUPE24_MASK;
                WUU0->PE2 &= !WUU_PE2_WUPE24_MASK;

                /* Change PTB12's function as WUU0_P24(IOMUXC_PTB12_WUU0_P24) */
                IOMUXC0->PCR0_IOMUXCARRAY1[i] = IOMUXC0_PCR0_IOMUXCARRAY1_MUX(13);

                WUU0->PE2 |= setting;
            }
        }
        else if ((i != 10) && (i != 11)) /* PTB10 and PTB11 is used as i2c function by upower */
        {
            IOMUXC0->PCR0_IOMUXCARRAY1[i] = 0;
        }
        backupIndex++;
    }

    /* Backup PTC IOMUXC and GPIOC ICR registers then disable */
    for (i = 0; i <= 23; i++)
    {
        iomuxBackup[backupIndex] = IOMUXC0->PCR0_IOMUXCARRAY2[i];

        gpioICRBackup[backupIndex] = GPIOC->ICR[i];

        GPIOC->ICR[i] = 0; /* disable interrupts */

                           /* Skip PTC12 ~ 23 for MIPI DSI and FlexSPI1 pinmux */
        if (i < 12)
        {
            IOMUXC0->PCR0_IOMUXCARRAY2[i] = 0;
        }

        backupIndex++;
    }
}

static void APP_Resume(bool resume)
{
    uint32_t i;
    uint32_t backupIndex;

    backupIndex = 0;

    /* Restore PTA IOMUXC and GPIOA ICR registers */
    for (i = 0; i <= 24; i++)
    {
        IOMUXC0->PCR0_IOMUXCARRAY0[i] = iomuxBackup[backupIndex];
        GPIOA->ICR[i]                 = gpioICRBackup[backupIndex];
        backupIndex++;
    }

    /* Restore PTB IOMUXC and GPIOB ICR registers */
    for (i = 0; i <= 15; i++)
    {
        IOMUXC0->PCR0_IOMUXCARRAY1[i] = iomuxBackup[backupIndex];
        GPIOB->ICR[i]                 = gpioICRBackup[backupIndex];
        backupIndex++;
    }

    /* Restore PTC IOMUXC and GPIOC ICR registers */
    for (i = 0; i <= 23; i++)
    {
        IOMUXC0->PCR0_IOMUXCARRAY2[i] = iomuxBackup[backupIndex];
        GPIOC->ICR[i]                 = gpioICRBackup[backupIndex];
        backupIndex++;
    }
}

/* Disable gpio to save power */
void APP_DisableGPIO(void)
{
    int i = 0;

    /* Disable PTA and set PTA to Analog/HiZ state to save power */
    for (i = 0; i <= 24; i++)
    {
        GPIOA->ICR[i] = 0; /* Disable interrupts */

        /*
         * Skip PTA3 for MIPI panel TPM backlight
         * DSL mode
         */
        if (3 != i)
        {
            IOMUXC0->PCR0_IOMUXCARRAY0[i] = 0; /* Set to Analog/HiZ state */
        }
    }

    /* Disable PTB and set PTB to Analog/HiZ state to save power */
    for (i = 0; i <= 15; i++)
    {
        if ((i != 10) && (i != 11))            /* PTB10 and PTB11 is used as i2c function by upower */
        {
            GPIOB->ICR[i]                 = 0; /* Disable interrupts */
            IOMUXC0->PCR0_IOMUXCARRAY1[i] = 0; /* Set to Analog/HiZ state */
        }
    }

    /* Disable PTC and set PTC to Analog/HiZ state to save power */
    for (i = 0; i <= 23; i++)
    {
        GPIOC->ICR[i] = 0; /* Disable interrupts */

                           /* Skip PTC12 ~ 23 for MIPI DSI and PSRAM */
        if (i < 12)
        {
            IOMUXC0->PCR0_IOMUXCARRAY2[i] = 0; /* Set to Analog/HiZ state */
        }
    }
}

void APP_PowerPreSwitchHook(lpm_rtd_power_mode_e targetMode)
{
    uint32_t setting;

    if ((LPM_PowerModeActive != targetMode))
    {
        /* Wait for debug console output finished. */
        while (!(kLPUART_TransmissionCompleteFlag & LPUART_GetStatusFlags((LPUART_Type *)BOARD_DEBUG_UART_BASEADDR)))
        {
        }
        DbgConsole_Deinit();
        /*
         * Set pin for current leakage.
         * Debug console RX pin: Set to pinmux to analog.
         * Debug console TX pin: Set to pinmux to analog.
         */
        IOMUXC_SetPinMux(IOMUXC_PTA10_LPUART1_TX, 0);
        IOMUXC_SetPinConfig(IOMUXC_PTA10_LPUART1_TX, 0);
        IOMUXC_SetPinMux(IOMUXC_PTA11_LPUART1_RX, 0);
        IOMUXC_SetPinConfig(IOMUXC_PTA11_LPUART1_RX, 0);

        if (LPM_PowerModePowerDown == targetMode || LPM_PowerModeDeepSleep == targetMode ||
            LPM_PowerModeSleep == targetMode)
        {
            APP_Suspend();
        }
        else if (LPM_PowerModeDeepPowerDown == targetMode)
        {
            APP_DisableGPIO();
            /* If PTB12 is wakeup source, set to WUU0_P24 */
            if ((WUU0->PE2 & WUU_PE2_WUPE24_MASK) != 0)
            {
                /* Disable interrupt temperarily to prevent glitch
                 * interrupt during switching IOMUXC pin selection
                 */
                setting = WUU0->PE2 & WUU_PE2_WUPE24_MASK;
                WUU0->PE2 &= !WUU_PE2_WUPE24_MASK;

                IOMUXC0->PCR0_IOMUXCARRAY1[12] = IOMUXC0_PCR0_IOMUXCARRAY0_MUX(13);

                WUU0->PE2 |= setting;
            }

            /* Cleare any potential interrupts before enter Deep Power Down */
            WUU0->PF = WUU0->PF;
        }
    }
}

void APP_PowerPostSwitchHook(lpm_rtd_power_mode_e targetMode, bool result)
{
    if (LPM_PowerModeActive != targetMode)
    {
        if (LPM_PowerModePowerDown == targetMode || LPM_PowerModeDeepSleep == targetMode ||
            LPM_PowerModeSleep == targetMode)
        {
            APP_Resume(result);
        }

        /*
         * Debug console RX pin was set to disable for current leakage, need to re-configure pinmux.
         * Debug console TX pin was set to disable for current leakage, need to re-configure pinmux.
         */
        IOMUXC_SetPinMux(IOMUXC_PTA10_LPUART1_TX, 0U);
        IOMUXC_SetPinConfig(IOMUXC_PTA10_LPUART1_TX, IOMUXC_PCR_PE_MASK | IOMUXC_PCR_PS_MASK);
        IOMUXC_SetPinMux(IOMUXC_PTA11_LPUART1_RX, 0U);
        IOMUXC_SetPinConfig(IOMUXC_PTA11_LPUART1_RX, IOMUXC_PCR_PE_MASK | IOMUXC_PCR_PS_MASK);

        BOARD_InitClock(); /* initialize system osc for uart(using osc as clock source) */
        BOARD_InitDebugConsole();
    }
    PRINTF("== Power switch %s ==\r\n", result ? "OK" : "FAIL");
    /* Reinitialize TRDC */
    if (AD_CurrentMode == AD_PD)
    {
        BOARD_SetTrdcGlobalConfig();
    }
    else if (AD_CurrentMode == AD_DPD)
    {
        /*
         * For oem_open lifecycle and closed lifecycle part,
         * there are different default setting,
         * Sentinel will apply the default settings after sentinel exit from power down mode.
         * If the sillicon is in oem_open lifecycle, sentinel apply default settings A.
         * If the sillicon is in closed lifecycle, sentinel apply default settings B.
         * A and B is different.
         * So whatever sillicon is in which lifecycle,
         * change default settings directly.
         */
        BOARD_SetTrdcGlobalConfig();
        BOARD_SetTrdcAfterApdReset();
    }
}

static status_t flexspi_hyper_ram_get_mcr(FLEXSPI_Type *base, uint8_t regAddr, uint32_t *mrVal)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Read data */
    flashXfer.deviceAddress = regAddr;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = 2;
    flashXfer.data          = mrVal;
    flashXfer.dataSize      = 2;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

static status_t flexspi_hyper_ram_write_mcr(FLEXSPI_Type *base, uint8_t regAddr, uint32_t *mrVal)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = regAddr;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = 3;
    flashXfer.data          = mrVal;
    flashXfer.dataSize      = 1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

static status_t flexspi_hyper_ram_reset(FLEXSPI_Type *base)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write data */
    flashXfer.deviceAddress = 0x0U;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = 4;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    if (status == kStatus_Success)
    {
        /* for loop of 50000 is about 1ms (@200 MHz CPU) */
        for (uint32_t i = 2000000U; i > 0; i--)
        {
            __NOP();
        }
    }
    return status;
}

/* Initialize psram. */
status_t BOARD_InitPsRam_Fro(void)
{
    flexspi_device_config_t deviceconfig = {
        .flexspiRootClk       = 192000000, /* 192MHZ SPI serial clock, DDR serial clock 96M */
        .isSck2Enabled        = false,
        .flashSize            = 0x2000,    /* 64Mb/KByte */
        .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
        .CSInterval           = 5,
        .CSHoldTime           = 3,
        .CSSetupTime          = 3,
        .dataValidTime        = 1,
        .columnspace          = 0,
        .enableWordAddress    = false,
        .AWRSeqIndex          = 1,
        .AWRSeqNumber         = 1,
        .ARDSeqIndex          = 0,
        .ARDSeqNumber         = 1,
        .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
        .AHBWriteWaitInterval = 0,
        .enableWriteMask      = true,
    };

    uint32_t customLUT[64] = {
        /* Read Data */
        [0] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, 0x20, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x20),
        [1] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07, kFLEXSPI_Command_READ_DDR,
                              kFLEXSPI_8PAD, 0x04),

        /* Write Data */
        [4] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, 0xA0, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x20),
        [5] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07, kFLEXSPI_Command_WRITE_DDR,
                              kFLEXSPI_8PAD, 0x04),

        /* Read Register */
        [8] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, 0x40, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x20),
        [9] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_DUMMY_RWDS_DDR, kFLEXSPI_8PAD, 0x07, kFLEXSPI_Command_READ_DDR,
                              kFLEXSPI_8PAD, 0x04),

        /* Write Register */
        [12] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, 0xC0, kFLEXSPI_Command_RADDR_DDR, kFLEXSPI_8PAD, 0x20),
        [13] = FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_DDR, kFLEXSPI_8PAD, 0x08, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD,
                               0x00),

        /* reset */
        [16] =
            FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_8PAD, 0xFF, kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_8PAD, 0x03),

    };

    uint32_t mr0mr1[1];
    uint32_t mr4mr8[1];
    uint32_t mr0Val[1];
    uint32_t mr4Val[1];
    uint32_t mr8Val[1];
    flexspi_config_t config;
    status_t status = kStatus_Success;

    UPOWER_PowerOnMemPart(0U, (uint32_t)kUPOWER_MP1_FLEXSPI1);

    /* 192MHz * 1U / 1U = 192MHz */
    CLOCK_SetIpSrcDiv(kCLOCK_Flexspi1, kCLOCK_Pcc1PlatIpSrcFro, 0U, 0U);
    RESET_PeripheralReset(kRESET_Flexspi1);

    /* Get FLEXSPI default settings and configure the flexspi. */
    FLEXSPI_GetDefaultConfig(&config);

    /* Init FLEXSPI. */
    config.rxSampleClock = kFLEXSPI_ReadSampleClkExternalInputFromDqsPad;
    /*Set AHB buffer size for reading data through AHB bus. */
    config.ahbConfig.enableAHBPrefetch    = true;
    config.ahbConfig.enableAHBBufferable  = true;
    config.ahbConfig.enableAHBCachable    = true;
    config.ahbConfig.enableReadAddressOpt = true;
    for (uint8_t i = 1; i < FSL_FEATURE_FLEXSPI_AHB_BUFFER_COUNT - 1; i++)
    {
        config.ahbConfig.buffer[i].bufferSize = 0;
    }
    /* FlexSPI1 has total 2KB RX buffer.
     * Set GPU/Display master to use AHB Rx Buffer0.
     */
    config.ahbConfig.buffer[0].masterIndex    = 2;    /* DMA0 */
    config.ahbConfig.buffer[0].bufferSize     = 1024; /* Allocate 1KB bytes for DMA0 */
    config.ahbConfig.buffer[0].enablePrefetch = true;
    config.ahbConfig.buffer[0].priority       = 7;    /* Set DMA0 to highest priority. */
    /* All other masters use last buffer with 1KB bytes. */
    config.ahbConfig.buffer[FSL_FEATURE_FLEXSPI_AHB_BUFFER_COUNT - 1].bufferSize = 1024;
    config.enableCombination                                                     = true;
    FLEXSPI_Init(BOARD_FLEXSPI_PSRAM, &config);

    /* Configure flash settings according to serial flash feature. */
    FLEXSPI_SetFlashConfig(BOARD_FLEXSPI_PSRAM, &deviceconfig, kFLEXSPI_PortA1);

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(BOARD_FLEXSPI_PSRAM, 0, customLUT, ARRAY_SIZE(customLUT));

    /* Do software reset. */
    FLEXSPI_SoftwareReset(BOARD_FLEXSPI_PSRAM);

    /* Reset hyper ram. */
    status = flexspi_hyper_ram_reset(BOARD_FLEXSPI_PSRAM);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_hyper_ram_get_mcr(BOARD_FLEXSPI_PSRAM, 0x0, mr0mr1);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_hyper_ram_get_mcr(BOARD_FLEXSPI_PSRAM, 0x4, mr4mr8);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable RBX, burst length set to 1K. - MR8 */
    mr8Val[0] = (mr4mr8[0] & 0xFF00U) >> 8U;
    mr8Val[0] = mr8Val[0] | 0x0F;
    status    = flexspi_hyper_ram_write_mcr(BOARD_FLEXSPI_PSRAM, 0x8, mr8Val);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Set LC code to 0x04(LC=7, maximum frequency 200M) - MR0. */
    mr0Val[0] = mr0mr1[0] & 0x00FFU;
    mr0Val[0] = (mr0Val[0] & ~0x3CU) | (4U << 2U);
    status    = flexspi_hyper_ram_write_mcr(BOARD_FLEXSPI_PSRAM, 0x0, mr0Val);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Set WLC code to 0x01(WLC=7, maximum frequency 200M) - MR4. */
    mr4Val[0] = mr4mr8[0] & 0x00FFU;
    mr4Val[0] = (mr4Val[0] & ~0xE0U) | (1U << 5U);
    status    = flexspi_hyper_ram_write_mcr(BOARD_FLEXSPI_PSRAM, 0x4, mr4Val);
    if (status != kStatus_Success)
    {
        return status;
    }

    return status;
}

void BOARD_SwitchToFRO_ForSleep(void)
{
    cgc_fro_config_t cgcFroConfig = {.enableMode = kCGC_FroEnableInDeepSleep};

    /* Step 1. Configure clock. */
    CGC_RTD->FROCSR = (uint32_t)cgcFroConfig.enableMode;

    /* Step 2. Wait for FRO clock to be valid. */
    while (0UL == (CGC_RTD->FROCSR & CGC_FROCSR_FROVLD_MASK))
    {
    }

    if (!CLOCK_IsSysOscValid())
    {
        CLOCK_InitSysOsc(&g_cgcSysOscConfig);
    }

    CLOCK_SetCm33SysClkConfig(&g_sysClkConfigFroSource);
    SystemCoreClockUpdate();

    for (int i = 0; i < 100; i++)
    {
        __asm volatile("NOP");
    }
}
static inline const char *APP_GetAllowCombiName(allow_combi_e allow)
{
    switch (allow)
    {
        GEN_CASE_ENUM_NAME(MODE_COMBI_NO);
        GEN_CASE_ENUM_NAME(MODE_COMBI_YES);
        default:
            return (char *)"WRONG_MODE_COMBI";
    }
}
static inline const char *APP_GetRtdPwrModeName(lpm_rtd_power_mode_e mode)
{
    switch (mode)
    {
        GEN_CASE_ENUM_NAME(LPM_PowerModeActive);
        GEN_CASE_ENUM_NAME(LPM_PowerModeWait);
        GEN_CASE_ENUM_NAME(LPM_PowerModeStop);
        GEN_CASE_ENUM_NAME(LPM_PowerModeSleep);
        GEN_CASE_ENUM_NAME(LPM_PowerModeDeepSleep);
        GEN_CASE_ENUM_NAME(LPM_PowerModePowerDown);
        GEN_CASE_ENUM_NAME(LPM_PowerModeDeepPowerDown);
        default:
            return (char *)"WRONG_LPM_RTD_PowerMode";
    }
}

static inline const char *APP_GetAdPwrModeName(lpm_ad_power_mode_e mode)
{
    switch (mode)
    {
        GEN_CASE_ENUM_NAME(AD_UNKOWN);
        GEN_CASE_ENUM_NAME(AD_ACT);
        GEN_CASE_ENUM_NAME(AD_PD);
        GEN_CASE_ENUM_NAME(AD_DPD);
        default:
            return (char *)"WRONG_LPM_AD_PowerMode";
    }
}

static allow_combi_e APP_GetModeAllowCombi(lpm_ad_power_mode_e ad_mode, lpm_rtd_power_mode_e rtd_mode)
{
    int i               = 0;
    allow_combi_e allow = MODE_COMBI_NO;
    ;

    if (BOARD_IsSingleBootType())
    {
        for (i = 0; i < ARRAY_SIZE(mode_combi_array_for_single_boot); i++)
        {
            if ((mode_combi_array_for_single_boot[i].rtd_mode == rtd_mode) &&
                (mode_combi_array_for_single_boot[i].ad_mode == ad_mode))
            {
                allow = mode_combi_array_for_single_boot[i].allow_combi;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < ARRAY_SIZE(mode_combi_array_for_dual_or_lp_boot); i++)
        {
            if ((mode_combi_array_for_dual_or_lp_boot[i].rtd_mode == rtd_mode) &&
                (mode_combi_array_for_dual_or_lp_boot[i].ad_mode == ad_mode))
            {
                allow = mode_combi_array_for_dual_or_lp_boot[i].allow_combi;
                break;
            }
        }
    }

    return allow;
}

static void APP_ShowModeCombi(void)
{
    int i = 0;

    PRINTF("###############################################\r\n");
    PRINTF("For Single Boot Type\r\n");
    for (i = 0; i < ARRAY_SIZE(mode_combi_array_for_single_boot); i++)
    {
        PRINTF("%s + %s: %s\r\n", APP_GetAdPwrModeName(mode_combi_array_for_single_boot[i].ad_mode),
               APP_GetRtdPwrModeName(mode_combi_array_for_single_boot[i].rtd_mode),
               APP_GetAllowCombiName(mode_combi_array_for_single_boot[i].allow_combi));
    }
    PRINTF("###############################################\r\n");
    PRINTF("\r\n");
    PRINTF("\r\n");

    PRINTF("###############################################\r\n");
    PRINTF("For Dual Boot Type/Low Power Boot Type\r\n");
    for (i = 0; i < ARRAY_SIZE(mode_combi_array_for_dual_or_lp_boot); i++)
    {
        PRINTF("%s + %s: %s\r\n", APP_GetAdPwrModeName(mode_combi_array_for_dual_or_lp_boot[i].ad_mode),
               APP_GetRtdPwrModeName(mode_combi_array_for_dual_or_lp_boot[i].rtd_mode),
               APP_GetAllowCombiName(mode_combi_array_for_dual_or_lp_boot[i].allow_combi));
    }
    PRINTF("###############################################\r\n");
}

static allow_give_sig_e APP_AllowGiveSig(lpm_rtd_power_mode_e current_mode, uint32_t irq)
{
    int i                  = 0;
    allow_give_sig_e allow = false;

    for (i = 0; i < ARRAY_SIZE(mode_combi_array_for_give_sig); i++)
    {
        if ((current_mode == mode_combi_array_for_give_sig[i].current_rtd_mode) &&
            (irq == mode_combi_array_for_give_sig[i].irq_num))
        {
            allow = mode_combi_array_for_give_sig[i].give_semaphore_flag;
            break;
        }
    }
    return allow;
}

/* WUU0 interrupt handler. */
void APP_WUU0_IRQHandler(void)
{
    bool wakeup = false;

    if (WUU_GetInternalWakeupModuleFlag(WUU0, WUU_MODULE_LPTMR1))
    {
        /* Woken up by LPTMR, then clear LPTMR flag. */
        LPTMR_ClearStatusFlags(LPTMR1, kLPTMR_TimerCompareFlag);
        LPTMR_DisableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
        LPTMR_StopTimer(LPTMR1);
        wakeup = true;
    }

    if (WUU_GetExternalWakeupPinFlag(WUU0, WUU_WAKEUP_PIN_IDX))
    {
        /* Woken up by external pin. */
        WUU_ClearExternalWakeupPinFlag(WUU0, WUU_WAKEUP_PIN_IDX);
        wakeup = true;
    }

    if (WUU_GetInternalWakeupModuleFlag(WUU0, WUU_MODULE_SYSTICK))
    {
        /* Woken up by Systick LPTMR, then clear LPTMR flag. */
        LPTMR_ClearStatusFlags(SYSTICK_BASE, kLPTMR_TimerCompareFlag);
    }

    if (wakeup)
    {
        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }
}

/* LPTMR1 interrupt handler. */
void LPTMR1_IRQHandler(void)
{
    bool wakeup = false;

    if (kLPTMR_TimerInterruptEnable & LPTMR_GetEnabledInterrupts(LPTMR1))
    {
        LPTMR_ClearStatusFlags(LPTMR1, kLPTMR_TimerCompareFlag);
        LPTMR_DisableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
        LPTMR_StopTimer(LPTMR1);
        wakeup = true;
    }

    current_state.give_semaphore_flag = APP_AllowGiveSig(s_curMode, LPTMR1_IRQn);
    if (!current_state.give_semaphore_flag)
    {
        /* skip give semaphore */
        return;
    }

    if (wakeup)
    {
        xSemaphoreGiveFromISR(s_wakeupSig, NULL);
        portYIELD_FROM_ISR(pdTRUE);
    }
}

/*
 * Wakeup RTD: lptimer or button(sw8).
 *
 *              |APD at PD: option(W), sw6(on/off), sw7, sw8.
 * Wakeup APD:  |
 *              |APD at DPD: option(W), sw6(on/off).
 *
 * RTD Wait/Stop/Sleep mode: sw8 will trigger two GPIO interrupt(kRGPIO_FlagEitherEdge) determined by APD IO service.
 * RTD DSL/PD/DPD mode: sw8 will trigger one WUU + one GPIO interrupt.
 */
static void APP_IRQDispatcher(IRQn_Type irq, void *param)
{
    /* ensure that only RTD/APD is woken up at a time button */
    current_state.last_rtd_mode       = current_state.current_rtd_mode;
    current_state.current_rtd_mode    = s_curMode;
    current_state.irq_num             = irq;
    current_state.give_semaphore_flag = RTD_GIVE_SIG_YES;

    if (current_state.current_rtd_mode == current_state.last_rtd_mode ||
        irq == BBNSM_IRQn) /* Always set the wake_acore_flag to true for the interrupt from BBNSM */
    {
        /*
         * RTD don't update low power state by using button wakeup APD.
         * so only when the last state is same with current state, button sw8 can wakeup APD.
         */
        wake_acore_flag = true;
    }
    else
    {
        wake_acore_flag = false;
    }

    current_state.give_semaphore_flag = APP_AllowGiveSig(s_curMode, irq);

    if (!current_state.give_semaphore_flag)
    {
        /* skip give semaphore */
        return;
    }
    switch (irq)
    {
        case WUU0_IRQn:
            APP_WUU0_IRQHandler();
            break;
        case GPIOB_INT0_IRQn:
            if ((1U << APP_PIN_IDX(APP_PIN_RTD_BTN2)) &
                RGPIO_GetPinsInterruptFlags(BOARD_SW8_GPIO, kRGPIO_InterruptOutput2))
            {
                /* Flag will be cleared by app_srtm.c */
                xSemaphoreGiveFromISR(s_wakeupSig, NULL);
                portYIELD_FROM_ISR(pdTRUE);
            }
            break;
        default:
            break;
    }
}

/* Get input from user about wakeup timeout. */
static uint32_t APP_GetWakeupTimeout(void)
{
    uint32_t timeout = 0U;
    uint8_t c;

    while (1)
    {
        PRINTF("Select the wake up timeout in seconds.\r\n");
        PRINTF("The allowed range is 1s ~ 999s.\r\n");
        PRINTF("Eg. enter 5 to wake up in 5 seconds.\r\n");
        PRINTF("\r\nWaiting for input timeout value...\r\n\r\n");

        do
        {
            c = GETCHAR();
            if ((c >= '0') && (c <= '9'))
            {
                PRINTF("%c", c);
                timeout = timeout * 10U + c - '0';
            }
            else if ((c == '\r') || (c == '\n'))
            {
                break;
            }
            else
            {
                PRINTF("%c\r\nWrong value!\r\n", c);
                timeout = 0U;
            }
        } while (timeout != 0U && timeout < 100U);

        if (timeout > 0U)
        {
            PRINTF("\r\n");
            break;
        }
    }

    return timeout;
}

/* Get wakeup source by user input. */
static app_wakeup_source_t APP_GetWakeupSource(void)
{
    uint8_t ch;

    while (1)
    {
        PRINTF("Select the wake up source:\r\n");
        PRINTF("Press T for LPTMR - Low Power Timer\r\n");
        PRINTF("Press S for switch/button %s. \r\n", APP_WAKEUP_BUTTON_NAME);

        PRINTF("\r\nWaiting for key press..\r\n\r\n");

        ch = GETCHAR();

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }

        if (ch == 'T')
        {
            return kAPP_WakeupSourceLptmr;
        }
        else if (ch == 'S')
        {
            return kAPP_WakeupSourcePin;
        }
        else
        {
            PRINTF("Wrong value!\r\n");
        }
    }
}

/* Get wakeup timeout and wakeup source. */
static void APP_GetWakeupConfig(void)
{
    /* Get wakeup source by user input. */
    s_wakeupSource = APP_GetWakeupSource();

    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        /* Wakeup source is LPTMR, user should input wakeup timeout value. */
        s_wakeupTimeout = APP_GetWakeupTimeout();
        PRINTF("Will wakeup in %d seconds.\r\n", s_wakeupTimeout);
    }
    else
    {
        PRINTF("Press %s to wake up.\r\n", APP_WAKEUP_BUTTON_NAME);
    }
}

static void APP_SetWakeupConfig(lpm_rtd_power_mode_e targetMode)
{
    if (kAPP_WakeupSourceLptmr == s_wakeupSource)
    {
        LPTMR_SetTimerPeriod(LPTMR1, (1000UL * s_wakeupTimeout / 16U));
        LPTMR_StartTimer(LPTMR1);
        LPTMR_EnableInterrupts(LPTMR1, kLPTMR_TimerInterruptEnable);
    }

    /* To avoid conflicting access of WUU with SRTM dispatcher, we put the WUU setting into SRTM dispatcher context.*/
    /* If targetMode is PD/DPD, setup WUU. */
    if ((LPM_PowerModePowerDown == targetMode) || (LPM_PowerModeDeepPowerDown == targetMode))
    {
        if (kAPP_WakeupSourceLptmr == s_wakeupSource)
        {
            /* Set WUU LPTMR1 module wakeup source. */
            APP_SRTM_SetWakeupModule(WUU_MODULE_LPTMR1, LPTMR1_WUU_WAKEUP_EVENT);
            PCC1->PCC_LPTMR1 &= ~PCC1_PCC_LPTMR1_SSADO_MASK;
            PCC1->PCC_LPTMR1 |= PCC1_PCC_LPTMR1_SSADO(1);
        }
        else
        {
            /* Set PORT and WUU wakeup pin. */
            APP_SRTM_SetWakeupPin(APP_PIN_RTD_BTN2, (uint16_t)WUU_WAKEUP_PIN_TYPE | 0x100);
        }
    }
    else
    {
        /* Set PORT pin. */
        if (kAPP_WakeupSourcePin == s_wakeupSource)
        {
            uint16_t event = (uint16_t)WUU_WAKEUP_PIN_TYPE;
            /*
             * Need setup SSADO field when gate core, platform, bus clock(RTD clock mode), unless failed to wakeup
             * cortex-m33 by button. Currently will gate core, platform, bus clock when RTD enter Deep Sleep
             * Mode(LPM_SystemDeepSleep->RTDCMC_SetClockMode), so setup SSADO field here for Deep Sleep Mode.
             */
            if (LPM_PowerModeDeepSleep == targetMode)
            {
                PCC1->PCC_RGPIOB &= ~PCC1_PCC_RGPIOB_SSADO_MASK;
                PCC1->PCC_RGPIOB |= PCC1_PCC_RGPIOB_SSADO(1);
                event |= 0x100; /* enable wakeup flag */
            }
            APP_SRTM_SetWakeupPin(APP_PIN_RTD_BTN2, event);
        }
    }
}

static void APP_ClearWakeupConfig(lpm_rtd_power_mode_e targetMode)
{
    if (kAPP_WakeupSourcePin == s_wakeupSource)
    {
        APP_SRTM_SetWakeupPin(APP_PIN_RTD_BTN2, (uint16_t)kWUU_ExternalPinDisable);
    }
    else if ((LPM_PowerModePowerDown == targetMode) || (LPM_PowerModeDeepPowerDown == targetMode))
    {
        APP_SRTM_ClrWakeupModule(WUU_MODULE_LPTMR1, LPTMR1_WUU_WAKEUP_EVENT);
    }
}

static void DEMO_EnterSleep()
{
    lpm_rtd_power_mode_e targetPowerMode = LPM_PowerModeSleep;
    if (!LPM_SetPowerMode(targetPowerMode))
    {
        PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[targetPowerMode]);
    }
    else /* Idle task will handle the low power state. */
    {
        /* Wakeup source is LPTMR, set wakeup timeout to 1s, actually depends on lcdif irq */
        s_wakeupSource  = kAPP_WakeupSourceLptmr;
        s_wakeupTimeout = 1;

        APP_SetWakeupConfig(targetPowerMode);
        xSemaphoreTake(s_wakeupSig, portMAX_DELAY);
        /* The call might be blocked by SRTM dispatcher task. Must be called after power mode reset. */
        APP_ClearWakeupConfig(targetPowerMode);
    }
}

static void DEMO_EnableLPAV()
{
    /* Restore BUCK3 voltage to 1.0V, please refer to PCA9460 for the specific data */
    UPOWER_SetPmicReg(PCA9460_BUCK3OUT_DVS0_ADDR, 0x20);

    SIM_SEC->SYSCTRL0 &= ~0x00000080;               /* LPAV alloc to RTD */
    SIM_SEC->LPAV_MASTER_ALLOC_CTRL &= ~0x00000018; /* DSI/LCDIF alloc to RTD */

    /* Power on LPAV domain */
    UPOWER_PowerOnMemPart((uint32_t)(kUPOWER_MP0_DCNANO_A | kUPOWER_MP0_DCNANO_B | kUPOWER_MP0_MIPI_DSI), 0U);
    UPOWER_PowerOnSwitches((upower_ps_mask_t)(kUPOWER_PS_AV_NIC | kUPOWER_PS_MIPI_DSI));

    /* Init LPAV domain */
    BOARD_LpavInit();

    /* Off DDR */
    PCC5->PCC_LPDDR4 &= ~(PCC5_PCC_LPDDR4_CGC_MASK | PCC5_PCC_LPDDR4_SWRST_MASK);
    /* Enable PLL4 PFD1DIV2 */
    CGC_LPAV->PLL4DIV_PFD_0 &= ~CGC_LPAV_PLL4DIV_PFD_0_DIV4HALT_MASK;
}

void DEMO_InitFrameBuffer(void)
{
    uint32_t i;
    uint16_t *fb;

    fb = (uint16_t *)s_frameBufferAddr[0];
    for (i = 0; i < DEMO_IMG_HEIGHT * DEMO_IMG_WIDTH; i++)
        fb[i] = DEMO_COLOR_RED;

    /* Clean cache, flush */
    DCACHE_CleanInvalidateByRange(s_frameBufferAddr[0], 0x200000);

    fb = (uint16_t *)s_frameBufferAddr[1];
    for (i = 0; i < DEMO_IMG_HEIGHT * DEMO_IMG_WIDTH; i++)
        fb[i] = DEMO_COLOR_GREEN;

    DCACHE_CleanInvalidateByRange(s_frameBufferAddr[1], 0x200000);

    fb = (uint16_t *)s_frameBufferAddr[2];
    for (i = 0; i < DEMO_IMG_HEIGHT * DEMO_IMG_WIDTH; i++)
        fb[i] = DEMO_COLOR_BLUE;

    DCACHE_CleanInvalidateByRange(s_frameBufferAddr[2], 0x200000);
}

void DEMO_LCDIF_Init(void)
{
    lcdif_dpi_config_t dpiConfig = {
        .panelWidth    = DEMO_IMG_WIDTH,
        .panelHeight   = DEMO_IMG_HEIGHT,
        .hsw           = DEMO_HSW,
        .hfp           = DEMO_HFP,
        .hbp           = DEMO_HBP,
        .vsw           = DEMO_VSW,
        .vfp           = DEMO_VFP,
        .vbp           = DEMO_VBP,
        .polarityFlags = DEMO_POL_FLAGS,
        .format        = kLCDIF_Output24Bit,
    };

    LCDIF_Init(DEMO_LCDIF);

    LCDIF_DpiModeSetConfig(DEMO_LCDIF, 0, &dpiConfig);

    LCDIF_SetFrameBufferStride(DEMO_LCDIF, 0, DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL);

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }
}

void DEMO_LCDIF_Color_Change(void)
{
    lcdif_fb_config_t fbConfig;
    uint32_t frameBufferIndex = 0;

    /* Enable the LCDIF to show. */
    LCDIF_FrameBufferGetDefaultConfig(&fbConfig);

    fbConfig.enable      = true;
    fbConfig.enableGamma = false;
    fbConfig.format      = kLCDIF_PixelFormatRGB565;

    DEMO_InitFrameBuffer();
    LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_frameBufferAddr[frameBufferIndex]);
    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);

    while (1)
    {
        /* put CM33 into sleep mode */
        DEMO_EnterSleep();
        LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_frameBufferAddr[frameBufferIndex]);
        ++frameBufferIndex;
        frameBufferIndex %= 3;
        PRINTF("Update screen...\r\n");
    }
}

/* Low Power Display task */
void LowPowerDisplayTask(void *pvParameters)
{
    lptmr_config_t lptmrConfig;
    lpm_rtd_power_mode_e targetPowerMode;
    uint32_t freq = 0U, freq1, freq2;
    uint8_t ch;

    /* As IRQ handler main entry locates in app_srtm.c to support services, here need an entry to handle application
     * IRQ events.
     */
    APP_SRTM_SetIRQHandler(APP_IRQDispatcher, NULL);
    /* Add Systick as Power Down wakeup source, depending on SYSTICK_WUU_WAKEUP_EVENT value. */
    APP_SRTM_SetWakeupModule(WUU_MODULE_SYSTICK, SYSTICK_WUU_WAKEUP_EVENT);

    /* Setup LPTMR. */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;  /* Use RTC 1KHz as clock source. */
    lptmrConfig.bypassPrescaler      = false;
    lptmrConfig.value                = kLPTMR_Prescale_Glitch_3; /* Divide clock source by 16. */
    LPTMR_Init(LPTMR1, &lptmrConfig);
    NVIC_SetPriority(LPTMR1_IRQn, APP_LPTMR1_IRQ_PRIO);

    EnableIRQ(LPTMR1_IRQn);

    SIM_SEC->DGO_GP10  = 2;
    SIM_SEC->DGO_CTRL1 = SIM_SEC_DGO_CTRL1_UPDATE_DGO_GP10_MASK;
    /* Wait DGO GP0 updated */
    while ((SIM_SEC->DGO_CTRL1 & SIM_SEC_DGO_CTRL1_WR_ACK_DGO_GP10_MASK) == 0)
    {
    }
    /* Clear DGO GP0 ACK and UPDATE bits */
    SIM_SEC->DGO_CTRL1 =
        (SIM_SEC->DGO_CTRL1 & ~(SIM_SEC_DGO_CTRL1_UPDATE_DGO_GP10_MASK)) | SIM_SEC_DGO_CTRL1_WR_ACK_DGO_GP10_MASK;

    SIM_SEC->DGO_GP11  = 1; // PTB range to 1.8V
    SIM_SEC->DGO_CTRL1 = SIM_SEC_DGO_CTRL1_UPDATE_DGO_GP11_MASK;
    /* Wait DGO GP0 updated */
    while ((SIM_SEC->DGO_CTRL1 & SIM_SEC_DGO_CTRL1_WR_ACK_DGO_GP11_MASK) == 0)
    {
    }
    /* Clear DGO GP0 ACK and UPDATE bits */
    SIM_SEC->DGO_CTRL1 =
        (SIM_SEC->DGO_CTRL1 & ~(SIM_SEC_DGO_CTRL1_UPDATE_DGO_GP11_MASK)) | SIM_SEC_DGO_CTRL1_WR_ACK_DGO_GP11_MASK;

    SIM_RTD->PTC_COMPCELL = 0x0; // PTC compensation off

    for (;;)
    {
        freq  = CLOCK_GetFreq(kCLOCK_Cm33CorePlatClk);
        freq1 = CLOCK_GetFreq(kCLOCK_Cm33BusClk);
        freq2 = CLOCK_GetFreq(kCLOCK_Cm33SlowClk);
        PRINTF("\r\n####################  Power Mode Switch Task ####################\n\r\n");
        PRINTF("    Build Time: %s--%s \r\n", __DATE__, __TIME__);
        PRINTF("    Core Clock: %dHz, %dHz, %dHz \r\n", freq, freq1, freq2);
        PRINTF("    Boot Type: %s \r\n", BOARD_GetBootTypeName());
        PRINTF("\r\nSelect the desired operation \n\r\n");
        PRINTF("Press Z for for low power display\r\n");
        // clang-format off
        /*
         * OD: Over Drive Mode
         * ND: Norminal Drive Mode
         * Ud: Under Drive Mode
         *+-----------------------------------------------------------------------------------------------------+
         *|      Drive Mode of Cortex-M33   |           OD          |           ND         |          UD        |
         *+-----------------------------------------------------------------------------------------------------+
         *|         Voltage of BUCK2        |    1.05 V ~ 1.10 V    |     0.95 V ~ 1.00 V  |         0.9 V      |
         *+-----------------------------------------------------------------------------------------------------+
         *|         Biasing Option          |         AFBB          |         AFBB         |         ARBB       |
         *+-----------------------------------------------------------------------------------------------------+
         *|         Maximum frequency       |        216 MHz        |        160 MHz       |      38.4 MHz      |
         *+-----------------------------------------------------------------------------------------------------+
         */
        // clang-format on
        PRINTF("Press  M for switch Voltage Drive Mode between OD/ND/UD.\r\n");
        PRINTF("\r\nWaiting for power mode select..\r\n\r\n");

        /* Wait for user response */
        do
        {
            ch = GETCHAR();
        } while ((ch == '\r') || (ch == '\n'));

        if ((ch >= 'a') && (ch <= 'z'))
        {
            ch -= 'a' - 'A';
        }
        targetPowerMode = (lpm_rtd_power_mode_e)(ch - 'A');
        if (targetPowerMode <= LPM_PowerModeDeepPowerDown)
        {
            if (targetPowerMode == s_curMode)
            {
                /* Same mode, skip it */
                continue;
            }
            if (APP_GetModeAllowCombi(AD_CurrentMode, targetPowerMode) == MODE_COMBI_NO)
            {
                PRINTF("Not support the mode combination: %s + %s\r\n", APP_GetAdPwrModeName(AD_CurrentMode),
                       APP_GetRtdPwrModeName(targetPowerMode));
                continue;
            }
            if (!LPM_SetPowerMode(targetPowerMode))
            {
                PRINTF("Some task doesn't allow to enter mode %s\r\n", s_modeNames[targetPowerMode]);
            }
            else /* Idle task will handle the low power state. */
            {
                APP_GetWakeupConfig();
                APP_SetWakeupConfig(targetPowerMode);
                xSemaphoreTake(s_wakeupSig, portMAX_DELAY);
                /* The call might be blocked by SRTM dispatcher task. Must be called after power mode reset. */
                APP_ClearWakeupConfig(targetPowerMode);
            }
        }
        else if ('W' == ch)
        {
            if (!wake_acore_flag)
            {
                wake_acore_flag = true;
            }
            APP_SRTM_WakeupCA35();
        }
        else if ('T' == ch)
        {
            APP_RebootCA35();
        }
        else if ('U' == ch)
        {
            APP_ShutdownCA35();
        }
        else if ('V' == ch)
        {
            option_v_boot_flag = true;
            APP_BootCA35();
        }
        else if ('S' == ch)
        {
            APP_ShowModeCombi();
        }
        else if ('M' == ch)
        {
            BOARD_SwitchDriveMode();
        }
        else if ('Z' == ch)
        {
            /* Switch clock source from PLL0/1 to FRO to save more power */
            BOARD_SwitchToFRO_ForSleep();
            BOARD_DisablePlls();

            BOARD_InitMipiPanelPins();
            BOARD_InitPsRamPins();

            /* Set LPAV to RTD */
            DEMO_EnableLPAV();

            BOARD_InitLcdifClock();

            /* MIPI panel reset GPIO. */
            CLOCK_EnableClock(kCLOCK_RgpioC);

            /* PCA6416A I2C. */
            CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_Pcc1BusIpSrcFroDiv2);
            RESET_PeripheralReset(kRESET_Lpi2c0);
            BOARD_InitPCA6416A(&g_pca6416aHandle);

            status_t status = BOARD_InitPsRam_Fro();
            if (status != kStatus_Success)
            {
                assert(false);
            }

            DEMO_LCDIF_Init();
            DEMO_LCDIF_Color_Change();
        }
        else
        {
            PRINTF("Invalid command %c[0x%x]\r\n", ch, ch);
        }
        /*update Mode state*/
        s_lastMode = s_curMode;
        s_curMode  = LPM_PowerModeActive;
        PRINTF("\r\nNext loop\r\n");
    }
}

void vApplicationMallocFailedHook(void)
{
    PRINTF("Malloc Failed!!!\r\n");
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    uint32_t irqMask;
    lpm_rtd_power_mode_e targetPowerMode;
    /* lpm_rtd_power_mode_e targetMode; */
    upwr_pwm_param_t param;
    bool result;

    /* targetMode = LPM_GetPowerMode(); */

    /* Workround for PD/DPD exit fail if sleep more than 1 second */
    /* if ((LPM_PowerModePowerDown == targetMode) || (LPM_PowerModeDeepPowerDown == targetMode)) */
    {
        param.R              = 0;
        param.B.DPD_ALLOW    = 0;
        param.B.DSL_DIS      = 0;
        param.B.SLP_ALLOW    = 0;
        param.B.DSL_BGAP_OFF = 1;
        param.B.DPD_BGAP_ON  = 0;

        UPOWER_SetPwrMgmtParam(&param);
    }

    irqMask = DisableGlobalIRQ();

    /* Only when no context switch is pending and no task is waiting for the scheduler
     * to be unsuspended then enter low power entry.
     */
    if (eTaskConfirmSleepModeStatus() != eAbortSleep)
    {
        targetPowerMode = LPM_GetPowerMode();
        if (targetPowerMode != LPM_PowerModeActive)
        {
            /* Only wait when target power mode is not running */
            APP_PowerPreSwitchHook(targetPowerMode);
            result = LPM_WaitForInterrupt((uint64_t)1000 * xExpectedIdleTime / configTICK_RATE_HZ);
            APP_PowerPostSwitchHook(targetPowerMode, result);
        }
    }
    EnableGlobalIRQ(irqMask);
    /* Recovery clock(switch clock source from FRO to PLL0/1) after interrupt is enabled */
    // BOARD_ResumeClockInit();
}

/* Called in LowPowerDisplayTask */
static bool APP_LpmListener(lpm_rtd_power_mode_e curMode, lpm_rtd_power_mode_e newMode, void *data)
{
    PRINTF("WorkingTask %d: Transfer from %s to %s\r\n", (uint32_t)data, s_modeNames[curMode], s_modeNames[newMode]);

    /* Do necessary preparation for this mode change */

    return true; /* allow this switch */
}

/*!
 * @brief simulating working task.
 */
static void WorkingTask(void *pvParameters)
{
    LPM_RegisterPowerListener(APP_LpmListener, pvParameters);

    for (;;)
    {
        /* Use App task logic to replace vTaskDelay */
        PRINTF("Task %d is working now\r\n", (uint32_t)pvParameters);
        vTaskDelay(portMAX_DELAY);
    }
}

/*! @brief Main function */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    UPOWER_PowerOnMemPart(0U, (uint32_t)kUPOWER_MP1_DMA0);

    CLOCK_SetIpSrcDiv(kCLOCK_Tpm0, kCLOCK_Pcc1BusIpSrcCm33Bus, 1U, 0U);
    CLOCK_SetIpSrcDiv(kCLOCK_Lpi2c0, kCLOCK_Pcc1BusIpSrcCm33Bus, 0U, 0U);
    CLOCK_SetIpSrcDiv(kCLOCK_Lpi2c1, kCLOCK_Pcc1BusIpSrcCm33Bus, 0U, 0U);
    /* Use Pll1Pfd2Div clock source 12.288MHz. */
    CLOCK_SetIpSrc(kCLOCK_Sai0, kCLOCK_Cm33SaiClkSrcPll1Pfd2Div);

    CLOCK_EnableClock(kCLOCK_Dma0Ch0);
    CLOCK_EnableClock(kCLOCK_Dma0Ch16);
    CLOCK_EnableClock(kCLOCK_Dma0Ch17);
    CLOCK_EnableClock(kCLOCK_RgpioA);
    CLOCK_EnableClock(kCLOCK_RgpioB);
    CLOCK_EnableClock(kCLOCK_Wuu0);
    CLOCK_EnableClock(kCLOCK_Bbnsm);

    RESET_PeripheralReset(kRESET_Sai0);
    RESET_PeripheralReset(kRESET_Lpi2c0);
    RESET_PeripheralReset(kRESET_Lpi2c1);
    RESET_PeripheralReset(kRESET_Tpm0);

    Fusion_Init();

    APP_SRTM_Init();

    /* If RTD reset is due to DPD exit, should go different flow here */
    if (CMC_RTD->SSRS & CMC_SSRS_WAKEUP_MASK)
    {
        CMC_RTD->SSRS = CMC_SSRS_WAKEUP_MASK;
        /* Assume that Application Domain is entered Deep Power Down Mode */
        AD_CurrentMode = AD_DPD;
        /*
         * AD is also in Deep Power Down mode when RTD is in Deep Power Down Mode.
         * AD/RTD exiting from Deep Power Down Mode is same with cold boot flow.
         * So don't need setup TRDC when RTD exit from Deep Power Down mode.
         *
         */
        // BOARD_SetTrdcAfterApdReset();
        MU_Init(MU0_MUA);
        MU_BootOtherCore(MU0_MUA, (mu_core_boot_mode_t)0);
    }
    else
    {
        APP_SRTM_StartCommunication();
    }

    LPM_Init();

    s_wakeupSig = xSemaphoreCreateBinary();

    xTaskCreate(LowPowerDisplayTask, "Main Task", 512U, NULL, tskIDLE_PRIORITY + 1U, NULL);
    xTaskCreate(WorkingTask, "Working Task", configMINIMAL_STACK_SIZE, (void *)1, tskIDLE_PRIORITY + 2U, NULL);

    /* Start FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Application should never reach this point. */
    for (;;)
    {
    }
}
