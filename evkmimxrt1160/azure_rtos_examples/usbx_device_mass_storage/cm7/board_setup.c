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
#define CONTROLLER_ID            kUSB_ControllerEhci0

usb_device_handle   deviceHandle;

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

static void usb_interrupt_setup(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ((IRQn_Type)irqNumber);
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

void board_setup(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
}

void usb_device_setup(void)
{
    _ux_dcd_nxp_dci_initialize(CONTROLLER_ID, &deviceHandle);

    usb_interrupt_setup();
}

void usb_device_hw_setup(void)
{
    uint32_t usbClockFreq = 24000000;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
    CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);

    delay_ms(200);
}

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(deviceHandle);
}
