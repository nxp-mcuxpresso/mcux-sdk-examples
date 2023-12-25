/** @file host_sleep.c
 *
 *  @brief Host sleep file
 *
 *  Copyright 2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef CONFIG_HOST_SLEEP
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "host_sleep.h"
#include "lpm.h"
#include "cli.h"
#include "wlan.h"
#include "wifi.h"
#include "fsl_power.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_rtc.h"
#include "fsl_usart.h"
#ifdef CONFIG_NCP_BRIDGE
#include "fsl_gpio.h"
#include "ncp_bridge_cmd.h"
#include "app_notify.h"
#ifdef CONFIG_CRC32_HW_ACCELERATE
#include "fsl_crc.h"
#endif
#ifdef CONFIG_USB_BRIDGE
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "cdc_app.h"
#include "usb_device_config.h"
#include "usb_device_cdc_acm.h"
#endif
#endif

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
static struct cli_command host_sleep_commands[] = {
    {"wlan-suspend", "<power mode>", test_wlan_suspend},
};
extern bool wlan_is_manual;
extern int wakeup_by;
#ifdef CONFIG_NCP_BRIDGE
extern power_cfg_t global_power_config;
#endif
#ifdef CONFIG_POWER_MANAGER
extern pm_handle_t pm_handle;
extern pm_wakeup_source_t wlanWakeupSource;
extern pm_wakeup_source_t rtcWakeupSource;
extern int wlan_host_sleep_state;
os_timer_t wake_timer;
#ifdef CONFIG_UART_INTERRUPT
extern bool usart_suspend_flag;
#endif
#endif
uint64_t rtc_timeout = 0;
static uart_clock_context_t s_uartClockCtx;
#ifdef CONFIG_NCP_BRIDGE
#ifdef CONFIG_UART_BRIDGE
extern int bridge_uart_reinit();
extern int bridge_uart_deinit();
extern void bridge_uart_notify();
#endif
#ifdef CONFIG_CRC32_HW_ACCELERATE
extern void hw_crc32_init();
#endif
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void RTC_IRQHandler()
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
        DisableIRQ(RTC_IRQn);
        POWER_ClearWakeupStatus(RTC_IRQn);
        POWER_DisableWakeup(RTC_IRQn);
        wakeup_by = WAKEUP_BY_RTC;
    }
}

void PIN1_INT_IRQHandler()
{
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    DisableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_DisableWakeup(PIN1_INT_IRQn);
    wakeup_by = WAKEUP_BY_PIN1;
}

#ifdef CONFIG_POWER_MANAGER
static void wake_timer_cb(os_timer_arg_t arg)
{
    if(wakelock_isheld())
        wakelock_put();
}
#endif

#ifdef CONFIG_NCP_BRIDGE
void wlan_gpio_wakeup_host()
{
    gpio_pin_config_t gpio_out_config = {
        kGPIO_DigitalOutput,
        0,
    };
    GPIO_PortInit(GPIO, 0);
    GPIO_PinInit(GPIO, 0, 4, &gpio_out_config);
    gpio_out_config.outputLogic = 1;
    GPIO_PortInit(GPIO, 0);
    GPIO_PinInit(GPIO, 0, 4, &gpio_out_config);
}
#endif

AT_QUICKACCESS_SECTION_CODE(void host_sleep_pre_hook(void))
{
    uint32_t freq = CLK_XTAL_OSC_CLK / 40U; //frequency of LPOSC

    /* In PM2, only LPOSC and CLK32K are available. To use UART as wakeup source,
       We have to use main_clk as UART clock source, and main_clk comes from LPOSC.
       Use register access directly to avoid possible flash access in function call */
    s_uartClockCtx.selA   = CLKCTL0->MAINCLKSELA;
    s_uartClockCtx.selB   = CLKCTL0->MAINCLKSELB;
#ifdef CONFIG_NCP_BRIDGE
    s_uartClockCtx.frgSel = CLKCTL1->FLEXCOMM[0].FRGCLKSEL;
    s_uartClockCtx.frgctl = CLKCTL1->FLEXCOMM[0].FRGCTL;
    s_uartClockCtx.osr    = USART0->OSR;
    s_uartClockCtx.brg    = USART0->BRG;
#else
    s_uartClockCtx.frgSel = CLKCTL1->FLEXCOMM[3].FRGCLKSEL;
    s_uartClockCtx.frgctl = CLKCTL1->FLEXCOMM[3].FRGCTL;
    s_uartClockCtx.osr    = USART3->OSR;
    s_uartClockCtx.brg    = USART3->BRG;
#endif
    /* Switch main_clk to LPOSC */
    CLKCTL0->MAINCLKSELA = 2;
    CLKCTL0->MAINCLKSELB = 0;
#ifdef CONFIG_NCP_BRIDGE
    /* Change UART0 clock source to main_clk */
    CLKCTL1->FLEXCOMM[0].FRGCLKSEL = 0;
    CLKCTL1->FLEXCOMM[0].FRGCTL    = 0x11C7;
    USART0->OSR                    = 7;
    USART0->BRG                    = 0;
#else
    /* Change UART3 clock source to main_clk */
    CLKCTL1->FLEXCOMM[3].FRGCLKSEL = 0;
    /* bit[0:7] div, bit[8:15] mult.
     * freq(new) = freq(old)/(1 + mult/(div+1))
     * freq(new) is the frequency of LPOSC clock
     * freq(old) is the frequency of main_pll clock
     * Use the equation here to get div and mult.
     */
    CLKCTL1->FLEXCOMM[3].FRGCTL = 0x11C7;
    USART3->OSR                 = 7;
    USART3->BRG                 = 0;
#endif
    /* Update system core clock */
    SystemCoreClock = freq / ((CLKCTL0->SYSCPUAHBCLKDIV & CLKCTL0_SYSCPUAHBCLKDIV_DIV_MASK) + 1U);
}

void host_sleep_post_hook(uint32_t mode, void *param)
{
    /* Recover main_clk and UART clock source after wakeup.
     Use register access directly to avoid possible flash access in function call. */
#ifdef CONFIG_NCP_BRIDGE
    USART0->OSR                    = s_uartClockCtx.osr;
    USART0->BRG                    = s_uartClockCtx.brg;
    CLKCTL1->FLEXCOMM[0].FRGCLKSEL = s_uartClockCtx.frgSel;
    CLKCTL1->FLEXCOMM[0].FRGCTL    = s_uartClockCtx.frgctl;
#else
    USART3->OSR                    = s_uartClockCtx.osr;
    USART3->BRG                    = s_uartClockCtx.brg;
    CLKCTL1->FLEXCOMM[3].FRGCLKSEL = s_uartClockCtx.frgSel;
    CLKCTL1->FLEXCOMM[3].FRGCTL    = s_uartClockCtx.frgctl;
#endif
    CLKCTL0->MAINCLKSELA           = s_uartClockCtx.selA;
    CLKCTL0->MAINCLKSELB           = s_uartClockCtx.selB;
    SystemCoreClockUpdate();
}

void host_sleep_cli_notify(void)
{
#ifdef CONFIG_UART_INTERRUPT
#ifdef CONFIG_POWER_MANAGER
    if (pm_handle.targetState == PM_LP_STATE_PM3)
    {
        usart_suspend_flag = true;
#ifdef CONFIG_NCP_BRIDGE
#ifdef CONFIG_UART_BRIDGE
        bridge_uart_notify();
#endif
#endif
        cli_uart_notify();
    }
#endif
#endif
}

void host_sleep_pre_cfg(int mode)
{
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeLow);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    EnableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_EnableWakeup(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(WL_MCI_WAKEUP0_IRQn);
    POWER_EnableWakeup(WL_MCI_WAKEUP0_IRQn);
#ifdef CONFIG_NCP_BRIDGE
    if(global_power_config.subscribe_evt)
        app_notify_event(APP_EVT_MCU_SLEEP_ENTER, APP_EVT_REASON_SUCCESS, NULL, 0);
#endif
    (void)PRINTF("Enter low power mode PM%d\r\n", mode);
    /* PM2, enable UART3 as wakeup source */
    if (mode == 2U)
    {
#ifdef CONFIG_NCP_BRIDGE
#ifdef CONFIG_USB_BRIDGE
        POWER_ClearWakeupStatus(USB_IRQn);
        POWER_EnableWakeup(USB_IRQn);
        CLOCK_AttachClk(kLPOSC_to_MAIN_CLK);
#else
        /* Enable RX interrupt. */
        USART_EnableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
        POWER_ClearWakeupStatus(FLEXCOMM0_IRQn);
        POWER_EnableWakeup(FLEXCOMM0_IRQn);
#endif /* CONFIG_USB_BRIDGE */
#else
        /* Enable RX interrupt. */
        USART_EnableInterrupts(USART3, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
        POWER_ClearWakeupStatus(FLEXCOMM3_IRQn);
        POWER_EnableWakeup(FLEXCOMM3_IRQn);
#endif
        /*Delay UART clock switch after resume from PM2 to avoid UART FIFO read error*/
        POWER_SetPowerSwitchCallback((power_switch_callback_t)host_sleep_pre_hook, NULL, NULL, NULL);
    }
}

void host_sleep_post_cfg(int mode)
{
    uint32_t irq_mask;

    /* Disable wakeup source of WLAN and PIN1 interrupt after waking up */
    irq_mask = DisableGlobalIRQ();
    POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
    NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    DisableIRQ(PIN1_INT_IRQn);
    POWER_ClearWakeupStatus(PIN1_INT_IRQn);
    POWER_DisableWakeup(PIN1_INT_IRQn);
    EnableGlobalIRQ(irq_mask);
    POWER_ClearWakeupStatus(WL_MCI_WAKEUP0_IRQn);

    if (mode == 2U)
    {
#ifdef CONFIG_NCP_BRIDGE
#ifdef CONFIG_USB_BRIDGE
        CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);
        POWER_ClearWakeupStatus(USB_IRQn);
        POWER_DisableWakeup(USB_IRQn);
#else
        if (POWER_GetWakeupStatus(FLEXCOMM0_IRQn))
            wakeup_by = WAKEUP_BY_USART0;
        POWER_ClearWakeupStatus(FLEXCOMM0_IRQn);
        POWER_DisableWakeup(FLEXCOMM0_IRQn);
        POWER_SetPowerSwitchCallback(NULL, NULL, NULL, NULL);
        host_sleep_post_hook(2, NULL);
#endif
#else
        if (POWER_GetWakeupStatus(FLEXCOMM3_IRQn))
            wakeup_by = WAKEUP_BY_USART3;
        POWER_ClearWakeupStatus(FLEXCOMM3_IRQn);
        POWER_DisableWakeup(FLEXCOMM3_IRQn);
        POWER_SetPowerSwitchCallback(NULL, NULL, NULL, NULL);
        host_sleep_post_hook(2, NULL);
#endif
    }
#ifdef CONFIG_POWER_MANAGER
    if(!wlan_is_manual && wlan_host_sleep_state == HOST_SLEEP_PERIODIC)
    {
        wakelock_get();
        os_timer_activate(&wake_timer);
    }
#endif
#ifdef CONFIG_NCP_BRIDGE
    if(global_power_config.wakeup_host && wakeup_by == 0x1)
        wlan_gpio_wakeup_host();
    if(global_power_config.subscribe_evt)        
        app_notify_event(APP_EVT_MCU_SLEEP_EXIT, APP_EVT_REASON_SUCCESS, NULL, 0);
#endif
    PRINTF("Exit low power mode\r\n");
}

void host_sleep_dump_wakeup_source()
{
    if (wakeup_by == WAKEUP_BY_WLAN)
        wifi_print_wakeup_reason();
    else if (wakeup_by == WAKEUP_BY_RTC)
        PRINTF("Woken up by RTC\r\n");
    else if (wakeup_by == WAKEUP_BY_PIN1)
        PRINTF("Woken up by PIN1\r\n");
#ifdef CONFIG_NCP_BRIDGE
    else if (wakeup_by == WAKEUP_BY_USART0)
#else
    else if (wakeup_by == WAKEUP_BY_USART3)
#endif
        PRINTF("Woken up by USART\r\n");
}

static void wlan_GetSleepConfig(power_sleep_config_t *config)
{
    config->pm2MemPuCfg = WLAN_PM2_MEM_PU_CFG;
    config->pm2AnaPuCfg = WLAN_PM2_ANA_PU_CFG;
    config->clkGate     = WLAN_SOURCE_CLK_GATE;
    config->memPdCfg    = WLAN_MEM_PD_CFG;
    config->pm3BuckCfg  = WLAN_PM3_BUCK_CFG;
}

int wlan_config_suspend_mode(int mode)
{
    power_sleep_config_t config;

    if (!wlan_is_manual)
    {
#ifdef CONFIG_NCP_BRIDGE
        app_notify_event(APP_EVT_SUSPEND, APP_EVT_REASON_FAILURE, NULL, 0);
#endif
        PRINTF("Error: Maunal mode is not selected!\r\n");
        return -1;
    }
    memset(&config, 0x0, sizeof(power_sleep_config_t));
    if (mode >= 2)
        wlan_GetSleepConfig(&config);
#ifdef CONFIG_NCP_BRIDGE
    app_notify_event(APP_EVT_SUSPEND, APP_EVT_REASON_SUCCESS, NULL, 0);
#endif
    host_sleep_pre_cfg(mode);
    if(mode == 3)
    {
#ifdef CONFIG_NCP_BRIDGE
#ifdef CONFIG_UART_BRIDGE
        usart_suspend_flag = true;
        bridge_uart_notify();
        taskYIELD();
        bridge_uart_deinit();
#endif
#endif
#ifdef CONFIG_UART_INTERRUPT
#ifdef CONFIG_NCP_BRIDGE
        cli_uart_notify();
        taskYIELD();
#endif
        cli_uart_deinit();
#endif
        DbgConsole_Deinit();
#ifdef CONFIG_NCP_BRIDGE
#ifdef CONFIG_CRC32_HW_ACCELERATE
        CRC_Reset(CRC);
#endif
#ifdef CONFIG_USB_BRIDGE
        usb_device_app_deinit();
#endif
#endif
    }
    POWER_EnterPowerMode(mode, &config);
    if (mode == 3)
    {
        /* Perihperal state lost, need reinitialize in exit from PM3 */
        lpm_pm3_exit_hw_reinit();
    }
    if (wlan_is_started())
        wlan_cancel_host_sleep();
    host_sleep_post_cfg(mode);
    if (mode == 1)
    {
        (void)PRINTF("Exit from PM1.\r\n");
        (void)PRINTF("Wakeup status is not available for PM1 due to PMU HW design\r\n");
    }
    else
        host_sleep_dump_wakeup_source();
    wakeup_by = 0;
    wifi_clear_wakeup_reason();
    wlan_is_manual = MFALSE;
    return 0;
}

void test_wlan_suspend(int argc, char **argv)
{
    int mode = 0;

    if (argc < 2)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("    wlan-suspend <power mode>\r\n");
        (void)PRINTF("    1:PM1 2:PM2 3:PM3 4:PM4\r\n");
        (void)PRINTF("Example:\r\n");
        (void)PRINTF("    wlan-suspend 3\r\n");
        return;
    }
    mode = atoi(argv[1]);
    if (mode < 1 || mode > 4)
    {
        (void)PRINTF("Invalid low power mode\r\n");
        (void)PRINTF("Only PM1/PM2/PM3/PM4 supported here\r\n");
        return;
    }
    if (mode == 4)
    {
        (void)PRINTF(
            "MCU can't resume from PM4 for SRAM are forced power off, because this is HW design and PM4 modes is used "
            "for power."
            "consumption validation test\r\n");
    }
    wlan_config_suspend_mode(mode);
}

#ifdef CONFIG_POWER_MANAGER
#define SOCCTRL_CHIP_INFO_REV_NUM_MASK (0xFU)
uint8_t get_chip_info(void)
{
    return (uint8_t)(SOCCTRL->CHIP_INFO & SOCCTRL_CHIP_INFO_REV_NUM_MASK);
}

void powerManager_EnterLowPower()
{
    /* Check is_wakeup_cond_set first, as wakelcok will be deleted in wlan-reset 0 */
    if (wlan_host_sleep_state && pm_handle.enable && !wakelock_isheld())
    {
#ifndef CONFIG_NCP_BRIDGE
        if((get_chip_info() == 1) || (get_chip_info() == 2))
            PM_SetConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
        else
            PM_SetConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
#endif
        /* duration unit is us here */
        PM_EnterLowPower(rtc_timeout);
        host_sleep_dump_wakeup_source();
        wakeup_by = 0;
        wifi_clear_wakeup_reason();
#ifndef CONFIG_NCP_BRIDGE
        /* Exit low power and reset constraints */
        if((get_chip_info() == 1) || (get_chip_info() == 2))
            PM_ReleaseConstraints(PM_LP_STATE_PM3, APP_PM3_CONSTRAINTS);
        else
            PM_ReleaseConstraints(PM_LP_STATE_PM2, APP_PM2_CONSTRAINTS);
#endif
    }
}
#endif

int host_sleep_cli_init(void)
{
    unsigned int i;

    for (i = 0; i < sizeof(host_sleep_commands) / sizeof(struct cli_command); i++)
    {
        if (cli_register_command(&host_sleep_commands[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}

int hostsleep_init(void)
{
    if (kStatus_PMSuccess != LPM_Init())
    {
        PRINTF("LPM Init Failed!\r\n");
        return -1;
    }
#ifdef CONFIG_POWER_MANAGER
    os_timer_create(&wake_timer, "wake_timer", HOST_SLEEP_DEF_WAKE_TIME, &wake_timer_cb, NULL,
                    OS_TIMER_ONE_SHOT, OS_TIMER_NO_ACTIVATE);
#endif
    return 1;
}

#endif /* CONFIG_HOST_SLEEP */
