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

#include "audio_microphone.h"

#include "fsl_debug_console.h"

#include <stdbool.h>
#include "fsl_sysctl.h"
#include "fsl_codec_common.h"
#include "fsl_wm8904.h"
#include "fsl_codec_adapter.h"
#include "fsl_power.h"
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#include "usb_phy.h"
#endif
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
codec_handle_t codecHandle   = {0};
wm8904_config_t wm8904Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .recordSource = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate48kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = DEMO_I2S_MASTER_CLOCK_FREQUENCY,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};

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

void BOARD_InitSysctrl(void)
{
    SYSCTL_Init(SYSCTL);
    /* select signal source for share set */
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalSCK, kSYSCTL_Flexcomm7);
    SYSCTL_SetShareSignalSrc(SYSCTL, kSYSCTL_ShareSet0, kSYSCTL_SharedCtrlSignalWS, kSYSCTL_Flexcomm7);
    /* select share set for special flexcomm signal */
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm7, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm7, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm6, kSYSCTL_FlexcommSignalSCK, kSYSCTL_ShareSet0);
    SYSCTL_SetShareSet(SYSCTL, kSYSCTL_Flexcomm6, kSYSCTL_FlexcommSignalWS, kSYSCTL_ShareSet0);
}

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
    ANACTRL->FRO192M_CTRL = (ANACTRL->FRO192M_CTRL & ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK));
#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif

#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUnused, 0U);
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif
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

    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* I2C clock */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);

    PMC->PDRUNCFGCLR0 |= PMC_PDRUNCFG0_PDEN_XTAL32M_MASK;   /*!< Ensure XTAL16M is on  */
    PMC->PDRUNCFGCLR0 |= PMC_PDRUNCFG0_PDEN_LDOXO32M_MASK;  /*!< Ensure XTAL16M is on  */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK; /*!< Ensure CLK_IN is on  */
    ANACTRL->XO32M_CTRL |= ANACTRL_XO32M_CTRL_ENABLE_SYSTEM_CLK_OUT_MASK;

    /*!< Switch PLL0 clock source selector to EXT_CLK */
    CLOCK_AttachClk(kEXT_CLK_to_PLL0);

/*!< Set up PLL, use FRO for full speed */
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(8U) | SYSCON_PLL0CTRL_SELP(31U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(125U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(8U),
        .pllsscg = {0x0U, (SYSCON_PLL0SSCG1_MDIV_EXT(3072U) | SYSCON_PLL0SSCG1_SEL_EXT_MASK)},
        .pllRate = 24576000U,
        .flags   = PLL_SETUPFLAG_WAITLOCK};
/*!< Set up PLL, use external OSC for high speed */
#elif (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    POWER_DisablePD(kPDRUNCFG_PD_PLL0); /* Ensure PLL is on  */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0_SSCG);
    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_LIMUPOFF_MASK | SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(4U) |
                   SYSCON_PLL0CTRL_SELP(3U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(4U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(7U),
        .pllsscg = {(SYSCON_PLL0SSCG0_MD_LBS(2886218023U)), (SYSCON_PLL0SSCG1_MD_MBS(0U) | (uint32_t)(kSS_MF_512) |
                                                             (uint32_t)(kSS_MR_K0) | (uint32_t)(kSS_MC_NOC))},
        .pllRate = 24576000U,
        .flags   = PLL_SETUPFLAG_POWERUP};
#endif

    /*!< Configure PLL to the desired values */
    CLOCK_SetPLL0Freq(&pll0Setup);

    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivPll0Clk, 1U, false);

    /* I2S clocks */
    CLOCK_AttachClk(kPLL0_DIV_to_FLEXCOMM6);
    CLOCK_AttachClk(kPLL0_DIV_to_FLEXCOMM7);

    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_AttachClk(kPLL0_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
    SYSCON->MCLKIO  = 1U;

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC4_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for DMA0 */
    RESET_PeripheralReset(kDMA0_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

    /* reset NVIC for FLEXCOMM6 and FLEXCOMM7 */
    NVIC_ClearPendingIRQ(FLEXCOMM6_IRQn);
    NVIC_ClearPendingIRQ(FLEXCOMM7_IRQn);

    /* Enable interrupts for I2S */
    EnableIRQ(FLEXCOMM6_IRQn);
    EnableIRQ(FLEXCOMM7_IRQn);

    /* Initialize the rest */
    BOARD_InitPins();
    BOARD_BootClockPLL1_150M();
    BOARD_InitDebugConsole();
    BOARD_InitSysctrl();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */

    /* Reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);

#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /* Put PHY powerdown under software control */
    *((uint32_t *)(USBHSH_BASE + 0x50)) = USBHSH_PORTMODE_SW_PDCOM_MASK;
    /* According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /* Enable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* Enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /* According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* Disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    CLOCK_AttachClk(kPLL0_to_CTIMER0);
#elif (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    CLOCK_AttachClk(kPLL0_to_CTIMER0);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);
#endif

    USB_DeviceClockInit();

    PRINTF("\r\n");
    PRINTF("*************************************************\r\n");
    PRINTF("Maestro audio USB microphone solutions demo start\r\n");
    PRINTF("*************************************************\r\n");
    PRINTF("\r\n");

    /* Initialize OSA*/
    OSA_Init();

    ret = BOARD_CODEC_Init();
    if (ret)
    {
        PRINTF("CODEC_Init failed\r\n");
        return -1;
    }

    /* USB microphone initialization */
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
