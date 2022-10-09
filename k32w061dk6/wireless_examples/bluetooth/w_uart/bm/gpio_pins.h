/*
 * Copyright (c) 2013-2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __GPIO_PINS_H__
#define __GPIO_PINS_H__

#include "GPIO_Adapter.h"

/*! @file */
/*!*/
/*! This file contains gpio pin definitions used by gpio peripheral driver.*/
/*! The enums in _gpio_pins map to the real gpio pin numbers defined in*/
/*! gpioPinLookupTable. And this might be different in different board.*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/* There are 2 red LEDs on DK6 board: PIO0 and PIO3 */
#define BOARD_LED_RED1_GPIO              GPIO
#define BOARD_LED_RED1_GPIO_PORT         0U
#define BOARD_LED_RED1_GPIO_PIN          0U
#define IOCON_LED_RED1_PIN               BOARD_LED_RED1_GPIO_PIN

#define BOARD_LED_RED2_GPIO              GPIO
#define BOARD_LED_RED2_GPIO_PORT         0U
#define BOARD_LED_RED2_GPIO_PIN          3U
#define IOCON_LED_RED2_PIN               BOARD_LED_RED2_GPIO_PIN

#define IOCON_LED_MODE_FUNC              (0U)

/* We have 2 switch push-buttons on DK6 board */
#define BOARD_USER_BUTTON1_GPIO              GPIO
#define BOARD_USER_BUTTON1_GPIO_PORT         0U
#define BOARD_USER_BUTTON1_GPIO_PIN          1U
#define IOCON_USER_BUTTON1_PIN               BOARD_USER_BUTTON1_GPIO_PIN

#define BOARD_USER_BUTTON2_GPIO              GPIO
#define BOARD_USER_BUTTON2_GPIO_PORT         0U
#define BOARD_USER_BUTTON2_GPIO_PIN          5U    /* shared with ISP entry */
#define IOCON_USER_BUTTON2_PIN               BOARD_USER_BUTTON2_GPIO_PIN

#define IOCON_USER_BUTTON_MODE_FUNC          (0U)



extern const gpioInputPinConfig_t            dk6_button_io_pins[];
extern const gpioOutputPinConfig_t           dk6_leds_io_pins[];

#define ledPins                              dk6_leds_io_pins
#define switchPins                           dk6_button_io_pins

#if gDbgUseDbgIos
extern const gpioOutputPinConfig_t           dk6_dbg_io_pins[];
#define dbgPins                              dk6_dbg_io_pins
#define gDbgNbDbgIos_c                       5
#endif

#endif /* __GPIO_PINS_H__ */
