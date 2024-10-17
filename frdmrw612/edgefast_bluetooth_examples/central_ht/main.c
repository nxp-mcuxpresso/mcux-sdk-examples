/*
 * Copyright 2021-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "task.h"

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
#include "PWR_Interface.h"
#include "fwk_platform_lowpower.h"
#endif /* APP_LOWPOWER_ENABLED */

#include "central_ht.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "controller.h"
#include "fsl_power.h"
#include "usb_host_config.h"
#include "usb_host.h"
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
#include "els_pkc_mbedtls.h"
#include "platform_hw_ip.h"
#endif /* CONFIG_BT_SMP */
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_InitHardware(void);
extern void APP_InitServices(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
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

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
    CRYPTO_InitHardware();
#endif /* CONFIG_BT_SMP */

#if (defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0))|| \
    (defined(APP_USE_SENSORS) && (APP_USE_SENSORS > 0))
    APP_InitServices();
#endif

    if (xTaskCreate(central_ht_task, "central_ht_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("central ht task creation failed!\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    bool abortIdle = false;
    uint64_t expectedIdleTimeUs, actualIdleTimeUs;
    uint32_t irqMask = DisableGlobalIRQ();

    /* Disable and prepare systicks for low power. */
    abortIdle = PWR_SysticksPreProcess((uint32_t)xExpectedIdleTime, &expectedIdleTimeUs);

    if (abortIdle == false)
    {
        /* Enter low power with a maximal timeout. */
        actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

        /* Re enable systicks and compensate systick timebase. */
        PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
    }

    /* Exit from critical section. */
    EnableGlobalIRQ(irqMask);
}
#endif /* APP_LOWPOWER_ENABLED */

