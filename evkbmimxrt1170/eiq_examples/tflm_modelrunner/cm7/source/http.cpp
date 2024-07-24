/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lwip/opt.h"
#include "http_handler.h"
#include "modelrunner.h"

#if LWIP_IPV4 && LWIP_DHCP && LWIP_NETCONN

#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/sys.h"
#include "ethernetif.h"
#include "lwip/sockets.h"

#include "pin_mux.h"
#include "board.h"
#include "app.h"
#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif

static void print_dhcp_state(void *arg);

static phy_handle_t phyHandle;
static int ip_ready = 0;

/*!
 * @brief Initializes lwIP stack.
 *
 * @param arg unused
 */
static void stack_init(void *arg)
{
    static struct netif netif;
    ethernetif_config_t enet_config = {.phyHandle   = &phyHandle,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = EXAMPLE_PHY_OPS,
        .phyResource = EXAMPLE_PHY_RESOURCE,
        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
#ifdef configMAC_ADDR
        .macAddress = configMAC_ADDR
#endif
    };

    LWIP_UNUSED_ARG(arg);

    /* Set MAC address. */
#ifndef configMAC_ADDR
    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, NULL, NULL, NULL, &enet_config,
                       EXAMPLE_NETIF_INIT_FN, tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    netifapi_dhcp_start(&netif);

    if (sys_thread_new("print_dhcp", print_dhcp_state, &netif,
                       PRINT_THREAD_STACKSIZE, PRINT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("stack_init(): Task creation failed.", 0);
    }

    vTaskDelete(NULL);
}

/*!
 * @brief Prints DHCP status of the interface when it has changed from last status.
 *
 * @param arg pointer to network interface structure
 */
static void print_dhcp_state(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp;
    u8_t dhcp_last_state = DHCP_STATE_OFF;

    while (netif_is_up(netif))
    {
        dhcp = netif_dhcp_data(netif);

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
                PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
                PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
                PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
                break;
            }
        }
        sys_msleep(20U);
    }
    ip_ready = 1;
    vTaskDelete(NULL);
}
static void serve(void *arg){
    NNServer *server = (NNServer *) arg;
    int                 err;
    int                 listener;
    struct sockaddr_in  addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(10818);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        PRINTF("listener err\r\n");

    }

    err = bind(listener, (struct sockaddr*) &addr, sizeof(addr));
    if (err) {
        PRINTF("bind err\r\n");
        //goto finish;
    }

    err = listen(listener, 16);
    if (err) {
        PRINTF("listen err\r\n");

        //goto finish;
    }
    while(!ip_ready){
        asm("nop");
    };

    PRINTF("Initialized TFLiteMicro modelrunner server at port %d\r\n",10818);

    //char server[1000];


    struct http_router routes[] = {{"/", "GET", (NNServer*)(server), v1_handler_get},
                                   {"/v1", "GET", (NNServer*)(server), v1_handler_get},
                                   {"/v1/model", "GET", (NNServer*)(server), v1_model_handler_get},
                                   {"/v1", "PUT", (NNServer*)(server), v1_handler_put},
                                   {"/v1", "POST", (NNServer*)(server), v1_handler_post},
                                   {NULL, NULL, NULL, NULL}};

    int sock;
    while (1) {
        sock   = accept(listener, NULL, NULL);
        if (sock == -1) {
            PRINTF("Invalid socket: %s\r\n",strerror(errno));
            continue;
        }
        handle_client(sock, routes);
        closesocket(sock);
    }
    PRINTF("HTTP handler 10919\r\n");
}

/*!
 * @brief Main function.
 *
 */
int http(NNServer* server)
{
#ifdef MODELRUNNER_HTTP
    /* Initialize lwIP from thread */
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }
    if (sys_thread_new("serve", serve, server, 4096, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }
#else
    if (sys_thread_new("cmd", parse_cmd, server, 4096, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }
#endif

    vTaskStartScheduler();
    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif
