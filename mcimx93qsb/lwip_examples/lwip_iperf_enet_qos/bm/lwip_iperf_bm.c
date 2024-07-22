/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020,2022-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lwip/opt.h"

#if LWIP_IPV4 && LWIP_TCP && LWIP_UDP

#include "lwip/apps/lwiperf.h"
#include "lwip/timeouts.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include <stdbool.h>
#include "fsl_phy.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_enet_qos.h"
#include "fsl_phyrtl8211f.h"
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

/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x60 \
    }

/* Ethernet configuration. */
extern phy_rtl8211f_resource_t g_phy_resource;
#define EXAMPLE_PHY_ADDRESS  (0x01U)
#define EXAMPLE_PHY_OPS      &phyrtl8211f_ops
#define EXAMPLE_PHY_RESOURCE &g_phy_resource

/* ENET clock frequency. */
#define ENET_QOS_CLOCK_ROOT        kCLOCK_Root_Enet
#define ENET_QOS_CLOCK_GATE        kCLOCK_Enet_Qos
#define ENET_QOS_SYSTEM_CLOCK_ROOT kCLOCK_Root_WakeupAxi
#define EXAMPLE_CLOCK_FREQ         CLOCK_GetIpFreq(ENET_QOS_SYSTEM_CLOCK_ROOT)

/* ENET IRQ priority. Used in FreeRTOS. */
#ifndef ENET_PRIORITY
#define ENET_PRIORITY (6U)
#endif


/* Must be after include of app.h */
#ifndef configMAC_ADDR
#include "fsl_silicon_id.h"
#endif

#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

#ifndef IPERF_UDP_CLIENT_RATE
#define IPERF_UDP_CLIENT_RATE (100 * 1024 * 1024) /* 100 Mbit/s */
#endif                                            /* IPERF_UDP_CLIENT_RATE */

#ifndef IPERF_CLIENT_AMOUNT
#define IPERF_CLIENT_AMOUNT (-1000) /* 10 seconds */
#endif                              /* IPERF_CLIENT_AMOUNT */

/* @TEST_ANCHOR */

#ifndef EXAMPLE_PORT
#define EXAMPLE_PORT LWIPERF_TCP_PORT_DEFAULT
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_rtl8211f_resource_t g_phy_resource;

static phy_handle_t phyHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
void ENET_QOS_EnableClock(bool enable)
{
    BLK_CTRL_WAKEUPMIX->GPR =
        (BLK_CTRL_WAKEUPMIX->GPR & (~BLK_CTRL_WAKEUPMIX_GPR_ENABLE_MASK)) | BLK_CTRL_WAKEUPMIX_GPR_ENABLE(enable);
}
void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
    BLK_CTRL_WAKEUPMIX->GPR |= BLK_CTRL_WAKEUPMIX_GPR_MODE(miiMode);
}

static void MDIO_Init(void)
{
    /* Set SMI first. */
    CLOCK_EnableClock(s_enetqosClock[ENET_QOS_GetInstance(ENET_QOS)]);
    ENET_QOS_SetSMI(ENET_QOS, EXAMPLE_CLOCK_FREQ);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_QOS_MDIOWrite(ENET_QOS, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_QOS_MDIORead(ENET_QOS, phyAddr, regAddr, pData);
}


/*!
 * @brief Interrupt service for SysTick timer.
 */
void SysTick_Handler(void)
{
    time_isr();
}

/* Report state => string */
const char *report_type_str[] = {
    "TCP_DONE_SERVER (RX)",        /* LWIPERF_TCP_DONE_SERVER_RX,*/
    "TCP_DONE_SERVER (TX)",        /* LWIPERF_TCP_DONE_SERVER_TX,*/
    "TCP_DONE_CLIENT (TX)",        /* LWIPERF_TCP_DONE_CLIENT_TX,*/
    "TCP_DONE_CLIENT (RX)",        /* LWIPERF_TCP_DONE_CLIENT_RX,*/
    "TCP_ABORTED_LOCAL",           /* LWIPERF_TCP_ABORTED_LOCAL, */
    "TCP_ABORTED_LOCAL_DATAERROR", /* LWIPERF_TCP_ABORTED_LOCAL_DATAERROR, */
    "TCP_ABORTED_LOCAL_TXERROR",   /* LWIPERF_TCP_ABORTED_LOCAL_TXERROR, */
    "TCP_ABORTED_REMOTE",          /* LWIPERF_TCP_ABORTED_REMOTE, */
    "UDP_DONE_SERVER (RX)",        /* LWIPERF_UDP_DONE_SERVER_RX, */
    "UDP_DONE_SERVER (TX)",        /* LWIPERF_UDP_DONE_SERVER_TX, */
    "UDP_DONE_CLIENT (TX)",        /* LWIPERF_UDP_DONE_CLIENT_TX, */
    "UDP_DONE_CLIENT (RX)",        /* LWIPERF_UDP_DONE_CLIENT_RX, */
    "UDP_ABORTED_LOCAL",           /* LWIPERF_UDP_ABORTED_LOCAL, */
    "UDP_ABORTED_LOCAL_DATAERROR", /* LWIPERF_UDP_ABORTED_LOCAL_DATAERROR, */
    "UDP_ABORTED_LOCAL_TXERROR",   /* LWIPERF_UDP_ABORTED_LOCAL_TXERROR, */
    "UDP_ABORTED_REMOTE",          /* LWIPERF_UDP_ABORTED_REMOTE, */
};

/** Prototype of a report function that is called when a session is finished.
    This report function shows the test results. */
static void lwiperf_report(void *arg,
                           enum lwiperf_report_type report_type,
                           const ip_addr_t *local_addr,
                           u16_t local_port,
                           const ip_addr_t *remote_addr,
                           u16_t remote_port,
                           u64_t bytes_transferred,
                           u32_t ms_duration,
                           u32_t bandwidth_kbitpsec)
{
    PRINTF("-------------------------------------------------\r\n");
    if (report_type < (sizeof(report_type_str) / sizeof(report_type_str[0])))
    {
        PRINTF(" %s \r\n", report_type_str[report_type]);
        if (local_addr && remote_addr)
        {
            PRINTF(" Local address : %u.%u.%u.%u ", ((u8_t *)local_addr)[0], ((u8_t *)local_addr)[1],
                   ((u8_t *)local_addr)[2], ((u8_t *)local_addr)[3]);
            PRINTF(" Port %d \r\n", local_port);
            PRINTF(" Remote address : %u.%u.%u.%u ", ((u8_t *)remote_addr)[0], ((u8_t *)remote_addr)[1],
                   ((u8_t *)remote_addr)[2], ((u8_t *)remote_addr)[3]);
            PRINTF(" Port %u \r\n", remote_port);
            PRINTF(" Bytes Transferred %llu \r\n", bytes_transferred);
            PRINTF(" Duration (ms) %u \r\n", ms_duration);
            PRINTF(" Bandwidth (kbitpsec) %u \r\n", bandwidth_kbitpsec);
        }
    }
    else
    {
        PRINTF(" IPERF Report error\r\n");
    }
    PRINTF("\r\n");
}

/** Prints available modes to start iperf in. */
static void print_mode_menu()
{
    PRINTF("Please select one of the following modes to run IPERF with:\r\n\r\n");
    PRINTF("    0: TCP server mode (RX test)\r\n");
    PRINTF("    1: TCP client mode (TX only test)\r\n");
    PRINTF("    2: TCP client reverse mode (RX only test)\r\n");
    PRINTF("    3: TCP client dual mode (TX and RX in parallel)\r\n");
    PRINTF("    4: TCP client tradeoff mode (TX and RX sequentially)\r\n");
    PRINTF("    5: UDP server mode (RX test)\r\n");
    PRINTF("    6: UDP client mode (TX only test)\r\n");
    PRINTF("    7: UDP client reverse mode (RX only test)\r\n");
    PRINTF("    8: UDP client dual mode (TX and RX in parallel)\r\n");
    PRINTF("    9: UDP client tradeoff mode (TX and RX sequentially)\r\n\r\n");
    PRINTF("Enter mode number: ");
}

/** Lets user select a mode to run IPERF with. */
static bool select_mode(bool *server_mode, bool *tcp, enum lwiperf_client_type *client_type)
{
    status_t status;
    char option;

    status = DbgConsole_TryGetchar(&option);
    if (status != kStatus_Success)
    {
        return false;
    }

    PUTCHAR(option);
    PRINTF("\r\n");

    switch (option)
    {
        case '0':
            *server_mode = true;
            *tcp         = true;
            *client_type = LWIPERF_CLIENT;
            return true;
        case '1':
            *server_mode = false;
            *tcp         = true;
            *client_type = LWIPERF_CLIENT;
            return true;
        case '2':
            *server_mode = false;
            *tcp         = true;
            *client_type = LWIPERF_REVERSE;
            return true;
        case '3':
            *server_mode = false;
            *tcp         = true;
            *client_type = LWIPERF_DUAL;
            return true;
        case '4':
            *server_mode = false;
            *tcp         = true;
            *client_type = LWIPERF_TRADEOFF;
            return true;
        case '5':
            *server_mode = true;
            *tcp         = false;
            *client_type = LWIPERF_CLIENT;
            return true;
        case '6':
            *server_mode = false;
            *tcp         = false;
            *client_type = LWIPERF_CLIENT;
            return true;
        case '7':
            *server_mode = false;
            *tcp         = false;
            *client_type = LWIPERF_REVERSE;
            return true;
        case '8':
            *server_mode = false;
            *tcp         = false;
            *client_type = LWIPERF_DUAL;
            return true;
        case '9':
            *server_mode = false;
            *tcp         = false;
            *client_type = LWIPERF_TRADEOFF;
            return true;
        default:
            PRINTF("Enter mode number: ");
            return false;
    }
}

static void *try_start_iperf(ip4_addr_t *remote_addr)
{
    bool server_mode;
    bool tcp;
    enum lwiperf_client_type client_type;
    void *iperf_session;

    if (!select_mode(&server_mode, &tcp, &client_type))
    {
        return NULL;
    }

    if (server_mode)
    {
        if (tcp)
        {
            iperf_session = lwiperf_start_tcp_server(IP_ADDR_ANY, EXAMPLE_PORT, lwiperf_report, 0);
        }
        else
        {
            iperf_session = lwiperf_start_udp_server(netif_ip_addr4(netif_default), EXAMPLE_PORT, lwiperf_report, 0);
        }
    }
    else
    {
        if (tcp)
        {
            iperf_session = lwiperf_start_tcp_client(remote_addr, EXAMPLE_PORT, client_type, IPERF_CLIENT_AMOUNT, 0,
                                                     LWIPERF_TOS_DEFAULT, lwiperf_report, 0);
        }
        else
        {
            iperf_session = lwiperf_start_udp_client(netif_ip_addr4(netif_default), EXAMPLE_PORT, remote_addr,
                                                     EXAMPLE_PORT, client_type, IPERF_CLIENT_AMOUNT, 0,
                                                     IPERF_UDP_CLIENT_RATE, 0, lwiperf_report, NULL);
        }
    }

    if (iperf_session == NULL)
    {
        PRINTF("IPERF initialization failed!\r\n");
    }
    else
    {
        PRINTF("Press SPACE to abort the test and return to main menu\r\n");
    }

    return iperf_session;
}

/*!
 * @brief Main function.
 */
int main(void)
{
    void *iperf_session = NULL;
    status_t status;
    char key;
    struct netif netif;
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {.phyHandle   = &phyHandle,
                                       .phyAddr     = EXAMPLE_PHY_ADDRESS,
                                       .phyOps      = EXAMPLE_PHY_OPS,
                                       .phyResource = EXAMPLE_PHY_RESOURCE,
#ifdef configMAC_ADDR
                                       .macAddress = configMAC_ADDR
#endif
    };

    /* clang-format off */

    /* enetqosSysClk 250MHz */
    const clock_root_config_t enetqosSysClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
	.div = 4
    };

    /* enetqosClk 250MHz (For 125MHz TX_CLK ) */
    const clock_root_config_t enetqosClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_ENET_ClockRoot_MuxSysPll1Pfd0Div2, // 500MHz
	.div = 2
    };

    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };
    /* clang-format on */

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(ENET_QOS_SYSTEM_CLOCK_ROOT, &enetqosSysClkCfg);
    CLOCK_SetRootClock(ENET_QOS_CLOCK_ROOT, &enetqosClkCfg);
    CLOCK_EnableClock(ENET_QOS_CLOCK_GATE);
    CLOCK_SetRootClock(BOARD_PCAL6524_I2C_CLOCK_ROOT, &lpi2cClkCfg);
    CLOCK_EnableClock(BOARD_PCAL6524_I2C_CLOCK_GATE);

    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    pcal6524_handle_t handle;
    BOARD_InitPCAL6524(&handle);
    PCAL6524_SetDirection(&handle, (1 << BOARD_PCAL6524_ENET1_NRST), kPCAL6524_Output);
    PCAL6524_ClearPins(&handle, (1 << BOARD_PCAL6524_ENET1_NRST));
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    PCAL6524_SetPins(&handle, (1 << BOARD_PCAL6524_ENET1_NRST));
    SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    EnableIRQ(ENET_QOS_IRQn);
    NVIC_SetPriority(ENET_QOS_IRQn, ENET_PRIORITY);

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

    time_init();

    /* Set MAC address. */
#ifndef configMAC_ADDR
    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif

    /* Get clock after hardware init. */
    enet_config.srcClockHz = EXAMPLE_CLOCK_FREQ;

    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    lwip_init();

    netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN, ethernet_input);
    netif_set_default(&netif);
    netif_set_up(&netif);

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" IPERF example\r\n");
    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
           ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
           ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]);
    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
           ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]);
    PRINTF("************************************************\r\n");

    print_mode_menu();

    while (1)
    {
        if (iperf_session == NULL)
        {
            iperf_session = try_start_iperf(&netif_gw);
        }
        else
        {
            status = DbgConsole_TryGetchar(&key);
            if ((status == kStatus_Success) && (key == ' '))
            {
                lwiperf_abort(iperf_session);
                iperf_session = NULL;
                print_mode_menu();
            }
        }

        /* Poll UDP client */
        lwiperf_poll_udp_client();

        /* Poll the driver, get any outstanding frames */
        ethernetif_input(&netif);

        sys_check_timeouts(); /* Handle all system timeouts for all core protocols */
    }
}
#endif
