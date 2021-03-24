/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "math.h"
#include "fsl_tpm.h"
#include "fsl_fxos.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer instance/channel used for board */
#define BOARD_TIMER_BASEADDR       TPM0
#define BOARD_FIRST_TIMER_CHANNEL  4U
#define BOARD_SECOND_TIMER_CHANNEL 5U
/* Get source clock for TPM driver */
#define BOARD_TIMER_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_McgIrc48MClk)
#define TIMER_CLOCK_MODE         1U
/* I2C source clock */
#define I2C_BAUDRATE 100000U

/* Accelerometer Reset PIN */
#define BOARD_ACCEL_RESET_GPIO GPIOE
#define BOARD_ACCEL_RESET_PIN  1U
/* Upper bound and lower bound angle values */
#define ANGLE_UPPER_BOUND 85U
#define ANGLE_LOWER_BOUND 5U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ACCEL_Reset(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* FXOS device address */
const uint8_t g_accel_address[] = {0x1CU, 0x1DU, 0x1EU, 0x1FU};

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ACCEL_Reset(void)
{
    uint32_t i;
    gpio_pin_config_t pin_config;

    /* Reset sensor by reset pin*/
    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic  = 1;
    GPIO_PinInit(BOARD_ACCEL_RESET_GPIO, BOARD_ACCEL_RESET_PIN, &pin_config);
    GPIO_PinWrite(BOARD_ACCEL_RESET_GPIO, BOARD_ACCEL_RESET_PIN, 1);
    /* Delay to ensure reliable sensor reset */
    for (i = 0; i < SystemCoreClock / 1000U; i++)
    {
        __NOP();
    }
    GPIO_PinWrite(BOARD_ACCEL_RESET_GPIO, BOARD_ACCEL_RESET_PIN, 0);

    /* Delay to wait sensor stable after reset */
    for (i = 0; i < SystemCoreClock / 1000U; i++)
    {
        __NOP();
    }
}
/* Initialize timer module */
static void Timer_Init(void)
{
    /* convert to match type of data */
    tpm_config_t tpmInfo;
    tpm_chnl_pwm_signal_param_t tpmParam[2];

    /* Configure tpm params with frequency 24kHZ */
    tpmParam[0].chnlNumber       = (tpm_chnl_t)BOARD_FIRST_TIMER_CHANNEL;
    tpmParam[0].level            = kTPM_LowTrue;
    tpmParam[0].dutyCyclePercent = 0U;

    tpmParam[1].chnlNumber       = (tpm_chnl_t)BOARD_SECOND_TIMER_CHANNEL;
    tpmParam[1].level            = kTPM_LowTrue;
    tpmParam[1].dutyCyclePercent = 0U;

    /*
     * tpmInfo.prescale = kTPM_Prescale_Divide_1;
     * tpmInfo.useGlobalTimeBase = false;
     * tpmInfo.enableDoze = false;
     * tpmInfo.enableDebugMode = false;
     * tpmInfo.enableReloadOnTrigger = false;
     * tpmInfo.enableStopOnOverflow = false;
     * tpmInfo.enableStartOnTrigger = false;
     * tpmInfo.enablePauseOnTrigger = false;
     * tpmInfo.triggerSelect = kTPM_Trigger_Select_0;
     * tpmInfo.triggerSource = kTPM_TriggerSource_External;
     */
    TPM_GetDefaultConfig(&tpmInfo);
    /* Initialize TPM module */
    TPM_Init(BOARD_TIMER_BASEADDR, &tpmInfo);

    CLOCK_SetTpmClock(1U);

    TPM_SetupPwm(BOARD_TIMER_BASEADDR, tpmParam, 2U, kTPM_EdgeAlignedPwm, 24000U, BOARD_TIMER_SOURCE_CLOCK);
    TPM_StartTimer(BOARD_TIMER_BASEADDR, kTPM_SystemClock);
}

/* Update the duty cycle of an active pwm signal */
static void Board_UpdatePwm(uint16_t x, uint16_t y)
{
    /* Updated duty cycle */
    TPM_UpdatePwmDutycycle(BOARD_TIMER_BASEADDR, (tpm_chnl_t)BOARD_FIRST_TIMER_CHANNEL, kTPM_EdgeAlignedPwm, x);
    TPM_UpdatePwmDutycycle(BOARD_TIMER_BASEADDR, (tpm_chnl_t)BOARD_SECOND_TIMER_CHANNEL, kTPM_EdgeAlignedPwm, y);
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
    int16_t xAngle          = 0;
    int16_t yAngle          = 0;
    int16_t xDuty           = 0;
    int16_t yDuty           = 0;
    uint8_t i               = 0;
    uint8_t array_addr_size = 0;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_ACCEL_Reset();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();

    /* I2C initialize */
    BOARD_Accel_I2C_Init();
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
        PRINTF("\r\nSensor device initialize failed!\r\n");
        return -1;
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
    /* Init timer */
    Timer_Init();

    /* Print a note to terminal */
    PRINTF("\r\nWelcome to the BUBBLE example\r\n");
    PRINTF("\r\nYou will see angle data change in the console when change the angles of board\r\n");

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
        xAngle = (int16_t)floor((double)xData * (double)dataScale * 90 / 8192);
        if (xAngle < 0)
        {
            xAngle *= -1;
        }
        yAngle = (int16_t)floor((double)yData * (double)dataScale * 90 / 8192);
        if (yAngle < 0)
        {
            yAngle *= -1;
        }

        xDuty = xAngle * 10 / 9;
        yDuty = yAngle * 10 / 9;

        /* Update duty cycle to turn on LEDs when angles ~ 90 */
        if (xAngle > ANGLE_UPPER_BOUND)
        {
            xDuty = 100;
        }
        if (yAngle > ANGLE_UPPER_BOUND)
        {
            yDuty = 100;
        }
        /* Update duty cycle to turn off LEDs when angles ~ 0 */
        if (xAngle < ANGLE_LOWER_BOUND)
        {
            xDuty = 0;
        }
        if (yAngle < ANGLE_LOWER_BOUND)
        {
            yDuty = 0;
        }

        Board_UpdatePwm(xDuty, yDuty);

        /* Print out the angle data. */
        PRINTF("x= %2d y = %2d\r\n", xAngle, yAngle);
    }
}
