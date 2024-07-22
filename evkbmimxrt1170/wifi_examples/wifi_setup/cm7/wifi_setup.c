/** @file wifi_setup.c
 *
 *  @brief main file
 *
 *  Copyright 2020-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "lwip/tcpip.h"
#include "ping.h"
#include "wpl.h"
#include "stdbool.h"

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


#ifndef main_task_PRIORITY
#define main_task_PRIORITY 0
#endif

#ifndef main_task_STACK_DEPTH
#define main_task_STACK_DEPTH 800
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

static char ssid[WPL_WIFI_SSID_LENGTH];
static char password[WPL_WIFI_PASSWORD_LENGTH];
static ip4_addr_t ip;
TaskHandle_t mainTaskHandle;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void LinkStatusChangeCallback(bool linkState);
static void promptJoinNetwork(void);
static void promptPingAddress(void);
static void main_task(void *param);

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


/* Link lost callback */
static void LinkStatusChangeCallback(bool linkState)
{
    if (linkState == false)
    {
        PRINTF("-------- LINK LOST --------\r\n");
    }
    else
    {
        PRINTF("-------- LINK REESTABLISHED --------\r\n");
    }
}

static void promptJoinNetwork(void)
{
    wpl_ret_t result;
    int i = 0;
    char ch;

    while (true)
    {
        PRINTF("\r\nPlease enter parameters of WLAN to connect\r\n");

        /* SSID prompt */
        PRINTF("\r\nSSID: ");
        i = 0;
        while ((ch = GETCHAR()) != '\r' && i < WPL_WIFI_SSID_LENGTH - 1)
        {
            ssid[i] = ch;
            PUTCHAR(ch);
            i++;
        }
        ssid[i] = '\0';

        /* Password prompt */
        PRINTF("\r\nPassword (for unsecured WLAN press Enter): ");
        i = 0;
        while ((ch = GETCHAR()) != '\r' && i < WPL_WIFI_PASSWORD_LENGTH - 1)
        {
            password[i] = ch;
            PUTCHAR('*');
            i++;
        }
        password[i] = '\0';
        PRINTF("\r\n");

        /* Add Wifi network as known network */
        result = WPL_AddNetwork(ssid, password, ssid);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] WPL_AddNetwork: Failed to add network, error:  %d\r\n", (uint32_t)result);
            continue;
        }
        PRINTF("[i] WPL_AddNetwork: Success\r\n");

        /* Join the network using label */
        PRINTF("[i] Trying to join the network...\r\n");
        result = WPL_Join(ssid);
        if (result != WPLRET_SUCCESS)
        {
            PRINTF("[!] WPL_Join: Failed to join network, error: %d\r\n", (uint32_t)result);
            if (WPL_RemoveNetwork(ssid) != WPLRET_SUCCESS)
                __BKPT(0);
            continue;
        }
        PRINTF("[i] WPL_Join: Success\r\n");

        /* SSID and password was OK, exit the prompt */
        break;
    }
}

static void promptPingAddress(void)
{
    int i = 0;
    char ip_string[IP4ADDR_STRLEN_MAX];
    char ch;

    while (true)
    {
        PRINTF("\r\nPlease enter a valid IPv4 address to test the connection\r\n");

        /* Ping IP address prompt */
        PRINTF("\r\nIP address: ");
        i = 0;
        while ((ch = GETCHAR()) != '\r' && i < IP4ADDR_STRLEN_MAX - 1)
        {
            ip_string[i] = ch;
            PUTCHAR(ch);
            i++;
        }
        ip_string[i] = '\0';
        PRINTF("\r\n");

        if (ipaddr_aton(ip_string, &ip) == 0)
        {
            PRINTF("[!] %s is not a valid IPv4 address\r\n", ip_string);
            continue;
        }

        /* Ping IP address was OK */
        break;
    }
}

static void main_task(void *param)
{
    wpl_ret_t result;
    char *scan_result;

    PRINTF(
        "\r\n"
        "Starting wifi_setup DEMO\r\n");

    result = WPL_Init();
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Init: Failed, error: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }
    PRINTF("[i] WPL_Init: Success\r\n");

    result = WPL_Start(LinkStatusChangeCallback);
    if (result != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Start: Failed, error: %d\r\n", (uint32_t)result);
        __BKPT(0);
    }
    PRINTF("[i] WPL_Start: Success\r\n");

    /* Scan the local area for available Wifi networks. The scan will print the results to the terminal
     * and return the results as JSON string */
    PRINTF("\r\nInitiating scan...\r\n\r\n");
    scan_result = WPL_Scan();
    if (scan_result == NULL)
    {
        PRINTF("[!] WPL_Scan: Failed to scan\r\n");
        __BKPT(0);
    }
    vPortFree(scan_result);

    promptJoinNetwork();
    promptPingAddress();

    /* Setup a ping task to test the connection */
    PRINTF("Starting ping task...\r\n");
    ping_init(&ip);

    vTaskDelete(NULL);
}

int main(void)
{
    /* Initialize the hardware */
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


    /* Create the main Task */
    if (xTaskCreate(main_task, "main_task", main_task_STACK_DEPTH, NULL, main_task_PRIORITY, &mainTaskHandle) != pdPASS)
    {
        PRINTF("[!] MAIN Task creation failed!\r\n");
        while (true)
        {
            ;
        }
    }

    /* Run RTOS */
    vTaskStartScheduler();

    while (true)
    {
        ;
    }
}
