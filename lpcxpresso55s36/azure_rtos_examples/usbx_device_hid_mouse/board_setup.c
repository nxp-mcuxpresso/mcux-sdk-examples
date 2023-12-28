
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "usb.h"
#include "fsl_power.h"
#include "board.h"

#include "ux_api.h"
#include "ux_dcd_ip3511.h"

#define USB_INTERRUPT_PRIORITY   6

usb_device_handle deviceHandle;

static void usb_interrupt_setup(void)
{
    IRQn_Type irqNumber = USB0_IRQn;

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ(irqNumber);
    NVIC_SetPriority(irqNumber, USB_INTERRUPT_PRIORITY);
    EnableIRQ(irqNumber);
}

void board_setup(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY); /*< Turn on USB0 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0_DEV_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
}

void usb_device_hw_setup(void)
{
    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY); /*< Turn on USB0 Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    USBFSH->PORTMODE |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);

    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());

}

void usb_device_setup(void)
{
    _ux_dcd_ip3511_initialize(kUSB_ControllerLpcIp3511Fs0, &deviceHandle);

    usb_interrupt_setup();
}

void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(deviceHandle);
}
