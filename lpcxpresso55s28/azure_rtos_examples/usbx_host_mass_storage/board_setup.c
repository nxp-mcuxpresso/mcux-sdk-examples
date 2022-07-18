
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "usb.h"
#include "usb_phy.h"
#include "fsl_power.h"
#include "board.h"

#include "ux_api.h"
#include "ux_hcd_ip3516.h"
#include "ux_hcd_ohci.h"

/* use high speed host controller */
#define USB_HOST_CONFIG_IP3516HS    (1U)
#define USB_INTERRUPT_PRIORITY      (6U)

#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
#define UX_HCD_NAME         "IP3516HS"
#else
#define UX_HCD_NAME         "OHCI"
#endif

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE                (60 * 1024)
#endif

ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)];

static void USB_HostClockInit(void)
{
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
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

#else
    CLOCK_EnableUsbfs0HostClock(kCLOCK_UsbfsSrcPll1, 48000000U);
#if ((defined FSL_FEATURE_USBFSH_USB_RAM) && (FSL_FEATURE_USBFSH_USB_RAM > 0U))
    for (int i = 0; i < (FSL_FEATURE_USBFSH_USB_RAM >> 2); i++)
    {
        ((uint32_t *)FSL_FEATURE_USBFSH_USB_RAM_BASE_ADDRESS)[i] = 0U;
    }
#endif
#endif /* USB_HOST_CONFIG_IP3511HS */
}

static ULONG usb_host_base(VOID)
{
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    return USBHSH_BASE;
#else
    return USBFSH_BASE;
#endif
}

static void usb_interrupt_setup(void)
{
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    IRQn_Type irqNumber = USB1_IRQn;
#else
    IRQn_Type irqNumber = USB0_IRQn;
#endif

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ(irqNumber);
    NVIC_SetPriority(irqNumber, USB_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
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

void usb_host_hw_setup(void)
{
    USB_HostClockInit();

    usb_interrupt_setup();
}

UINT usbx_host_hcd_register(void)
{
    UINT status;

#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    status = ux_host_stack_hcd_register((UCHAR *)UX_HCD_NAME,
                                        _ux_hcd_ip3516_initialize,
                                        usb_host_base(), (ULONG)USB1_IRQn);
#else
    status = ux_host_stack_hcd_register((UCHAR *)UX_HCD_NAME,
                                        _ux_hcd_ohci_initialize,
                                        usb_host_base(), (ULONG)USB0_IRQn);
#endif
    return status;
}

VOID usbx_mem_init(VOID)
{
    ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
}

#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
/* interrupt handler for USB1 */
void USB1_IRQHandler(void)
{
    _ux_hcd_ip3516_interrupt_handler();

    SDK_ISR_EXIT_BARRIER;
}
#else
/* interrupt handler for USB0 */
void USB0_IRQHandler(void)
{
    _ux_hcd_ohci_interrupt_handler();

    SDK_ISR_EXIT_BARRIER;
}
#endif
