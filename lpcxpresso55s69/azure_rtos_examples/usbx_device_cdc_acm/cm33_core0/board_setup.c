
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "usb.h"
#include "usb_phy.h"
#include "fsl_power.h"
#include "board.h"

#include "ux_api.h"
#include "ux_dcd_ip3511.h"

#define USB_INTERRUPT_PRIORITY   6

usb_device_handle deviceHandle;

__STATIC_INLINE void board_delay(void)
{
    volatile int i;

    for (i = 0; i < 1000; i++)
    {
        __ASM volatile ("nop");
    }
}

static void USB_DeviceClockInit(void)
{
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUnused, 0U);

    USBPHY->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL2_MASK;
    USBPHY->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL3_MASK;
    USBPHY->CTRL_SET = USBPHY_CTRL_SET_ENAUTOCLR_CLKGATE_MASK;
    USBPHY->CTRL_SET = USBPHY_CTRL_SET_ENAUTOCLR_PHY_PWD_MASK;

    /* put the USB PHY in the normal operation state */
    USBPHY->PWD = 0U;

#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
}

static void usb_interrupt_setup(void)
{
    IRQn_Type irqNumber = USB1_IRQn;

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ(irqNumber);
    NVIC_SetPriority(irqNumber, USB_INTERRUPT_PRIORITY);
    EnableIRQ(irqNumber);
}

void board_setup(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);
}

void usb_device_setup(void)
{
    _ux_dcd_ip3511_initialize(kUSB_ControllerLpcIp3511Hs0, &deviceHandle);

    usb_interrupt_setup();
}

void usb_device_hw_setup(void)
{
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_Usbh1);

    /* Put PHY powerdown under software control */
    USBHSH->PORTMODE |= USBHSH_PORTMODE_SW_PDCOM_MASK;

    board_delay();

    /* device mode setting has to be set by access usb host register */
    USBHSH->PORTMODE |= USBHSH_PORTMODE_DEV_ENABLE_MASK;

    /* disable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);

    USB_DeviceClockInit();
}

/* Interrupt handler for USB1 interrupt */
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(deviceHandle);

    SDK_ISR_EXIT_BARRIER;
}
