/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "specific.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "lpm.h"
#include "fsl_iomuxc.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

const clock_arm_pll_config_t armPllConfig_PowerMode = {
    .loopDivider = 100, /* PLL loop divider, Fout = Fin * 50 */
    .src         = 0,   /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
};
const clock_sys_pll_config_t sysPllConfig_PowerMode = {
    .loopDivider = 1, /* PLL loop divider, Fout = Fin * ( 20 + loopDivider*2 + numerator / denominator ) */
    .numerator   = 0, /* 30 bit numerator of fractional loop divider */
    .denominator = 1, /* 30 bit denominator of fractional loop divider */
    .src         = 0, /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
};
const clock_usb_pll_config_t usb1PllConfig_PowerMode = {
    .loopDivider = 0, /* PLL loop divider, Fout = Fin * 20 */
    .src         = 0, /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
};

AT_QUICKACCESS_SECTION_CODE(void SwitchSystemClocks(lpm_power_mode_t power_mode));

void SwitchSystemClocks(lpm_power_mode_t power_mode)
{
#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    LPM_EnterCritical();
    while (!((FLEXSPI_INST->STS0 & FLEXSPI_STS0_ARBIDLE_MASK) && (FLEXSPI_INST->STS0 & FLEXSPI_STS0_SEQIDLE_MASK)))
    {
    }
    FLEXSPI_INST->MCR0 |= FLEXSPI_MCR0_MDIS_MASK;

    /* Disable clock gate of flexspi2. */
    CCM->CCGR7 &= (~CCM_CCGR7_CG1_MASK);
#endif
    switch (power_mode)
    {
        case LPM_PowerModeOverRun:
            CLOCK_SET_DIV(kCLOCK_Flexspi2Div, 3);
            CLOCK_SET_MUX(kCLOCK_Flexspi2Mux, 0); // FLEXSPI2 mux to PLL2 PFD2
            /* CORE CLK to 600MHz, AHB, IPG to 150MHz, PERCLK to 75MHz */
            CLOCK_SET_DIV(kCLOCK_PerclkDiv, 1);
            CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
            CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
            CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux to IPG CLK
            CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 3); // PRE_PERIPH_CLK mux to ARM PLL
            CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // PERIPH_CLK mux to PRE_PERIPH_CLK
            break;
        case LPM_PowerModeFullRun:
            CLOCK_SET_DIV(kCLOCK_Flexspi2Div, 3);
            CLOCK_SET_MUX(kCLOCK_Flexspi2Mux, 0); // FLEXSPI2 mux to PLL2 PFD2
            /* CORE CLK to 528MHz, AHB, IPG to 132MHz, PERCLK to 66MHz */
            CLOCK_SET_DIV(kCLOCK_PerclkDiv, 1);
            CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
            CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
            CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux to IPG CLK
            CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 0); // PRE_PERIPH_CLK mux to SYS PLL
            CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // PERIPH_CLK mux to PRE_PERIPH_CLK
            break;
        case LPM_PowerModeLowSpeedRun:
        case LPM_PowerModeSysIdle:
            CLOCK_SET_DIV(kCLOCK_Flexspi2Div, 3);
            CLOCK_SET_MUX(kCLOCK_Flexspi2Mux, 0); // FLEXSPI2 mux to PLL2 PFD2
            /* CORE CLK to 132MHz and AHB, IPG, PERCLK to 33MHz */
            CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
            CLOCK_SET_DIV(kCLOCK_IpgDiv, 3);
            CLOCK_SET_DIV(kCLOCK_AhbDiv, 3);
            CLOCK_SET_MUX(kCLOCK_PerclkMux, 0);    // PERCLK mux to IPG CLK
            CLOCK_SET_MUX(kCLOCK_PrePeriphMux, 0); // Switch PRE_PERIPH_CLK to SYS PLL
            CLOCK_SET_MUX(kCLOCK_PeriphMux, 0);    // Switch PERIPH_CLK to PRE_PERIPH_CLK
            break;
        case LPM_PowerModeLowPowerRun:
        case LPM_PowerModeLPIdle:
            CLOCK_SET_DIV(kCLOCK_PeriphClk2Div, 0);
            CLOCK_SET_MUX(kCLOCK_PeriphClk2Mux, 1); // PERIPH_CLK2 mux to OSC
            CLOCK_SET_MUX(kCLOCK_PeriphMux, 1);     // PERIPH_CLK mux to PERIPH_CLK2
            /* Set PLL3 PFD0 BYPASS output directly */
            CCM_ANALOG->PLL_USB1_SET = CCM_ANALOG_PLL_USB1_BYPASS_MASK;
            CCM_ANALOG->PLL_USB1_SET = CCM_ANALOG_PLL_USB1_ENABLE_MASK;
            CCM_ANALOG->PFD_480_CLR  = CCM_ANALOG_PFD_480_PFD0_CLKGATE_MASK;
            CLOCK_SET_DIV(kCLOCK_Flexspi2Div, 0);
            CLOCK_SET_MUX(kCLOCK_Flexspi2Mux, 1); // FLEXSPI2 mux to PLL3 PFD0 BYPASS
            /* CORE CLK to 24MHz and AHB, IPG, PERCLK to 12MHz */
            CLOCK_SET_DIV(kCLOCK_PerclkDiv, 0);
            CLOCK_SET_DIV(kCLOCK_IpgDiv, 1);
            CLOCK_SET_DIV(kCLOCK_AhbDiv, 0);
            CLOCK_SET_MUX(kCLOCK_PerclkMux, 0); // PERCLK mux to IPG CLK
            break;
        default:
            break;
    }

#if (defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Enable clock gate of flexspi2. */
    CCM->CCGR7 |= (CCM_CCGR7_CG1_MASK);

    FLEXSPI_INST->MCR0 &= ~FLEXSPI_MCR0_MDIS_MASK;
    FLEXSPI_INST->MCR0 |= FLEXSPI_MCR0_SWRESET_MASK;
    while (FLEXSPI_INST->MCR0 & FLEXSPI_MCR0_SWRESET_MASK)
    {
    }
    while (!((FLEXSPI_INST->STS0 & FLEXSPI_STS0_ARBIDLE_MASK) && (FLEXSPI_INST->STS0 & FLEXSPI_STS0_SEQIDLE_MASK)))
    {
    }
    LPM_ExitCritical();
#endif
}

void ClockSetToOverDriveRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);

    /* Init ARM PLL */
    CLOCK_SetDiv(kCLOCK_ArmDiv, 1);
    CLOCK_InitArmPll(&armPllConfig_PowerMode);

    /* Init SYS PLL*/
    CLOCK_InitSysPll(&sysPllConfig_PowerMode);
    /* Init System pfd0. */
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 27);
    /* Init System pfd1. */
    CLOCK_InitSysPfd(kCLOCK_Pfd1, 16);
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);
    /* Init System pfd3. */
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 16);

    /* Init USB2 PLL*/
    CCM_ANALOG->PLL_USB2_SET = CCM_ANALOG_PLL_USB2_BYPASS_MASK;
    CCM_ANALOG->PLL_USB2_SET = CCM_ANALOG_PLL_USB2_ENABLE_MASK;
    CCM_ANALOG->PLL_USB2_SET = CCM_ANALOG_PLL_USB2_POWER_MASK;
    while ((CCM_ANALOG->PLL_USB2 & CCM_ANALOG_PLL_USB2_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_USB2_CLR = CCM_ANALOG_PLL_USB2_BYPASS_MASK;

    /* Init AUDIO PLL */
    CCM_ANALOG->PLL_AUDIO_SET = CCM_ANALOG_PLL_AUDIO_BYPASS_MASK;
    CCM_ANALOG->PLL_AUDIO_CLR = CCM_ANALOG_PLL_AUDIO_POWERDOWN_MASK;
    CCM_ANALOG->PLL_AUDIO_SET = CCM_ANALOG_PLL_AUDIO_ENABLE_MASK;
    while ((CCM_ANALOG->PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_AUDIO_CLR = CCM_ANALOG_PLL_AUDIO_BYPASS_MASK;

    /* Init VIDEO PLL */
    CCM_ANALOG->PLL_VIDEO_SET = CCM_ANALOG_PLL_VIDEO_BYPASS_MASK;
    CCM_ANALOG->PLL_VIDEO_CLR = CCM_ANALOG_PLL_VIDEO_POWERDOWN_MASK;
    CCM_ANALOG->PLL_VIDEO_SET = CCM_ANALOG_PLL_VIDEO_ENABLE_MASK;
    while ((CCM_ANALOG->PLL_VIDEO & CCM_ANALOG_PLL_VIDEO_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_VIDEO_CLR = CCM_ANALOG_PLL_VIDEO_BYPASS_MASK;

    /* Init ENET PLL */
    CCM_ANALOG->PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS_MASK;
    CCM_ANALOG->PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN_MASK;
    CCM_ANALOG->PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENABLE_MASK;
    CCM_ANALOG->PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN_MASK;
    while ((CCM_ANALOG->PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS_MASK;

    SwitchSystemClocks(LPM_PowerModeOverRun);

    /* Init USB1 PLL. */
    CLOCK_InitUsb1Pll(&usb1PllConfig_PowerMode);
    /* Init Usb1 pfd0. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 12);
    /* Init Usb1 pfd1. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 16);
    /* Init Usb1 pfd2. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
    /* Init Usb1 pfd3. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);
    /* Disable Usb1 PLL output for USBPHY1. */
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;
}

void ClockSetToFullSpeedRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);

    /* Init ARM PLL */
    CLOCK_SetDiv(kCLOCK_ArmDiv, 1);
    CLOCK_InitArmPll(&armPllConfig_PowerMode);

    /* Init SYS PLL. */
    CLOCK_InitSysPll(&sysPllConfig_PowerMode);
    /* Init System pfd0. */
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 27);
    /* Init System pfd1. */
    CLOCK_InitSysPfd(kCLOCK_Pfd1, 16);
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);
    /* Init System pfd3. */
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 16);

    /* Init USB2 PLL*/
    CCM_ANALOG->PLL_USB2_SET = CCM_ANALOG_PLL_USB2_BYPASS_MASK;
    CCM_ANALOG->PLL_USB2_SET = CCM_ANALOG_PLL_USB2_ENABLE_MASK;
    CCM_ANALOG->PLL_USB2_SET = CCM_ANALOG_PLL_USB2_POWER_MASK;
    while ((CCM_ANALOG->PLL_USB2 & CCM_ANALOG_PLL_USB2_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_USB2_CLR = CCM_ANALOG_PLL_USB2_BYPASS_MASK;

    /* Init AUDIO PLL */
    CCM_ANALOG->PLL_AUDIO_SET = CCM_ANALOG_PLL_AUDIO_BYPASS_MASK;
    CCM_ANALOG->PLL_AUDIO_CLR = CCM_ANALOG_PLL_AUDIO_POWERDOWN_MASK;
    CCM_ANALOG->PLL_AUDIO_SET = CCM_ANALOG_PLL_AUDIO_ENABLE_MASK;
    while ((CCM_ANALOG->PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_AUDIO_CLR = CCM_ANALOG_PLL_AUDIO_BYPASS_MASK;

    /* Init VIDEO PLL */
    CCM_ANALOG->PLL_VIDEO_SET = CCM_ANALOG_PLL_VIDEO_BYPASS_MASK;
    CCM_ANALOG->PLL_VIDEO_CLR = CCM_ANALOG_PLL_VIDEO_POWERDOWN_MASK;
    CCM_ANALOG->PLL_VIDEO_SET = CCM_ANALOG_PLL_VIDEO_ENABLE_MASK;
    while ((CCM_ANALOG->PLL_VIDEO & CCM_ANALOG_PLL_VIDEO_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_VIDEO_CLR = CCM_ANALOG_PLL_VIDEO_BYPASS_MASK;

    /* Init ENET PLL */
    CCM_ANALOG->PLL_ENET_SET = CCM_ANALOG_PLL_ENET_BYPASS_MASK;
    CCM_ANALOG->PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN_MASK;
    CCM_ANALOG->PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENABLE_MASK;
    CCM_ANALOG->PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN_MASK;
    while ((CCM_ANALOG->PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK_MASK) == 0)
    {
    }
    CCM_ANALOG->PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS_MASK;

    SwitchSystemClocks(LPM_PowerModeFullRun);

    /* Init USB1 PLL. */
    CLOCK_InitUsb1Pll(&usb1PllConfig_PowerMode);
    /* Init Usb1 pfd0. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 12);
    /* Init Usb1 pfd1. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 16);
    /* Init Usb1 pfd2. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
    /* Init Usb1 pfd3. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);
    /* Disable Usb1 PLL output for USBPHY1. */
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;
}

void ClockSetToLowSpeedRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);

    /* Deinit ARM PLL */
    CLOCK_DeinitArmPll();

    /* Init SYS PLL */
    CLOCK_InitSysPll(&sysPllConfig_PowerMode);

    /* Deinit SYS PLL PFD 0 1 3 */
    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);

    /* Deinit USB2 PLL */
    CLOCK_DeinitUsb2Pll();

    /* Deinit AUDIO PLL */
    CLOCK_DeinitAudioPll();

    /* Deinit VIDEO PLL */
    CLOCK_DeinitVideoPll();

    /* Deinit ENET PLL */
    CLOCK_DeinitEnetPll();

    SwitchSystemClocks(LPM_PowerModeLowSpeedRun);

    /* Deinit USB1 PLL */
    CLOCK_DeinitUsb1Pll();

    /* Deinit USB1 PLL PFD 0 1 2 3 */
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd0);
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd1);
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd2);
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd3);
}

void ClockSetToLowPowerRun(void)
{
    // CORE CLK mux to 24M before reconfigure PLLs
    SwitchSystemClocks(LPM_PowerModeLowPowerRun);

    /* Deinit ARM PLL */
    CLOCK_DeinitArmPll();

    /* Deinit SYS PLL */
    CLOCK_DeinitSysPll();

    /* Deinit SYS PLL PFD 0 1 2 3 */
    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd2);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);

    /* Disable PLL3, keep BYPASS and ENABLE bit for PFD0 output */
    CCM_ANALOG->PLL_USB1 = CCM_ANALOG_PLL_USB1_BYPASS_MASK | CCM_ANALOG_PLL_USB1_ENABLE_MASK;
    /* Clear PLL3 PFD0 clock gate */
    CCM_ANALOG->PFD_480_CLR = CCM_ANALOG_PFD_480_PFD0_CLKGATE_MASK;
    /* Deinit USB1 PLL PFD 1 2 3 */
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd1);
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd2);
    CLOCK_DeinitUsb1Pfd(kCLOCK_Pfd3);

    /* Deinit USB2 PLL */
    CLOCK_DeinitUsb2Pll();

    /* Deinit AUDIO PLL */
    CLOCK_DeinitAudioPll();

    /* Deinit VIDEO PLL */
    CLOCK_DeinitVideoPll();

    /* Deinit ENET PLL */
    CLOCK_DeinitEnetPll();
}

void SetLowPowerClockGate(void)
{
    CCM->CCGR0 = CCM_CCGR0_CG0(1) | CCM_CCGR0_CG1(1) | CCM_CCGR0_CG3(3) | CCM_CCGR0_CG11(1) | CCM_CCGR0_CG12(1);
    CCM->CCGR1 = CCM_CCGR1_CG9(3) | CCM_CCGR1_CG10(1) | CCM_CCGR1_CG13(1) | CCM_CCGR1_CG14(1) | CCM_CCGR1_CG15(1);
    CCM->CCGR2 = CCM_CCGR2_CG2(1) | CCM_CCGR2_CG8(1) | CCM_CCGR2_CG9(1) | CCM_CCGR2_CG10(1);
    CCM->CCGR3 = CCM_CCGR3_CG2(1) | CCM_CCGR3_CG4(1) | CCM_CCGR3_CG9(1) | CCM_CCGR3_CG14(1) | CCM_CCGR3_CG15(1);
    CCM->CCGR4 =
        CCM_CCGR4_CG1(1) | CCM_CCGR4_CG2(1) | CCM_CCGR4_CG4(1) | CCM_CCGR4_CG5(1) | CCM_CCGR4_CG6(1) | CCM_CCGR4_CG7(1);
    CCM->CCGR5 = CCM_CCGR5_CG0(1) | CCM_CCGR5_CG1(1) | CCM_CCGR5_CG4(1) | CCM_CCGR5_CG6(1) | CCM_CCGR5_CG12(1) |
                 CCM_CCGR5_CG14(1) | CCM_CCGR5_CG15(1);
    /* We can enable DCDC when need to config it and close it after configuration */
    CCM->CCGR6 = CCM_CCGR6_CG3(1) | CCM_CCGR6_CG4(1) | CCM_CCGR6_CG5(1) | CCM_CCGR6_CG9(1) | CCM_CCGR6_CG10(1) |
                 CCM_CCGR6_CG11(1);
}

void PowerDownUSBPHY(void)
{
    USBPHY1->CTRL = 0xFFFFFFFF;
    USBPHY2->CTRL = 0xFFFFFFFF;
}

void ConfigUartRxPinToGpio(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_GPIO1_IO13, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_GPIO1_IO13,
                        IOMUXC_SW_PAD_CTL_PAD_PKE_MASK | IOMUXC_SW_PAD_CTL_PAD_PUS(2) | IOMUXC_SW_PAD_CTL_PAD_PUE_MASK);
}

void ReConfigUartRxPin(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_LPUART1_RX, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_LPUART1_RX, IOMUXC_SW_PAD_CTL_PAD_SPEED(2));
}

#define GPR4_STOP_REQ_BITS                                                                                          \
    (IOMUXC_GPR_GPR4_ENET_STOP_REQ_MASK | IOMUXC_GPR_GPR4_SAI1_STOP_REQ_MASK | IOMUXC_GPR_GPR4_SAI2_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR4_SAI3_STOP_REQ_MASK | IOMUXC_GPR_GPR4_SEMC_STOP_REQ_MASK | IOMUXC_GPR_GPR4_PIT_STOP_REQ_MASK |  \
     IOMUXC_GPR_GPR4_FLEXIO1_STOP_REQ_MASK | IOMUXC_GPR_GPR4_FLEXIO2_STOP_REQ_MASK)

#define GPR4_STOP_ACK_BITS                                                                                          \
    (IOMUXC_GPR_GPR4_ENET_STOP_ACK_MASK | IOMUXC_GPR_GPR4_SAI1_STOP_ACK_MASK | IOMUXC_GPR_GPR4_SAI2_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR4_SAI3_STOP_ACK_MASK | IOMUXC_GPR_GPR4_SEMC_STOP_ACK_MASK | IOMUXC_GPR_GPR4_PIT_STOP_ACK_MASK |  \
     IOMUXC_GPR_GPR4_FLEXIO1_STOP_ACK_MASK | IOMUXC_GPR_GPR4_FLEXIO2_STOP_ACK_MASK)

#define GPR7_STOP_REQ_BITS                                                           \
    (IOMUXC_GPR_GPR7_LPI2C1_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPI2C2_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPI2C3_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPI2C4_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI1_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPSPI2_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI3_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPSPI4_STOP_REQ_MASK |   \
     IOMUXC_GPR_GPR7_LPUART1_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART2_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR7_LPUART3_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART4_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR7_LPUART5_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART6_STOP_REQ_MASK | \
     IOMUXC_GPR_GPR7_LPUART7_STOP_REQ_MASK | IOMUXC_GPR_GPR7_LPUART8_STOP_REQ_MASK)

#define GPR7_STOP_ACK_BITS                                                           \
    (IOMUXC_GPR_GPR7_LPI2C1_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPI2C2_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPI2C3_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPI2C4_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI1_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPSPI2_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPSPI3_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPSPI4_STOP_ACK_MASK |   \
     IOMUXC_GPR_GPR7_LPUART1_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART2_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR7_LPUART3_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART4_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR7_LPUART5_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART6_STOP_ACK_MASK | \
     IOMUXC_GPR_GPR7_LPUART7_STOP_ACK_MASK | IOMUXC_GPR_GPR7_LPUART8_STOP_ACK_MASK)

#define GPR8_DOZE_BITS                                                               \
    (IOMUXC_GPR_GPR8_LPI2C1_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPI2C2_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPI2C3_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPI2C4_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI1_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPSPI2_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI3_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPSPI4_IPG_DOZE_MASK |   \
     IOMUXC_GPR_GPR8_LPUART1_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART2_IPG_DOZE_MASK | \
     IOMUXC_GPR_GPR8_LPUART3_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART4_IPG_DOZE_MASK | \
     IOMUXC_GPR_GPR8_LPUART5_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART6_IPG_DOZE_MASK | \
     IOMUXC_GPR_GPR8_LPUART7_IPG_DOZE_MASK | IOMUXC_GPR_GPR8_LPUART8_IPG_DOZE_MASK)

#define GPR8_STOP_MODE_BITS                                                                    \
    (IOMUXC_GPR_GPR8_LPI2C1_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPI2C2_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPI2C3_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPI2C4_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI1_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPSPI2_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPSPI3_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPSPI4_IPG_STOP_MODE_MASK |   \
     IOMUXC_GPR_GPR8_LPUART2_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPUART3_IPG_STOP_MODE_MASK | \
     IOMUXC_GPR_GPR8_LPUART4_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPUART5_IPG_STOP_MODE_MASK | \
     IOMUXC_GPR_GPR8_LPUART6_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR8_LPUART7_IPG_STOP_MODE_MASK | \
     IOMUXC_GPR_GPR8_LPUART8_IPG_STOP_MODE_MASK)

#define GPR12_DOZE_BITS (IOMUXC_GPR_GPR12_FLEXIO1_IPG_DOZE_MASK | IOMUXC_GPR_GPR12_FLEXIO2_IPG_DOZE_MASK)

#define GPR12_STOP_MODE_BITS (IOMUXC_GPR_GPR12_FLEXIO1_IPG_STOP_MODE_MASK | IOMUXC_GPR_GPR12_FLEXIO2_IPG_STOP_MODE_MASK)

void PeripheralEnterDozeMode(void)
{
    IOMUXC_GPR->GPR8  = GPR8_DOZE_BITS;
    IOMUXC_GPR->GPR12 = GPR12_DOZE_BITS;
}

void PeripheralExitDozeMode(void)
{
    IOMUXC_GPR->GPR8  = 0x00000000;
    IOMUXC_GPR->GPR12 = 0x00000000;
}

void PeripheralEnterStopMode(void)
{
    IOMUXC_GPR->GPR4 = IOMUXC_GPR_GPR4_ENET_STOP_REQ_MASK;
    while ((IOMUXC_GPR->GPR4 & IOMUXC_GPR_GPR4_ENET_STOP_ACK_MASK) != IOMUXC_GPR_GPR4_ENET_STOP_ACK_MASK)
    {
    }
    IOMUXC_GPR->GPR4  = GPR4_STOP_REQ_BITS;
    IOMUXC_GPR->GPR7  = GPR7_STOP_REQ_BITS;
    IOMUXC_GPR->GPR8  = GPR8_DOZE_BITS | GPR8_STOP_MODE_BITS;
    IOMUXC_GPR->GPR12 = GPR12_DOZE_BITS | GPR12_STOP_MODE_BITS;
    while ((IOMUXC_GPR->GPR4 & GPR4_STOP_ACK_BITS) != GPR4_STOP_ACK_BITS)
    {
    }
    while ((IOMUXC_GPR->GPR7 & GPR7_STOP_ACK_BITS) != GPR7_STOP_ACK_BITS)
    {
    }
}

void APP_PrintRunFrequency(int32_t run_freq_only)
{
    PRINTF("\r\n");
    PRINTF("***********************************************************\r\n");
    PRINTF("CPU:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_CpuClk));
    PRINTF("AHB:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AhbClk));
    PRINTF("SEMC:            %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SemcClk));
    PRINTF("IPG:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_IpgClk));
    PRINTF("PER:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_PerClk));
    PRINTF("OSC:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_OscClk));
    PRINTF("RTC:             %d Hz\r\n", CLOCK_GetFreq(kCLOCK_RtcClk));
    if (!run_freq_only)
    {
        PRINTF("ARMPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_ArmPllClk));
        PRINTF("USB1PLL:         %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllClk));
        PRINTF("USB1PLLPFD0:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk));
        PRINTF("USB1PLLPFD1:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd1Clk));
        PRINTF("USB1PLLPFD2:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd2Clk));
        PRINTF("USB1PLLPFD3:     %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb1PllPfd3Clk));
        PRINTF("USB2PLL:         %d Hz\r\n", CLOCK_GetFreq(kCLOCK_Usb2PllClk));
        PRINTF("SYSPLL:          %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllClk));
        PRINTF("SYSPLLPFD0:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd0Clk));
        PRINTF("SYSPLLPFD1:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd1Clk));
        PRINTF("SYSPLLPFD2:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk));
        PRINTF("SYSPLLPFD3:      %d Hz\r\n", CLOCK_GetFreq(kCLOCK_SysPllPfd3Clk));
        PRINTF("ENETPLL0:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_EnetPll0Clk));
        PRINTF("ENETPLL1:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_EnetPll1Clk));
        PRINTF("ENETPLL2:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_EnetPll2Clk));
        PRINTF("AUDIOPLL:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_AudioPllClk));
        PRINTF("VIDEOPLL:        %d Hz\r\n", CLOCK_GetFreq(kCLOCK_VideoPllClk));
    }
    PRINTF("***********************************************************\r\n");
    PRINTF("\r\n");
}
