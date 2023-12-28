/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "math.h"
#include "fsl_qtmr.h"
#include "fsl_fxls.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

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
/* FXLS device address */
const uint8_t g_accel_address = 0x19U;

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
        Board_UpdatePwm(g_yDuty);
        QTMR_ClearStatusFlags(TIMER_PERIPHERAL, TIMER_CHANNEL_0_CHANNEL, kQTMR_Compare2Flag);
    }
    SDK_ISR_EXIT_BARRIER;
}

void UpdatePwmDutycycle(TMR_Type *base, qtmr_channel_selection_t channel, uint32_t pwmFreqHz, uint8_t dutyCyclePercent)
{
    QTMR_StopTimer(base, channel);
    QTMR_SetupPwm(base, channel, pwmFreqHz, dutyCyclePercent, false, TIMER_CHANNEL_0_CLOCK_SOURCE);
    if ((base->CHANNEL[channel].CTRL & TMR_CTRL_OUTMODE_MASK) != TMR_CTRL_OUTMODE(kQTMR_ToggleOnAltCompareReg))
    {
        base->CHANNEL[channel].CTRL &= ~(uint16_t)TMR_CTRL_OUTMODE_MASK;
        base->CHANNEL[channel].CTRL |= (TMR_CTRL_LENGTH_MASK | TMR_CTRL_OUTMODE(kQTMR_ToggleOnAltCompareReg));
    }
    QTMR_StartTimer(base, channel, kQTMR_PriSrcRiseEdge);
}

/* Update the duty cycle of an active pwm signal */
static void Board_UpdatePwm(uint16_t x)
{
    UpdatePwmDutycycle(TIMER_PERIPHERAL, (qtmr_channel_selection_t)TIMER_CHANNEL_0_CHANNEL, 500, x);
}

int main(void)
{
    fxls_handle_t fxlsHandle    = {0};
    fxls_accel_data_t accelData = {0};
    fxls_config_t config        = {0};
    status_t result;
    uint8_t sensorRange = 0;
    uint8_t dataScale   = 0;
    uint32_t i          = 0;

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
    config.slaveAddress    = g_accel_address;

    /* Initialize accelerometer sensor */
    result = FXLS_Init(&fxlsHandle, &config);

    if (result != kStatus_Success)
    {
        QTMR_StopTimer(TIMER_PERIPHERAL, TIMER_CHANNEL_0_CHANNEL);
        PRINTF("\r\nSensor device initialize failed!\r\n");
        PRINTF("\r\nPlease check the sensor chip U115\r\n");
        while (1)
        {
            for (i = 0; i < 30000000; i++)
            {
                __NOP();
            }
            GPIO_PortToggle(BOARD_USER_LED_GPIO, 1U << BOARD_USER_LED_GPIO_PIN);
        }
    }

    /* Get sensor range */
    if (FXLS_ReadReg(&fxlsHandle, SENS_CONFIG1_REG, &sensorRange, 1) != kStatus_Success)
    {
        return -1;
    }

    sensorRange = (sensorRange & 0x6) >> 1;

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
    else if (sensorRange == 0x11)
    {
        dataScale = 16U;
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
        if (FXLS_ReadAccelData(&fxlsHandle, &accelData) != kStatus_Success)
        {
            return -1;
        }

        /* Convert raw data to angle (normalize to 0-90 degrees). No negative angles. */
        g_xAngle = (int16_t)floor((double)accelData.accelX * (double)dataScale * 90 / 2048);
        if (g_xAngle < 0)
        {
            g_xAngle *= -1;
        }
        g_yAngle = (int16_t)floor((double)accelData.accelY * (double)dataScale * 90 / 2048);
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
