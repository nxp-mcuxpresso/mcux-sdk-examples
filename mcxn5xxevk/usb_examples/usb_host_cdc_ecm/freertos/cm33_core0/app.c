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

#if LWIP_IPV4 && LWIP_DNS && LWIP_RAW && LWIP_IGMP && LWIP_UDP
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "ping.h"
#include "udpecho_raw.h"
#include "usb_ethernetif.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#endif

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

/*! @brief Stack size of the temporary lwIP initialization thread. */
#define INIT_THREAD_STACKSIZE 0x400

/*! @brief Priority of the temporary lwIP initialization thread. */
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

/*! @brief Stack size of the EthernetIfProcess thread. */
#define PROCESS_THREAD_STACKSIZE 0x200

/*! @brief Priority of the EthernetIfProcess thread. */
#define PROCESS_THREAD_PRIO DEFAULT_THREAD_PRIO

#ifndef NETIF_STATIC_ADDR
#define NETIF_STATIC_ADDR ("192.168.0.1")
#endif

#ifndef NETIF_STATIC_MASK
#define NETIF_STATIC_MASK ("255.255.255.0")
#endif

#ifndef NETIF_STATIC_GW
#define NETIF_STATIC_GW ("192.168.0.254")
#endif

#ifndef NETIF_STATIC_DNS
#define NETIF_STATIC_DNS ("1.1.1.1")
#endif

#define PING_DOMAIN_NAME    ("nxp.com")
#define IGMP_MUTICAST_GROUP ("239.0.0.1")

#define NETIF_LINK_STATE_OK (1U << 0)
#define NETIF_LINK_IP_OK    (1U << 1)
#define NETIF_LINK_IGMP_OK  (1U << 2)
#define NETIF_LINK_READY    (1U << 3)
#define NETIF_LINK_MASK     (0x0000000FU)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void stack_init(void *arg);
void BOARD_InitHardware(void);
void USB_EthernetIfProcess(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_host_handle g_HostHandle;
struct netif netif;
ethernetifConfig_t ethernetConfig;
uint32_t netifLinkState = 0;

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

    /* Initialize lwIP from thread */
    if (sys_thread_new("stack_init", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("stack_init(): Task creation failed.", 0);
    }

    vTaskStartScheduler();
}

/**
 * @brief initializes LwIP stack
 */
static void stack_init(void *arg)
{
    ethernetConfig.controllerId = CONTROLLER_ID;
    ethernetConfig.privateData  = NULL;
    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, &ethernetConfig, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    netifLinkState &= ~NETIF_LINK_MASK;

    if (sys_thread_new("EthernetIfProcess", USB_EthernetIfProcess, &netif, PROCESS_THREAD_STACKSIZE,
                       PROCESS_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("USB_EthernetIfProcess(): Task creation failed.", 0);
    }

    vTaskDelete(NULL);
}

/**
 * @brief check DHCP state and process ICMP & IGMP
 * @param netif network interface structure
 */
void USB_EthernetIfProcess(void *arg)
{
    struct netif *netif          = (struct netif *)arg;
    struct netconn *netconn_igmp = NULL;
    ip4_addr_t netif_multicastGroup;
    while (netif_is_up(netif))
    {
        if (netif_is_link_up(netif) && !(netifLinkState & NETIF_LINK_STATE_OK))
        {
            netifLinkState |= NETIF_LINK_STATE_OK;

#if LWIP_DHCP
            PRINTF("Get IPv4 information from DHCP\r\n");
            netifapi_dhcp_start(netif);
            PRINTF("Waiting DHCP server process...");
            u8_t dhcp_last_state = DHCP_STATE_OFF;
            u8_t dhcp_state      = DHCP_STATE_OFF;
            while (!(netifLinkState & NETIF_LINK_IP_OK))
            {
                if (!netif_is_link_up(netif))
                {
                    PRINTF(" ERROR!\r\n");
                    break;
                }

                dhcp_state = ((struct dhcp *)(netif_dhcp_data(netif)))->state;
                if (dhcp_state != dhcp_last_state)
                {
                    PRINTF(" %d ", dhcp_state);
                }
                dhcp_last_state = dhcp_state;

                if (dhcp_state == DHCP_STATE_BOUND)
                {
                    netifLinkState |= NETIF_LINK_IP_OK;
                    PRINTF(" OK!\r\n");
                }
            }
#else
            PRINTF("Setting IPv4 information...");
            ip4_addr_t netif_addr, netif_mask, netif_gw, netif_dns;
            ip4addr_aton(NETIF_STATIC_ADDR, &netif_addr);
            ip4addr_aton(NETIF_STATIC_MASK, &netif_mask);
            ip4addr_aton(NETIF_STATIC_GW, &netif_gw);
            ip4addr_aton(NETIF_STATIC_DNS, &netif_dns);
            if (netifapi_netif_set_addr(netif, &netif_addr, &netif_mask, &netif_gw) != ERR_OK)
            {
                PRINTF("Setting IP address error\r\n");
            }
            else
            {
                dns_setserver(0, &netif_dns);
                netifLinkState |= NETIF_LINK_IP_OK;
                PRINTF(" OK!\r\n");
            }
#endif

            if (netifLinkState & NETIF_LINK_IP_OK)
            {
                ip4addr_aton(IGMP_MUTICAST_GROUP, &netif_multicastGroup);
                if (netconn_igmp == NULL)
                {
                    netconn_igmp = netconn_new(NETCONN_UDP);
                }
                if (netconn_join_leave_group_netif(netconn_igmp, &netif_multicastGroup, netif_get_index(netif),
                                                   NETCONN_JOIN) != ERR_OK)
                {
                    PRINTF("Joining group error\r\n");
                }
                else
                {
                    netifLinkState |= NETIF_LINK_IGMP_OK;
                }

                if (netifLinkState & NETIF_LINK_IGMP_OK)
                {
                    netifLinkState |= NETIF_LINK_READY;
                    PRINTF(
                        "\r\n"
                        "************************************************\r\n"
                        " Network Interface Information\r\n"
                        "************************************************\r\n");
                    PRINTF(" IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
                    PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
                    PRINTF(" IPv4 Gateway     : %s\r\n", ipaddr_ntoa(&netif->gw));
                    for (u8_t num = 0; num < DNS_MAX_SERVERS; num++)
                    {
                        PRINTF(" IPv4 DNS%2d       : %s\r\n", num, ipaddr_ntoa(dns_getserver(num)));
                    }
                    PRINTF(" IPv4 Group       : %s\r\n", ipaddr_ntoa(&netif_multicastGroup));
                    PRINTF(" MAC Address      : %X:%X:%X:%X:%X:%X\r\n\r\n", netif->hwaddr[0], netif->hwaddr[1],
                           netif->hwaddr[2], netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
                }
            }

            if (netifLinkState & NETIF_LINK_READY)
            {
                LOCK_TCPIP_CORE();
                udpecho_raw_init();
                UNLOCK_TCPIP_CORE();
                PRINTF("Server listening on UDP port 7\r\n");

                ip4_addr_t dnsResolution = *IP4_ADDR_ANY;
                u8_t retryCount          = 0;
                u8_t linkErr             = 0;
                PRINTF("\r\nStart resolving domain name (%s)...\r\n", PING_DOMAIN_NAME);
                while (netconn_gethostbyname(PING_DOMAIN_NAME, &dnsResolution) != ERR_OK && retryCount++ < 5)
                {
                    if (!netif_is_link_up(netif))
                    {
                        linkErr = 1;
                        break;
                    }
                    PRINTF("Retrying to resolve domain name (%s) %d...\r\n", PING_DOMAIN_NAME, retryCount);
                }

                if (!linkErr)
                {
                    if (ip4_addr_cmp(&dnsResolution, IP4_ADDR_ANY))
                    {
                        PRINTF("Resolving domain name (%s) error\r\n", PING_DOMAIN_NAME);
                        ip4addr_aton(ipaddr_ntoa(dns_getserver(0)), &dnsResolution);
                        PRINTF("\r\nTry to ping DNS server\r\n");
                    }
                    else
                    {
                        PRINTF("Domain name resolution success\r\n");
                        PRINTF("\r\nTry to ping %s\r\n", PING_DOMAIN_NAME);
                    }

                    PRINTF("Start pinging [%s]:\r\n", ipaddr_ntoa(&dnsResolution));
                    LOCK_TCPIP_CORE();
                    ping_init(&dnsResolution);
                    UNLOCK_TCPIP_CORE();
                }
            }
        }

        if (!netif_is_link_up(netif) && (netifLinkState & NETIF_LINK_STATE_OK))
        {
            netifLinkState &= ~NETIF_LINK_READY;

            LOCK_TCPIP_CORE();
            ping_stop();
            UNLOCK_TCPIP_CORE();

            if (netifLinkState & NETIF_LINK_IGMP_OK)
            {
                if (netconn_join_leave_group_netif(netconn_igmp, &netif_multicastGroup, netif_get_index(netif),
                                                   NETCONN_LEAVE) != ERR_OK)
                {
                    PRINTF("Leaving group error\r\n");
                }
                else
                {
                    netifLinkState &= ~NETIF_LINK_IGMP_OK;
                }
            }

            if (netifLinkState & NETIF_LINK_IP_OK)
            {
#if LWIP_DHCP
                if (netifapi_dhcp_release_and_stop(netif) != ERR_OK)
                {
                    PRINTF("Releasing & stopping DHCP server error\r\n");
                }
#else
                if (netifapi_netif_set_addr(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY) != ERR_OK)
                {
                    PRINTF("Clearing IP address error\r\n");
                }
#endif
                netifLinkState &= ~NETIF_LINK_IP_OK;
            }

            netifLinkState &= ~NETIF_LINK_STATE_OK;
        }
    }
}
#endif
