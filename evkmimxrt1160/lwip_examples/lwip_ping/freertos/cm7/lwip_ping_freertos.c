/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020,2022-2023  NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"
#include "fsl_adapter_gpio.h"
#include "fsl_debug_console.h"

#if LWIP_IPV4 && LWIP_RAW && LWIP_SOCKET

#include "ping.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "ethernetif.h"

#include "pin_mux.h"
#include "board.h"
#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif
#include "fsl_phy.h"

#include "fsl_enet.h"
#if BOARD_NETWORK_USE_100M_ENET_PORT
#include "fsl_phyksz8081.h"
#else
#include "fsl_phyrtl8211f.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* IP address configuration. */
#ifndef configIP_ADDR0
#define configIP_ADDR0 192
#endif
#ifndef configIP_ADDR1
#define configIP_ADDR1 168
#endif
#ifndef configIP_ADDR2
#define configIP_ADDR2 0
#endif
#ifndef configIP_ADDR3
#define configIP_ADDR3 102
#endif

/* Netmask configuration. */
#ifndef configNET_MASK0
#define configNET_MASK0 255
#endif
#ifndef configNET_MASK1
#define configNET_MASK1 255
#endif
#ifndef configNET_MASK2
#define configNET_MASK2 255
#endif
#ifndef configNET_MASK3
#define configNET_MASK3 0
#endif

/* Gateway address configuration. */
#ifndef configGW_ADDR0
#define configGW_ADDR0 192
#endif
#ifndef configGW_ADDR1
#define configGW_ADDR1 168
#endif
#ifndef configGW_ADDR2
#define configGW_ADDR2 0
#endif
#ifndef configGW_ADDR3
#define configGW_ADDR3 100
#endif

#if BOARD_NETWORK_USE_100M_ENET_PORT
#define EXAMPLE_ENET ENET
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS &phyksz8081_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init

extern phy_ksz8081_resource_t g_phy_resource;
#else
#define EXAMPLE_ENET          ENET_1G
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS   BOARD_ENET1_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS       &phyrtl8211f_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif1_init

extern phy_rtl8211f_resource_t g_phy_resource;
#endif

/* PHY resource. */
#define EXAMPLE_PHY_RESOURCE &g_phy_resource

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)


#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*! @brief Stack size of the temporary lwIP initialization thread. */
#define INIT_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary lwIP initialization thread. */
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

/*! @brief Selection of GPIO perihperal and its pin for the reception of PHY interrupts. */
#if ETH_LINK_POLLING_INTERVAL_MS == 0
#ifndef EXAMPLE_PHY_INT_PORT
#if (!defined(BOARD_NETWORK_USE_100M_ENET_PORT) || !BOARD_NETWORK_USE_100M_ENET_PORT) && \
    defined(BOARD_INITENET1GPINS_PHY_INTR_PERIPHERAL)
#define EXAMPLE_PHY_INT_PORT BOARD_INITENET1GPINS_PHY_INTR_PERIPHERAL
#elif defined(BOARD_INITENETPINS_PHY_INTR_PERIPHERAL)
#define EXAMPLE_PHY_INT_PORT BOARD_INITENETPINS_PHY_INTR_PERIPHERAL
#elif defined(BOARD_INITPINS_PHY_INTR_PERIPHERAL)
#define EXAMPLE_PHY_INT_PORT BOARD_INITPINS_PHY_INTR_PERIPHERAL
#else
#error "Interrupt-based link-state detection was enabled on an unsupported board."
#endif
#endif // #ifndef EXAMPLE_PHY_INT_PORT

#ifndef EXAMPLE_PHY_INT_PIN
#if (!defined(BOARD_NETWORK_USE_100M_ENET_PORT) || !BOARD_NETWORK_USE_100M_ENET_PORT) && \
    defined(BOARD_INITENET1GPINS_PHY_INTR_CHANNEL)
#define EXAMPLE_PHY_INT_PIN BOARD_INITENET1GPINS_PHY_INTR_CHANNEL
#elif defined(BOARD_INITENETPINS_PHY_INTR_CHANNEL)
#define EXAMPLE_PHY_INT_PIN BOARD_INITENETPINS_PHY_INTR_CHANNEL
#elif defined(BOARD_INITPINS_PHY_INTR_CHANNEL)
#define EXAMPLE_PHY_INT_PIN BOARD_INITPINS_PHY_INTR_CHANNEL
#else
#error "Interrupt-based link-state detection was enabled on an unsupported board."
#endif
#endif // #ifndef EXAMPLE_PHY_INT_PIN
#endif // #if ETH_LINK_POLLING_INTERVAL_MS == 0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if BOARD_NETWORK_USE_100M_ENET_PORT
phy_ksz8081_resource_t g_phy_resource;
#else
phy_rtl8211f_resource_t g_phy_resource;
#endif

static phy_handle_t phyHandle;
static struct netif netif;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

#if BOARD_NETWORK_USE_100M_ENET_PORT
    clock_root_config_t rootCfg = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg);
#else
    clock_root_config_t rootCfg = {.mux = 4, .div = 4};       /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);
#endif
}

void IOMUXC_SelectENETClock(void)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    IOMUXC_GPR->GPR4 |= IOMUXC_GPR_GPR4_ENET_REF_CLK_DIR_MASK; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
#else
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
#endif
}

void BOARD_ENETFlexibleConfigure(enet_config_t *config)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    config->miiMode = kENET_RmiiMode;
#else
    config->miiMode = kENET_RgmiiMode;
#endif
}

static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET)]);
    ENET_SetSMI(EXAMPLE_ENET, EXAMPLE_CLOCK_FREQ, false);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(EXAMPLE_ENET, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(EXAMPLE_ENET, phyAddr, regAddr, pData);
}


/*!
 * @brief Initializes lwIP stack.
 */
static void stack_init(void *arg)
{
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyHandle   = &phyHandle,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = EXAMPLE_PHY_OPS,
        .phyResource = EXAMPLE_PHY_RESOURCE,
        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
#ifdef configMAC_ADDR
        .macAddress = configMAC_ADDR,
#endif
#if ETH_LINK_POLLING_INTERVAL_MS == 0
        .phyIntGpio    = EXAMPLE_PHY_INT_PORT,
        .phyIntGpioPin = EXAMPLE_PHY_INT_PIN
#endif
    };

    LWIP_UNUSED_ARG(arg);

    /* Set MAC address. */
#ifndef configMAC_ADDR
    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif

    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    tcpip_init(NULL, NULL);

    HAL_GpioPreInit();

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" PING example\r\n");
    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
           ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
           ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]);
    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
           ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]);
    PRINTF("************************************************\r\n");

    ping_init(&netif_gw);

    vTaskDelete(NULL);
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

#if BOARD_NETWORK_USE_100M_ENET_PORT
    BOARD_InitEnetPins();
    BOARD_ENET_PHY0_RESET;
#else
    BOARD_InitEnet1GPins();
    BOARD_ENET_PHY1_RESET;

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);
#endif

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

    /* Initialize lwIP from thread */
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
        LWIP_ASSERT("main(): Task creation failed.", 0);

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif
