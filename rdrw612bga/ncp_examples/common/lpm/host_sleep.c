/** @file host_sleep.c
 *
 *  @brief Host sleep file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "board.h"
#include "host_sleep.h"
#include "ncp_lpm.h"
#if CONFIG_NCP_WIFI
#include "host_sleep_wifi.h"
#include "wlan.h"
#include "ncp_cmd_wifi.h"
#endif
#include "ncp_cmd_common.h"
#include "cli.h"
#include "fsl_power.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_rtc.h"
#include "fsl_usart.h"
#include "fsl_gpio.h"
#include "fsl_io_mux.h"
#include "app_notify.h"
#if CONFIG_CRC32_HW_ACCELERATE
#include "fsl_crc.h"
#endif
#if CONFIG_NCP_USB
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "usb_device_cdc_app.h"
#include "usb_device_config.h"
#include "usb_device_cdc_acm.h"
#endif
#include "ncp_intf_pm.h"

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef struct
{
    uint32_t selA;
    uint32_t selB;
    uint32_t frgSel;
    uint32_t frgctl;
    uint32_t osr;
    uint32_t brg;
} uart_clock_context_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
static void ncp_suspend_task(void *argv);
OSA_TASK_HANDLE_DEFINE(ncp_suspend_thread);
OSA_TASK_DEFINE(ncp_suspend_task, PRIORITY_RTOS_TO_OSA(1), 1, CONFIG_NCP_SUSPEND_STACK_SIZE, 0);
OSA_EVENT_HANDLE_DEFINE(ncp_suspend_event);
int suspend_mode = 0;
power_cfg_t global_power_config;
int wakeup_reason = 0;
extern uint8_t suspend_notify_flag;
#if CONFIG_POWER_MANAGER
extern pm_handle_t pm_handle;
#endif
uint64_t rtc_timeout = 0;
static uart_clock_context_t s_uartClockCtx;
#if CONFIG_NCP_WIFI
extern int is_hs_handshake_done;
#endif
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
extern void USB_DevicePmStartResume(void);
#endif
OSA_SEMAPHORE_HANDLE_DEFINE(hs_cfm);

/*******************************************************************************
 * Code
 ******************************************************************************/
static void ncp_hs_delay(uint32_t loop)
{
    if (loop > 0U)
    {
        __ASM volatile(
            "1:                             \n"
            "    SUBS   %0, %0, #1          \n"
            "    CMP    %0, #0              \n"
            "    BNE    1b                  \n"
            :
            : "r"(loop));
    }
}

static void ncp_hs_delay_us(uint32_t us)
{
    uint32_t instNum;

    instNum = ((SystemCoreClock + 999999UL) / 1000000UL) * us;
    ncp_hs_delay((instNum + 2U) / 3U);
}

void GPIO_INTA_DriverIRQHandler()
{
    DisableIRQ(GPIO_INTA_IRQn);
    /* clear the interrupt status */
    GPIO_PinClearInterruptFlag(GPIO, 1, 10, 0);
    POWER_ClearWakeupStatus(GPIO_INTA_IRQn);
    POWER_DisableWakeup(GPIO_INTA_IRQn);
    wakeup_reason = WAKEUP_BY_PIN;
}

void PIN1_INT_IRQHandler()
{
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    DisableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_DisableWakeup(PIN1_INT_IRQn);
    wakeup_reason = WAKEUP_BY_PIN;
}

void ncp_gpio_wakeup_host()
{
    GPIO_PortToggle(GPIO, 1, 0x40000);
    ncp_hs_delay_us(1);
    GPIO_PortToggle(GPIO, 1, 0x40000);
}

AT_QUICKACCESS_SECTION_CODE(void host_sleep_pre_hook(void))
{
    uint32_t freq = CLK_XTAL_OSC_CLK / 40U; //frequency of LPOSC

    /* In PM2, only LPOSC and CLK32K are available. To use interface as wakeup source,
       We have to use main_clk as system clock source, and main_clk comes from LPOSC.
       Use register access directly to avoid possible flash access in function call */
    s_uartClockCtx.selA   = CLKCTL0->MAINCLKSELA;
    s_uartClockCtx.selB   = CLKCTL0->MAINCLKSELB;
#if CONFIG_NCP_UART
    s_uartClockCtx.frgSel = CLKCTL1->FLEXCOMM[0].FRGCLKSEL;
    s_uartClockCtx.frgctl = CLKCTL1->FLEXCOMM[0].FRGCTL;
    s_uartClockCtx.osr    = USART0->OSR;
    s_uartClockCtx.brg    = USART0->BRG;
#endif
    /* Switch main_clk to LPOSC */
    CLKCTL0->MAINCLKSELA = 2;
    CLKCTL0->MAINCLKSELB = 0;
#if CONFIG_NCP_UART
    /* Change UART0 clock source to main_clk */
    CLKCTL1->FLEXCOMM[0].FRGCLKSEL = 0;
    /* bit[0:7] div, bit[8:15] mult.
     * freq(new) = freq(old)/(1 + mult/(div+1))
     * freq(new) is the frequency of LPOSC clock
     * freq(old) is the frequency of main_pll clock
     * Use the equation here to get div and mult.
     */
    CLKCTL1->FLEXCOMM[0].FRGCTL    = 0x11C7;
    USART0->OSR                    = 7;
    USART0->BRG                    = 0;
#endif
    /* Update system core clock */
    SystemCoreClock = freq / ((CLKCTL0->SYSCPUAHBCLKDIV & CLKCTL0_SYSCPUAHBCLKDIV_DIV_MASK) + 1U);
}

AT_QUICKACCESS_SECTION_CODE(void host_sleep_post_hook(uint32_t mode, void *param))
{
    /* Recover main_clk clock source after wakeup.
     Use register access directly to avoid possible flash access in function call. */
#if CONFIG_NCP_UART
    USART0->OSR                    = s_uartClockCtx.osr;
    USART0->BRG                    = s_uartClockCtx.brg;
    CLKCTL1->FLEXCOMM[0].FRGCLKSEL = s_uartClockCtx.frgSel;
    CLKCTL1->FLEXCOMM[0].FRGCTL    = s_uartClockCtx.frgctl;
#endif
    CLKCTL0->MAINCLKSELA           = s_uartClockCtx.selA;
    CLKCTL0->MAINCLKSELB           = s_uartClockCtx.selB;
#if !(CONFIG_NCP_SDIO)
    SystemCoreClockUpdate();
#endif
}

void host_sleep_cli_notify(void)
{
    return;
}

int host_sleep_pre_cfg(int mode)
{
    if(strcmp(BOARD_NAME, "FRDM-RW612") == 0)
    {
        NVIC_ClearPendingIRQ(GPIO_INTA_IRQn);
        EnableIRQ(GPIO_INTA_IRQn);
        POWER_ClearWakeupStatus(GPIO_INTA_IRQn);
        POWER_EnableWakeup(GPIO_INTA_IRQn);
    }
    else
    {
        POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeLow);
        NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
        EnableIRQ(PIN1_INT_IRQn);
        POWER_ClearWakeupStatus(PIN1_INT_IRQn);
        POWER_EnableWakeup(PIN1_INT_IRQn);
    }
#if CONFIG_NCP_WIFI
    ncp_enable_wlan_wakeup(true);
#endif
#if CONFIG_NCP_USB
    /* For usb PM2 trigger by remote wake up.
     * Maybe some unknow error if any data transfer on usb interface after remote wakeup.
     */
    if (mode != 2)
    {
#endif
    /* Notify host about entering sleep mode */
    /* Wait until receiving NCP_CMD_WLAN_POWERMGMT_MCU_SLEEP_CFM from host */
    if (suspend_notify_flag == 0)
    {
        suspend_notify_flag |= APP_NOTIFY_SUSPEND_EVT;
#if CONFIG_NCP_WIFI
        if (global_power_config.is_manual == true)
        {
            app_notify_event(APP_EVT_MCU_SLEEP_ENTER, APP_EVT_REASON_SUCCESS, NULL, 0);
            while (suspend_notify_flag & APP_NOTIFY_SUSPEND_EVT)
                OSA_TimeDelay(1);
        }
        else
        {
            is_hs_handshake_done = WLAN_HOSTSLEEP_IN_PROCESS;
#endif
            lpm_setHandshakeState(NCP_LMP_HANDSHAKE_IN_PROCESS);
            /* For PM mode, get wakelock to wait for NCP_CMD_WLAN_POWERMGMT_MCU_SLEEP_CFM from host */
            if(OSA_SemaphoreWait((osa_semaphore_handle_t)hs_cfm, osaWaitNone_c) != 0)
            {
                ncp_d("Failed to get hs_cfm semaphor\r\n");
                return kStatus_PMPowerStateNotAllowed;
            }
            ncp_d("send sleep enter evt to app notify\r\n");
            app_notify_event(APP_EVT_MCU_SLEEP_ENTER, APP_EVT_REASON_SUCCESS, NULL, 0);
            return kStatus_PMPowerStateNotAllowed;
#if CONFIG_NCP_WIFI
        }
#endif
    }
#if CONFIG_NCP_USB
    }
#endif
    (void)PRINTF("Enter low power mode PM%d\r\n", mode);
    /* PM2, enable UART3 as wakeup source */
    if (mode == 2U)
    {
#if CONFIG_NCP_USB
        POWER_ClearWakeupStatus(USB_IRQn);
        POWER_EnableWakeup(USB_IRQn);
#elif CONFIG_NCP_UART
        /* Enable RX interrupt. */
        USART_EnableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
        POWER_ClearWakeupStatus(FLEXCOMM0_IRQn);
        POWER_EnableWakeup(FLEXCOMM0_IRQn);
#elif CONFIG_NCP_SPI
        SYSCTL0->HWWAKE = 0x10;
        NVIC_ClearPendingIRQ(WKDEEPSLEEP_IRQn);
        POWER_ClearWakeupStatus(WKDEEPSLEEP_IRQn);
        POWER_EnableWakeup(WKDEEPSLEEP_IRQn);
#endif
        /* Delay UART clock switch after resume from PM2 to avoid UART FIFO read error*/
        POWER_SetPowerSwitchCallback((power_switch_callback_t)host_sleep_pre_hook, NULL,
#if CONFIG_NCP_SDIO
                                     (power_switch_callback_t)host_sleep_post_hook,
#else
                                     NULL,
#endif
                                     NULL);
    }
    return kStatus_PMSuccess;
}

void host_sleep_post_cfg(int mode)
{
    uint32_t irq_mask;

    if(strcmp(BOARD_NAME, "FRDM-RW612") == 0)
    {
        NVIC_ClearPendingIRQ(GPIO_INTA_IRQn);
        DisableIRQ(GPIO_INTA_IRQn);
        POWER_ClearWakeupStatus(GPIO_INTA_IRQn);
        POWER_DisableWakeup(GPIO_INTA_IRQn);
    }
    else
    {
        /* Disable wakeup source of WLAN and PIN1 interrupt after waking up */
        irq_mask = DisableGlobalIRQ();
        POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
        NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
        DisableIRQ(PIN1_INT_IRQn);
        POWER_ClearWakeupStatus(PIN1_INT_IRQn);
        POWER_DisableWakeup(PIN1_INT_IRQn);
        EnableGlobalIRQ(irq_mask);
    }
#if CONFIG_NCP_WIFI
    ncp_enable_wlan_wakeup(false);
    ncp_check_wlan_wakeup();
#endif

    if (mode == 2U)
    {
#if CONFIG_NCP_USB
        POWER_ClearWakeupStatus(USB_IRQn);
        POWER_DisableWakeup(USB_IRQn);
#elif CONFIG_NCP_UART
        if (POWER_GetWakeupStatus(FLEXCOMM0_IRQn))
            wakeup_reason = WAKEUP_BY_USART0;
        POWER_ClearWakeupStatus(FLEXCOMM0_IRQn);
        POWER_DisableWakeup(FLEXCOMM0_IRQn);
#elif CONFIG_NCP_SPI
        if (POWER_GetWakeupStatus(WKDEEPSLEEP_IRQn))
            wakeup_reason = WKDEEPSLEEP_IRQn;
        POWER_ClearWakeupStatus(WKDEEPSLEEP_IRQn);
        POWER_DisableWakeup(WKDEEPSLEEP_IRQn);
        SYSCTL0->HWWAKE = 0x0;
#endif
        POWER_SetPowerSwitchCallback(NULL, NULL, NULL, NULL);
#if !(CONFIG_NCP_SDIO)
        host_sleep_post_hook(2, NULL);
#endif
    }
    if (global_power_config.wakeup_host && wakeup_reason == 0x1)
    {
        ncp_gpio_wakeup_host();
	/* Only wakeup host for 1 time */
        global_power_config.wakeup_host = 0;
    }
#if CONFIG_NCP_USB
    /* For usb PM2 trigger by remote wake up.
     * Maybe some unknow error if any data transfer on usb interface after remote wakeup.
     */
    if (global_power_config.subscribe_evt && 2 != mode)
    {
        app_notify_event(APP_EVT_MCU_SLEEP_EXIT, APP_EVT_REASON_SUCCESS, NULL, 0);
    }
#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
    if (mode == 2)
    {
        // start usb wakeup process
        LPM_ConfigureNextLowPowerMode(0, 0);
        USB_DevicePmStartResume();
    }
#endif
#else
    if (suspend_notify_flag)
    {
        PRINTF("send sleep exit evt to app notify\r\n");
        lpm_setHandshakeState(NCP_LMP_HANDSHAKE_NOT_START);
        app_notify_event(APP_EVT_MCU_SLEEP_EXIT, APP_EVT_REASON_SUCCESS, NULL, 0);
    }
#endif
    suspend_notify_flag = 0;
    PRINTF("Exit low power mode\r\n");
}

void host_sleep_dump_wakeup_source()
{
    if (wakeup_reason == WAKEUP_BY_RTC)
        PRINTF("Woken up by RTC\r\n");
#if CONFIG_NCP_WIFI
    else if (wakeup_reason == 0x1)
        ncp_print_wlan_wakeup();
#endif
    else if (wakeup_reason == WAKEUP_BY_PIN)
        PRINTF("Woken up by PIN1\r\n");
#if CONFIG_NCP_UART
    else if (wakeup_reason == WAKEUP_BY_USART0)
        PRINTF("Woken up by USART\r\n");
#endif
#if CONFIG_NCP_SDIO
    else if (POWER_GetWakeupStatus(SDU_IRQn))
    {
        PRINTF("Woken up by SDIO\r\n");
        POWER_ClearWakeupStatus(SDU_IRQn);
    }
#endif
#if CONFIG_NCP_SPI
    else if (POWER_GetWakeupStatus(WKDEEPSLEEP_IRQn))
    {
        PRINTF("Woken up by SPI DMA DEEPSLEEP\r\n");
        POWER_ClearWakeupStatus(WKDEEPSLEEP_IRQn);
    }
#endif
}

static void ncp_GetSleepConfig(power_sleep_config_t *config)
{
    config->pm2MemPuCfg = NCP_PM2_MEM_PU_CFG;
    config->pm2AnaPuCfg = NCP_PM2_ANA_PU_CFG;
    config->clkGate     = NCP_SOURCE_CLK_GATE;
    config->memPdCfg    = NCP_MEM_PD_CFG;
    config->pm3BuckCfg  = NCP_PM3_BUCK_CFG;
}

static void ncp_suspend_task(void *argv)
{
    power_sleep_config_t config;
    ncp_pm_status_t ret = NCP_PM_STATUS_SUCCESS;

    for (;;)
    {
        /* Wait for wlan-suspend command */
        (void)OSA_EventWait((osa_event_handle_t)ncp_suspend_event, SUSPEND_EVENT_TRIGGERS, 0, osaWaitForever_c, NULL);
        memset(&config, 0x0, sizeof(power_sleep_config_t));
        if (suspend_mode >= 2)
            ncp_GetSleepConfig(&config);
        host_sleep_pre_cfg(suspend_mode);

        ret = (ncp_pm_status_t)ncp_intf_pm_enter(suspend_mode);
        while(ret == NCP_PM_STATUS_NOT_READY)
        {
            /* Some interface needs some time to do deinit. */
            OSA_TimeDelay(1);
            ret = (ncp_pm_status_t)ncp_intf_pm_enter(suspend_mode);
        }
        if(suspend_mode == 3)
        {
#if CONFIG_NCP_DEBUG
#if CONFIG_UART_INTERRUPT
            cli_uart_notify();
            OSA_TimeDelay(1);
            cli_uart_deinit();
#endif
#endif
            DbgConsole_Deinit();
#if CONFIG_CRC32_HW_ACCELERATE
            CRC_Reset(CRC);
#endif
        }
        POWER_EnterPowerMode(suspend_mode, &config);
        /* Perihperal state lost, need reinitialize in exit from PM3 */
        if (suspend_mode == 3)
            lpm_pm3_exit_hw_reinit();
        else if (suspend_mode == 2)
            ncp_intf_pm_exit(suspend_mode);
#if CONFIG_NCP_WIFI
        if (ncp_cancel_wlan_wakeup() != 0)
            break;
#endif
        host_sleep_post_cfg(suspend_mode);
        if (suspend_mode == 1)
        {
            (void)PRINTF("Exit from PM1.\r\n");
            (void)PRINTF("Wakeup status is not available for PM1 due to PMU HW design\r\n");
        }
        else
            host_sleep_dump_wakeup_source();
#if CONFIG_NCP_WIFI
        ncp_clear_wlan_wakeup();
#endif
        wakeup_reason = 0;
        global_power_config.is_manual = false;
        suspend_mode = 0;
    }

#if CONFIG_NCP_WIFI
    vTaskSuspend(NULL);
    assert(0);
#endif
}

int ncp_config_suspend_mode(int mode)
{
    suspend_mode = mode;

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
    if (!global_power_config.enable)
#else
    if (!global_power_config.is_manual)
#endif
        return -WM_FAIL;

#if defined(configUSE_TICKLESS_IDLE) && (configUSE_TICKLESS_IDLE == 1)
    LPM_ConfigureNextLowPowerMode(2, 0);
#else
    (void)OSA_EventSet((osa_event_handle_t)ncp_suspend_event, SUSPEND_EVENT_TRIGGERS);
#endif

    return WM_SUCCESS;
}

#if defined(configUSE_IDLE_HOOK) && (configUSE_IDLE_HOOK == 1)
void powerManager_EnterLowPower()
{
    if (global_power_config.is_periodic && pm_handle.enable
         && OSA_MsgQAvailableMsgs((osa_msgq_handle_t)hs_cfm))
    {
        /* duration unit is us here */
        PM_EnterLowPower(rtc_timeout);
        host_sleep_dump_wakeup_source();
        wakeup_reason = 0;
#if CONFIG_NCP_WIFI
        ncp_clear_wlan_wakeup();
#endif
    }
}
#endif

void ncp_gpio_init()
{
    /* Define the init structure for the input switch pin */
    gpio_pin_config_t gpio_in_config = {
        kGPIO_DigitalInput,
        0,
    };
    gpio_pin_config_t gpio_out_config = {
        kGPIO_DigitalOutput,
        0,
    };
    gpio_interrupt_config_t gpio_lp_int_config = {
        .mode = kGPIO_PinIntEnableEdge,
        .polarity = kGPIO_PinIntEnableLowOrFall
    };

#if CONFIG_NCP_UART || CONFIG_NCP_USB
    GPIO_PortInit(GPIO, 0);
#endif
    GPIO_PortInit(GPIO, 1);


    if(strcmp(BOARD_NAME, "FRDM-RW612") == 0)
    {
        /* Initialize GPIO functionality on GPIO42 */
        GPIO_PinInit(GPIO, 1U, 10U, &gpio_in_config);
        GPIO_SetPinInterruptConfig(GPIO, 1U, 10U, &gpio_lp_int_config);
        GPIO_PinEnableInterrupt(GPIO, 1U, 10U, (uint32_t)kGPIO_InterruptA);
        NVIC_ClearPendingIRQ(GPIO_INTA_IRQn);
        EnableIRQ(GPIO_INTA_IRQn);
    }
    else if (strcmp(BOARD_NAME, "RD-RW61X-BGA") == 0)
    {
#if defined(BOARD_SW4_GPIO)
        GPIO_PinInit(GPIO, BOARD_SW4_GPIO_PORT, BOARD_SW4_GPIO_PIN, &gpio_in_config);
#endif
    }

    /* Init output GPIO50 */
    GPIO_PinInit(GPIO, 1, 18, &gpio_out_config);
}

int hostsleep_init(void)
{
    osa_status_t status = KOSA_StatusSuccess;

    if (kStatus_PMSuccess != LPM_Init())
    {
        PRINTF("LPM Init Failed!\r\n");
        return -1;
    }

    status = OSA_EventCreate((osa_event_handle_t)ncp_suspend_event, 1U);
    if (status != KOSA_StatusSuccess)
    {
        PRINTF("Create ncp suspend event failed");
        return -1;
    }

    /* Create task to handle wlan-suspend command */
    status = OSA_TaskCreate((osa_task_handle_t)ncp_suspend_thread, OSA_TASK(ncp_suspend_task), NULL);
    if (status != KOSA_StatusSuccess)
    {
        PRINTF("Create ncp suspend thread failed");
        return -1;
    }

    status = OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)hs_cfm);
    if (status != KOSA_StatusSuccess)
    {
        PRINTF("Create hs cfm semaphor failed");
        return -1;
    }
    OSA_SemaphorePost((osa_semaphore_handle_t)hs_cfm);

    ncp_gpio_init();
    return 1;
}

void ncp_notify_host_gpio_init(void)
{
    /* Define the init structure for the output switch pin */
    gpio_pin_config_t notify_output_pin = {
        kGPIO_DigitalOutput,
        1
    };

    IO_MUX_SetPinMux(IO_MUX_GPIO27);
    GPIO_PortInit(GPIO, 0);
    /* Init output GPIO. Default level is high */
    /* After wakeup from PM3, usb device use GPIO 27 to notify usb host */
    GPIO_PinInit(GPIO, 0, NCP_NOTIFY_HOST_GPIO, &notify_output_pin);
}

void ncp_notify_host_gpio_output(void)
{
    /* Toggle GPIO to notify usb host that device is ready for re-enumeration */
    GPIO_PortToggle(GPIO, 0, NCP_NOTIFY_HOST_GPIO_MASK);
}
