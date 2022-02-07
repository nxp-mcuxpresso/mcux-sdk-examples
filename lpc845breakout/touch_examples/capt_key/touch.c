/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include "touch.h"
#include "app_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
int16_t touchWindowsBuff[TOUCH_X_CHANNEL_COUNT][TOUCH_WINDOW_LENGTH];
uint8_t touchWindowsIndex;

/* Baseline after calibration. */
int32_t touchDataBaseline[TOUCH_X_CHANNEL_COUNT];
/* Touch data removed baseline. */
int32_t touchData[TOUCH_X_CHANNEL_COUNT];

/*******************************************************************************
 * Code
 ******************************************************************************/

void TOUCH_WINDOWS_Init(void)
{
    touchWindowsIndex = 0u;
}

/*
 * Push the raw data into the calculation window array,
 * the window array is actually FIFO.
 */
void TOUCH_WINDOWS_Puth(int16_t *input)
{
    for (uint8_t i = 0u; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
        touchWindowsBuff[i][touchWindowsIndex] = input[i];
    }
    touchWindowsIndex = (touchWindowsIndex + 1u) % TOUCH_WINDOW_LENGTH;
}

/*
 * Calculate the sum of one channel's sample data in the window.
 * Input is one channel's sample data in the window.
 * Return the sum.
 */
static int32_t TOUCH_WINDOW_CalcSum(int16_t *input)
{
    int32_t sum = 0;

    for (uint8_t i = 0u; i < TOUCH_WINDOW_LENGTH; i++)
    {
        sum += input[i];
    }

    return sum;
}

/* Calculate the sum of each channel's sample data separately. */
void TOUCH_WINDOWS_CalcSum(int32_t *output)
{
    for (uint8_t i = 0u; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
        output[i] = TOUCH_WINDOW_CalcSum(touchWindowsBuff[i]);
    }
}

/*
 * Calculate the average value of one channel's sample data in the window.
 * Input is one channel's sample data in the window.
 * Return the average value.
 */
static int16_t TOUCH_WINDOW_CalcAverage(int16_t *input)
{
    return (int16_t)(TOUCH_WINDOW_CalcSum(input) / TOUCH_WINDOW_LENGTH);
}

/* Calculate the average value of each channel's sample data separately. */
void TOUCH_WINDOWS_CalcAverage(int32_t *output)
{
    for (uint8_t i = 0u; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
        output[i] = TOUCH_WINDOW_CalcAverage(touchWindowsBuff[i]);
    }
}

/*
 * Calculate the variance of one channel's sample data in the window.
 * Input is one channel's average value and sample data in the window.
 * Return the variance value.
 */
static int32_t TOUCH_WINDOW_CalcVariance(int16_t average, int16_t *input)
{
    int32_t sum2 = 0;
    int32_t diff;

    for (uint8_t i = 0u; i < TOUCH_WINDOW_LENGTH; i++)
    {
        diff = input[i] - average;
        sum2 += (diff * diff);
    }

    return sum2 / TOUCH_WINDOW_LENGTH;
}

/* Calculate the variance value of each channel's sample data separately. */
void TOUCH_WINDOWS_CalcVariance(int32_t *average, int32_t *output)
{
    for (uint8_t i = 0u; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
        output[i] = TOUCH_WINDOW_CalcVariance(average[i], touchWindowsBuff[i]);
    }
}

void TOUCH_WINDOWS_SetBaseline(int32_t *input)
{
    for (uint8_t i = 0u; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
        touchDataBaseline[i] = input[i];
    }
}

int32_t TOUCH_GetPressedKeyIndex(int32_t *input)
{
    for (int i = 0; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
#if TOUCH_PRESSED_KEY_COUNT_DECREASE
        touchData[i] = touchDataBaseline[i] - input[i];
#else
        touchData[i] = input[i] - touchDataBaseline[i];
#endif
    }

#if (TOUCH_X_CHANNEL_COUNT > 1)
    uint8_t touchValMaxIdx = 0;
    uint8_t touchValMinIdx = 0;

    for (int i = 1; i < TOUCH_X_CHANNEL_COUNT; i++)
    {
        if (touchData[i] > touchData[touchValMaxIdx])
        {
            touchValMaxIdx = i;
        }

        if (touchData[i] < touchData[touchValMinIdx])
        {
            touchValMinIdx = i;
        }
    }

    /* No significant difference, no touch happend. */
    if ((touchData[touchValMaxIdx] - touchData[touchValMinIdx]) < TOUCH_YES_TOUCH_THRESHOLD_LOW)
    {
        return TOUCH_X_CHANNEL_COUNT;
    }
    else
    {
        /* Touch happened. */
        return touchValMaxIdx;
    }
#else
    if (touchData[0] < TOUCH_YES_TOUCH_THRESHOLD_LOW)
    {
        /* Touch not happened. */
        return TOUCH_X_CHANNEL_COUNT;
    }
    else
    {
        /* Touch happened. */
        return 0;
    }
#endif
}
