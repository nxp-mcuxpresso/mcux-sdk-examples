/*
 * FreeRTOS Pre-Release V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* CMSE includes. */
#include <arm_cmse.h>

/* NSC functions includes. */
#include "nsc_functions.h"

/* Board specific includes. */
#include "board.h"

/**
 * @brief Counter returned from NSCFunction.
 */
static uint32_t ulSecureCounter = 0;

/**
 * @brief LED port and pins.
 */
#define LED_PORT      BOARD_LED_BLUE_GPIO_PORT
#define GREEN_LED_PIN BOARD_LED_GREEN_GPIO_PIN
#define BLUE_LED_PIN  BOARD_LED_BLUE_GPIO_PIN

/**
 * @brief typedef for non-secure callback.
 */
#if defined(__IAR_SYSTEMS_ICC__)
typedef __cmse_nonsecure_call void (*NonSecureCallback_t)(void);
#else
typedef void (*NonSecureCallback_t)(void) __attribute__((cmse_nonsecure_call));
#endif
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    uint32_t
    NSCFunction(Callback_t pxCallback)
{
    NonSecureCallback_t pxNonSecureCallback;

    /* Return function pointer with cleared LSB. */
    pxNonSecureCallback = (NonSecureCallback_t)cmse_nsfptr_create(pxCallback);

    /* Invoke the supplied callback. */
    pxNonSecureCallback();

    /* Increment the secure side counter. */
    ulSecureCounter += 1;

    /* Return the secure side counter. */
    return ulSecureCounter;
}
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    void
    vToggleGreenLED(void)
{
    /* Toggle the on-board green LED. */
    GPIO_PortToggle(GPIO, LED_PORT, (1U << GREEN_LED_PIN));
}
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    void
    vToggleBlueLED()
{
    /* Toggle the on-board blue LED. */
    GPIO_PortToggle(GPIO, LED_PORT, (1U << BLUE_LED_PIN));
}
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    uint32_t
    getSystemCoreClock(void)
{
    /* Return the secure side SystemCoreClock. */
    return SystemCoreClock;
}
/*-----------------------------------------------------------*/
