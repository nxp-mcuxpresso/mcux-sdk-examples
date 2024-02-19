/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "usb.h"
#include "usb_phy.h"
#include "board.h"

#include "ux_api.h"
#include "ux_dcd_nxp_dci.h"

#define USB_INTERRUPT_PRIORITY   6

usb_device_handle   deviceHandle;

static void _usb_interrupt_setup(void)
{
    IRQn_Type irqNumber = USB1_HS_IRQn;

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ(irqNumber);
    NVIC_SetPriority(irqNumber, USB_INTERRUPT_PRIORITY);
    EnableIRQ(irqNumber);
}

static void _system_power_control_init(void)
{
    SPC0->ACTIVE_VDELAY = 0x0500;

    /* Change the power DCDC to 1.8v (By deafult, DCDC is 1.8V), CORELDO to 1.1v (By deafult, CORELDO is 1.0V) */
    SPC0->ACTIVE_CFG &= ~SPC_ACTIVE_CFG_CORELDO_VDD_DS_MASK;
    SPC0->ACTIVE_CFG |= SPC_ACTIVE_CFG_DCDC_VDD_LVL(0x3) | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(0x3) |
                        SPC_ACTIVE_CFG_SYSLDO_VDD_DS_MASK | SPC_ACTIVE_CFG_DCDC_VDD_DS(0x2u);

    /* Wait until it is done */
    while (SPC0->SC & SPC_SC_BUSY_MASK)
        ;

    /* Enable LDO. */
    if (0u == (SCG0->LDOCSR & SCG_LDOCSR_LDOEN_MASK))
    {
        SCG0->TRIM_LOCK = 0x5a5a0001U;
        SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
        /* wait LDO ready */
        while (0U == (SCG0->LDOCSR & SCG_LDOCSR_VOUT_OK_MASK))
            ;
    }
}

static void _system_clock_init(void)
{
    /* Enable clock for USB_HS and USB_HS_PHY in AHB Clock Control 2. */
    SYSCON->AHBCLKCTRLSET[2] |= SYSCON_AHBCLKCTRL2_USB_HS_MASK | SYSCON_AHBCLKCTRL2_USB_HS_PHY_MASK;

    SCG0->SOSCCFG &= ~(SCG_SOSCCFG_RANGE_MASK | SCG_SOSCCFG_EREFS_MASK);
    /* Select the frequency range of 20 ~ 30 MHz. Select the internal OSC. */
    SCG0->SOSCCFG |= SCG_SOSCCFG_RANGE(1) | SCG_SOSCCFG_EREFS(1);

    /* Eable SOSC. */
    SCG0->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
    while (0 == (SCG0->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK))
        ;

    /* Enables clk_in clock for USBFS, USB HS, ... */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK | SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT_MASK;
}

void board_setup(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
}

void usb_device_setup(void)
{
    _ux_dcd_nxp_dci_initialize(kUSB_ControllerEhci0, &deviceHandle);

    _usb_interrupt_setup();
}

void usb_device_hw_setup(void)
{
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    _system_power_control_init();
    _system_clock_init();

    CLOCK_EnableClock(kCLOCK_UsbHs);
    CLOCK_EnableClock(kCLOCK_UsbHsPhy);
    CLOCK_EnableUsbhsPhyPllClock(kCLOCK_Usbphy480M, 24000000U);
    CLOCK_EnableUsbhsClock();

    USB_EhciPhyInit(kUSB_ControllerEhci0, BOARD_XTAL0_CLK_HZ, &phyConfig);
}

void USB1_HS_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(deviceHandle);
}
