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

#if !defined(RW610_SERIES)
#include "wifi.h"
#include "fsl_sai.h"

#include "fsl_sdmmc_host.h"
#endif /* RW610_SERIES */

#include "fsl_common.h"
#include "fsl_device_registers.h"


#include "app.h"

#include "BT_common.h"
#include "BT_version.h"
#include "BT_hci_api.h"

#include "fsl_gpio.h"
#if !defined(RW610_SERIES)
#include "fsl_iomuxc.h"
#endif /* RW610_SERIES */

#include "appl_utils.h"

#include "fsl_adapter_uart.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_phy.h"
#include "fsl_gpc.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "fsl_enet.h"
#include "fsl_lpuart_edma.h"
#include "fsl_dmamux.h"
#include "app_config.h"
#include "dhcp-server.h"
#include "cli.h"
#include "wlan.h"
#include "wifi_ping.h"
#include "iperf.h"
#include <wm_net.h>
#include "fsl_adapter_uart.h"
#include "coex.h"

#include "controller_hci_uart.h"
#include "fsl_edma.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define os_thread_sleep(ticks) vTaskDelay(ticks)
#define os_msec_to_ticks(msecs) ((msecs) / (portTICK_PERIOD_MS))

/* Allocate the memory for the heap. */
#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)
APP_FREERTOS_HEAP_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif

const int TASK_MAIN_PRIO       = (configMAX_PRIORITIES-5);
const int TASK_MAIN_STACK_SIZE = (2 * 1024);

portSTACK_TYPE *task_main_stack = NULL;
TaskHandle_t task_main_task_handler;

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

#if (defined(HPS) || defined(IPSPR) ||                                                              \
     defined(PAN)) /* Enet have pin(B0_09) conflict with Sco, so default we will not set Enet pins. \
                    */
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

/*Deep-Sleep C2H GPIO/IRQ Config, RT1060-EVKB J33.1*/
#define APP_C2H_SLEEP_WAKEUP_GPIO     GPIO1
#define APP_C2H_SLEEP_WAKEUP_GPIO_PIN (26U)
#define APP_C2H_GPIO_IRQ_HANDLER      GPIO1_Combined_16_31_IRQHandler
#define APP_C2H_GPIO_IRQ              GPIO1_Combined_16_31_IRQn

/*Deep-Sleep H2C GPIO/IRQ Config, RT1060-EVKB J16.3*/
#define APP_H2C_SLEEP_WAKEUP_GPIO     GPIO1
#define APP_H2C_SLEEP_WAKEUP_GPIO_PIN (11U)

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t g_savedPrimask;
int sleep_host;

void Configure_H2C_gpio(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_11_GPIO1_IO11, /* GPIO_AD_B0_11 is configured as GPIO1_IO11 */
                     0U);

    gpio_pin_config_t out_config = {
        kGPIO_DigitalOutput,
        0,
        kGPIO_NoIntmode,
    };

    GPIO_PinInit(APP_H2C_SLEEP_WAKEUP_GPIO, APP_H2C_SLEEP_WAKEUP_GPIO_PIN, &out_config);
}

void H2C_wakeup(void)
{
    GPIO_PinWrite(APP_H2C_SLEEP_WAKEUP_GPIO, APP_H2C_SLEEP_WAKEUP_GPIO_PIN, 0);
}

void H2C_sleep(void)
{
    GPIO_PinWrite(APP_H2C_SLEEP_WAKEUP_GPIO, APP_H2C_SLEEP_WAKEUP_GPIO_PIN, 1);
}

static void APP_C2H_GPIO_IRQ_HANDLER(void)
{
    if ((1U << APP_C2H_SLEEP_WAKEUP_GPIO_PIN) & GPIO_GetPinsInterruptFlags(APP_C2H_SLEEP_WAKEUP_GPIO))
    {
        if (GPIO_PinRead(APP_C2H_SLEEP_WAKEUP_GPIO, APP_C2H_SLEEP_WAKEUP_GPIO_PIN) == 1)
        {
            printf("Host can now go to sleep\n");
            GPIO_PortClearInterruptFlags(APP_C2H_SLEEP_WAKEUP_GPIO, 1U << APP_C2H_SLEEP_WAKEUP_GPIO_PIN);
            sleep_host = 1;
        }
        if (GPIO_PinRead(APP_C2H_SLEEP_WAKEUP_GPIO, APP_C2H_SLEEP_WAKEUP_GPIO_PIN) == 0)
        {
            printf("Host is now wakeup\n");
            GPIO_PortClearInterruptFlags(APP_C2H_SLEEP_WAKEUP_GPIO, 1U << APP_C2H_SLEEP_WAKEUP_GPIO_PIN);
        }
    }

    SDK_ISR_EXIT_BARRIER;
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

static void BOARD_USER_BUTTON_IRQ_HANDLER(void)
{
    H2C_wakeup();

    /* Disable interrupt. */
    GPIO_PortClearInterruptFlags(BOARD_USER_BUTTON_GPIO, 1U << BOARD_USER_BUTTON_GPIO_PIN);

    SDK_ISR_EXIT_BARRIER;
}

void C2H_sleep_gpio_cfg(void)
{
    gpio_pin_config_t gpio_config = {
        .direction = kGPIO_DigitalInput, .outputLogic = 0U, .interruptMode = kGPIO_IntRisingOrFallingEdge};

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_10_GPIO1_IO26, 0U);

    GPIO_PinInit(APP_C2H_SLEEP_WAKEUP_GPIO, APP_C2H_SLEEP_WAKEUP_GPIO_PIN, &gpio_config);
    /*For RB3+/CA2, C2H IRQ disabled due to WSW-27403*/
    GPIO_ClearPinsInterruptFlags(APP_C2H_SLEEP_WAKEUP_GPIO, 1U << APP_C2H_SLEEP_WAKEUP_GPIO_PIN);
    GPIO_EnableInterrupts(APP_C2H_SLEEP_WAKEUP_GPIO, 1U << APP_C2H_SLEEP_WAKEUP_GPIO_PIN);
    /* Enable the Interrupt */
    EnableIRQ(APP_C2H_GPIO_IRQ);
}

AT_QUICKACCESS_SECTION_CODE(void LPM_EnterSleepMode(clock_mode_t mode));
void LPM_EnterSleepMode(clock_mode_t mode)
{
    assert(mode != kCLOCK_ModeRun);

    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();

    if (mode == kCLOCK_ModeWait)
    {
        /* Clear the SLEEPDEEP bit to go into sleep mode (WAIT) */
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }
    else
    {
        /* Set the SLEEPDEEP bit to enable deep sleep mode (STOP) */
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }

    /* WFI instruction will start entry into WAIT/STOP mode */
    __WFI();

    EnableGlobalIRQ(g_savedPrimask);
    __DSB();
    __ISB();
}

#if 0
void CpuModeTransition(void)
{
    GPC_CM_SetNextCpuMode(GPC_CPU_MODE_CTRL_0, kGPC_WaitMode);
    GPC_CM_EnableCpuSleepHold(GPC_CPU_MODE_CTRL_0, true);

    GPC_CPU_MODE_CTRL_0->CM_NON_IRQ_WAKEUP_MASK |=
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_EVENT_WAKEUP_MASK_MASK |
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_DEBUG_WAKEUP_MASK_MASK; /* Mask debugger wakeup */

    GPC_CM_RequestStandbyMode(GPC_CPU_MODE_CTRL_0, kGPC_WaitMode);
    LPM_EnterSleepMode(kCLOCK_ModeWait);
}

void LPM_EnableWakeupSource(uint32_t irq)
{
    GPC_EnableIRQ(GPC, irq);
}

void LPM_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}

void GPC_EnableWakeupSource(uint32_t irq)
{
    GPC_EnableIRQ(GPC, irq);
}

void GPC_DisableWakeupSource(uint32_t irq)
{
    GPC_DisableIRQ(GPC, irq);
}
#endif

void Host_sleep(void)
{
    fflush(stdout);
    // CpuModeTransition();
    printf("System host is wakeup from sleep\r\n");
}
#endif /*OOB_WAKEUP*/


#if (defined(WIFI_88W8987_BOARD_AW_CM358_USD) || defined(WIFI_88W8987_BOARD_MURATA_1ZM_USD) || \
     defined(WIFI_IW416_BOARD_MURATA_1XK_USD))
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc        = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate = 115200u;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance        = BOARD_BT_UART_INSTANCE;
    config->enableRxRTS     = 1u;
    config->enableTxCTS     = 1u;
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

#elif defined(WIFI_IW416_BOARD_AW_AM457_USD)
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
#elif defined(WIFI_IW612_BOARD_RD_USD)
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
#elif defined(WIFI_BT_USE_M2_INTERFACE)
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

static struct wlan_network network;
void coex_controller_init();

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

            ret = wlan_get_current_network(&network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get External AP network\r\n");
                return 0;
            }

            PRINTF("Connected to following BSS:\r\n");
            PRINTF("SSID = [%s]\r\n", network.ssid);
            if (addr.ipv4.address != 0U)
            {
                PRINTF("IPv4 Address: [%s]\r\n", ip);
            }
#ifdef CONFIG_IPV6
            int i;
            for (i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
            {
                if (ip6_addr_isvalid(addr.ipv6[i].addr_state))
                {
                    (void)PRINTF("IPv6 Address: %-13s:\t%s (%s)\r\n", ipv6_addr_type_to_desc((struct net_ipv6_config *)&addr.ipv6[i]),
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
            ret = wlan_get_current_network(&network);

            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get Soft AP network\r\n");
                return 0;
            }

            printSeparator();
            PRINTF("Soft AP \"%s\" started successfully\r\n", network.ssid);
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
            PRINTF("Soft AP \"%s\" stopped successfully\r\n", network.ssid);
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

void task_main(void *param)
{
    int32_t result = 0;
    (void)result;

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
#endif
    printSeparator();

    coex_cli_init();
    coex_controller_init();

    result = wlan_start(wlan_event_callback);
    assert(WM_SUCCESS == result);

    extern int USB_HostMsdFatfsInit(void);

#if (defined(CONFIG_BT_SNOOP) && (CONFIG_BT_SNOOP > 0))
    (void)USB_HostMsdFatfsInit();
#endif

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

void stackOverflowHookHandler(void* task_name)
{
    printf("stack-overflow exception from task: %s\r\n",(char*)task_name);
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(void)
{
    BaseType_t result = 0;
    (void)result;

    extern void BOARD_InitHardware(void);    /*fix build warning: function declared implicitly.*/
#if 0
    DMAMUX_Type *dmaMuxBases[] = DMAMUX_BASE_PTRS;
    edma_config_t config;
    DMA_Type *dmaBases[] = DMA_BASE_PTRS;
#endif
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
#if defined(WIFI_IW416_BOARD_AW_AM510_USD)
    BOARD_DeinitArduinoUARTPins();
#elif defined(WIFI_88W8987_BOARD_AW_CM358_USD) || defined(WIFI_88W8987_BOARD_MURATA_1ZM_USD) || \
    defined(WIFI_IW416_BOARD_MURATA_1XK_USD)
    BOARD_InitArduinoUARTPins();
#else
#endif
#if defined(WIFI_88W8987_BOARD_AW_CM358MA) || defined(WIFI_IW416_BOARD_AW_AM510MA) || \
    defined(WIFI_IW416_BOARD_MURATA_1XK_M2) || defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2)
    BOARD_Init_Audio_Pins();
#else
    // BOARD_Init_USDAudio_Pins();
    BOARD_Init_Audio_Pins();
#endif
    BOARD_InitBootClocks();
    /* Configure UART divider to default */
    BOARD_InitDebugConsole();
    SCB_DisableDCache();

    /* ENET Hardware Init. */
#if 0
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
    CRYPTO_InitHardware();
#endif /* HPS | IPSPR | PAN */
#endif

#if (defined(HPS) || defined(IPSPR) ||                                                              \
     defined(PAN)) /* Enet have pin(B0_09) conflict with Sco, so default we will not set Enet pins. \
                    */
    BOARD_EnetHardwareInit();
#endif /* HPS | IPSPR | PAN */
#ifdef OOB_WAKEUP
    Configure_H2C_gpio();
    C2H_sleep_gpio_cfg();
#endif

    /*Initialize global EDMA and DMA-MUX for LPUART/SAI interfaces*/
    edma_config_t dmaConfig = {0};
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DMA0, &dmaConfig);
    DMAMUX_Init(DMAMUX);

    PRINTF("       coex_cli\r\n");
    printSeparator();


    result =
        xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_MAIN_PRIO, &task_main_task_handler);
    assert(pdPASS == result);

#ifdef A2DP_SINK
    result =
        xTaskCreate(AudioTask, "audio", TASK_MAIN_STACK_SIZE, NULL, BT_TASK_PRIORITY, &audio_task_handler);
    assert(pdPASS == result);
#endif

    EM_register_sof_handler(stackOverflowHookHandler);

    vTaskStartScheduler();
    for (;;)
    {
        ;
    }
}

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
