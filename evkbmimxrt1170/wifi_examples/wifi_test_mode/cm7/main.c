/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2020 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <osa.h>
#include "dhcp-server.h"
#include "cli.h"
#include "iperf.h"


#include "fsl_common.h"
#if CONFIG_WIFI_SMOKE_TESTS
#include "fsl_iomuxc.h"
#include "fsl_enet.h"
#endif
#if CONFIG_WIFI_SMOKE_TESTS
#if BOARD_NETWORK_USE_100M_ENET_PORT
#include "fsl_phyrtl8201.h"
#else
#include "fsl_phyrtl8211f.h"
#endif
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if CONFIG_WIFI_SMOKE_TESTS
#if BOARD_NETWORK_USE_100M_ENET_PORT
extern phy_rtl8201_resource_t g_phy_resource;
#define EXAMPLE_ENET ENET
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS &phyrtl8201_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#else
extern phy_rtl8211f_resource_t g_phy_resource;
#define EXAMPLE_ENET          ENET_1G
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS   BOARD_ENET1_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS       &phyrtl8211f_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif1_init
#endif

/* PHY resource. */
#define EXAMPLE_PHY_RESOURCE &g_phy_resource

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)
#endif


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#if CONFIG_WIFI_SMOKE_TESTS
#if BOARD_NETWORK_USE_100M_ENET_PORT
phy_rtl8201_resource_t g_phy_resource;
#else
phy_rtl8211f_resource_t g_phy_resource;
#endif

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

    /* Select syspll2pfd3, 528*18/24 = 396M */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    rootCfg.mux = 7;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg); /* Generate 198M bus clock. */
}

void IOMUXC_SelectENETClock(void)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    IOMUXC_GPR->GPR4 |= 0x3; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
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
#endif

#define MAIN_TASK_STACK_SIZE 4096

static void main_task(osa_task_param_t arg);

static OSA_TASK_DEFINE(main_task, OSA_PRIORITY_BELOW_NORMAL, 1, MAIN_TASK_STACK_SIZE, 0);

OSA_TASK_HANDLE_DEFINE(main_task_Handle);

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

static struct wlan_network sta_network;
static struct wlan_network uap_network;

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    int ret;
    struct wlan_ip_config addr;
    char ip[16];
    static int auth_fail = 0;

    printSeparator();
    PRINTF("app_cb: WLAN: received event %d\r\n", reason);
    printSeparator();

    switch (reason)
    {
        case WLAN_REASON_INITIALIZED:
            PRINTF("app_cb: WLAN initialized\r\n");
            printSeparator();

            ret = wlan_basic_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize BASIC WLAN CLIs\r\n");
                return 0;
            }
#if CONFIG_RF_TEST_MODE
            ret = wlan_test_mode_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN Test Mode CLIs\r\n");
                return 0;
            }
            PRINTF("WLAN Test Mode CLIs are initialized\r\n");
            printSeparator();
#endif
            PRINTF("CLIs Available:\r\n");
            printSeparator();
            help_command(0, NULL);
            printSeparator();
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;
        case WLAN_REASON_AUTH_SUCCESS:
            PRINTF("app_cb: WLAN: authenticated to network\r\n");
            break;
        case WLAN_REASON_SUCCESS:
            PRINTF("app_cb: WLAN: connected to network\r\n");
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                PRINTF("failed to get IP address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            ret = wlan_get_current_network(&sta_network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get External AP network\r\n");
                return 0;
            }

            PRINTF("Connected to following BSS:\r\n");
            PRINTF("SSID = [%s]\r\n", sta_network.ssid);
            if (addr.ipv4.address != 0U)
            {
                PRINTF("IPv4 Address: [%s]\r\n", ip);
            }
#if CONFIG_IPV6
            int i;
            for (i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
            {
                if (ip6_addr_isvalid(addr.ipv6[i].addr_state))
                {
                    (void)PRINTF("IPv6 Address: %-13s:\t%s (%s)\r\n",
                                 ipv6_addr_type_to_desc((struct net_ipv6_config *)&addr.ipv6[i]),
                                 inet6_ntoa(addr.ipv6[i].address), ipv6_addr_state_to_desc(addr.ipv6[i].addr_state));
                }
            }
            (void)PRINTF("\r\n");
#endif
            PRINTF("SSID = [%s], IP = [%s]\r\n", sta_network.ssid, ip);
            auth_fail = 0;
            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("app_cb: WLAN: connect failed\r\n");
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("app_cb: WLAN: network not found\r\n");
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("app_cb: WLAN: network authentication failed\r\n");
            auth_fail++;
            if (auth_fail >= 3)
            {
                PRINTF("Authentication Failed. Disconnecting ... \r\n");
                wlan_disconnect();
                auth_fail = 0;
            }
            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            PRINTF("network mgr: DHCP new lease\r\n");
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("app_cb: disconnected\r\n");
            auth_fail = 0;
            break;
        case WLAN_REASON_LINK_LOST:
            PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        case WLAN_REASON_CHAN_SWITCH:
            PRINTF("app_cb: WLAN: channel switch\r\n");
            break;
        case WLAN_REASON_UAP_SUCCESS:
            PRINTF("app_cb: WLAN: UAP Started\r\n");

            ret = wlan_get_current_uap_network(&uap_network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get Soft AP network\r\n");
                return 0;
            }

            printSeparator();
            PRINTF("Soft AP \"%s\" started successfully\r\n", uap_network.ssid);
            printSeparator();
            if (dhcp_server_start(net_get_uap_handle()))
                PRINTF("Error in starting dhcp server\r\n");

            PRINTF("DHCP Server started successfully\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_CLIENT_ASSOC:
            PRINTF("app_cb: WLAN: UAP a Client Associated\r\n");
            printSeparator();
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Associated with Soft AP\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_CLIENT_DISSOC:
            PRINTF("app_cb: WLAN: UAP a Client Dissociated\r\n");
            printSeparator();
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Dis-Associated from Soft AP\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_STOPPED:
            PRINTF("app_cb: WLAN: UAP Stopped\r\n");
            printSeparator();
            PRINTF("Soft AP \"%s\" stopped successfully\r\n", uap_network.ssid);
            printSeparator();

            dhcp_server_stop();

            PRINTF("DHCP Server stopped successfully\r\n");
            printSeparator();
            break;
        case WLAN_REASON_PS_ENTER:
            PRINTF("app_cb: WLAN: PS_ENTER\r\n");
            break;
        case WLAN_REASON_PS_EXIT:
            PRINTF("app_cb: WLAN: PS EXIT\r\n");
            break;
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}

static void main_task(osa_task_param_t arg)
{
    int32_t result = 0;
    (void)result;

    PRINTF("Initialize CLI\r\n");
    printSeparator();

    result = cli_init();

    assert(WM_SUCCESS == result);

    PRINTF("Initialize WLAN Driver\r\n");
    printSeparator();

    /* Initialize WIFI Driver */
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);

    assert(WM_SUCCESS == result);

    result = wlan_start(wlan_event_callback);

    assert(WM_SUCCESS == result);

    while (1)
    {
        /* wait for interface up */
        OSA_TimeDelay(5000);
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(void)
{
    OSA_Init();

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitPinsM2();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if CONFIG_WIFI_SMOKE_TESTS
    BOARD_InitModuleClock();

    IOMUXC_SelectENETClock();

    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

#if BOARD_NETWORK_USE_100M_ENET_PORT
    BOARD_InitEnetPins();
    GPIO_PinInit(GPIO12, 12, &gpio_config);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO12, 12, 1);
    SDK_DelayAtLeastUs(150000, CLOCK_GetFreq(kCLOCK_CpuClk));
#else
    BOARD_InitEnet1GPins();
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);
#endif

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
#endif


    printSeparator();
    PRINTF("wifi test mode demo\r\n");
    printSeparator();

    (void)OSA_TaskCreate((osa_task_handle_t)main_task_Handle, OSA_TASK(main_task), NULL);

    OSA_Start();

    return 0;
}
