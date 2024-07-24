/**
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "app.h"

#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_DHCP && LWIP_DNS && LWIP_RAW && LWIP_IGMP && LWIP_UDP
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwip/prot/dhcp.h"
#include "lwip/timeouts.h"
#include "netif/ethernet.h"
#include "ping.h"
#include "udpecho_raw.h"
#include "usb_ethernetif.h"

#include "fsl_device_registers.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_phy.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief network interface initialization function */
#define EXAMPLE_NETIF_INIT_FN USB_EthernetIfInIt
#endif

#define PING_DOMAIN_NAME   ("www.nxp.com")
#define MUTICAST_GROUP_IP1 (239)
#define MUTICAST_GROUP_IP2 (0)
#define MUTICAST_GROUP_IP3 (0)
#define MUTICAST_GROUP_IP4 (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void lwip_dns_found(const char *name, const ip_addr_t *ipaddr, void *arg);
static void print_dhcp_state(struct netif *netif);
void BOARD_InitHardware(void);
void USB_EthernetIfProcess(struct netif *netif);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_host_handle g_HostHandle;
struct netif netif;
ethernetifConfig_t ethernetConfig;
ip4_addr_t multicastGroup;
ip4_addr_t dnsResolution;
uint8_t linkState = 0;
uint8_t dhcpState = 0;
uint8_t dnsState  = 0;
uint8_t pingState = 0;
uint8_t igmpState = 0;
uint8_t udpState  = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
void USB1_HS_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}
#endif /* USB_HOST_CONFIG_EHCI */
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
void USB0_FS_IRQHandler(void)
{
    USB_HostKhciIsrFunction(g_HostHandle);
}
#endif /* USB_HOST_CONFIG_KHCI */

void USB_HostClockInit(void)
{
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif

#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    SPC0->ACTIVE_VDELAY = 0x0500;
    /* Change the power DCDC to 1.8v (By deafult, DCDC is 1.8V), CORELDO to 1.1v (By deafult, CORELDO is 1.0V) */
    SPC0->ACTIVE_CFG &= ~SPC_ACTIVE_CFG_CORELDO_VDD_DS_MASK;
    SPC0->ACTIVE_CFG |= SPC_ACTIVE_CFG_DCDC_VDD_LVL(0x3) | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(0x3) |
                        SPC_ACTIVE_CFG_SYSLDO_VDD_DS_MASK | SPC_ACTIVE_CFG_DCDC_VDD_DS(0x2u);
    /* Wait until it is done */
    while (SPC0->SC & SPC_SC_BUSY_MASK)
        ;
    if (0u == (SCG0->LDOCSR & SCG_LDOCSR_LDOEN_MASK))
    {
        SCG0->TRIM_LOCK = 0x5a5a0001U;
        SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
        /* wait LDO ready */
        while (0U == (SCG0->LDOCSR & SCG_LDOCSR_VOUT_OK_MASK))
            ;
    }
    SYSCON->AHBCLKCTRLSET[2] |= SYSCON_AHBCLKCTRL2_USB_HS_MASK | SYSCON_AHBCLKCTRL2_USB_HS_PHY_MASK;
    SCG0->SOSCCFG &= ~(SCG_SOSCCFG_RANGE_MASK | SCG_SOSCCFG_EREFS_MASK);
    /* xtal = 20 ~ 30MHz */
    SCG0->SOSCCFG = (1U << SCG_SOSCCFG_RANGE_SHIFT) | (1U << SCG_SOSCCFG_EREFS_SHIFT);
    SCG0->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
    while (1)
    {
        if (SCG0->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)
        {
            break;
        }
    }
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK | SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT_MASK;
    CLOCK_EnableClock(kCLOCK_UsbHs);
    CLOCK_EnableClock(kCLOCK_UsbHsPhy);
    CLOCK_EnableUsbhsPhyPllClock(kCLOCK_Usbphy480M, 24000000U);
    CLOCK_EnableUsbhsClock();
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    CLOCK_AttachClk(kCLK_48M_to_USB0);
    CLOCK_EnableClock(kCLOCK_Usb0Ram);
    CLOCK_EnableClock(kCLOCK_Usb0Fs);
    CLOCK_EnableUsbfsClock();
#endif
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
#endif /* USB_HOST_CONFIG_EHCI */
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    uint8_t usbHOSTKhciIrq[] = USBFS_IRQS;
    irqNumber                = usbHOSTKhciIrq[CONTROLLER_ID - kUSB_ControllerKhci0];
#endif /* USB_HOST_CONFIG_KHCI */

/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    USB_HostEhciTaskFunction(param);
#endif
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    USB_HostKhciTaskFunction(param);
#endif
}
/**
 * @brief main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    time_init();
    lwip_init();

    ethernetConfig.controllerId = CONTROLLER_ID;
    ethernetConfig.privateData  = NULL;
    netif_add(&netif, NULL, NULL, NULL, &ethernetConfig, EXAMPLE_NETIF_INIT_FN, ethernet_input);
    netif_set_default(&netif);
    netif_set_up(&netif);

    while (1)
    {
        USB_EthernetIfInput(&netif);
        sys_check_timeouts();
        USB_EthernetIfProcess(&netif);
    }
}

/**
 * @brief interrupt service for SysTick timer
 */
void SysTick_Handler(void)
{
    time_isr();
}

/**
 * @brief prints DHCP status of the interface when it has changed from last status
 * @param netif network interface structure
 */
static void print_dhcp_state(struct netif *netif)
{
    static u8_t dhcp_last_state = DHCP_STATE_OFF;
    struct dhcp *dhcp           = netif_dhcp_data(netif);

    if (dhcp == NULL)
    {
        dhcp_last_state = DHCP_STATE_OFF;
    }
    else if (dhcp_last_state != dhcp->state)
    {
        dhcp_last_state = dhcp->state;

        PRINTF(" DHCP state       : ");
        switch (dhcp_last_state)
        {
            case DHCP_STATE_OFF:
                PRINTF("OFF");
                break;

            case DHCP_STATE_REQUESTING:
                PRINTF("REQUESTING");
                break;

            case DHCP_STATE_INIT:
                PRINTF("INIT");
                break;

            case DHCP_STATE_REBOOTING:
                PRINTF("REBOOTING");
                break;

            case DHCP_STATE_REBINDING:
                PRINTF("REBINDING");
                break;

            case DHCP_STATE_RENEWING:
                PRINTF("RENEWING");
                break;

            case DHCP_STATE_SELECTING:
                PRINTF("SELECTING");
                break;

            case DHCP_STATE_INFORMING:
                PRINTF("INFORMING");
                break;

            case DHCP_STATE_CHECKING:
                PRINTF("CHECKING");
                break;

            case DHCP_STATE_BOUND:
                PRINTF("BOUND");
                break;

            case DHCP_STATE_BACKING_OFF:
                PRINTF("BACKING_OFF");
                break;

            default:
                PRINTF("%u", dhcp_last_state);
                assert(0);
                break;
        }
        PRINTF("\r\n");

        if (dhcp_last_state == DHCP_STATE_BOUND)
        {
            dhcpState = 1;
            PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
            PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
            PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
        }
    }
}

/**
 * @brief check DHCP state and process ICMP & IGMP
 * @param netif network interface structure
 */
void USB_EthernetIfProcess(struct netif *netif)
{
    if (netif_is_link_up(netif) && !linkState)
    {
        if (!dhcpState)
        {
            dhcp_start(netif);
            PRINTF(
                "\r\n"
                "************************************************\r\n"
                " DHCP Information\r\n"
                "************************************************\r\n");
            while (!dhcp_supplied_address(netif))
            {
                USB_EthernetIfInput(netif);
                sys_check_timeouts();
                print_dhcp_state(netif);
            }
        }

        if (!igmpState)
        {
            IP4_ADDR(&multicastGroup, MUTICAST_GROUP_IP1, MUTICAST_GROUP_IP2, MUTICAST_GROUP_IP3, MUTICAST_GROUP_IP4);
            igmp_joingroup_netif(netif, &multicastGroup);
            igmpState = 1;

            PRINTF(
                "************************************************\r\n"
                " IGMP Information\r\n"
                "************************************************\r\n");
            PRINTF(" IPv4 Group       : %s\r\n", ipaddr_ntoa(&multicastGroup));
            PRINTF("\r\n");
        }

        if (!udpState)
        {
            udpecho_raw_init();
            udpState = 1;
        }
        PRINTF("Server listening on UDP port 7\r\n");

        if (!dnsState)
        {
            if (dns_gethostbyname(PING_DOMAIN_NAME, &dnsResolution, lwip_dns_found, NULL) != ERR_OK)
            {
                while (!dnsState)
                {
                    USB_EthernetIfInput(netif);
                    sys_check_timeouts();
                }
            }
        }

        if (!pingState)
        {
            PRINTF("Pinging %s [%s]:\r\n", PING_DOMAIN_NAME, ipaddr_ntoa((ip_addr_t *)(&dnsResolution)));
            ping_init((ip_addr_t *)(&dnsResolution));
            pingState = 1;
        }

        linkState = 1;
    }
    else if (!netif_is_link_up(netif) && linkState)
    {
        if (pingState)
        {
            ping_stop();
            pingState = 0;
        }

        if (igmpState)
        {
            igmp_leavegroup_netif(netif, &multicastGroup);
            igmpState = 0;
        }

        if (dhcpState)
        {
            dhcp_release_and_stop(netif);
            dhcpState = 0;
        }

        if (dnsState)
        {
            dnsState = 0;
        }

        linkState = 0;
    }
}

/**
 * @brief callback when DNS resolution of dns_gethostbyname is successful
 * @param name DNS address input
 * @param ipaddr resolved ip address
 * @param arg arguments invoked
 */
static void lwip_dns_found(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr != NULL)
    {
        dnsResolution.addr = ipaddr->addr;
        dnsState           = 1;
    }
    else
    {
        PRINTF("DNS request error.\r\n");
    }

    return;
}
#endif
