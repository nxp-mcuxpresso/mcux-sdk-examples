/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NORMAL_GPIO_OUT_PORT                GPIO3
#define NORMAL_GPIO_OUT_PIN                 3
#define NORMAL_GPIO_IN_PORT                 GPIO3
#define NORMAL_GPIO_IN_PIN                  4U
#define NORMAL_GPIO_OP_CLK_IN_HZ            CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)
#define NORMAL_GPIO_OUT_PIN_SET_TO_NORMAL() IOMUXC_GPR->GPR42 &= ~(1UL << NORMAL_GPIO_OUT_PIN)
#define NORMAL_GPIO_IN_PIN_SET_TO_NORMAL()  IOMUXC_GPR->GPR42 &= ~(1UL << NORMAL_GPIO_IN_PIN)

#define FAST_GPIO_OUT_PORT              CM7_GPIO3
#define FAST_GPIO_OUT_PIN               3U
#define FAST_GPIO_IN_PORT               CM7_GPIO3
#define FAST_GPIO_IN_PIN                4U
#define FAST_GPIO_OP_CLK_IN_HZ          CLOCK_GetRootClockFreq(kCLOCK_Root_M7)
#define FAST_GPIO_OUT_PIN_SET_TO_FAST() IOMUXC_GPR->GPR42 |= (1UL << FAST_GPIO_OUT_PIN)
#define FAST_GPIO_IN_PIN_SET_TO_FAST()  IOMUXC_GPR->GPR42 |= (1UL << FAST_GPIO_IN_PIN)

#define FAST_GPIO_IRQ_HANDLER     CM7_GPIO2_3_IRQHandler
#define FAST_GPIO_IN_IRQ          CM7_GPIO2_3_IRQn
#define FAST_GPIO_IN_IRQ_PRIORITY 1
#define GPIO_LOOP_NUM 1000000UL

#define NORMAL_GPIO_OUT_PIN_MASK (1UL << NORMAL_GPIO_OUT_PIN)

#if (defined(FSL_FEATURE_IGPIO_HAS_DR_SET) && FSL_FEATURE_IGPIO_HAS_DR_SET)
#define NORMAL_GPIO_OUT_SET() NORMAL_GPIO_OUT_PORT->DR_SET = NORMAL_GPIO_OUT_PIN_MASK
#else
#define NORMAL_GPIO_OUT_SET() NORMAL_GPIO_OUT_PORT->DR |= NORMAL_GPIO_OUT_PIN_MASK
#endif /* FSL_FEATURE_IGPIO_HAS_DR_SET */

#if (defined(FSL_FEATURE_IGPIO_HAS_DR_CLEAR) && FSL_FEATURE_IGPIO_HAS_DR_CLEAR)
#define NORMAL_GPIO_OUT_CLR() NORMAL_GPIO_OUT_PORT->DR_CLEAR = NORMAL_GPIO_OUT_PIN_MASK
#else
#define NORMAL_GPIO_OUT_CLR() NORMAL_GPIO_OUT_PORT->DR &= ~NORMAL_GPIO_OUT_PIN_MASK
#endif /* FSL_FEATURE_IGPIO_HAS_DR_CLEAR */

#if (defined(FSL_FEATURE_IGPIO_HAS_DR_TOGGLE) && (FSL_FEATURE_IGPIO_HAS_DR_TOGGLE == 1))
#define NORMAL_GPIO_OUT_TOG() NORMAL_GPIO_OUT_PORT->DR_TOGGLE = NORMAL_GPIO_OUT_PIN_MASK
#else
#define NORMAL_GPIO_OUT_TOG() NORMAL_GPIO_OUT_PORT->DR ^= NORMAL_GPIO_OUT_PIN_MASK
#endif /* FSL_FEATURE_IGPIO_HAS_DR_TOGGLE */

#define NORMAL_GPIO_IN_PIN_MASK  (1UL << NORMAL_GPIO_IN_PIN)
#define NORMAL_GPIO_GET_IN_PIN() ((bool)((NORMAL_GPIO_IN_PORT->DR & NORMAL_GPIO_IN_PIN_MASK) >> NORMAL_GPIO_IN_PIN))

#define FAST_GPIO_OUT_PIN_MASK (1UL << FAST_GPIO_OUT_PIN)

#if (defined(FSL_FEATURE_IGPIO_HAS_DR_SET) && FSL_FEATURE_IGPIO_HAS_DR_SET)
#define FAST_GPIO_OUT_SET() FAST_GPIO_OUT_PORT->DR_SET = FAST_GPIO_OUT_PIN_MASK
#else
#define FAST_GPIO_OUT_SET() FAST_GPIO_OUT_PORT->DR |= FAST_GPIO_OUT_PIN_MASK
#endif /* FSL_FEATURE_IGPIO_HAS_DR_SET */

#if (defined(FSL_FEATURE_IGPIO_HAS_DR_CLEAR) && FSL_FEATURE_IGPIO_HAS_DR_CLEAR)
#define FAST_GPIO_OUT_CLR() FAST_GPIO_OUT_PORT->DR_CLEAR = FAST_GPIO_OUT_PIN_MASK
#else
#define FAST_GPIO_OUT_CLR() FAST_GPIO_OUT_PORT->DR &= ~FAST_GPIO_OUT_PIN_MASK
#endif /* FSL_FEATURE_IGPIO_HAS_DR_CLEAR */

#if (defined(FSL_FEATURE_IGPIO_HAS_DR_TOGGLE) && (FSL_FEATURE_IGPIO_HAS_DR_TOGGLE == 1))
#define FAST_GPIO_OUT_TOG() FAST_GPIO_OUT_PORT->DR_TOGGLE = FAST_GPIO_OUT_PIN_MASK
#else
#define FAST_GPIO_OUT_TOG() FAST_GPIO_OUT_PORT->DR ^= FAST_GPIO_OUT_PIN_MASK
#endif /* FSL_FEATURE_IGPIO_HAS_DR_TOGGLE */

#define FAST_GPIO_IN_PIN_MASK  (1UL << FAST_GPIO_IN_PIN)
#define FAST_GPIO_GET_IN_PIN() ((bool)((FAST_GPIO_IN_PORT->DR & FAST_GPIO_IN_PIN_MASK) >> FAST_GPIO_IN_PIN))
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_u32SysTimer;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    g_u32SysTimer++;
}

uint32_t TIMER_GetTime(void)
{
    return g_u32SysTimer;
}

uint32_t TIMER_GetElapsedTime(uint32_t u32PreTime)
{
    uint32_t u32ElapsedTime, u32CurrTime = TIMER_GetTime();

    if (u32CurrTime >= u32PreTime)
    {
        u32ElapsedTime = u32CurrTime - u32PreTime;
    }
    else
    {
        u32ElapsedTime = u32CurrTime + (0xFFFFFFFFUL - u32PreTime) + 1;
    }
    return u32ElapsedTime;
}

/* Fast GPIO IRQ handler */
void FAST_GPIO_IRQ_HANDLER(void)
{
    /* Just disable the IRQ */
    NVIC_DisableIRQ(FAST_GPIO_IN_IRQ);

    PRINTF("Fast GPIO IRQ occurred!\r\n\r\n");

    SDK_ISR_EXIT_BARRIER;
}

void FastGpio_Init(void)
{
    gpio_pin_config_t gpio_config_output = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    gpio_pin_config_t gpio_config_input  = {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};

    /* Switch pin to fast gpio */
    FAST_GPIO_OUT_PIN_SET_TO_FAST();
    GPIO_PinInit(FAST_GPIO_OUT_PORT, FAST_GPIO_OUT_PIN, &gpio_config_output);

    /* Switch pin to fast gpio */
    FAST_GPIO_IN_PIN_SET_TO_FAST();
    GPIO_PinInit(FAST_GPIO_IN_PORT, FAST_GPIO_IN_PIN, &gpio_config_input);
}

void FastGpio_3CyclesOutput(void)
{
    uint32_t i;
    for (i = 0; i < GPIO_LOOP_NUM; i++)
    {
        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();

        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();

        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();
    }
}

void FastGpio_4CyclesOutput(void)
{
    uint32_t i;
    for (i = 0; i < GPIO_LOOP_NUM; i++)
    {
        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();

        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();

        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();

        FAST_GPIO_OUT_SET();
        FAST_GPIO_OUT_CLR();
    }
}

void FastGpio_Input(void)
{
    uint32_t i;
    for (i = 0; i < GPIO_LOOP_NUM; i++)
    {
        if (FAST_GPIO_GET_IN_PIN())
        {
            FAST_GPIO_OUT_SET();
        }
        else
        {
            FAST_GPIO_OUT_CLR();
        }
    }
}

void FastGpio_EnableInputInterrupt(void)
{
    NVIC_SetPriority(FAST_GPIO_IN_IRQ, FAST_GPIO_IN_IRQ_PRIORITY);
    NVIC_EnableIRQ(FAST_GPIO_IN_IRQ);

    GPIO_PinSetInterruptConfig(FAST_GPIO_IN_PORT, FAST_GPIO_IN_PIN, kGPIO_IntLowLevel);
    GPIO_EnableInterrupts(FAST_GPIO_IN_PORT, FAST_GPIO_IN_PIN_MASK);
}

void NormalGpio_Init(void)
{
    gpio_pin_config_t gpio_config_output = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    gpio_pin_config_t gpio_config_input  = {kGPIO_DigitalInput, 0, kGPIO_NoIntmode};

    NORMAL_GPIO_OUT_PIN_SET_TO_NORMAL();
    GPIO_PinInit(NORMAL_GPIO_OUT_PORT, NORMAL_GPIO_OUT_PIN, &gpio_config_output);

    NORMAL_GPIO_IN_PIN_SET_TO_NORMAL();
    GPIO_PinInit(NORMAL_GPIO_IN_PORT, NORMAL_GPIO_IN_PIN, &gpio_config_input);
}

void NormalGpio_3CyclesOutput(void)
{
    uint32_t i;
    for (i = 0; i < GPIO_LOOP_NUM; i++)
    {
        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();

        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();

        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();
    }
}

void NormalGpio_4CyclesOutput(void)
{
    uint32_t i;
    for (i = 0; i < GPIO_LOOP_NUM; i++)
    {
        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();

        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();

        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();

        NORMAL_GPIO_OUT_SET();
        NORMAL_GPIO_OUT_CLR();
    }
}

void NormalGpio_Input(void)
{
    uint32_t i;
    for (i = 0; i < GPIO_LOOP_NUM; i++)
    {
        if (NORMAL_GPIO_GET_IN_PIN())
        {
            NORMAL_GPIO_OUT_SET();
        }
        else
        {
            NORMAL_GPIO_OUT_CLR();
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t u32Time;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    /* Print a note to terminal. */
    PRINTF("\r\nGPIO Driver example\r\n");
    PRINTF(
        "This example is intended to run in debug/release configuration, \r\nwith all code and data in TCM, to show "
        "the potential of fast gpio\r\n");

    PRINTF("\r\nCore Clock = %uMHz, Bus Clock = %uMHz\r\n", FAST_GPIO_OP_CLK_IN_HZ / 1000000UL,
           NORMAL_GPIO_OP_CLK_IN_HZ / 1000000UL);
    PRINTF("Loop num = %u\r\n", GPIO_LOOP_NUM);

    /* The comparison for normal and fast gpio operation, shows advantage of fast gpio */

    NormalGpio_Init();

    /* The comparison for 3 cycles and 4 cycles within GPIO_LOOP_NUM loop, shows access time to normal gpio */
    u32Time = TIMER_GetTime();
    NormalGpio_3CyclesOutput();
    PRINTF("  Normal GPIO cycles 3 output takes %ums\r\n", TIMER_GetElapsedTime(u32Time));

    u32Time = TIMER_GetTime();
    NormalGpio_4CyclesOutput();
    PRINTF("  Normal GPIO cycles 4 output takes %ums\r\n", TIMER_GetElapsedTime(u32Time));

    u32Time = TIMER_GetTime();
    NormalGpio_Input();
    PRINTF("  Normal GPIO input takes %ums\r\n", TIMER_GetElapsedTime(u32Time));

    FastGpio_Init();

    /* The comparison for 3 cycles and 4 cycles within GPIO_LOOP_NUM loop, shows access time to fast gpio */
    u32Time = TIMER_GetTime();
    FastGpio_3CyclesOutput();
    PRINTF("  Fast GPIO cycles 3 output takes %ums\r\n", TIMER_GetElapsedTime(u32Time));

    u32Time = TIMER_GetTime();
    FastGpio_4CyclesOutput();
    PRINTF("  Fast GPIO cycles 4 output takes %ums\r\n", TIMER_GetElapsedTime(u32Time));

    u32Time = TIMER_GetTime();
    FastGpio_Input();
    PRINTF("  Fast GPIO input takes %ums\r\n", TIMER_GetElapsedTime(u32Time));

    /* Fast GPIO interrupt example */
    FastGpio_EnableInputInterrupt();

    while (1)
    {
    }
}
