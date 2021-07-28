/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_CLOCK_OUT_SELECT_ARRAY                                                                                   \
    {                                                                                                                \
        kOSC_CLK_to_CLKOUT, kLPOSC_to_CLKOUT, kFRO_DIV2_to_CLKOUT, kMAIN_CLK_to_CLKOUT, kDSP_MAIN_to_CLKOUT,         \
            kMAIN_PLL_to_CLKOUT, kAUX0_PLL_to_CLKOUT, kDSP_PLL_to_CLKOUT, kAUX1_PLL_to_CLKOUT, kAUDIO_PLL_to_CLKOUT, \
            kOSC32K_to_CLKOUT,                                                                                       \
    }
#define APP_CLOCK_OUT_NAME_ARRAY                                                                                       \
    {                                                                                                                  \
        "OSC_CLK Clock", "Low Power Oscillator Clock", "FRO_DIV2 Clock", "Main Clock", "Dsp Main Clock",               \
            "Main System PLL", "SYSPLL0 AUX0_PLL_Clock", "DSP PLL Clock", "SYSPLL0 AUX1_PLL_Clock", "AUDIO PLL Clock", \
            "32 KHz RTC Clock",                                                                                        \
    }


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static clock_attach_id_t APP_GetClockOutputSelection(void);
static uint8_t APP_GetClockOutDivider(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    clock_attach_id_t selectedAttachId;
    uint8_t divideValue;
    uint32_t dividedFreq;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Configure 32K OSC clock. */
    CLOCK_EnableOsc32K(true);               /* Enable 32KHz Oscillator clock */
    CLOCK_EnableClock(kCLOCK_Rtc);          /* Enable the RTC peripheral clock */
    RTC->CTRL &= ~RTC_CTRL_SWRESET_MASK;    /* Make sure the reset bit is cleared */
    RTC->CTRL &= ~RTC_CTRL_RTC_OSC_PD_MASK; /* The RTC Oscillator is powered up */

    CLOCK_InitSysPfd(kCLOCK_Pfd1, 24); /* Enable DSP PLL clock */
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 24); /* Enable AUX1 PLL clock */

    PRINTF("\r\nClock Output Driver Example.\r\n");

    while (1)
    {
        selectedAttachId = APP_GetClockOutputSelection();
        CLOCK_AttachClk(selectedAttachId);
        PRINTF("\r\nPlease set the divider of the output clock signal, range from 1 to 256.\r\n");
        divideValue = APP_GetClockOutDivider();
#if (defined(APP_CLK_DIV_SUPPORT_RESET) && APP_CLK_DIV_SUPPORT_RESET)
        CLOCK_SetClkDiv(kCLOCK_DivClkOut, divideValue, true);
#else
        CLOCK_SetClkDiv(kCLOCK_DivClockOut, divideValue);
#endif /* APP_CLK_DIV_SUPPORT_RESET */
        PRINTF("Please use oscilloscope to probe the output clock signal.\r\n");
        dividedFreq = CLOCK_GetClockOutClkFreq();
        PRINTF("\r\nTheoretically, the frequency of divided output clock is %ld Hz.\r\n", dividedFreq);
        PRINTF("\r\nTheoretically, the frequency of undivided output clock is %ld Hz.\r\n", dividedFreq * divideValue);
        PRINTF("Please press any key to continue.\r\n");
        GETCHAR();
    }
}

static clock_attach_id_t APP_GetClockOutputSelection(void)
{
    char *outputClockName[]           = APP_CLOCK_OUT_NAME_ARRAY;
    clock_attach_id_t outputClockId[] = APP_CLOCK_OUT_SELECT_ARRAY;
    uint8_t i                         = 0U;
    char ch;
    clock_attach_id_t selectedClockId = outputClockId[0];
    uint8_t clockIdIndex;

    PRINTF("Please choose one output clock.\r\n");

    for (i = 0U; i < ARRAY_SIZE(outputClockName); i++)
    {
        PRINTF("\t%c -- %s.\r\n", 'A' + i, outputClockName[i]);
    }

    ch = GETCHAR();
    PRINTF("%c\r\n", ch);

    if (ch >= 'a')
    {
        ch -= 'a' - 'A';
    }

    clockIdIndex = (uint8_t)(ch - 'A');
    if (clockIdIndex < ARRAY_SIZE(outputClockName))
    {
        selectedClockId = outputClockId[clockIdIndex];
    }

    return selectedClockId;
}

static uint8_t APP_GetClockOutDivider(void)
{
    uint8_t divider = 0U;
    char chDivider;

    do
    {
        chDivider = GETCHAR();
        if (chDivider == 0x0D)
        {
            break;
        }
        PRINTF("%c", chDivider);
        divider = divider * 10U + (uint8_t)(chDivider - '0');
    } while (1);
    PRINTF("\r\n");

    return divider;
}
