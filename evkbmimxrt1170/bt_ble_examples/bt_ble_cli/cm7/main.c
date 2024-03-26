/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2008-2020,2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
*/
///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "sdk_ver.h"
#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
#include "wifi.h"
#include "fsl_sai.h"

#include "fsl_sdmmc_host.h"
#endif /* (RW610_SERIES) && !defined(RW612_SERIES */

#include "fsl_common.h"
#include "fsl_device_registers.h"

#include "app.h"

#include "BT_common.h"
#include "BT_version.h"
#include "BT_hci_api.h"

#include "fsl_gpio.h"
#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
#include "fsl_iomuxc.h"
#endif /* (RW610_SERIES) && !defined(RW612_SERIES */

#include "controller.h"
#include "controller_features.h"
#include "appl_utils.h"
#include "fsl_adapter_audio.h"

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
#include "PWR_Interface.h"
#endif /* APP_LOWPOWER_ENABLED */

#include "controller_features.h"
#include "fsl_adapter_uart.h"
#include "controller_hci_uart.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_phy.h"
#include "fsl_adapter_gpio.h"
#include "fsl_gpc.h"
#ifdef OOB_WAKEUP
#include "fsl_mu.h"
#include "fsl_soc_src.h"
#endif
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "fsl_enet.h"
#include "fsl_lpuart_edma.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define os_thread_sleep(ticks) vTaskDelay(ticks)
#define os_msec_to_ticks(msecs) ((msecs) / (portTICK_PERIOD_MS))

#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif /*defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)*/

const int TASK_MAIN_PRIO       = (configMAX_PRIORITIES - 3U);
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


/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern void app_audio_streamer_task_signal(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(ENET_1G, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(ENET_1G, phyAddr, regAddr, pData);
}

void get_enetif(netif_init_fn *enetif)
{
    *enetif = ethernetif1_init;
    return;
}

void set_mdio(uint32_t srcClockHz)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(ENET_1G)]);
    ENET_SetSMI(ENET_1G, srcClockHz, false);
    return;
}

void get_mdio_resource(mdioRead *rd, mdioWrite *wr)
{
    *rd = MDIO_Read;
    *wr = MDIO_Write;
    return;
}

#ifdef OOB_WAKEUP

void APP_Observe_chip_deepsleep(void);

#ifdef WIFI_IW612_BOARD_MURATA_2EL_M2
/*Default C2H Settings for RT1170-EVKB+2EL-Direct-M2 Board*/
#define APP_C2H_SLEEP_WAKEUP_GPIO_INSTANCE 3U
#define APP_C2H_SLEEP_WAKEUP_GPIO_PIN      (26U)
#else
/*Default H2C and C2H Settings for RT1170+ FC RD Board*/
#define APP_C2H_SLEEP_WAKEUP_GPIO_INSTANCE 5U
#define APP_C2H_SLEEP_WAKEUP_GPIO_PIN      (13U)
#define APP_H2C_SLEEP_WAKEUP_GPIO_INSTANCE 9U
#define APP_H2C_SLEEP_WAKEUP_GPIO_PIN      (4U)
#endif /*WIFI_IW612_BOARD_MURATA_2EL_M2*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
GPIO_HANDLE_DEFINE(c2h_gpioHandle);
GPIO_HANDLE_DEFINE(h2c_gpioHandle);

static uint32_t g_savedPrimask;
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
		HAL_GpioInit(h2c_gpioHandle, &gpio_config);
	};

#endif /*defined(WIFI_IW612_BOARD_MURATA_2EL_M2) && defined (PCAL6408A_IO_EXP_ENABLE)*/
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

void GPC_EnableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(GPC_CPU_MODE_CTRL_0, irq, true);
}

void GPC_DisableWakeupSource(uint32_t irq)
{
    GPC_CM_EnableIrqWakeup(GPC_CPU_MODE_CTRL_0, irq, false);
}

static void c2h_indication(void *param)
{
	/*TODO:Below log is commented as c2h pulses comes for every controller uart-tx packets to RT,
	  which affects CPU performance during streaming profile tests!
	  Need to understand the usage of this signal from host perspective!
	  */
	// printf("c2h-ind rx'd, wake-up host if in-sleep!\n");
}

void GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL_Type *base)
{
    uint8_t i;

    for (i = 0; i < GPC_CPU_MODE_CTRL_CM_IRQ_WAKEUP_MASK_COUNT; i++)
    {
        base->CM_IRQ_WAKEUP_MASK[i] |= 0xFFFFFFFF;
    }
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

void APP_Observe_chip_deepsleep(void)
{
	/* Mask all interrupt first */
	GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL_0);
	/* Enable GPC interrupt */
}

void C2H_sleep_gpio_cfg(void)
{
	hal_gpio_pin_config_t gpio_config = {
		kHAL_GpioDirectionIn,
		0,
		APP_C2H_SLEEP_WAKEUP_GPIO_INSTANCE,
		APP_C2H_SLEEP_WAKEUP_GPIO_PIN,
	};

#ifdef WIFI_IW612_BOARD_MURATA_2EL_M2
	IOMUXC_GPR->GPR43 = ((IOMUXC_GPR->GPR43 & (~(IOMUXC_GPR_GPR43_GPIO_MUX3_GPIO_SEL_HIGH_MASK))) |
			IOMUXC_GPR_GPR43_GPIO_MUX3_GPIO_SEL_HIGH(0x0400U));
#endif /*WIFI_IW612_BOARD_MURATA_2EL_M2*/

	HAL_GpioInit(c2h_gpioHandle, &gpio_config);
	HAL_GpioSetTriggerMode(c2h_gpioHandle, kHAL_GpioInterruptFallingEdge);
	HAL_GpioInstallCallback(c2h_gpioHandle, c2h_indication, NULL);
}

AT_QUICKACCESS_SECTION_CODE(void SystemEnterSleepMode(gpc_cpu_mode_t cpuMode));
void SystemEnterSleepMode(gpc_cpu_mode_t cpuMode)
{
    assert(cpuMode != kGPC_RunMode);

    g_savedPrimask = DisableGlobalIRQ();
    __DSB();
    __ISB();

    if (cpuMode == kGPC_WaitMode)
    {
        /* Clear the SLEEPDEEP bit to go into sleep mode (WAIT) */
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }
    else /* STOP and SUSPEND mode */
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

void CpuModeTransition(void)
{
    GPC_CM_SetNextCpuMode(GPC_CPU_MODE_CTRL_0, kGPC_WaitMode);
    GPC_CM_EnableCpuSleepHold(GPC_CPU_MODE_CTRL_0, true);

    GPC_CPU_MODE_CTRL_0->CM_NON_IRQ_WAKEUP_MASK |=
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_EVENT_WAKEUP_MASK_MASK |
        GPC_CPU_MODE_CTRL_CM_NON_IRQ_WAKEUP_MASK_DEBUG_WAKEUP_MASK_MASK; /* Mask debugger wakeup */

    GPC_CM_RequestStandbyMode(GPC_CPU_MODE_CTRL_0, kGPC_WaitMode);
    SystemEnterSleepMode(kGPC_WaitMode);
}

void Host_sleep(void)
{
    fflush(stdout);
    CpuModeTransition();
    printf("System host is wakeup from sleep\r\n");
}
#endif

void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);

    /* Select syspll2pfd3, 528*18/24 = 396M */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    rootCfg.mux = 7;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg); /* Generate 198M bus clock. */
}

void IOMUXC_SelectENETClock(void)
{
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
}

static void BOARD_EnetHardwareInit(void)
{
    IOMUXC_SelectENETClock();
    BOARD_InitEnet1GPins();
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);
}


#if defined(WIFI_88W8987_BOARD_AW_CM358_USD)
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc        = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2);
    config->defaultBaudrate = 115200u;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance        = 2;
    config->enableRxRTS     = 1u;
    config->enableTxCTS     = 1u;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = 0U;
    config->rx_channel       = 4U;
    config->tx_channel       = 5U;
    config->dma_mux_instance = 0U;
    config->rx_request       = kDmaRequestMuxLPUART7Rx;
    config->tx_request       = kDmaRequestMuxLPUART7Tx;
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
    config->clockSrc = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate = BOARD_BT_UART_BAUDRATE;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance = BOARD_BT_UART_INSTANCE;
    config->enableRxRTS = 1u;
    config->enableTxCTS = 1u;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance = 0U;
    config->rx_channel = 4U;
    config->tx_channel = 5U;
    config->dma_mux_instance = 0U;
    config->rx_request = kDmaRequestMuxLPUART7Rx;
    config->tx_request = kDmaRequestMuxLPUART7Tx;
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
    config->clockSrc        = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate = BOARD_BT_UART_BAUDRATE;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance = BOARD_BT_UART_INSTANCE;
    config->enableRxRTS = 1u;
    config->enableTxCTS = 1u;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance = 0U;
    config->rx_channel = 4U;
    config->tx_channel = 5U;
    config->dma_mux_instance = 0U;
    config->rx_request = kDmaRequestMuxLPUART7Rx;
    config->tx_request = kDmaRequestMuxLPUART7Tx;
#endif
    return 0;
}
#elif defined(WIFI_IW612_BOARD_MURATA_2EL_USD)
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
    config->rx_request       = kDmaRequestMuxLPUART7Rx;
    config->tx_request       = kDmaRequestMuxLPUART7Tx;
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
    config->clockSrc         = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2);
    config->defaultBaudrate  = 115200u;
    config->runningBaudrate  = BOARD_BT_UART_BAUDRATE;
    config->instance         = 2;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = 0U;
    config->rx_channel       = 4U;
    config->tx_channel       = 5U;
    config->dma_mux_instance = 0U;
    config->rx_request       = kDmaRequestMuxLPUART2Rx;
    config->tx_request       = kDmaRequestMuxLPUART2Tx;
#endif
    config->enableRxRTS      = 1u;
    config->enableTxCTS      = 1u;
    return 0;
}
#else
#endif

void USB_HostClockInit(void)
{
    uint32_t usbClockFreq;

    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    usbClockFreq = 24000000;

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

void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}

/*!
 * @brief USB isr function.
 */

#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
extern usb_status_t USB_HostTestEvent(usb_device_handle deviceHandle,
                                      usb_host_configuration_handle configurationHandle,
                                      uint32_t eventCode);
#endif


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
        if (NULL != a2dp_snk_cb)
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

    controller_init();

#ifdef OOB_WAKEUP
    extern void Configure_H2C_gpio(void);
    extern void C2H_sleep_gpio_cfg(void);
    extern void Configure_SW7(void);

    Configure_H2C_gpio();
    C2H_sleep_gpio_cfg();
    Configure_SW7();
#endif /*OOB_WAKEUP*/

    extern int USB_HostMsdFatfsInit(void);

#if (defined(CONFIG_BT_SNOOP) && (CONFIG_BT_SNOOP > 0))
    (void)USB_HostMsdFatfsInit();
#endif

    usb_echo("host init done\r\n");
    /* wait for interface up */
    os_thread_sleep(os_msec_to_ticks(3000));

#ifdef LC3_TEST
    extern INT32 appl_lc3();
    appl_lc3();
#else
    appl_main(0 , NULL);
#endif

    while (1)
    {
        os_thread_sleep(os_msec_to_ticks(1000));
    }
}

#if defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) \
        && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U)
void stackOverflowHookHandler(void* task_name)
{
	printf("stack-overflow exception from task: %s\r\n",(char*)task_name);
}
#endif /*defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U)*/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(void)
{
    BaseType_t result = 0;
    (void)result;

    extern void BOARD_InitHardware(void);    /*fix build warning: function declared implicitly.*/
#if defined(WIFI_BOARD_AW_CM358)
    /* GPIO configuration of wifi_reset on GPIO_AD_16 (pin N17) */
    gpio_pin_config_t wifi_reset_config = {
        .direction = kGPIO_DigitalOutput, .outputLogic = 0U, .interruptMode = kGPIO_NoIntmode};
#endif

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
#if defined(WIFI_BOARD_AW_CM358)
    BOARD_InitM2UARTPins();
    BOARD_InitM2WifiResetPins();
    /* Initialize GPIO functionality on GPIO_AD_16 (pin N17) */
    GPIO_PinInit(CM7_GPIO3, 15U, &wifi_reset_config);
#elif defined(WIFI_BT_USE_M2_INTERFACE)
    BOARD_InitM2UARTPins();
    BOARD_InitPinsM2();
#else
    BOARD_InitArduinoUARTPins();
#endif
    BOARD_InitM2CodecPins();
    BOARD_InitM2ScoPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();
#if defined (LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
    /* 2EL use GPT2 as SyncTimer. */
    BOARD_InitGPT2Pins();
#elif defined(WIFI_IW612_BOARD_RD_USD)
    /* RD_USD use GPT3 as SyncTimer. */
    BOARD_InitGPT3Pins();
#endif /*defined(WIFI_IW612_BOARD_MURATA_2EL_M2)*/
#endif /*defined (LE_AUDIO_ENABLE_SYNC_SIG_SUPP) && (LE_AUDIO_ENABLE_SYNC_SIG_SUPP > 0)*/

    #if defined(WIFI_BOARD_AW_CM358)
    /* Wirte GPIO pin value on GPIO_AD_16 (pin N17) */
    GPIO_PinWrite(CM7_GPIO3, 15U, 1U);
#endif

#if (defined(HPS) || defined(IPSPR) || defined(PAN))
#if !defined(WIFI_BT_USE_M2_INTERFACE)
    /* Enet have pin(B0_09) conflict with Sco, so default we will not set Enet pins.*/
    BOARD_EnetHardwareInit();
#else
        printf("*** Ethernet is disabled for 2EL-M2,use FC RDB board to test this feature!***\r\n");
#endif /*!defined (WIFI_BT_USE_M2_INTERFACE) */
#endif /*(defined(HPS) || defined(IPSPR) || defined(PAN))*/

    /*Initialize global EDMA and DMA-MUX for LPUART/SAI interfaces*/
    edma_config_t dmaConfig = {0};
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DMA0, &dmaConfig);
    DMAMUX_Init(DMAMUX0);

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
    /* Stay in WFI only when idle for now to keep CLI working */
    PWR_SetLowPowerModeConstraint(PWR_WFI);
#endif

#ifndef LC3_TEST
    printSeparator();
    PRINTF("       EtherMind Menu Application\r\n");
    (void)PRINTF("    BT Driver Version   : %s\r\n", BT_DRV_VERSION);
    printSeparator();
#else /* LC3_TEST */
    printSeparator();
    PRINTF("       EtherMind LC3 Test\r\n");
    printSeparator();
#endif /* LC3_TEST */

#if defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) \
        && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U)
    EM_register_sof_handler(stackOverflowHookHandler);
#endif /*defined (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK) && (APP_CONFIG_ENABLE_STACK_OVERFLOW_FREERTOS_HOOK == 1U)*/

    result =
        xTaskCreate(task_main, "main", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_MAIN_PRIO, &task_main_task_handler);
    assert(pdPASS == result);

#ifdef A2DP_SINK
    result =
        xTaskCreate(AudioTask, "audio", TASK_MAIN_STACK_SIZE, NULL, BT_TASK_PRIORITY + 1, &audio_task_handler);
    assert(pdPASS == result);
#endif

    vTaskStartScheduler();
    for (;;)
    {
        ;
    }
}

#if defined(RW610_SERIES) || defined(RW612_SERIES)
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
    eSleepModeStatus eSleepStatus;
    TickType_t xIdleTime_tick;
    uint64_t expectedIdleTimeUs, actualIdleTimeUs;

    uint32_t irqMask = DisableGlobalIRQ();

    /* Disable Systicks for tickless mode */
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);

    /* Make sure it's still ok to enter low power mode */
    eSleepStatus = eTaskConfirmSleepModeStatus();

    if (eSleepStatus != eAbortSleep)
    {
        if (eSleepStatus == eNoTasksWaitingTimeout)
        {
            /* if no tasks are waiting a timeout we set expectedIdleTimeUs,
             * PWR_EnterLowPowerWithTimeout() will just call PWR_EnterLowPower()
             */
            expectedIdleTimeUs = 0;
        }
        else
        {
            assert(eSleepStatus == eStandardSleep);

            /* Convert the expected idle time in us for the PWR function, the
             * sleep mode will not last any longer than this expected idle time
             */
            expectedIdleTimeUs = xExpectedIdleTime * (portTICK_PERIOD_MS * 1000);
        }

        /* Enter low power with timeout */
        actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

        xIdleTime_tick = (TickType_t)(actualIdleTimeUs / (portTICK_PERIOD_MS * 1000));

        /* Update the OS time ticks. */
        vTaskStepTick(xIdleTime_tick);
    }

    /* Re-enable Systicks before releasing interrupts in case an interrupt fires
     * directly, otherwise we could loose some precision */
    SysTick->CTRL |= (SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);

    /* Exit from critical section */
    EnableGlobalIRQ(irqMask);
#endif /* APP_LOWPOWER_ENABLED */
}
#endif /* RW610_SERIES || defined(RW612_SERIES) */

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
