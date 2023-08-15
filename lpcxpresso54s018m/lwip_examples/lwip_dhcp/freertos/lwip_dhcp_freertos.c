/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020,2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_DHCP && LWIP_NETCONN

#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/sys.h"
#include "ethernetif.h"

#include "pin_mux.h"
#include "board.h"
#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif
#include "fsl_phy.h"

#include "fsl_enet.h"
#include "fsl_phylan8720a.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* Ethernet configuration. */
extern phy_lan8720a_resource_t g_phy_resource;
#define EXAMPLE_ENET_BASE    ENET
#define EXAMPLE_PHY_ADDRESS  BOARD_ENET0_PHY_ADDRESS
#define EXAMPLE_PHY_OPS      &phylan8720a_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_resource
#define EXAMPLE_CLOCK_FREQ   CLOCK_GetFreq(kCLOCK_CoreSysClk)


#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*! @brief Stack size of the temporary lwIP initialization thread. */
#define INIT_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary lwIP initialization thread. */
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

/*! @brief Stack size of the thread which prints DHCP info. */
#define PRINT_THREAD_STACKSIZE 512

/*! @brief Priority of the thread which prints DHCP info. */
#define PRINT_THREAD_PRIO DEFAULT_THREAD_PRIO

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void print_dhcp_state(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_lan8720a_resource_t g_phy_resource;

static phy_handle_t phyHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET_BASE)]);
    ENET_SetSMI(EXAMPLE_ENET_BASE);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(EXAMPLE_ENET_BASE, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(EXAMPLE_ENET_BASE, phyAddr, regAddr, pData);
}


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

    netifapi_netif_add(&netif, NULL, NULL, NULL, &enet_config, EXAMPLE_NETIF_INIT_FN, tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    netifapi_dhcp_start(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" DHCP example\r\n");
    PRINTF("************************************************\r\n");

    if (sys_thread_new("print_dhcp", print_dhcp_state, &netif, PRINT_THREAD_STACKSIZE, PRINT_THREAD_PRIO) == NULL)
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
            }
        }

        sys_msleep(20U);
    }

    vTaskDelete(NULL);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    CLOCK_EnableClock(kCLOCK_InputMux);

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

    /* Initialize lwIP from thread */
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif
