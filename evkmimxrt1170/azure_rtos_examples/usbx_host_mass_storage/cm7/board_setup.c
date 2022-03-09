
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "usb.h"
#include "usb_phy.h"
#include "board.h"

#include "ux_api.h"
#include "ux_hcd_ehci.h"

#define USB_INTERRUPT_PRIORITY      (6U)

#define UX_HCD_NAME         "EHCI HOST"

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE            (60 * 1024)
#endif

#ifndef USBX_MEMORY_CACHESAFE_SIZE
#define USBX_MEMORY_CACHESAFE_SIZE  (60 * 1024)
#endif

ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)];
AT_NONCACHEABLE_SECTION_ALIGN(char usb_memory_cachesafe[USBX_MEMORY_CACHESAFE_SIZE], 64);

static ULONG usb_host_base(VOID)
{
    /* For EHCI core.  */
    return (USB_OTG1_BASE + 0x100);
}

static void usb_interrupt_setup(void)
{
    IRQn_Type irqNumber = USB_OTG1_IRQn;

    /* Clear pending IRQ, set priority, and enable IRQ. */
    NVIC_ClearPendingIRQ(irqNumber);
    NVIC_SetPriority(irqNumber, USB_INTERRUPT_PRIORITY);
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

void usb_host_hw_setup(void)
{
    uint32_t usbClockFreq = 24000000;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
    CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);

    USB_EhciPhyInit(kUSB_ControllerEhci0, BOARD_XTAL0_CLK_HZ, &phyConfig);

    usb_interrupt_setup();
}

UINT usbx_host_hcd_register(VOID)
{
    UINT status;

    status = ux_host_stack_hcd_register((UCHAR *)UX_HCD_NAME,
                                        _ux_hcd_ehci_initialize,
                                        usb_host_base(), (ULONG)USB_OTG1_IRQn);

    return status;
}

VOID usbx_mem_init(VOID)
{
    ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE,
                         usb_memory_cachesafe, USBX_MEMORY_CACHESAFE_SIZE);

}

/* this function is for the macro UX_HCD_EHCI_EXT_USBPHY_HIGHSPEED_MODE_SET in ux_user.h */
void usbphy_set_highspeed_mode(void *regs, int on_off)
{
    USB_Type* usb_base[] = USB_BASE_PTRS;
    USBPHY_Type* usbphy_base[] = USBPHY_BASE_PTRS;
    uint32_t ehci_base;
    UX_HCD_EHCI *hcd_ehci;
    int i;

    hcd_ehci = (UX_HCD_EHCI *)regs;
    if (hcd_ehci->ux_hcd_ehci_base == 0)
        return;

    /* the first value in USB_BASE_ADDRS is always zero, so skip it */
    for (i = 1; i < sizeof(usb_base) / sizeof(usb_base[0]); i++)
    {
        ehci_base = (uint32_t)(&usb_base[i]->CAPLENGTH);
        if ((uint32_t)hcd_ehci->ux_hcd_ehci_base == ehci_base) {
            if (on_off)
                usbphy_base[i]->CTRL_SET = USBPHY_CTRL_SET_ENHOSTDISCONDETECT_MASK;
            else
                usbphy_base[i]->CTRL_CLR = USBPHY_CTRL_CLR_ENHOSTDISCONDETECT_MASK;
        }
    }
}
