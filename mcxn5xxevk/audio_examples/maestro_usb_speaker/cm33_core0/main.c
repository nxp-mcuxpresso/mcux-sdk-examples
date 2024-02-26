/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Board includes */
#include "pin_mux.h"
#include "board.h"
#include "main.h"
#include "cmd.h"

#include "audio_speaker.h"

#include "fsl_debug_console.h"

#include <stdbool.h>
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "usb_phy.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#include "app_definitions.h"
#define APP_TASK_STACK_SIZE (8 * 1024)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int BOARD_CODEC_Init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
codec_handle_t codecHandle    = {0};
da7212_pll_config_t pllconfig = {
    .outputClock_HZ = kDA7212_PLLOutputClk12288000,
    .refClock_HZ    = 12288000U,
    .source         = kDA7212_PLLClkSourceMCLK,
};

da7212_config_t da7212Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 12000000U},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusI2S,
    .format       = {.mclk_HZ = 12288000U, .sampleRate = 48000U, .bitWidth = 16U},
    .isMaster     = true,
    .pll          = &pllconfig,
    .sysClkSource = kDA7212_SysClkSourcePLL,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

sai_master_clock_t mclkConfig;

static app_handle_t app;

/*******************************************************************************
 * Code
 ******************************************************************************/

int BOARD_CODEC_Init(void)
{
    BOARD_Codec_I2C_Init();

    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }

    /* Initial volume kept low for hearing safety. */
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 75) !=
        kStatus_Success)
    {
        assert(false);
    }

    return 0;
}

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    SPC0->ACTIVE_VDELAY = 0x0500;
    /* Change the power DCDC to 1.8v (By deafult, DCDC is 1.8V), CORELDO to 1.1v (By deafult, CORELDO is 1.0V) */
    SPC0->ACTIVE_CFG &= ~SPC_ACTIVE_CFG_CORELDO_VDD_DS_MASK;
    SPC0->ACTIVE_CFG |= SPC_ACTIVE_CFG_DCDC_VDD_LVL(0x3) | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(0x3) |
                        SPC_ACTIVE_CFG_SYSLDO_VDD_DS_MASK | SPC_ACTIVE_CFG_DCDC_VDD_DS(0x2u);
    /* Wait until it is done */
    while (SPC0->SC & SPC_SC_BUSY_MASK)
        ;
    if (0u == (SCG0->LDOCSR & SCG_LDOCSR_LDOEN_MASK))
    {
        SCG0->TRIM_LOCK = 0x5a5a0001U;
        SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
        /* wait LDO ready */
        while (0U == (SCG0->LDOCSR & SCG_LDOCSR_VOUT_OK_MASK))
            ;
    }
    SYSCON->AHBCLKCTRLSET[2] |= SYSCON_AHBCLKCTRL2_USB_HS_MASK | SYSCON_AHBCLKCTRL2_USB_HS_PHY_MASK;
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK | SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT_MASK;
    CLOCK_EnableClock(kCLOCK_UsbHs);
    CLOCK_EnableClock(kCLOCK_UsbHsPhy);
    CLOCK_EnableUsbhsPhyPllClock(kCLOCK_Usbphy480M, 24000000U);
    CLOCK_EnableUsbhsClock();
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    CLOCK_AttachClk(kCLK_48M_to_USB0);
    CLOCK_EnableClock(kCLOCK_Usb0Ram);
    CLOCK_EnableClock(kCLOCK_Usb0Fs);
    CLOCK_EnableUsbfsClock();
#endif
}


void BOARD_MASTER_CLOCK_CONFIG(void)
{
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = 12288000U;
    mclkConfig.mclkSourceClkHz = 24576000U;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}


void handleShellMessage(void *arg)
{
    /* Wait for response message to be processed before returning to shell. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void APP_Shell_Task(void *param)
{
    PRINTF("[APP_Shell_Task] start\r\n");

    /* Handle shell commands.  Return when 'exit' command entered. */
    shellCmd(handleShellMessage, param);
    vTaskSuspend(NULL);
    while (1)
        ;
}

int main(void)
{
    int ret;

    BOARD_InitBootPins();
    BOARD_BootClockFROHF144M();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* attach FRO 12M to LPFLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to LPFLEXCOMM2 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

#if (defined USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI)
    /*!< Set up PLL0 */
    const pll_setup_t pll0Setup = {
        .pllctrl = SCG_APLLCTRL_SOURCE(1U) | SCG_APLLCTRL_LIMUPOFF_MASK | SCG_APLLCTRL_SELI(4U) |
                   SCG_APLLCTRL_SELP(3U) | SCG_APLLCTRL_SELR(4U),
        .pllndiv = SCG_APLLNDIV_NDIV(10U),
        .pllpdiv = SCG_APLLPDIV_PDIV(10U),
        .pllsscg = {(SCG_APLLSSCG0_SS_MDIV_LSB(3435973837U)),
                    ((SCG0->APLLSSCG1 & ~SCG_APLLSSCG1_SS_PD_MASK) | SCG_APLLSSCG1_SS_MDIV_MSB(0U) |
                     (uint32_t)(kSS_MF_512) | (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC) |
                     SCG_APLLSSCG1_SEL_SS_MDIV_MASK)},
        .pllRate = 24576000U};
    CLOCK_SetPLL0Freq(&pll0Setup); /*!< Configure PLL0 to the desired values */
#elif (defined USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI)
    CLOCK_SetupExtClocking(24000000U);
    CLOCK_SetSysOscMonitorMode(kSCG_SysOscMonitorDisable); /* System OSC Clock Monitor is disabled */
    /*!< Set up PLL0 */
    const pll_setup_t pll0Setup = {
        .pllctrl = SCG_APLLCTRL_SOURCE(0U) | SCG_APLLCTRL_LIMUPOFF_MASK | SCG_APLLCTRL_SELI(4U) |
                   SCG_APLLCTRL_SELP(3U) | SCG_APLLCTRL_SELR(4U),
        .pllndiv = SCG_APLLNDIV_NDIV(6U),
        .pllpdiv = SCG_APLLPDIV_PDIV(7U),
        .pllsscg = {(SCG_APLLSSCG0_SS_MDIV_LSB(2886218023U)),
                    ((SCG0->APLLSSCG1 & ~SCG_APLLSSCG1_SS_PD_MASK) | SCG_APLLSSCG1_SS_MDIV_MSB(0U) |
                     (uint32_t)(kSS_MF_512) | (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC) |
                     SCG_APLLSSCG1_SEL_SS_MDIV_MASK)},
        .pllRate = 24576000U};
    CLOCK_SetPLL0Freq(&pll0Setup); /*!< Configure PLL0 to the desired values */
#endif
    /* force the APLL power on and gate on */
    SCG0->APLL_OVRD |= SCG_APLL_OVRD_APLLPWREN_OVRD_MASK | SCG_APLL_OVRD_APLLCLKEN_OVRD_MASK;
    SCG0->APLL_OVRD |= SCG_APLL_OVRD_APLL_OVRD_EN_MASK;
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 1U);

    /* attach PLL0 to SAI1 */
    CLOCK_SetClkDiv(kCLOCK_DivSai1Clk, 1u);
    CLOCK_AttachClk(kPLL0_to_SAI1);

#if (defined USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI)
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
    CLOCK_AttachClk(kPLL0_to_CTIMER0);
#elif (defined USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI)
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
    CLOCK_AttachClk(kPLL0_to_CTIMER0);
    CLOCK_SetClkDiv(kCLOCK_DivCtimer1Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);
#endif

    USB_DeviceClockInit();

    PRINTF("\r\n");
    PRINTF("**********************************************\r\n");
    PRINTF("Maestro audio USB speaker solutions demo start\r\n");
    PRINTF("**********************************************\r\n");
    PRINTF("\r\n");

    /* Initialize OSA*/
    OSA_Init();

    ret = BOARD_CODEC_Init();
    if (ret)
    {
        PRINTF("CODEC_Init failed\r\n");
        return -1;
    }

    /* USB speaker initialization */
    USB_DeviceApplicationInit();

    /* Set shell command task priority = 4 */
    if (xTaskCreate(APP_Shell_Task, "Shell Task", APP_TASK_STACK_SIZE, &app, configMAX_PRIORITIES - 3,
                    &app.shell_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create application task\r\n");
        while (1)
            ;
    }

    /* Run RTOS */
    vTaskStartScheduler();

    /* Should not reach this statement */
    return 0;
}
