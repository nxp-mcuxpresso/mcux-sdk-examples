/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "math.h"
#include "fsl_qtmr.h"
#include "fsl_fxos.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_DELAY_COUNT 8000000

/* Upper bound and lower bound angle values */
#define ANGLE_UPPER_BOUND 85U
#define ANGLE_LOWER_BOUND 5U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Board_UpdatePwm(uint16_t x);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile int16_t g_xAngle = 0;
volatile int16_t g_yAngle = 0;
volatile int16_t g_xDuty  = 0;
volatile int16_t g_yDuty  = 0;
/* FXOS device address */
const uint8_t g_accel_address[] = {0x1CU, 0x1DU, 0x1EU, 0x1FU};

/*******************************************************************************
 * Code
 ******************************************************************************/
void TIMER_IRQ_HANDLER(void)
{
    uint32_t status = QTMR_GetStatus(TIMER_PERIPHERAL, TIMER_CHANNEL_0_CHANNEL);

    if (status & kQTMR_Compare1Flag)
    {
        USER_LED_ON();
        QTMR_ClearStatusFlags(TIMER_PERIPHERAL, TIMER_CHANNEL_0_CHANNEL, kQTMR_Compare1Flag);
    }
    if (status & kQTMR_Compare2Flag)
    {
        USER_LED_OFF();
        Board_UpdatePwm(g_xDuty);
        QTMR_ClearStatusFlags(TIMER_PERIPHERAL, TIMER_CHANNEL_0_CHANNEL, kQTMR_Compare2Flag);
    }
    __DSB();
}

void UpdatePwmDutycycle(TMR_Type *base, qtmr_channel_selection_t channel, uint32_t pwmFreqHz, uint8_t dutyCyclePercent)
{
    QTMR_StopTimer(base, channel);
    QTMR_SetupPwm(base, channel, pwmFreqHz, dutyCyclePercent, false, TIMER_CHANNEL_0_CLOCK_SOURCE);
    QTMR_StartTimer(base, channel, kQTMR_PriSrcRiseEdge);
}

/* Update the duty cycle of an active pwm signal */
static void Board_UpdatePwm(uint16_t x)
{
    UpdatePwmDutycycle(TIMER_PERIPHERAL, (qtmr_channel_selection_t)TIMER_CHANNEL_0_CHANNEL, 50000, x);
}

void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < EXAMPLE_DELAY_COUNT; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

int main(void)
{
    fxos_handle_t fxosHandle = {0};
    fxos_data_t sensorData   = {0};
    fxos_config_t config     = {0};
    status_t result;
    uint8_t sensorRange     = 0;
    uint8_t dataScale       = 0;
    int16_t xData           = 0;
    int16_t yData           = 0;
    uint8_t i               = 0;
    uint8_t array_addr_size = 0;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();
    BOARD_InitPeripherals();

    /* Configure the I2C function */
    config.I2C_SendFunc    = BOARD_Accel_I2C_Send;
    config.I2C_ReceiveFunc = BOARD_Accel_I2C_Receive;

    /* Initialize sensor devices */
    array_addr_size = sizeof(g_accel_address) / sizeof(g_accel_address[0]);
    for (i = 0; i < array_addr_size; i++)
    {
        config.slaveAddress = g_accel_address[i];
        /* Initialize accelerometer sensor */
        result = FXOS_Init(&fxosHandle, &config);
        if (result == kStatus_Success)
        {
            break;
        }
    }

    if (result != kStatus_Success)
    {
        QTMR_StopTimer(TIMER_PERIPHERAL, TIMER_CHANNEL_0_CHANNEL);
        PRINTF("\r\nSensor device initialize failed!\r\n");
        PRINTF("\r\nPlease check the sensor chip U32\r\n");
        while (1)
        {
            delay();
            GPIO_PortToggle(BOARD_USER_LED_GPIO, 1u << BOARD_USER_LED_GPIO_PIN);
        }
    }

    /* Get sensor range */
    if (FXOS_ReadReg(&fxosHandle, XYZ_DATA_CFG_REG, &sensorRange, 1) != kStatus_Success)
    {
        return -1;
    }

    if (sensorRange == 0x00)
    {
        dataScale = 2U;
    }
    else if (sensorRange == 0x01)
    {
        dataScale = 4U;
    }
    else if (sensorRange == 0x10)
    {
        dataScale = 8U;
    }
    else
    {
    }

    /* Print a note to terminal */
    PRINTF("\r\nWelcome to the BUBBLE example\r\n");
    PRINTF("\r\nYou will see the LED brightness change when the angle of the board changes\r\n");

    /* Main loop. Get sensor data and update duty cycle */
    while (1)
    {
        /* Get new accelerometer data. */
        if (FXOS_ReadSensorData(&fxosHandle, &sensorData) != kStatus_Success)
        {
            return -1;
        }

        /* Get the X and Y data from the sensor data structure in 14 bit left format data*/
        xData = (int16_t)((uint16_t)((uint16_t)sensorData.accelXMSB << 8) | (uint16_t)sensorData.accelXLSB) / 4U;
        yData = (int16_t)((uint16_t)((uint16_t)sensorData.accelYMSB << 8) | (uint16_t)sensorData.accelYLSB) / 4U;

        /* Convert raw data to angle (normalize to 0-90 degrees). No negative angles. */
        g_xAngle = (int16_t)floor((double)xData * (double)dataScale * 90 / 8192);
        if (g_xAngle < 0)
        {
            g_xAngle *= -1;
        }
        g_yAngle = (int16_t)floor((double)yData * (double)dataScale * 90 / 8192);
        if (g_yAngle < 0)
        {
            g_yAngle *= -1;
        }
        g_xDuty = g_xAngle * 100 / 90;
        g_yDuty = g_yAngle * 100 / 90;
        /* Update duty cycle to turn on LEDs when angles ~ 90 */
        if (g_xAngle > ANGLE_UPPER_BOUND)
        {
            g_xDuty = 100;
        }
        if (g_yAngle > ANGLE_UPPER_BOUND)
        {
            g_yDuty = 100;
        }
        /* Update duty cycle to turn off LEDs when angles ~ 0 */
        if (g_xAngle < ANGLE_LOWER_BOUND)
        {
            g_xDuty = 0;
        }
        if (g_yAngle < ANGLE_LOWER_BOUND)
        {
            g_yDuty = 0;
        }
        /* Print out the angle data. */
        PRINTF("x= %2d y = %2d\r\n", g_xAngle, g_yAngle);
    }
}
