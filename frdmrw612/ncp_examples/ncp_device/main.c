/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2020-2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_os_abstraction.h"
#include "app.h"

#include "fsl_rtc.h"
#include "fsl_power.h"
#if CONFIG_HOST_SLEEP
#include "host_sleep.h"
#endif

#include "ncp_adapter.h"
#include "osa.h"

#include "fsl_device_registers.h"
#if (CONFIG_WIFI_USB_FILE_ACCESS || (defined(CONFIG_BT_SNOOP) && (CONFIG_BT_SNOOP > 0)))
#include "usb_host_config.h"
#include "usb_host.h"
#if (CONFIG_WIFI_USB_FILE_ACCESS || (!defined(CONFIG_BT_SNOOP) || (CONFIG_BT_SNOOP == 0)))
#include "usb_support.h"
#endif
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined(CONFIG_BT_SNOOP) && (CONFIG_BT_SNOOP > 0))
#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif
#endif

#define NCP_INBUF_SIZE     4096

#if (CONFIG_NCP_WIFI) && !(CONFIG_NCP_BLE)
#define TASK_MAIN_PRIO         configMAX_PRIORITIES - 4
#else
#define TASK_MAIN_PRIO         OSA_TASK_PRIORITY_MIN - 2
#endif
#define TASK_MAIN_STACK_SIZE   3072

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern int system_ncp_init(void);
extern int ncp_cmd_list_init(void);
#if CONFIG_NCP_WIFI
extern int wifi_ncp_init(void);
#endif
#if CONFIG_NCP_BLE
extern int ble_ncp_init(void);
#endif
#if CONFIG_NCP_OT
extern void appOtStart(int argc, char *argv[]);
extern void otSysRunIdleTask(void);
#endif
extern void coex_controller_init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if CONFIG_WIFI_USB_FILE_ACCESS
extern usb_host_handle g_HostHandle;
#endif

void task_main(osa_task_param_t arg);
OSA_TASK_DEFINE(task_main, TASK_MAIN_PRIO, 1, TASK_MAIN_STACK_SIZE, 0);
static OSA_TASK_HANDLE_DEFINE(main_task_handle);

uint32_t current_cmd = 0;
uint16_t g_cmd_seqno = 0;
uint8_t cmd_buf[NCP_INBUF_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/

#if (CONFIG_WIFI_USB_FILE_ACCESS || (defined(CONFIG_BT_SNOOP) && (CONFIG_BT_SNOOP > 0)))

#if (CONFIG_WIFI_USB_FILE_ACCESS || (!defined(CONFIG_BT_SNOOP) || (CONFIG_BT_SNOOP == 0)))
void USBHS_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}
#endif

void USB_HostClockInit(void)
{
    /* reset USB */
    RESET_PeripheralReset(kUSB_RST_SHIFT_RSTn);
    /* enable usb clock */
    CLOCK_EnableClock(kCLOCK_Usb);
    /* enable usb phy clock */
    CLOCK_EnableUsbhsPhyClock();
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
    /* USB_HOST_CONFIG_EHCI */

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

#if (CONFIG_WIFI_USB_FILE_ACCESS || (!defined(CONFIG_BT_SNOOP) || (CONFIG_BT_SNOOP == 0)))
void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}
#endif
#endif

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

void task_main(void *param)
{
    int32_t result = 0;

    printSeparator();
    result = ncp_adapter_init();
    assert(NCP_SUCCESS == result);

    printSeparator();
    PRINTF("Initialize NCP config littlefs CLIs\r\n");
    result = system_ncp_init();
    assert(NCP_SUCCESS == result);

#if CONFIG_NCP_WIFI
    result = wifi_ncp_init();
    assert(NCP_SUCCESS == result);
#endif

    coex_controller_init();

#if CONFIG_NCP_BLE
    result = ble_ncp_init();
    assert(NCP_SUCCESS == result);
#endif

#if CONFIG_NCP_OT
    appOtStart(0, NULL);
    /* register ot idle function to os idle hook list */
    OSA_SetupIdleFunction(otSysRunIdleTask);
    PRINTF("OT initialized\r\n");
#endif

    result = ncp_cmd_list_init();
    assert(NCP_SUCCESS == result);

    printSeparator();
#if CONFIG_HOST_SLEEP
    hostsleep_init();
#endif

    while (1)
    {
        /* wait for interface up */
        OSA_TimeDelay(portMAX_DELAY);
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int main(void)
{
    BaseType_t result = 0;
    (void)result;
    BOARD_InitBootPins();
    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
    }
    BOARD_InitDebugConsole();
    /* Reset GMDA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);
    /* Keep CAU sleep clock here. */
    /* CPU1 uses Internal clock when in low power mode. */
    POWER_ConfigCauInSleep(false);
    BOARD_InitSleepPinConfig();

    RTC_Init(RTC);

    printSeparator();
    PRINTF("NCP device demo\r\n");
    printSeparator();

#if (CONFIG_NCP_USB) && (CONFIG_WIFI_USB_FILE_ACCESS)
    usb_init();
#endif
    (void)OSA_TaskCreate((osa_task_handle_t)main_task_handle, OSA_TASK(task_main), NULL);

    OSA_Start();
    for (;;)
        ;
}
