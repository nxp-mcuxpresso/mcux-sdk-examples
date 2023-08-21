
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "fsl_power.h"
#include "board.h"

#include "ux_api.h"
#include "ux_hcd_ohci.h"

#define USB_INTERRUPT_PRIORITY      (6U)

#define UX_HCD_NAME                 "OHCI"

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE            (60 * 1024)
#endif

ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)];

static void USB_HostClockInit(void)
{
    CLOCK_EnableUsbfs0HostClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
}

static ULONG usb_host_base(VOID)
{
    return USBFSH_BASE;
}

static void usb_interrupt_setup(void)
{
    IRQn_Type irqNumber = USB0_IRQn;

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ(irqNumber);
    NVIC_SetPriority(irqNumber, USB_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
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

    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY);

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0_DEV_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
}

void usb_host_hw_setup(void)
{
    USB_HostClockInit();

    usb_interrupt_setup();
}

UINT usbx_host_hcd_register(void)
{
    UINT status;

    status = ux_host_stack_hcd_register((UCHAR *)UX_HCD_NAME,
                                        _ux_hcd_ohci_initialize,
                                        usb_host_base(), (ULONG)USB0_IRQn);

    return status;
}

VOID usbx_mem_init(VOID)
{
    ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
}

/* interrupt handler for USB0 */
void USB0_IRQHandler(void)
{
    _ux_hcd_ohci_interrupt_handler();
}
