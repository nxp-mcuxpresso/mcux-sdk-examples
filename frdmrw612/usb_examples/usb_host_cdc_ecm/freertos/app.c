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
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "ping.h"
#include "udpecho_raw.h"
#include "usb_ethernetif.h"

#include "fsl_device_registers.h"
#include "usb_host_config.h"
#include "usb_host.h"
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

/*! @brief Stack size of the thread which prints DHCP info. */
#define PROCESS_THREAD_STACKSIZE 0x200

/*! @brief Priority of the thread which prints DHCP info. */
#define PROCESS_THREAD_PRIO DEFAULT_THREAD_PRIO

#define PING_DOMAIN_NAME    ("www.nxp.com")
#define MUTICAST_GROUP_IP1 (239)
#define MUTICAST_GROUP_IP2 (0)
#define MUTICAST_GROUP_IP3 (0)
#define MUTICAST_GROUP_IP4 (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void stack_init(void *arg);
static void print_dhcp_state(struct netif *netif);
void BOARD_InitHardware(void);
void USB_EthernetIfProcess(void *arg);

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

void USBHS_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}

void USB_HostClockInit(void)
{
    /* reset USB */
    RESET_PeripheralReset(kUSB_RST_SHIFT_RSTn);
    /* enable usb clock */
    CLOCK_EnableClock(kCLOCK_Usb);
    /* enable usb phy clock */
    CLOCK_EnableUsbhsPhyClock();
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
    /* USB_HOST_CONFIG_EHCI */

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}
/**
 * @brief main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

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
    netifapi_netif_add(&netif, NULL, NULL, NULL, &ethernetConfig, EXAMPLE_NETIF_INIT_FN, tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    if (sys_thread_new("EthernetIfProcess", USB_EthernetIfProcess, &netif, PROCESS_THREAD_STACKSIZE,
                       PROCESS_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("USB_EthernetIfProcess(): Task creation failed.", 0);
    }

    vTaskDelete(NULL);
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
void USB_EthernetIfProcess(void *arg)
{
    struct netif *netif                 = (struct netif *)arg;
    static struct netconn *netconn_igmp = NULL;
    while (netif_is_up(netif))
    {
        if (netif_is_link_up(netif) && !linkState)
        {
            if (!dhcpState)
            {
                netifapi_dhcp_start(netif);
                PRINTF(
                    "\r\n"
                    "************************************************\r\n"
                    " DHCP Information\r\n"
                    "************************************************\r\n");

                while (!dhcpState)
                {
                    taskENTER_CRITICAL();
                    print_dhcp_state(netif);
                    taskEXIT_CRITICAL();
                }
            }

            if (!igmpState)
            {
                IP4_ADDR(&multicastGroup, MUTICAST_GROUP_IP1, MUTICAST_GROUP_IP2, MUTICAST_GROUP_IP3,
                         MUTICAST_GROUP_IP4);
                if (netconn_igmp == NULL)
                {
                    netconn_igmp = netconn_new(NETCONN_UDP);
                }
                while (netconn_join_leave_group_netif(netconn_igmp, &multicastGroup, netif_get_index(netif),
                                                      NETCONN_JOIN) != ERR_OK)
                    ;
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
                LOCK_TCPIP_CORE();
                udpecho_raw_init();
                UNLOCK_TCPIP_CORE();
                udpState = 1;
            }
            PRINTF("Server listening on UDP port 7\r\n");

            if (!dnsState)
            {
                while (netconn_gethostbyname(PING_DOMAIN_NAME, &dnsResolution) != ERR_OK)
                    ;
                dnsState = 1;
            }

            if (!pingState)
            {
                PRINTF("Pinging %s [%s]:\r\n", PING_DOMAIN_NAME, ipaddr_ntoa((ip_addr_t *)(&dnsResolution)));
                LOCK_TCPIP_CORE();
                ping_init((ip_addr_t *)(&dnsResolution));
                UNLOCK_TCPIP_CORE();
                pingState = 1;
            }

            linkState = 1;
        }
        else if (!netif_is_link_up(netif) && linkState)
        {
            if (pingState)
            {
                LOCK_TCPIP_CORE();
                ping_stop();
                UNLOCK_TCPIP_CORE();
                pingState = 0;
            }

            if (igmpState)
            {
                while (netconn_join_leave_group_netif(netconn_igmp, &multicastGroup, netif_get_index(netif),
                                                      NETCONN_LEAVE) != ERR_OK)
                    ;
                igmpState = 0;
            }

            if (dhcpState)
            {
                netifapi_dhcp_release_and_stop(netif);
                dhcpState = 0;
            }

            if (dnsState)
            {
                dnsState = 0;
            }

            linkState = 0;
        }
    }
}
#endif
