
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "usb.h"
#include "usb_phy.h"
#include "fsl_power.h"
#include "board.h"

#include "fsl_debug_console.h"

/* use high speed host controller */
#define USB_HOST_CONFIG_IP3516HS    (1U)

#if defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif

extern void _ux_hcd_ip3516_interrupt_handler(void);

static void USB_HostClockInit(void)
{

    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0HostClock(kCLOCK_UsbSrcUnused, 0U);

    USBPHY->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL2_MASK;
    USBPHY->CTRL |= USBPHY_CTRL_SET_ENUTMILEVEL3_MASK;
    USBPHY->CTRL_SET = USBPHY_CTRL_SET_ENAUTOCLR_CLKGATE_MASK;
    USBPHY->CTRL_SET = USBPHY_CTRL_SET_ENAUTOCLR_PHY_PWD_MASK;

    /* put the USB PHY in the normal operation state */
    USBPHY->PWD = 0U;

#if ((defined FSL_FEATURE_USBHSH_USB_RAM) && (FSL_FEATURE_USBHSH_USB_RAM > 0U))
    for (int i = 0; i < (FSL_FEATURE_USBHSH_USB_RAM >> 2); i++)
    {
        ((uint32_t *)FSL_FEATURE_USBHSH_USB_RAM_BASE_ADDRESS)[i] = 0U;
    }
#endif
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

unsigned long usb_host_base(void)
{
    return USBHSH_BASE;
}

void usb_host_setup(void)
{
    USB_HostClockInit();
}

void usb_host_interrupt_setup(void)
{
    NVIC_SetPriority((IRQn_Type)USB1_IRQn, USB_HOST_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)USB1_IRQn);
}

/* interrupt handler for USB1 */
void USB1_IRQHandler(void)
{
    _ux_hcd_ip3516_interrupt_handler();

    SDK_ISR_EXIT_BARRIER;
}
