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
#include "fsl_debug_console.h"

#include "app_usbd_cfg.h"
#include "audio_usbd.h"
#include "audio_codec.h"
#include "Power_Tasks.h"

#include "fsl_usart.h"
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
    uint32_t pllFreq;

    gpio_pin_config_t gpioConfig;
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    BOARD_InitBootPins();
    BOARD_BootClockFROHF48M();
    BOARD_InitDebugConsole();
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*Turn on USB Phy */
    /*  board leds */
    gpioConfig.pinDirection = kGPIO_DigitalOutput;
    gpioConfig.outputLogic  = 1U;
    GPIO_PinInit(GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, &gpioConfig);
    GPIO_PinInit(GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, &gpioConfig);
    GPIO_PinInit(GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, &gpioConfig);
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0Clock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf));
    SystemCoreClockUpdate();
    SysTick_Init();

    PRINTF("USB audio demo:\r\n");
    /*PRINTF("build date: " __DATE__ " build time: " __TIME__ "\r\n");*/
    PRINTF("processor frequency: %ldMHz\r\n", SystemCoreClock / 1000000);

#if defined(LOWPOWEROPERATION)
    /* Setup up basic power prior to USB setup. USB setup also sets up
     * the audio interfaces and any necessary interface specific pin
     * muxing. */
    Power_Init();
#endif

    /* Initialize USB subsystem */
    ret     = usbd_init();
    pllFreq = CLOCK_GetPllOutFreq();
    PRINTF("Audio PLL frequency: %ld.%ldMHz\r\n", pllFreq / 1000000, (pllFreq - (pllFreq / 1000000) * 1000000) / 1000);

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
