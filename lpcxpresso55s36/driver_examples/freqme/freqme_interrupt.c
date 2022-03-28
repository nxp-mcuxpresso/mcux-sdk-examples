/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_freqme.h"
#include "fsl_inputmux.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CLOCK_SOURCE_COUNT 4U
#define DEMO_CLOCK_SOURCE_NAME                                               \
    {                                                                        \
        "XTAL32MHz", "FRO_OSC_12M", "FREQME_GPIO_CLK_A", "FREQME_GPIO_CLK_B" \
    }

#define DEMO_REFERENCE_CLOCK_SOURCE_SIGNAL                                                                           \
    {                                                                                                                \
        kINPUTMUX_Xtal32MhzToFreqmeasRef, kINPUTMUX_FroOsc12MhzToFreqmeasRef, kINPUTMUX_FreqmeGpioClkAToFreqmeasRef, \
            kINPUTMUX_FreqmeGpioClkBToFreqmeasRef                                                                    \
    }

#define DEMO_TARGET_CLOCK_SOURCE_SIGNAL                                                        \
    {                                                                                          \
        kINPUTMUX_Xtal32MhzToFreqmeasTarget, kINPUTMUX_FroOsc12MhzToFreqmeasTarget,            \
            kINPUTMUX_FreqmeGpioClkAToFreqmeasTarget, kINPUTMUX_FreqmeGpioClkBToFreqmeasTarget \
    }

#define DEMO_REF_CLK_SOURCE    kINPUTMUX_FroOsc96MhzToFreqmeasRef
#define DEMO_TARGET_CLK_SOURCE kINPUTMUX_FroOsc96MhzToFreqmeasTarget
#define DEMO_FREQME            FREQME
#define FREQME_IRQHANDLER      Freqme_IRQHandler
#define DEMO_MAXEXPECTVALUE    (0x6FFFFFFFUL)
#define DEMO_MINEXPECTVALUE    (0xFUL)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* Measure target clock's frequency. */
static void DEMO_DoFreqMeasurement(void);
/* Measure reference clock's pulse width. */
static void DEMO_DoPulseWidthMeasurement(void);
/* Select clock source. */
static void DEMO_GetClockSelection(freq_measure_config_t *config);
/* Get pulse polarity from user input. */
static void DEMO_GetPulsePolarity(freq_measure_config_t *config);
/* Get reference clock scale factor from user input. */
static void DEMO_GetReferenceClockScaleFactor(freq_measure_config_t *config);
/*******************************************************************************
 * Variables
 ******************************************************************************/

static const char *g_clockNameArray[DEMO_CLOCK_SOURCE_COUNT] = DEMO_CLOCK_SOURCE_NAME;
static const inputmux_connection_t g_referenceClockSourceSignalArray[DEMO_CLOCK_SOURCE_COUNT] =
    DEMO_REFERENCE_CLOCK_SOURCE_SIGNAL;
static const inputmux_connection_t g_targetClockSourceSignalArray[DEMO_CLOCK_SOURCE_COUNT] =
    DEMO_TARGET_CLOCK_SOURCE_SIGNAL;
volatile bool g_measurementCompleted = false;
volatile bool g_errorOccurred        = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

void FREQME_IRQHANDLER(void)
{
    if (FREQME_GetInterruptStatusFlags(DEMO_FREQME) &
        (kFREQME_UnderflowInterruptStatusFlag | kFREQME_OverflowInterruptStatusFlag))
    {
        FREQME_ClearInterruptStatusFlags(DEMO_FREQME,
                                         (kFREQME_UnderflowInterruptStatusFlag | kFREQME_OverflowInterruptStatusFlag));
        g_errorOccurred        = true;
        g_measurementCompleted = true;
        return;
    }
    if ((FREQME_GetInterruptStatusFlags(DEMO_FREQME) & kFREQME_ReadyInterruptStatusFlag) != 0UL)
    {
        FREQME_ClearInterruptStatusFlags(DEMO_FREQME, kFREQME_ReadyInterruptStatusFlag);
        g_measurementCompleted = true;
    }
}

int main(void)
{
    char inputCh;

    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Freqme);

    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO12MHZ_FREQM_ENA_MASK | SYSCON_CLOCK_CTRL_FRO_HF_FREQM_ENA_MASK |
                          SYSCON_CLOCK_CTRL_XTAL32MHZ_FREQM_ENA_MASK | SYSCON_CLOCK_CTRL_FRO1MHZ_UTICK_ENA_MASK |
                          SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK;
    INPUTMUX_Init(INPUTMUX);
    PRINTF("FREQME Interrupt Example!\r\n");

    EnableIRQ(Freqme_IRQn);
    while (1)
    {
        PRINTF("Please select operate mode...\r\n");
        PRINTF("\tA -- Frequency Measurement Mode.\r\n");
        PRINTF("\tB -- Pulse Width Measurement Mode.\r\n");

        inputCh = GETCHAR();

        if ((inputCh >= 'a') && (inputCh <= 'z'))
        {
            inputCh -= 'a' - 'A';
        }

        switch (inputCh)
        {
            case 'A':
            {
                RESET_PeripheralReset(kFREQME_RST_SHIFT_RSTn);
                FREQME_SetMaxExpectedValue(DEMO_FREQME, DEMO_MAXEXPECTVALUE);
                FREQME_SetMinExpectedValue(DEMO_FREQME, DEMO_MINEXPECTVALUE);
                DEMO_DoFreqMeasurement();
                break;
            }
            case 'B':
            {
                RESET_PeripheralReset(kFREQME_RST_SHIFT_RSTn);
                DEMO_DoPulseWidthMeasurement();
                break;
            }
            default:
            {
                PRINTF("Wrong input, please retry!\r\n");
                break;
            }
        }
    }
}

static void DEMO_DoFreqMeasurement(void)
{
    freq_measure_config_t config;
    uint32_t targetFreq = 0UL;

    PRINTF("Frequency Measurement Mode Selected!\r\n");

    /*
     * config->operateMode = kFREQME_FreqMeasurementMode;
     * config->operateModeAttribute.refClkScaleFactor = 0U;
     * config->enableContinuousMode                   = false;
     * config->startMeasurement                       = false;
     */

    FREQME_GetDefaultConfig(&config);
    INPUTMUX_AttachSignal(INPUTMUX, 0UL, DEMO_REF_CLK_SOURCE);
    DEMO_GetClockSelection(&config);
    DEMO_GetReferenceClockScaleFactor(&config);
    FREQME_Init(DEMO_FREQME, &config);

    FREQME_EnableInterrupts(
        DEMO_FREQME, kFREQME_ReadyInterruptEnable | kFREQME_UnderflowInterruptEnable | kFREQME_OverflowInterruptEnable);
    FREQME_StartMeasurementCycle(DEMO_FREQME);
    while (!g_measurementCompleted)
    {
    }
    g_measurementCompleted = false;
    if (g_errorOccurred)
    {
        PRINTF("Error Occurred! please retry by changing reference clock scaling factor.\r\n");
        g_errorOccurred = false;
    }
    else
    {
        targetFreq = FREQME_CalculateTargetClkFreq(DEMO_FREQME, CLOCK_GetFreq(kCLOCK_FroHf));
        PRINTF("Target clock frequency is %d Hz.\r\n", targetFreq);
    }
}

static void DEMO_DoPulseWidthMeasurement(void)
{
    freq_measure_config_t config;
    PRINTF("Pulse Width Measurement Mode.\r\n");
    uint32_t pulseWidth;

    /*
     * config->operateMode = kFREQME_FreqMeasurementMode;
     * config->operateModeAttribute.refClkScaleFactor = 0U;
     * config->enableContinuousMode                   = false;
     */
    FREQME_GetDefaultConfig(&config);
    INPUTMUX_AttachSignal(INPUTMUX, 0U, DEMO_TARGET_CLK_SOURCE);
    config.operateMode = kFREOME_PulseWidthMeasurementMode;
    DEMO_GetClockSelection(&config);
    DEMO_GetPulsePolarity(&config);
    FREQME_Init(DEMO_FREQME, &config);
    FREQME_EnableInterrupts(
        DEMO_FREQME, kFREQME_ReadyInterruptEnable | kFREQME_UnderflowInterruptEnable | kFREQME_OverflowInterruptEnable);
    FREQME_StartMeasurementCycle(DEMO_FREQME);
    while (!g_measurementCompleted)
    {
    }
    g_measurementCompleted = false;
    if (g_errorOccurred)
    {
        PRINTF("Error Occurred! please retry by changing reference clock scaling factor.\r\n");
        g_errorOccurred = false;
    }
    else
    {
        pulseWidth = FREQME_GetMeasurementResult(DEMO_FREQME);
        PRINTF("Pulse width count is %d.\r\n", pulseWidth);
    }
}

static void DEMO_GetClockSelection(freq_measure_config_t *config)
{
    uint8_t i;
    char chInput;

    while (1)
    {
        if (config->operateMode == kFREQME_FreqMeasurementMode)
        {
            /* If operate mode is selected as Frequency measurment mode, the target clock is selectable. */
            PRINTF("Please select the target clock:\r\n");
            for (i = 0U; i < DEMO_CLOCK_SOURCE_COUNT; i++)
            {
                PRINTF("\t\t%c -- %s\r\n", 'A' + i, g_clockNameArray[i]);
            }
            chInput = GETCHAR();
            if ((chInput >= 'a') && (chInput <= 'z'))
            {
                chInput -= 'a' - 'A';
            }
            chInput -= 'A';
            if (chInput < DEMO_CLOCK_SOURCE_COUNT)
            {
                INPUTMUX_AttachSignal(INPUTMUX, 0UL, g_targetClockSourceSignalArray[(uint8_t)chInput]);
                break;
            }
            else
            {
                PRINTF("Wrong Input Please Retry!\r\n");
                continue;
            }
        }
        else
        {
            /* In pulse width measurment mode, the reference clock is selectable. */
            PRINTF("Please select the reference clock:\r\n");
            for (i = 0U; i < DEMO_CLOCK_SOURCE_COUNT; i++)
            {
                PRINTF("\t\t%c -- %s\r\n", 'A' + i, g_clockNameArray[i]);
            }
            chInput = GETCHAR();
            if ((chInput >= 'a') && (chInput <= 'z'))
            {
                chInput -= 'a' - 'A';
            }
            chInput -= 'A';
            if (chInput < DEMO_CLOCK_SOURCE_COUNT)
            {
                INPUTMUX_AttachSignal(INPUTMUX, 0UL, g_referenceClockSourceSignalArray[(uint8_t)chInput]);
                break;
            }
            else
            {
                PRINTF("Wrong Input Please Retry!\r\n");
                continue;
            }
        }
    }
}

static void DEMO_GetReferenceClockScaleFactor(freq_measure_config_t *config)
{
    char chInput;
    uint8_t scaleFactor;
    for (;;)
    {
        scaleFactor = 0UL;
        PRINTF("Please input the scale factor of reference clock(Ranges from 0 to 31).\r\n");
        while (1)
        {
            chInput = GETCHAR();
            PUTCHAR(chInput);
            if (chInput == 0x0D)
            {
                break;
            }
            chInput -= '0';
            scaleFactor = scaleFactor * 10U + chInput;
        }
        PRINTF("\r\n");
        if (scaleFactor > 31U)
        {
            PRINTF("Wrong Scale Factor Input, Please Retry!\r\n");
            continue;
        }
        else
        {
            config->operateModeAttribute.refClkScaleFactor = scaleFactor;
            break;
        }
    }
}

static void DEMO_GetPulsePolarity(freq_measure_config_t *config)
{
    char chInput;

    for (;;)
    {
        PRINTF("Please select pulse polarity.\r\n");
        PRINTF("\t\tA -- High Period\r\n");
        PRINTF("\t\tB -- Low Period\r\n");
        chInput = GETCHAR();

        if ((chInput >= 'a') && (chInput <= 'z'))
        {
            chInput -= 'a' - 'A';
        }
        if (chInput > 'B')
        {
            PRINTF("Wrong Pulse Polarity Input, Please Retry!\r\n");
            continue;
        }
        else
        {
            config->operateModeAttribute.pulsePolarity = (freqme_pulse_polarity_t)(chInput - 'A');
            break;
        }
    }
}
