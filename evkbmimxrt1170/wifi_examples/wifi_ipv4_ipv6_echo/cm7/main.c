/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2022-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

#include "FreeRTOS.h"
#include "task.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "wpl.h"
#include "shell_task.h"

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


#define main_task_PRIORITY          1
#define main_task_STACK_DEPTH       800
#define SHELL_ADDITIONAL_STACK_SIZE 256
#define DEMO_WIFI_LABEL             "MyWifi"

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile bool wlan_connected = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t cmd_connect(void *shellHandle, int32_t argc, char **argv);
static shell_status_t cmd_scan(void *shellHandle, int32_t argc, char **argv);
static shell_status_t cmd_disconnect(void *shellHandle, int32_t argc, char **argv);

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


SHELL_COMMAND_DEFINE(wlan_scan, "\r\n\"wlan_scan\": Scans networks.\r\n", cmd_scan, 0);

SHELL_COMMAND_DEFINE(wlan_connect_with_password,
                     "\r\n\"wlan_connect_with_password ssid password\":\r\n"
                     "   Connects to the specified network with password.\r\n"
                     " Usage:\r\n"
                     "   ssid:        network SSID\r\n"
                     "   password:    password\r\n",
                     cmd_connect,
                     2);

SHELL_COMMAND_DEFINE(wlan_connect,
                     "\r\n\"wlan_connect ssid\":\r\n"
                     "   Connects to the specified network without password.\r\n"
                     " Usage:\r\n"
                     "   ssid:        network SSID\r\n",
                     cmd_connect,
                     1);

SHELL_COMMAND_DEFINE(wlan_disconnect,
                     "\r\n\"wlan_disconnect\":\r\n"
                     "   Disconnect from connected network\r\n",
                     cmd_disconnect,
                     0);

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

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

void task_main(void *param)
{
    wpl_ret_t err = WPLRET_FAIL;

    static shell_command_t *wifi_commands[] = {
        SHELL_COMMAND(wlan_scan), SHELL_COMMAND(wlan_connect), SHELL_COMMAND(wlan_connect_with_password),
        SHELL_COMMAND(wlan_disconnect), NULL // end of list
    };

    PRINTF("Initialize WLAN \r\n");
    printSeparator();

    /* Initialize WIFI*/
    err = WPL_Init();
    if (err != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Init: Failed, error: %d\r\n", (uint32_t)err);
        while (true)
        {
            ;
        }
    }

    err = WPL_Start(LinkStatusChangeCallback);
    if (err != WPLRET_SUCCESS)
    {
        PRINTF("[!] WPL_Start: Failed, error: %d\r\n", (uint32_t)err);
        while (true)
        {
            ;
        }
    }

    PRINTF("Initialize CLI\r\n");
    printSeparator();
    shell_task_init(wifi_commands, SHELL_ADDITIONAL_STACK_SIZE);

    vTaskDelete(NULL);
}

static shell_status_t cmd_connect(void *shellHandle, int32_t argc, char **argv)
{
    (void)shellHandle;

    wpl_ret_t err;
    if (wlan_connected == true)
    {
        PRINTF("Leave network before connecting to a new one!\r\n");
        return kStatus_SHELL_Success;
    }

    if (argc < 3)
        err = WPL_AddNetwork(argv[1], "", DEMO_WIFI_LABEL);
    else
        err = WPL_AddNetwork(argv[1], argv[2], DEMO_WIFI_LABEL);

    if (err != WPLRET_SUCCESS)
    {
        PRINTF("Failed to add network profile!\r\n");
        return kStatus_SHELL_Success;
    }

    PRINTF("Joining: %s\r\n", argv[1]);
    err = WPL_Join(DEMO_WIFI_LABEL);
    if (err != WPLRET_SUCCESS)
    {
        PRINTF("Failed to join network!\r\n");
        if (WPL_RemoveNetwork(DEMO_WIFI_LABEL) != WPLRET_SUCCESS)
        {
            PRINTF("Failed to remove network!\r\n");
        }
        return kStatus_SHELL_Success;
    }

    PRINTF("Network joined\r\n");
    wlan_connected = true;
    return kStatus_SHELL_Success;
}

static shell_status_t cmd_disconnect(void *shellHandle, int32_t argc, char **argv)
{
    if (wlan_connected == false)
    {
        PRINTF("No network connected!\r\n");
        return kStatus_SHELL_Success;
    }

    if (WPL_Leave() != WPLRET_SUCCESS)
    {
        PRINTF("Failed to leave the network!\r\n");
        return kStatus_SHELL_Success;
    }

    if (WPL_RemoveNetwork(DEMO_WIFI_LABEL) != WPLRET_SUCCESS)
    {
        PRINTF("Failed to remove network profile!\r\n");
        return kStatus_SHELL_Success;
    }

    PRINTF("Disconnected from network\r\n");
    wlan_connected = false;
    return kStatus_SHELL_Success;
}

static shell_status_t cmd_scan(void *shellHandle, int32_t argc, char **argv)
{
    (void)shellHandle;
    (void)argc;
    char *scanData = NULL;

    PRINTF("\r\nInitiating scan...\r\n");
    scanData = WPL_Scan();
    if (scanData == NULL)
    {
        PRINTF("Error while scanning!\r\n");
    }
    else
    {
        vPortFree(scanData);
    }

    return kStatus_SHELL_Success;
}

int main(void)
{
    BaseType_t result = 0;
    (void)result;

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

    result = xTaskCreate(task_main, "main", main_task_STACK_DEPTH, NULL, main_task_PRIORITY, NULL);
    assert(pdPASS == result);

    vTaskStartScheduler();
    for (;;)
        ;
}
