/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_plu.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* PLU module */
#define DEMO_PLU_BASE              PLU0
#define DEMO_PLU_LUT_IN_SRC_0      kPLU_LUT_IN_SRC_PLU_IN_2
#define DEMO_PLU_LUT_IN_SRC_1      kPLU_LUT_IN_SRC_PLU_IN_3
#define DEMO_PLU_LUT_IN_SRC_2      kPLU_LUT_IN_SRC_PLU_IN_5
#define DEMO_PLU_LUT_OUT_SRC_0     kPLU_OUT_SRC_LUT_0
#define DEMO_PLU_LUT_OUT_SRC_1     kPLU_OUT_SRC_LUT_1
#define DEMO_PLU_LUT_OUT_SRC_2     kPLU_OUT_SRC_LUT_2
#define DEMO_PLU_LUT_0_TRUTH_TABLE 0x000004D /* 0b01001101 */
#define DEMO_PLU_LUT_1_TRUTH_TABLE 0x000002B /* 0b00101011 */
#define DEMO_PLU_LUT_2_TRUTH_TABLE 0x0000017 /* 0b00010111 */

/* GPIO module */
#define DEMO_NO_HAS_GPIO_PORT_PARAM 1U
#define DEMO_GPIO_BASE              GPIO4
#define DEMO_GPIO_PLU_SRC_0_PIN     18U
#define DEMO_GPIO_PLU_SRC_1_PIN     19U
#define DEMO_GPIO_PLU_SRC_2_PIN     20U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static uint32_t DEMO_GetUserInput(uint32_t maxChoice);
static uint32_t DEMO_GetPluInputSourceValue(uint32_t src);
static uint32_t DEMO_GetPluInputSource(void);
static void SetPluInputSource(void);

static void PLU_Configuration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Get user's input
 */
static uint32_t DEMO_GetUserInput(uint32_t maxChoice)
{
    uint32_t ch = 0U;

    while (1)
    {
        ch = GETCHAR();
        if ((ch < '0') || (ch > (maxChoice + '0')))
        {
            continue;
        }
        else
        {
            ch = ch - '0';
            break;
        }
    }

    return ch;
}

/*!
 * @brief Get value be written to plu input source
 */
static uint32_t DEMO_GetPluInputSourceValue(uint32_t src)
{
    uint32_t value = 0U;

    if (src != 3)
    {
        PRINTF("\r\nSelect the input value.\r\n");
        PRINTF("0. Low level.\r\n");
        PRINTF("1. High level.\r\n");

        value = DEMO_GetUserInput(1);
    }
    else
    {
        PRINTF("\r\nInput the three values like 000.\r\n");
        PRINTF("0. Low level.\r\n");
        PRINTF("1. High level.\r\n");
        value = (DEMO_GetUserInput(1) << 2) + (DEMO_GetUserInput(1) << 1) + DEMO_GetUserInput(1);
    }

    return value;
}

/*!
 * @brief Get the selection of plu input source
 */
static uint32_t DEMO_GetPluInputSource(void)
{
    uint32_t src = 0U;

    PRINTF("\r\nSelect the input source.\r\n");
    PRINTF("0. Input source 0\r\n");
    PRINTF("1. Input source 1\r\n");
    PRINTF("2. Input source 2\r\n");
    PRINTF("3. Set all three input sources.\r\n");

    src = DEMO_GetUserInput(3);

    return src;
}

/*!
 * @brief Set the plu input source with the input value
 */
static void SetPluInputSource(void)
{
    uint32_t src           = 0U;
    uint32_t value         = 0U;
    uint32_t gpioPluSrcPin = 0U;

    src   = DEMO_GetPluInputSource();
    value = DEMO_GetPluInputSourceValue(src);

    if (src != 3)
    {
        switch (src)
        {
            case 0:
                gpioPluSrcPin = DEMO_GPIO_PLU_SRC_0_PIN;
                break;
            case 1:
                gpioPluSrcPin = DEMO_GPIO_PLU_SRC_1_PIN;
                break;
            case 2:
                gpioPluSrcPin = DEMO_GPIO_PLU_SRC_2_PIN;
                break;
            default:
                assert(false);
                break;
        }
#if defined(DEMO_NO_HAS_GPIO_PORT_PARAM) && DEMO_NO_HAS_GPIO_PORT_PARAM
        GPIO_PinWrite(DEMO_GPIO_BASE, gpioPluSrcPin, value);
#else
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PORT, gpioPluSrcPin, value);
#endif
    }
    else
    {
#if defined(DEMO_NO_HAS_GPIO_PORT_PARAM) && DEMO_NO_HAS_GPIO_PORT_PARAM
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PLU_SRC_2_PIN, (value & 4U) >> 2U);
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PLU_SRC_1_PIN, (value & 2U) >> 1U);
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PLU_SRC_0_PIN, value & 1U);
#else
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PORT, DEMO_GPIO_PLU_SRC_2_PIN, (value & 4U) >> 2U);
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PORT, DEMO_GPIO_PLU_SRC_1_PIN, (value & 2U) >> 1U);
        GPIO_PinWrite(DEMO_GPIO_BASE, DEMO_GPIO_PORT, DEMO_GPIO_PLU_SRC_0_PIN, value & 1U);
#endif
    }
}

/*!
 * @brief Configure the PLU combinational logic network
 */
static void PLU_Configuration(void)
{
    /* Set all three input sources to LUT0. */
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_0, kPLU_LUT_IN_0, DEMO_PLU_LUT_IN_SRC_0);
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_0, kPLU_LUT_IN_1, DEMO_PLU_LUT_IN_SRC_1);
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_0, kPLU_LUT_IN_2, DEMO_PLU_LUT_IN_SRC_2);
    /* Set truthtable for LUTO. */
    PLU_SetLutTruthTable(DEMO_PLU_BASE, kPLU_LUT_0, DEMO_PLU_LUT_0_TRUTH_TABLE);
    /* Set LUT0 output to output source 0. */
    PLU_SetOutputSource(DEMO_PLU_BASE, kPLU_OUTPUT_0, DEMO_PLU_LUT_OUT_SRC_0);

    /* Set all three input sources to LUT1. */
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_1, kPLU_LUT_IN_0, DEMO_PLU_LUT_IN_SRC_0);
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_1, kPLU_LUT_IN_1, DEMO_PLU_LUT_IN_SRC_1);
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_1, kPLU_LUT_IN_2, DEMO_PLU_LUT_IN_SRC_2);
    /* Set truthtable for LUT1. */
    PLU_SetLutTruthTable(DEMO_PLU_BASE, kPLU_LUT_1, DEMO_PLU_LUT_1_TRUTH_TABLE);
    /* Set LUT1 output to output source 1. */
    PLU_SetOutputSource(DEMO_PLU_BASE, kPLU_OUTPUT_1, DEMO_PLU_LUT_OUT_SRC_1);

    /* Set all three input sources to LUT2. */
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_2, kPLU_LUT_IN_0, DEMO_PLU_LUT_IN_SRC_0);
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_2, kPLU_LUT_IN_1, DEMO_PLU_LUT_IN_SRC_1);
    PLU_SetLutInputSource(DEMO_PLU_BASE, kPLU_LUT_2, kPLU_LUT_IN_2, DEMO_PLU_LUT_IN_SRC_2);
    /* Set truthtable for LUT2. */
    PLU_SetLutTruthTable(DEMO_PLU_BASE, kPLU_LUT_2, DEMO_PLU_LUT_2_TRUTH_TABLE);
    /* Set LUT2 output to output source 2. */
    PLU_SetOutputSource(DEMO_PLU_BASE, kPLU_OUTPUT_2, DEMO_PLU_LUT_OUT_SRC_2);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Init plu module */
    PLU_Init(DEMO_PLU_BASE);

    /* Configure input, output, truthtable one time through the API */
    PLU_Configuration();

    /* Once the PLU module is configured, the PLU bus clock can be shut-off to conserve power */
    PLU_Deinit(DEMO_PLU_BASE);
    PRINTF("\r\nPLU combination driver example.\r\n");

    while (1)
    {
        /* Get and set the user's input to the PLU network. */
        SetPluInputSource();
    }
}
