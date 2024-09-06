/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2008-2020 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code ("Materials") are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#if !defined(RW612_SERIES)
#include "wifi.h"
#include "fsl_sai.h"

#include "fsl_sdmmc_host.h"
#endif /* RW612_SERIES */

#include "fsl_common.h"
#include "fsl_device_registers.h"

#include "app.h"

#include "BT_common.h"
#include "BT_version.h"
#include "BT_hci_api.h"

#include "fsl_gpio.h"
#if !defined(RW612_SERIES)
#include "fsl_iomuxc.h"
#include "fsl_gpc.h"
#include "fsl_lpuart_edma.h"
#include "fsl_dmamux.h"
#endif /* RW612_SERIES */

#include "appl_utils.h"

#include "fsl_adapter_uart.h"
#include "usb_host_config.h"
#include "usb_host.h"

#include "netif/ethernet.h"
#include "ethernetif.h"
#include "fsl_enet.h"
#include "app_config.h"
#include "dhcp-server.h"
#include "cli.h"
#include "wlan.h"
#include "wifi_ping.h"
#include "iperf.h"
#include <wm_net.h>
#include "fsl_adapter_uart.h"
#include "coex.h"

#ifdef RW612_SERIES
#if defined(MBEDTLS_NXP_SSSAPI)
#include "sssapi_mbedtls.h"
#elif defined(MBEDTLS_MCUX_CSS_API)
#include "platform_hw_ip.h"
#include "css_mbedtls.h"
#elif defined(MBEDTLS_MCUX_CSS_PKC_API)
#include "platform_hw_ip.h"
#include "css_pkc_mbedtls.h"
#elif defined(MBEDTLS_MCUX_ELS_PKC_API)
#include "platform_hw_ip.h"
#include "els_pkc_mbedtls.h"
#elif defined(MBEDTLS_MCUX_ELS_API)
#include "platform_hw_ip.h"
#include "els_mbedtls.h"
#elif defined(MBEDTLS_MCUX_ELE_S400_API)
#include "ele_mbedtls.h"
#else
#ifdef CONFIG_KSDK_MBEDTLS
#include "ksdk_mbedtls.h"
#endif
#endif

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
#include "PWR_Interface.h"
#endif /* APP_LOWPOWER_ENABLED */
#endif /* RW612_SERIES */

#ifndef CONFIG_WIFI_BLE_COEX_APP
#define CONFIG_WIFI_BLE_COEX_APP 1 // needs to define CONFIG_WIFI_BLE_COEX_APP with value, 0 for disable Wi-Fi, 1 for enable Wi-Fi
#endif

#ifndef CONFIG_OT_CLI
#define CONFIG_OT_CLI 0 // needs to define CONFIG_OT_CLI with value, 0 for disable OT, 1 for enable OT
#endif

#ifndef CONFIG_DISABLE_BLE
#define CONFIG_DISABLE_BLE 0 // needs to define CONFIG_DISABLE_BLE with value, 0 for enable BLE, 1 for disable BLE
#endif

#include "fsl_adapter_gpio.h"
#include "controller_hci_uart.h"
#include "usb_phy.h"
#include "fsl_edma.h"
#include "controller_features.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define os_thread_sleep(ticks) vTaskDelay(ticks)
#define os_msec_to_ticks(msecs) ((msecs) / (portTICK_PERIOD_MS))

/* Allocate the memory for the heap. */
#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP == 1)
#ifndef RW612_SERIES
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#else
uint8_t __attribute__((section(".heap"))) ucHeap[configTOTAL_HEAP_SIZE];
#endif /* RW612_SERIES */
#endif

const int TASK_MAIN_PRIO       = (configMAX_PRIORITIES-5);
const int TASK_MAIN_STACK_SIZE = (2 * 1024);

portSTACK_TYPE *task_main_stack = NULL;
TaskHandle_t task_main_task_handler;
extern UINT32 a2dp_snk_sf;

#ifdef A2DP_SINK
TaskHandle_t audio_task_handler;
OSA_SEMAPHORE_HANDLE_DEFINE(xSemaphoreAudio);
#endif /* A2DP_SINK */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern int appl_main (int argc, char **argv);
#ifdef A2DP_SINK
extern void (* a2dp_snk_cb)(UCHAR *data, UINT16 datalen);
#endif /* A2DP_SINK */

/*******************************************************************************
 * Variables
 ******************************************************************************/

#ifdef RW612_SERIES
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
/* Tickless idle is allowed by default but can be disabled runtime with APP_SetTicklessIdle */
static int ticklessIdleAllowed = 1;
#endif
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(ENET, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(ENET, phyAddr, regAddr, pData);
}

void get_enetif(netif_init_fn *enetif)
{
    *enetif = ethernetif0_init;
    return;
}

void set_mdio(uint32_t srcClockHz)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(ENET)]);
    ENET_SetSMI(ENET, srcClockHz, false);
    return;
}

void get_mdio_resource(mdioRead *rd, mdioWrite *wr)
{
    *rd = MDIO_Read;
    *wr = MDIO_Write;
    return;
}

#if (defined(HPS) || defined(IPSPR) || defined(PAN) )
static void ENET_delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

static void BOARD_EnetHardwareInit(void)
{
    gpio_pin_config_t gpio_config        = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    const clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};

    BOARD_InitEnetPins();

    /* ENET PLL Init. */
    CLOCK_InitEnetPll(&config);

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

    /* Init ENET_RST, ENET_INT pin. */
    GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);

    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);

    /* Reset Enet. */
    GPIO_WritePinOutput(GPIO1, 9, 0);
    ENET_delay();
    GPIO_WritePinOutput(GPIO1, 9, 1);
}
#endif /* HPS | IPSPR | PAN */

#ifdef OOB_WAKEUP
#ifdef WIFI_BT_USE_M2_INTERFACE
/*Default C2H Settings for RT1060-EVKC+Direct-M2, GPIO1-IO1, PAD: GPIO_AD_B0_01*/
#define APP_C2H_SLEEP_WAKEUP_GPIO_INSTANCE 		1U
#define APP_C2H_SLEEP_WAKEUP_GPIO_PIN      		(1U)
/*Deep-Sleep H2C GPIO/IRQ Config,RT1060-EVKC+Direct-M2, GPIO1-IO2 PAD: GPIO_AD_B0_02*/
#ifndef WIFI_IW612_BOARD_MURATA_2EL_M2
	/* 2EL has H2C Lines through I2C, hence it is not included*/
	#define APP_H2C_SLEEP_WAKEUP_GPIO_INSTANCE 	1U
	#define APP_H2C_SLEEP_WAKEUP_GPIO_PIN       (2U)
#endif /*!define (WIFI_IW612_BOARD_MURATA_2EL_M2)*/
#endif /*WIFI_BT_USE_M2_INTERFACE*/
/*******************************************************************************
 * Variables
 ******************************************************************************/
GPIO_HANDLE_DEFINE(c2h_gpioHandle);
GPIO_HANDLE_DEFINE(h2c_gpioHandle);

int sleep_host;

void Configure_H2C_gpio(void)
{
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2) && defined(PCAL6408A_IO_EXP_ENABLE)
	PCAL6408A_control_op_port(PCAL6408A_OUTPUT_P3, 0);
#else
	hal_gpio_pin_config_t gpio_config = {
		kHAL_GpioDirectionOut,
		0,
		APP_H2C_SLEEP_WAKEUP_GPIO_INSTANCE,
		APP_H2C_SLEEP_WAKEUP_GPIO_PIN,
	};
	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_02_GPIO1_IO02, 0U);
	HAL_GpioInit(h2c_gpioHandle, &gpio_config);
#endif /*defined(WIFI_IW612_BOARD_MURATA_2EL_M2) && defined (PCAL6408A_IO_EXP_ENABLE)*/
}

void Configure_SW7(void)
{
    gpio_pin_config_t gpio_config = {
        .direction = kGPIO_DigitalInput, .outputLogic = 0U, .interruptMode = kGPIO_IntRisingEdge};

    /* Enable the Interrupt */
    EnableIRQ(BOARD_USER_BUTTON_IRQ);

    GPIO_PinInit(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN, &gpio_config);

    GPIO_PortEnableInterrupts(BOARD_USER_BUTTON_GPIO, 1U << BOARD_USER_BUTTON_GPIO_PIN);
}
static void c2h_indication(void *param)
{
	/*TODO:Below log is commented as c2h pulses comes for every controller uart-tx packets to RT,
	  which affects CPU performance during streaming profile tests!
	  Need to register ISR in H2C Sleep call, and deregister in C2H ISR CB.
	  */
	printf("c2h-ind rx'd, wake-up host if in-sleep!\n");
}

void C2H_sleep_gpio_cfg(void)
{
/*	hal_gpio_pin_config_t gpio_config = {
		kHAL_GpioDirectionIn,
		0,
		APP_C2H_SLEEP_WAKEUP_GPIO_INSTANCE,
		APP_C2H_SLEEP_WAKEUP_GPIO_PIN,
	};

	IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_01_GPIO1_IO01, 0U);
	HAL_GpioInit(c2h_gpioHandle, &gpio_config);
	HAL_GpioSetTriggerMode(c2h_gpioHandle, kHAL_GpioInterruptFallingEdge);
	HAL_GpioInstallCallback(c2h_gpioHandle, c2h_indication, NULL);*/
}

void H2C_wakeup(void)
{
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2) && defined(PCAL6408A_IO_EXP_ENABLE)
    PCAL6408A_control_op_port(PCAL6408A_OUTPUT_P3, 0x0U);
#else
    HAL_GpioSetOutput(h2c_gpioHandle, 0);
#endif /*!defined(WIFI_IW612_BOARD_MURATA_2EL_M2)*/
}

void H2C_sleep(void)
{
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2) && defined(PCAL6408A_IO_EXP_ENABLE)
    PCAL6408A_control_op_port(PCAL6408A_OUTPUT_P3, 0x1U);
#else
    HAL_GpioSetOutput(h2c_gpioHandle, 1U);
#endif /*defined(WIFI_IW612_BOARD_MURATA_2EL_M2) && defined (PCAL6408A_IO_EXP_ENABLE)*/
}

void Host_sleep(void)
{
    fflush(stdout);
    printf("System host is wakeup from sleep\r\n");
}
#endif /*OOB_WAKEUP*/


#if defined(WIFI_BT_USE_M2_INTERFACE)
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc         = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate  = 115200u;
    config->runningBaudrate  = BOARD_BT_UART_BAUDRATE;
    config->instance         = BOARD_BT_UART_INSTANCE;
    config->enableRxRTS      = 1u;
    config->enableTxCTS      = 1u;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = 0U;
    config->rx_channel       = 4U;
    config->tx_channel       = 5U;
    config->dma_mux_instance = 0U;
    config->rx_request       = kDmaRequestMuxLPUART3Rx;
    config->tx_request       = kDmaRequestMuxLPUART3Tx;
#endif
    return 0;
}
#else
#error "Use M.2 Module Only"
#endif

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
/* USB_HOST_CONFIG_EHCI */

/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostClockInit(void)
{
    uint32_t usbClockFreq;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    usbClockFreq = 480000000;
    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 1000000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

#ifdef A2DP_SINK
void AudioTask(void *handle)
{
    OSA_SemaphoreCreate(xSemaphoreAudio, 0);
    while (1U)
    {
        OSA_SemaphoreWait(xSemaphoreAudio, osaWaitForever_c);
        if(NULL != a2dp_snk_cb)
        {
            a2dp_snk_cb(NULL, 0);
        }

    }
}
#endif /* A2DP_SINK */

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
    static int auth_fail                      = 0;
    wlan_uap_client_disassoc_t *disassoc_resp = data;

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

            ret = wlan_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            PRINTF("WLAN CLIs are initialized\r\n");
            printSeparator();
#ifdef RW610
            ret = wlan_enhanced_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            PRINTF("ENHANCED WLAN CLIs are initialized\r\n");
            printSeparator();
#endif
            ret = ping_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize PING CLI\r\n");
                return 0;
            }

            ret = iperf_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize IPERF CLI\r\n");
                return 0;
            }

            ret = dhcpd_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize DHCP Server CLI\r\n");
                return 0;
            }

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
        case WLAN_REASON_UAP_CLIENT_CONN:
            PRINTF("app_cb: WLAN: UAP a Client Connected\r\n");
            printSeparator();
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Connected with Soft AP\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_CLIENT_DISSOC:
            printSeparator();
            PRINTF("app_cb: WLAN: UAP a Client Dissociated:");
            PRINTF(" Client MAC => ");
            print_mac((const char *)(disassoc_resp->sta_addr));
            PRINTF(" Reason code => ");
            PRINTF("%d\r\n", disassoc_resp->reason_code);
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
            break;
        case WLAN_REASON_PS_EXIT:
            break;
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
    return 0;
}

#if defined(RW612_SERIES)
#if defined(CONFIG_OT_CLI) && (CONFIG_OT_CLI == 1)
extern void appOtStart(int argc, char *argv[]);
#endif
#endif
#if defined (CONFIG_OT_CLI_IW612) && (CONFIG_OT_CLI_IW612 == 1) 
#include "fwk_platform_coex.h"
extern void appOtStart(int argc, char *argv[]);
extern void otPlatRadioInit(void);
extern void otSysRunIdleTask(void);
static void appIdleHook(void)
{
    otSysRunIdleTask();
}
#endif

void task_main(void *param)
{
    int32_t result = 0;
    (void)result;
#if defined (CONFIG_OT_CLI_IW612) && (CONFIG_OT_CLI_IW612 == 1) 	
	 PLATFORM_InitControllers(connBle_c|conn802_15_4_c|connWlan_c);
#endif

    printSeparator();
#if defined(WIFI_IW416_BOARD_AW_AM457_USD)
    PRINTF("     Initialize AW-AM457-uSD Driver\r\n");
#elif defined(WIFI_88W8987_BOARD_AW_CM358_USD)
    PRINTF("     Initialize AW-CM358-uSD Driver\r\n");
#elif defined(WIFI_IW612_BOARD_RD_USD)
    PRINTF("     Initialize Firecrest RD Board\r\n");
#elif defined(WIFI_IW416_BOARD_MURATA_1XK_USD)
    PRINTF("     Initialize RB3P 1XK USD Module\r\n");
#elif defined(WIFI_IW416_BOARD_MURATA_1XK_M2)
    PRINTF("     Initialize RB3P 1XK Direct-M2 Module\n");
#elif defined(WIFI_88W8987_BOARD_MURATA_1ZM_USD)
    PRINTF("     Initialize CA2 1ZM USD Module\r\n");
#elif defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2)
    PRINTF("     Initialize CA2 1ZM Direct-M2 Module\r\n");
#elif defined(WIFI_IW612_BOARD_MURATA_2EL_USD)
    PRINTF("     Initialize Firecrest-2EL (IW612) USD Module\r\n");
#elif defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
    PRINTF("     Initialize Firecrest-2EL (IW612) Direct-M2 Module\r\n");
#elif defined(WIFI_IW611_BOARD_MURATA_2DL_M2)
    PRINTF("     Initialize Firecrest-2DL (IW611) M2 Module\r\n");
#elif defined(WIFI_AW611_BOARD_UBX_JODY_W5_M2)
    PRINTF("     Initialize Firecrest-AW (AW611 UBX JODY W5) M2 Module\r\n");
#elif defined(WIFI_AW611_BOARD_UBX_JODY_W5_USD)
    PRINTF("     Initialize Firecrest-AW (AW611 UBX JODY W5) USD Module\r\n");
#elif defined(WIFI_IW611_BOARD_MURATA_2DL_USD)
    PRINTF("     Initialize Firecrest-2DL (IW611) USD Module\r\n");
#elif defined(RW612_SERIES)
    PRINTF("     Initialize RW612 Module\r\n");
#endif
    printSeparator();

    coex_cli_init();
    coex_controller_init();

#if defined(CONFIG_WIFI_BLE_COEX_APP) && (CONFIG_WIFI_BLE_COEX_APP == 1)
    result = wlan_start(wlan_event_callback);
    assert(WM_SUCCESS == result);
#endif

#if defined(CONFIG_OT_CLI) && (CONFIG_OT_CLI == 1)
    /* wait for interface up */
    os_thread_sleep(os_msec_to_ticks(3000));

#if !defined(CONFIG_WIFI_BLE_COEX_APP) || (CONFIG_WIFI_BLE_COEX_APP == 0)
    /* Initialize LWIP stack */
    tcpip_init(NULL, NULL);
#endif

    appOtStart(0, NULL);
#endif

#if defined(CONFIG_DISABLE_BLE) && (CONFIG_DISABLE_BLE == 0)
    extern int USB_HostMsdFatfsInit(void);

#if (defined(CONFIG_BT_SNOOP) && (CONFIG_BT_SNOOP > 0))
    (void)USB_HostMsdFatfsInit();
#endif
#endif
    usb_echo("host init done\r\n");
    /* wait for interface up */
    os_thread_sleep(os_msec_to_ticks(3000));

    /* wait for interface up */
    PRINTF(" Coex menu called\r\n");
    printSeparator();
    coex_menuPrint();
    printSeparator();
    while (1)
    {
        int ch = pollChar();
        if (ch != -1)
        {
            coex_menuAction(ch);
        }
    }
}

#if defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) \
        && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U)
void stackOverflowHookHandler(void* task_name)
{
    printf("stack-overflow exception from task: %s\r\n",(char*)task_name);
}
#endif /* #if defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U) */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(void)
{
    BaseType_t result = 0;
    (void)result;

    extern void BOARD_InitHardware(void);    /*fix build warning: function declared implicitly.*/
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    DMAMUX_Type *dmaMuxBases[] = DMAMUX_BASE_PTRS;
    edma_config_t config;
    DMA_Type *dmaBases[] = DMA_BASE_PTRS;
#endif

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
#if !defined(COEX_PERF_MODE) || (COEX_PERF_MODE == 0)
    SCB_DisableDCache();
#endif

#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    DMAMUX_Init(dmaMuxBases[0]);
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(dmaBases[0], &config);
#endif

#if defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP)
    CRYPTO_InitHardware();
#endif /* (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP))) */

    /* ENET Hardware Init. */
#if (defined(HPS) || defined(IPSPR) || defined(PAN))
    BOARD_EnetHardwareInit();
#endif /* HPS | IPSPR | PAN */

#ifdef OOB_WAKEUP
    Configure_H2C_gpio();
    C2H_sleep_gpio_cfg();
#endif

#ifdef RW612_SERIES
    CRYPTO_InitHardware();

    extern void APP_InitServices(void);
    APP_InitServices();

#ifdef OOB_WAKEUP
    Configure_H2C_gpio();
    C2H_sleep_gpio_cfg();
#endif
#endif /* RW612_SERIES */

    printSeparator();
    PRINTF("\n        coex cli\r\n");
    printSeparator();
#if defined (CONFIG_OT_CLI_IW612) && (CONFIG_OT_CLI_IW612 == 1) 
    int osError = OSA_SetupIdleFunction(appIdleHook);
    assert(osError == WM_SUCCESS);
#endif

    result =
        xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_MAIN_PRIO, &task_main_task_handler);
    assert(pdPASS == result);

#ifdef A2DP_SINK
    result =
        xTaskCreate(AudioTask, "audio", TASK_MAIN_STACK_SIZE, NULL, BT_TASK_PRIORITY, &audio_task_handler);
    assert(pdPASS == result);
#endif

#if defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) \
        && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U)
    EM_register_sof_handler(stackOverflowHookHandler);
#endif /* #if defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U) */

    vTaskStartScheduler();
    for (;;)
    {
        ;
    }
}


#ifdef RW612_SERIES
void vApplicationIdleHook(void)
{
#if defined(CONFIG_OT_CLI) && (CONFIG_OT_CLI == 1)
    otSysRunIdleTask();
#endif
}

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
    bool abortIdle = false;
    uint64_t expectedIdleTimeUs, actualIdleTimeUs;

    if(ticklessIdleAllowed > 0)
    {
        uint32_t irqMask = DisableGlobalIRQ();

        /* Disable and prepare systicks for low power */
        abortIdle = PWR_SysticksPreProcess((uint32_t)xExpectedIdleTime, &expectedIdleTimeUs);

        if (abortIdle == false)
        {
            /* Enter low power with a maximal timeout */
            actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

            /* Re enable systicks and compensate systick timebase */
            PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
        }

        /* Exit from critical section */
        EnableGlobalIRQ(irqMask);
    }
    else
    {
        /* Tickless idle is not allowed, wait for next tick interrupt */
        __WFI();
    }
#endif /* APP_LOWPOWER_ENABLED */
}

void APP_SetTicklessIdle(bool enable)
{
    if(enable == true)
    {
        ticklessIdleAllowed++;
    }
    else
    {
        ticklessIdleAllowed--;
    }
}

#endif /* configUSE_TICKLESS_IDLE */
#endif /* RW612_SERIES*/

#ifndef __GNUC__
void __assert_func(const char *file, int line, const char *func, const char *failedExpr)
{
    PRINTF("ASSERT ERROR \" %s \": file \"%s\" Line \"%d\" function name \"%s\" \n", failedExpr, file, line, func);
    for (;;)
    {
        __BKPT(0);
    }
}
#endif
