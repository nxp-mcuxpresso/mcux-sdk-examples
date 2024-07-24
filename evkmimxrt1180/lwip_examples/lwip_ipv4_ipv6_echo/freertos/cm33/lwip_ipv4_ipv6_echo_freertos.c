/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020, 2022-2024 NXP
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
#include "clock_config.h"
#include "board.h"
#include "fsl_phy.h"

#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"

#include "lwip/sockets.h"

#include "shell_task.h"

#include "fsl_phyrtl8201.h"
#include "fsl_netc_endpoint.h"
#include "fsl_netc_switch.h"
#include "fsl_netc_mdio.h"
#include "fsl_phyrtl8211f.h"
#include "fsl_msgintr.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PHY_PAGE_SELECT_REG 0x1FU /*!< The PHY page select register. */
#define EXAMPLE_EP0_PORT  0x00U
#define EXAMPLE_SWT_PORT0 0x01U
#define EXAMPLE_SWT_PORT1 0x02U
#define EXAMPLE_SWT_PORT2 0x03U
#define EXAMPLE_SWT_PORT3 0x04U

#define EXAMPLE_EP0_PHY_ADDR       0x03U
#define EXAMPLE_SWT_PORT0_PHY_ADDR 0x02U
#define EXAMPLE_SWT_PORT1_PHY_ADDR 0x05U
#define EXAMPLE_SWT_PORT2_PHY_ADDR 0x04U
#define EXAMPLE_SWT_PORT3_PHY_ADDR 0x08U
#define EXAMPLE_NETC_FREQ          CLOCK_GetRootClockFreq(kCLOCK_Root_Netc)

#define EXAMPLE_EP_RING_NUM          3U
#define EXAMPLE_EP_RXBD_NUM          8U
#define EXAMPLE_EP_TXBD_NUM          8U
#define EXAMPLE_EP_BD_ALIGN          128U
#define EXAMPLE_EP_BUFF_SIZE_ALIGN   64U
#define EXAMPLE_EP_RXBUFF_SIZE       1518U
#define EXAMPLE_EP_RXBUFF_SIZE_ALIGN SDK_SIZEALIGN(EXAMPLE_EP_RXBUFF_SIZE, EXAMPLE_EP_BUFF_SIZE_ALIGN)
#define EXAMPLE_EP_TEST_FRAME_SIZE   1000U

#define EXAMPLE_EP_TXFRAME_NUM 20U
#define EXAMPLE_TX_RX_INTERRUPT_HANDLE

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

/* Ethernet configuration. */

#define configMAC_ADDR                     \
    {                                      \
        0x00, 0x00, 0xfa, 0xfa, 0xdd, 0x05 \
    }

#define EXAMPLE_PHY_ADDRESS  EXAMPLE_EP0_PHY_ADDR
#define EXAMPLE_PHY_OPS      &g_app_phy_rtl8201_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_rtl8201_resource
#define EXAMPLE_CLOCK_FREQ   EXAMPLE_NETC_FREQ // CLOCK_GetFreq(kCLOCK_IpgClk)
/* Must be after include of app.h */
#if !defined(configMAC_ADDR) || !defined(configMAC_ADDR1)
#include "fsl_silicon_id.h"
#endif

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
status_t APP_EP0_MDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data);
status_t APP_EP0_MDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static status_t APP_PHY_RTL8201_Init(phy_handle_t *handle, const phy_config_t *config);
static void APP_MDIO_Init(void);

/* PHY operation. */
static netc_mdio_handle_t s_mdio_handle;
// static phy_handle_t s_phy_handle;
// static uint8_t s_phy_address       = EXAMPLE_EP0_PHY_ADDR;
static mdioRead s_mdio_func_read   = APP_EP0_MDIORead;
static mdioWrite s_mdio_func_write = APP_EP0_MDIOWrite;

phy_rtl8201_resource_t g_phy_rtl8201_resource /*= {.read = }*/;

const phy_operations_t g_app_phy_rtl8201_ops = {.phyInit            = APP_PHY_RTL8201_Init,
                                                .phyWrite           = PHY_RTL8201_Write,
                                                .phyRead            = PHY_RTL8201_Read,
                                                .getAutoNegoStatus  = PHY_RTL8201_GetAutoNegotiationStatus,
                                                .getLinkStatus      = PHY_RTL8201_GetLinkStatus,
                                                .getLinkSpeedDuplex = PHY_RTL8201_GetLinkSpeedDuplex,
                                                .setLinkSpeedDuplex = PHY_RTL8201_SetLinkSpeedDuplex,
                                                .enableLoopback     = PHY_RTL8201_EnableLoopback,
                                                .enableLinkInterrupt= PHY_RTL8201_EnableLinkInterrupt,
                                                .clearInterrupt     = PHY_RTL8201_ClearInterrupt};

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

/* This does initialization and then reconfigures CRS/DV pin to RXDV signal. */
static status_t APP_PHY_RTL8201_Init(phy_handle_t *handle, const phy_config_t *config)
{
    status_t result;
    uint16_t data;

    APP_MDIO_Init();
	
    /* Reset PHY8201 for ETH4. Reset 10ms, wait 72ms. */
    RGPIO_PinWrite(BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO, BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO_PIN, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    RGPIO_PinWrite(BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO, BOARD_INITPHYACCESSPINS_ENET4_RST_B_GPIO_PIN, 1);
    SDK_DelayAtLeastUs(72000, CLOCK_GetFreq(kCLOCK_CpuClk));

    result = PHY_RTL8201_Init(handle, config);
    if (result != kStatus_Success)
    {
        return result;
    }

    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 7);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_Read(handle, 16, &data);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* CRS/DV pin is RXDV signal. */
    data |= (1U << 2);
    result = PHY_Write(handle, 16, data);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 0);

    return result;
}

static void APP_MDIO_Init(void)
{
    status_t result = kStatus_Success;

    netc_mdio_config_t mdioConfig = {
        .mdio =
            {
                .type = kNETC_EMdio,
            },
        .isPreambleDisable = false,
        .isNegativeDriven  = false,
        .srcClockHz        = EXAMPLE_NETC_FREQ,
    };

    mdioConfig.mdio.port = (netc_hw_eth_port_idx_t)kNETC_ENETC0EthPort;
    result               = NETC_MDIOInit(&s_mdio_handle, &mdioConfig);
    while (result != kStatus_Success)
    {
        // failed
    }
}

status_t APP_EP0_MDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return NETC_MDIOWrite(&s_mdio_handle, phyAddr, regAddr, data);
}

status_t APP_EP0_MDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return NETC_MDIORead(&s_mdio_handle, phyAddr, regAddr, pData);
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
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* RMII mode */
    BLK_CTRL_WAKEUPMIX->NETC_LINK_CFG[0] = BLK_CTRL_WAKEUPMIX_NETC_LINK_CFG_MII_PROT(1);
    BLK_CTRL_WAKEUPMIX->NETC_LINK_CFG[4] = BLK_CTRL_WAKEUPMIX_NETC_LINK_CFG_MII_PROT(1);

    /* RGMII mode */
    BLK_CTRL_WAKEUPMIX->NETC_LINK_CFG[1] = BLK_CTRL_WAKEUPMIX_NETC_LINK_CFG_MII_PROT(2);

    /* Output reference clock for RMII */
    BLK_CTRL_WAKEUPMIX->NETC_PORT_MISC_CFG |= BLK_CTRL_WAKEUPMIX_NETC_PORT_MISC_CFG_PORT0_RMII_REF_CLK_DIR_MASK |
                                              BLK_CTRL_WAKEUPMIX_NETC_PORT_MISC_CFG_PORT4_RMII_REF_CLK_DIR_MASK;

    /* Unlock the IERB. It will warm reset whole NETC. */
    NETC_PRIV->NETCRR &= ~NETC_PRIV_NETCRR_LOCK_MASK;
    while ((NETC_PRIV->NETCRR & NETC_PRIV_NETCRR_LOCK_MASK) != 0U)
    {
    }

    /* Set PHY address in IERB to use MAC port MDIO, otherwise the access will be blocked. */
    NETC_IERB->L0BCR = NETC_IERB_L0BCR_MDIO_PHYAD_PRTAD(EXAMPLE_SWT_PORT0_PHY_ADDR);
    NETC_IERB->L1BCR = NETC_IERB_L0BCR_MDIO_PHYAD_PRTAD(EXAMPLE_SWT_PORT1_PHY_ADDR);
    NETC_IERB->L4BCR = NETC_IERB_L0BCR_MDIO_PHYAD_PRTAD(EXAMPLE_EP0_PHY_ADDR);

    /* Set the access attribute, otherwise MSIX access will be blocked. */
    NETC_IERB->ARRAY_NUM_RC[0].RCMSIAMQR &= ~(7U << 27);
    NETC_IERB->ARRAY_NUM_RC[0].RCMSIAMQR |= (1U << 27);

    /* Lock the IERB. */
    NETC_PRIV->NETCRR |= NETC_PRIV_NETCRR_LOCK_MASK;
    while ((NETC_PRIV->NETCSR & NETC_PRIV_NETCSR_STATE_MASK) != 0U)
    {
    }

    /*result = APP_MDIO_Init();

    if (result != kStatus_Success)
    {
        while (true)
        {
        }
    }*/

    g_phy_rtl8201_resource.write    = s_mdio_func_write;
    g_phy_rtl8201_resource.writeExt = NULL;
    g_phy_rtl8201_resource.read     = s_mdio_func_read;
    g_phy_rtl8201_resource.readExt  = NULL;

    /* Initialize lwIP from thread */
    if (sys_thread_new("main", stack_init, NULL, INIT_THREAD_STACKSIZE, INIT_THREAD_PRIO) == NULL)
    {
        LWIP_ASSERT("main(): Task creation failed.", 0);
    }

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
