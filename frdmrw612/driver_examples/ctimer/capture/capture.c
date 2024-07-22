/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ctimer.h"

#include "fsl_inputmux.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CTIMER CTIMER0
/* GPIO52: Port 1 Pin 20*/
#define DEMO_GPIO_PORT 1
#define DEMO_GPIO_PIN  20

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DEMO_InitCtimerInput(void);
void DEMO_InitGpioPin(void);
void DEMO_PullGpioPin(int level);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Route the GPIO to ctimer capture. */
void DEMO_InitCtimerInput(void)
{
    INPUTMUX_Init(INPUTMUX);

    /*
     * Connect INP11 to Channel 0. INP11 is GPIO51 is configured in pin_mux.c.
     */
    INPUTMUX_AttachSignal(INPUTMUX, 0U, kINPUTMUX_Gpio51Inp11ToTimer0CaptureChannels);
}

void DEMO_InitGpioPin(void)
{
    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

    /* Init to low level. */
    GPIO_PinInit(GPIO, DEMO_GPIO_PORT, DEMO_GPIO_PIN, &pinConfig);
}

void DEMO_PullGpioPin(int level)
{
    GPIO_PinWrite(GPIO, DEMO_GPIO_PORT, DEMO_GPIO_PIN, (uint8_t)level);
}

/*!
 * @brief Main function
 */
int main(void)
{
    ctimer_config_t ctimerConfig;
    uint32_t captureValue;

    /* Init hardware*/
    /* Use 16 MHz clock for the Ctimer0 */
    CLOCK_AttachClk(kSFRO_to_CTIMER0);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("CTimer capture example\r\n");
    PRINTF("Rising edge triggered and CTimer capture the edge periodically\r\n\r\n");

    DEMO_InitGpioPin();
    DEMO_PullGpioPin(0);
    DEMO_InitCtimerInput();

    /*
     * ctimerConfig.mode = kCTIMER_TimerMode;
     * ctimerConfig.input = kCTIMER_Capture_0;
     * ctimerConfig.prescale = 0;
     */
    CTIMER_GetDefaultConfig(&ctimerConfig);

    CTIMER_Init(DEMO_CTIMER, &ctimerConfig);

    /*
     * Setup the capture, but don't enable the interrupt. And enable the interrupt
     * later using CTIMER_EnableInterrupts. Because if enable interrupt usig
     * CTIMER_SetupCapture, the CTIMER interrupt is also enabled in NVIC, then default
     * driver IRQ handler is called, and callback is involed. To show the capture
     * function easily, the default ISR and callback is not used.
     */
    CTIMER_SetupCapture(DEMO_CTIMER, kCTIMER_Capture_0, kCTIMER_Capture_RiseEdge, false);

    /*
     * Enable the interrupt, so that the kCTIMER_Capture0Flag can be set when
     * edge captured.
     */
    CTIMER_EnableInterrupts(DEMO_CTIMER, kCTIMER_Capture0InterruptEnable);

    CTIMER_StartTimer(DEMO_CTIMER);

    while (1)
    {
        /* Pull up the capture pin, CTIMER will capture the edge. */
        DEMO_PullGpioPin(1);

        /* Wait until edge detected, and timer count saved to capture register */
        while (0U == (kCTIMER_Capture0Flag & CTIMER_GetStatusFlags(DEMO_CTIMER)))
        {
        }

        captureValue = CTIMER_GetCaptureValue(DEMO_CTIMER, kCTIMER_Capture_0);

        CTIMER_ClearStatusFlags(DEMO_CTIMER, kCTIMER_Capture0Flag);

        PRINTF("Timer value is %d when rising edge captured\r\n", captureValue);

        /* Pull down the capture pin, prepare for next capture. */
        DEMO_PullGpioPin(0);

        /* Delay for a while for next capture. */
        SDK_DelayAtLeastUs(1 * 1000 * 1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    }
}
