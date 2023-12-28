/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020, 2022-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"

#include "tcpecho.h"
#include "lwip/netifapi.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "ethernetif.h"

#include "pin_mux.h"
#include "board.h"
#if !defined(configMAC_ADDR) || !defined(configMAC_ADDR1)
#include "fsl_silicon_id.h"
#endif
#include "fsl_phy.h"

#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"

#include "lwip/sockets.h"

#include "shell_task.h"

#include "fsl_enet.h"
#include "ethernetif_priv.h"
#include "fsl_phyrtl8201.h"
#include "fsl_phyrtl8211f.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* IP address configuration port 0. */
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

/* Netmask configuration port 0. */
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

/* Gateway address configuration port 0. */
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

/* IP address configuration port 1. */
#ifndef configIP1_ADDR0
#define configIP1_ADDR0 192
#endif
#ifndef configIP1_ADDR1
#define configIP1_ADDR1 168
#endif
#ifndef configIP1_ADDR2
#define configIP1_ADDR2 1
#endif
#ifndef configIP1_ADDR3
#define configIP1_ADDR3 103
#endif

/* Netmask configuration port 1. */
#ifndef configNET1_MASK0
#define configNET1_MASK0 255
#endif
#ifndef configNET1_MASK1
#define configNET1_MASK1 255
#endif
#ifndef configNET1_MASK2
#define configNET1_MASK2 255
#endif
#ifndef configNET1_MASK3
#define configNET1_MASK3 0
#endif

/* Gateway address configuration port 1. */
#ifndef configGW1_ADDR0
#define configGW1_ADDR0 192
#endif
#ifndef configGW1_ADDR1
#define configGW1_ADDR1 168
#endif
#ifndef configGW1_ADDR2
#define configGW1_ADDR2 1
#endif
#ifndef configGW1_ADDR3
#define configGW1_ADDR3 200
#endif

#define EXAMPLE_ENET        ENET
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
#define EXAMPLE_PHY_OPS     &phyrtl8201_ops
extern phy_rtl8201_resource_t g_phy_resource;
#define EXAMPLE_PHY_RESOURCE  &g_phy_resource
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

#define EXAMPLE_ENET_1G      ENET_1G
#define EXAMPLE_PHY1_ADDRESS BOARD_ENET1_PHY_ADDRESS
#define EXAMPLE_PHY1_OPS     &phyrtl8211f_ops
extern phy_rtl8211f_resource_t g_phy1_resource;
#define EXAMPLE_PHY1_RESOURCE  &g_phy1_resource
#define EXAMPLE_NETIF1_INIT_FN ethernetif1_init
#define configMAC_ADDR1                    \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x12 \
    }

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)

/* Tell the app to build 2 network interface configurations */
#define BOARD_NETWORK_USE_DUAL_ENET


/*! @brief Stack size of the temporary lwIP initialization thread. */
#define INIT_THREAD_STACKSIZE 1024

/*! @brief Priority of the temporary lwIP initialization thread. */
#define INIT_THREAD_PRIO DEFAULT_THREAD_PRIO

#ifndef EXAMPLE_NETIF_INIT_FN
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif

#if defined(BOARD_NETWORK_USE_DUAL_ENET)
#define BOARD_PHY_COUNT 2
#else
#define BOARD_PHY_COUNT 1
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if BOARD_NETWORK_USE_100M_ENET_PORT
#error "BOARD_NETWORK_USE_100M_ENET_PORT is set: this demo initializes both ENETs on evkbmimxrt1170"
#endif

phy_rtl8201_resource_t g_phy_resource;
phy_rtl8211f_resource_t g_phy1_resource;


static phy_handle_t phyHandle;
#if defined(BOARD_NETWORK_USE_DUAL_ENET)
static phy_handle_t phyHandle1;

#if LWIP_SINGLE_NETIF == 1
#error \
    "Single netif limitation in lwIP must be disabled if this example were to use both interfaces. (LWIP_SINGLE_NETIF = 0)"
#endif // LWIP_SINGLE_NETIF == 1
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

    clock_root_config_t rootCfg0 = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg0);

    clock_root_config_t rootCfg1 = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg1);
}

void IOMUXC_SelectENETClock(void)
{
    IOMUXC_GPR->GPR4 |= IOMUXC_GPR_GPR4_ENET_REF_CLK_DIR_MASK; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK;  /* bit1:iomuxc_gpr_enet_clk_dir
                                                                  bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
}

void BOARD_ENETFlexibleConfigure(enet_config_t *config)
{
    phy_handle_t *phy_ = ethernetif_get_phy((struct netif *)config->userData);

    if (phy_->phyAddr == BOARD_ENET0_PHY_ADDRESS)
    {
        config->miiMode = kENET_RmiiMode;
    }
    else if (phy_->phyAddr == BOARD_ENET1_PHY_ADDRESS)
    {
        config->miiMode = kENET_RgmiiMode;
    }
    else
    {
        // Unknown PHY addr
    }
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

static void MDIO_Init_1G(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET_1G)]);
    ENET_SetSMI(EXAMPLE_ENET_1G, EXAMPLE_CLOCK_FREQ, false);
}

static status_t MDIO_Write_1G(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(EXAMPLE_ENET_1G, phyAddr, regAddr, data);
}

static status_t MDIO_Read_1G(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(EXAMPLE_ENET_1G, phyAddr, regAddr, pData);
}


/*!
 * @brief Initializes lwIP stack.
 *
 * @param arg unused
 */
static void stack_init(void *arg)
{
    LWIP_UNUSED_ARG(arg);

    ip4_addr_t netif0_ipaddr, netif0_netmask, netif0_gw;
    static struct netif s_netif0;
    ethernetif_config_t enet0_config = {.phyHandle   = &phyHandle,
                                        .phyAddr     = EXAMPLE_PHY_ADDRESS,
                                        .phyOps      = EXAMPLE_PHY_OPS,
                                        .phyResource = EXAMPLE_PHY_RESOURCE,
                                        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
#ifdef configMAC_ADDR
                                        .macAddress = configMAC_ADDR
#endif
    };
#ifndef configMAC_ADDR
    (void)SILICONID_ConvertToMacAddr(&enet0_config.macAddress);
#endif

    tcpip_init(NULL, NULL);

    IP4_ADDR(&netif0_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif0_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif0_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);
    netifapi_netif_add(&s_netif0, &netif0_ipaddr, &netif0_netmask, &netif0_gw, &enet0_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_up(&s_netif0);

#if defined(BOARD_NETWORK_USE_DUAL_ENET)
    ip4_addr_t netif1_ipaddr, netif1_netmask, netif1_gw;
    ethernetif_config_t enet1_config = {.phyHandle   = &phyHandle1,
                                        .phyAddr     = EXAMPLE_PHY1_ADDRESS,
                                        .phyOps      = EXAMPLE_PHY1_OPS,
                                        .phyResource = EXAMPLE_PHY1_RESOURCE,
                                        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
#ifdef configMAC_ADDR1
                                        .macAddress = configMAC_ADDR1
#endif
    };
    static struct netif s_netif1;
#ifndef configMAC_ADDR1
    (void)SILICONID_ConvertToMacAddr(&enet1_config.macAddress);
#endif
    IP4_ADDR(&netif1_ipaddr, configIP1_ADDR0, configIP1_ADDR1, configIP1_ADDR2, configIP1_ADDR3);
    IP4_ADDR(&netif1_netmask, configNET1_MASK0, configNET1_MASK1, configNET1_MASK2, configNET1_MASK3);
    IP4_ADDR(&netif1_gw, configGW1_ADDR0, configGW1_ADDR1, configGW1_ADDR2, configGW1_ADDR3);
    netifapi_netif_add(&s_netif1, &netif1_ipaddr, &netif1_netmask, &netif1_gw, &enet1_config, EXAMPLE_NETIF1_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_up(&s_netif1);
#else
    /*
     * Single netif is used, set is as default to avoid
     * the need to append zone indices to link-local IPv6 addresses.
     */
    netifapi_netif_set_default(&s_netif0);
#endif /* defined(BOARD_NETWORK_USE_DUAL_ENET) */

    LOCK_TCPIP_CORE();
    netif_create_ip6_linklocal_address(&s_netif0, 1);
#if defined(BOARD_NETWORK_USE_DUAL_ENET)
    netif_create_ip6_linklocal_address(&s_netif1, 1);
#endif
    UNLOCK_TCPIP_CORE();

    struct netif *netif_array[BOARD_PHY_COUNT];
    netif_array[0] = &s_netif0;
#if defined(BOARD_NETWORK_USE_DUAL_ENET)
    netif_array[1] = &s_netif1;
#endif

    while (ethernetif_wait_linkup_array(netif_array, BOARD_PHY_COUNT, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    shell_task_init(NULL, 0);

    vTaskDelete(NULL);
}

/*!
 * @brief Main function
 */
int main(void)
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

    BOARD_InitEnetPins();
    GPIO_PinInit(GPIO12, 12, &gpio_config);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO12, 12, 1);
    SDK_DelayAtLeastUs(150000, CLOCK_GetFreq(kCLOCK_CpuClk));

    BOARD_InitEnet1GPins();
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

    MDIO_Init_1G();
    g_phy1_resource.read  = MDIO_Read_1G;
    g_phy1_resource.write = MDIO_Write_1G;

    /* Initialize lwIP from thread */
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
