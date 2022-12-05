/*
 * @brief Main module
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <stdio.h>
#include <string.h>

#include "pin_mux.h"
#include "board.h"
#include "delay.h"

#include "app_usbd_cfg.h"
#include "audio_usbd.h"
#include "audio_codec.h"
#include "Power_Tasks.h"
#include "fsl_clock.h"
#include "fsl_debug_console.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void BOARD_InitHardware(void);

extern ErrorCode_t usbd_init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
    ErrorCode_t ret = LPC_OK;
    /*uint32_t pllFreq;*/

    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    //    CLOCK_EnableClock(kCLOCK_Gpio0);
    //    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset USB0 and USB1 device */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);

    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf));
    SystemCoreClockUpdate();
    SysTick_Init();

    PRINTF("USB audio demo:\n");
    /*PRINTF("build date: " __DATE__ " build time: " __TIME__ "\r\n");*/
    /*printf("processor frequency: %ldMHz\n", SystemCoreClock / 1000000);*/

#if defined(LOWPOWEROPERATION)
    /* Setup up basic power prior to USB setup. USB setup also sets up
     * the audio interfaces and any necessary interface specific pin
     * muxing. */
    Power_Init();
#endif

    /* Initialize USB subsystem */
    ret = usbd_init();
    /*pllFreq = CLOCK_GetAudioPllOutFreq();*/
    /*printf("Audio PLL frequency: %ld.%ldMHz\n", pllFreq / 1000000, (pllFreq - (pllFreq / 1000000) * 1000000) /
     * 1000);*/

    if (ret != LPC_OK)
    {
        return 0;
    }
    while (1U)
    {
        Codec_Tasks();
#if defined(LOWPOWEROPERATION)
        Power_Tasks();
#else
        __WFI();
#endif
    }
}
