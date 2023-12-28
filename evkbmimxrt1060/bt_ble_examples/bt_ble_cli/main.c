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
#include "controller_hci_uart.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_phy.h"
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
    config->defaultBaudrate  = BOARD_BT_UART_BAUDRATE;
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
    config->defaultBaudrate  = BOARD_BT_UART_BAUDRATE;
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

#if defined(PCAL6408A_IO_EXP_ENABLE)
    pcal6408a_pins_cfg_t config;
    status_t ret = PCAL6408A_Init(&config);
    printf ("2EL's i2c-io-expander initialized, %d\n", ret);
/*    printf ("#### IO Expander Reg Values ####\nIO-Configured:%x\nInput-Port:%x\nOutput-Port:%x\nOP-Port-Config:%x\nPolarity:%x\nPullupSelected:%x\nPullUp-Enabled:%x\nOP-Drive-Strength1:%x\nOP-Drive-Strength2:%x\n######################\n",
															config.configured, \
															config.input_port_value, \
	 														config.output_port_value, \
															config.output_port_config, \
															config.polarity, \
															config.pull_ups_selected, \
															config.pulls_enabled, \
															config.ouput_drive_strength1, \
															config.ouput_drive_strength2);*/
#endif /*defined(PCAL6408A_IO_EXP_ENABLE)*/

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

    EM_register_sof_handler(stackOverflowHookHandler);

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
