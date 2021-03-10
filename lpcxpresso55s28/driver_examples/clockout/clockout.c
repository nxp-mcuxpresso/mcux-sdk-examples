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

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_CLOCK_OUT_SELECT_ARRAY                                                                     \
    {                                                                                                  \
        kMAIN_CLK_to_CLKOUT, kPLL0_to_CLKOUT, kEXT_CLK_to_CLKOUT, kFRO_HF_to_CLKOUT, kFRO1M_to_CLKOUT, \
            kPLL1_to_CLKOUT, kOSC32K_to_CLKOUT,                                                        \
    }
#define APP_CLOCK_OUT_NAME_ARRAY                                                                        \
    {                                                                                                   \
        "Main Clock", "PLL0 Clock", "CLKIN Clock", "FRO 96 MHz Clock", "FRO 1 MHz Clock", "PLL1 clock", \
            "Oscillator 32 kHz clock",                                                                  \
    }
#define APP_CLK_DIV_SUPPORT_RESET 1

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
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK;
    ANACTRL->XO32M_CTRL |= ANACTRL_XO32M_CTRL_ENABLE_SYSTEM_CLK_OUT_MASK;

    /*!< Configure RTC OSC */
    POWER_DisablePD(kPDRUNCFG_PD_XTAL32K); /*!< Powered the XTAL 32 kHz RTC oscillator */
    POWER_EnablePD(kPDRUNCFG_PD_FRO32K);   /*!< Powered down the FRO 32 kHz RTC oscillator */
    CLOCK_AttachClk(kXTAL32K_to_OSC32K);   /*!< Switch OSC32K to XTAL32K */
    CLOCK_EnableClock(kCLOCK_Rtc);         /*!< Enable the RTC peripheral clock */
    RTC->CTRL &= ~RTC_CTRL_SWRESET_MASK;   /*!< Make sure the reset bit is cleared */

    /*!< Set up PLL1 */
    CLOCK_AttachClk(kEXT_CLK_to_PLL1);  /*!< Switch PLL1CLKSEL to EXT_CLK */
    POWER_DisablePD(kPDRUNCFG_PD_PLL1); /* Ensure PLL is on  */
    const pll_setup_t pll1Setup = {
        .pllctrl = SYSCON_PLL1CTRL_CLKEN_MASK | SYSCON_PLL1CTRL_SELI(19U) | SYSCON_PLL1CTRL_SELP(9U),
        .pllndec = SYSCON_PLL1NDEC_NDIV(1U),
        .pllpdec = SYSCON_PLL1PDEC_PDIV(2U),
        .pllmdec = SYSCON_PLL1MDEC_MDIV(32U),
        .pllRate = 128000000U,
        .flags   = PLL_SETUPFLAG_WAITLOCK};
    CLOCK_SetPLL1Freq(&pll1Setup);

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
