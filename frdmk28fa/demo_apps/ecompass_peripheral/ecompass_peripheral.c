/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_fxos.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
#include "math.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define I2C_RELEASE_BUS_COUNT 100U
#define MAX_ACCEL_AVG_COUNT 25U
#define HWTIMER_PERIOD      10000U
/* multiplicative conversion constants */
#define DegToRad 0.017453292
#define RadToDeg 57.295779
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);

/*!
 * @brief Read all data from sensor function
 *
 * @param Ax The pointer store x axis acceleration value
 * @param Ay The pointer store y axis acceleration value
 * @param Az The pointer store z axis acceleration value
 * @param Mx The pointer store x axis magnetic value
 * @param My The pointer store y axis magnetic value
 * @param Mz The pointer store z axis magnetic value
 * @note Must calculate g_dataScale before use this function.
 */
static void Sensor_ReadData(int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz);

/*!
 * @brief Magnetometer calibration
 *
 */
static void Magnetometer_Calibrate(void);

/*!
 * @brief Hardware timer initialize
 *
 */
static void HW_Timer_init(void);

/*!
 * @brief Delay function
 *
 * @param ticks Cycle clock delay
 */
static void Delay(uint32_t ticks);
/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint16_t SampleEventFlag;
fxos_handle_t g_fxosHandle;
uint8_t g_sensor_address[] = {0x1CU, 0x1EU, 0x1DU, 0x1FU};
uint8_t g_sensorRange      = 0;
uint8_t g_dataScale        = 0;

int16_t g_Ax_Raw = 0;
int16_t g_Ay_Raw = 0;
int16_t g_Az_Raw = 0;

double g_Ax = 0;
double g_Ay = 0;
double g_Az = 0;

int16_t g_Ax_buff[MAX_ACCEL_AVG_COUNT] = {0};
int16_t g_Ay_buff[MAX_ACCEL_AVG_COUNT] = {0};
int16_t g_Az_buff[MAX_ACCEL_AVG_COUNT] = {0};

int16_t g_Mx_Raw = 0;
int16_t g_My_Raw = 0;
int16_t g_Mz_Raw = 0;

int16_t g_Mx_Offset = 0;
int16_t g_My_Offset = 0;
int16_t g_Mz_Offset = 0;

double g_Mx = 0;
double g_My = 0;
double g_Mz = 0;

double g_Mx_LP = 0;
double g_My_LP = 0;
double g_Mz_LP = 0;

double g_Yaw    = 0;
double g_Yaw_LP = 0;
double g_Pitch  = 0;
double g_Roll   = 0;

bool g_FirstRun = true;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;

    BOARD_GPIO_ConfigurePins();

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(BOARD_GPIO_ACCEL_SDA_GPIO, BOARD_GPIO_ACCEL_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(BOARD_GPIO_ACCEL_SCL_GPIO, BOARD_GPIO_ACCEL_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(BOARD_GPIO_ACCEL_SDA_GPIO, BOARD_GPIO_ACCEL_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(BOARD_GPIO_ACCEL_SCL_GPIO, BOARD_GPIO_ACCEL_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(BOARD_GPIO_ACCEL_SCL_GPIO, BOARD_GPIO_ACCEL_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(BOARD_GPIO_ACCEL_SDA_GPIO, BOARD_GPIO_ACCEL_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(BOARD_GPIO_ACCEL_SCL_GPIO, BOARD_GPIO_ACCEL_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(BOARD_GPIO_ACCEL_SDA_GPIO, BOARD_GPIO_ACCEL_SDA_PIN, 1U);
    i2c_release_bus_delay();
}
static void HW_Timer_init(void)
{
    /* Configure the SysTick timer */
    SysTick_Config(SystemCoreClock / HWTIMER_PERIOD);
}

void SysTick_Handler(void)
{
    SampleEventFlag = 1;
}

static void Delay(uint32_t ticks)
{
    while (ticks--)
    {
        __asm("nop");
    }
}

static void Sensor_ReadData(int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz)
{
    fxos_data_t fxos_data;

    if (kStatus_Success != FXOS_ReadSensorData(&g_fxosHandle, &fxos_data))
    {
        PRINTF("Failed to read acceleration data!\r\n");
    }
    /* Get the accel data from the sensor data structure in 14 bit left format data*/
    *Ax = (int16_t)((uint16_t)((uint16_t)fxos_data.accelXMSB << 8) | (uint16_t)fxos_data.accelXLSB) / 4U;
    *Ay = (int16_t)((uint16_t)((uint16_t)fxos_data.accelYMSB << 8) | (uint16_t)fxos_data.accelYLSB) / 4U;
    *Az = (int16_t)((uint16_t)((uint16_t)fxos_data.accelZMSB << 8) | (uint16_t)fxos_data.accelZLSB) / 4U;
    *Ax *= g_dataScale;
    *Ay *= g_dataScale;
    *Az *= g_dataScale;
    *Mx = (int16_t)((uint16_t)((uint16_t)fxos_data.magXMSB << 8) | (uint16_t)fxos_data.magXLSB);
    *My = (int16_t)((uint16_t)((uint16_t)fxos_data.magYMSB << 8) | (uint16_t)fxos_data.magYLSB);
    *Mz = (int16_t)((uint16_t)((uint16_t)fxos_data.magZMSB << 8) | (uint16_t)fxos_data.magZLSB);
}

static void Magnetometer_Calibrate(void)
{
    int16_t Mx_max = 0;
    int16_t My_max = 0;
    int16_t Mz_max = 0;
    int16_t Mx_min = 0;
    int16_t My_min = 0;
    int16_t Mz_min = 0;

    uint32_t times = 0;
    PRINTF("\r\nCalibrating magnetometer...");
    while (times < 5000)
    {
        Sensor_ReadData(&g_Ax_Raw, &g_Ay_Raw, &g_Az_Raw, &g_Mx_Raw, &g_My_Raw, &g_Mz_Raw);
        if (times == 0)
        {
            Mx_max = Mx_min = g_Mx_Raw;
            My_max = My_min = g_My_Raw;
            Mz_max = Mz_min = g_Mz_Raw;
        }
        Mx_max = MAX(Mx_max, g_Mx_Raw);
        My_max = MAX(My_max, g_My_Raw);
        Mz_max = MAX(Mz_max, g_Mz_Raw);
        Mx_min = MIN(Mx_min, g_Mx_Raw);
        My_min = MIN(My_min, g_My_Raw);
        Mz_min = MIN(Mz_min, g_Mz_Raw);
        if (times == 4999)
        {
            if ((Mx_max > (Mx_min + 500)) && (My_max > (My_min + 500)) && (Mz_max > (Mz_min + 500)))
            {
                g_Mx_Offset = (Mx_max + Mx_min) / 2;
                g_My_Offset = (My_max + My_min) / 2;
                g_Mz_Offset = (Mz_max + Mz_min) / 2;
                PRINTF("\r\nCalibrate magnetometer successfully!");
                PRINTF("\r\nMagnetometer offset Mx: %d - My: %d - Mz: %d\r\n", g_Mx_Offset, g_My_Offset, g_Mz_Offset);
            }
            else
            {
                PRINTF("Calibrating magnetometer failed! Press any key to recalibrate...\r\n");
                GETCHAR();
                PRINTF("\r\nCalibrating magnetometer...");
                times = 0;
            }
        }
        times++;
        Delay(3000);
    }
}

int main(void)
{
    fxos_config_t config = {0};
    status_t result;
    uint16_t i              = 0;
    uint16_t loopCounter    = 0;
    double sinAngle         = 0;
    double cosAngle         = 0;
    double Bx               = 0;
    double By               = 0;
    uint8_t array_addr_size = 0;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();
    BOARD_InitPeripherals();

    HW_Timer_init();

    /* Configure the I2C function */
    config.I2C_SendFunc    = BOARD_Accel_I2C_Send;
    config.I2C_ReceiveFunc = BOARD_Accel_I2C_Receive;

    /* Initialize sensor devices */
    array_addr_size = sizeof(g_sensor_address) / sizeof(g_sensor_address[0]);
    for (i = 0; i < array_addr_size; i++)
    {
        config.slaveAddress = g_sensor_address[i];
        /* Initialize accelerometer sensor */
        result = FXOS_Init(&g_fxosHandle, &config);
        if (result == kStatus_Success)
        {
            break;
        }
    }

    if (kStatus_Success != result)
    {
        PRINTF("\r\nSensor device initialize failed!\r\n");
    }

    /* Get sensor range */
    if (kStatus_Success != FXOS_ReadReg(&g_fxosHandle, XYZ_DATA_CFG_REG, &g_sensorRange, 1))
    {
        PRINTF("\r\nGet sensor range failed!\r\n");
    }

    switch (g_sensorRange)
    {
        case 0x00:
            g_dataScale = 2U;
            break;
        case 0x01:
            g_dataScale = 4U;
            break;
        case 0x10:
            g_dataScale = 8U;
            break;
        default:
            break;
    }

    PRINTF("\r\nTo calibrate Magnetometer, roll the board on all orientations to get max and min values\r\n");
    PRINTF("\r\nPress any key to start calibrating...\r\n");
    GETCHAR();
    Magnetometer_Calibrate();

    /* Infinite loops */
    for (;;)
    {
        if (SampleEventFlag == 1) /* Fix loop */
        {
            SampleEventFlag = 0;
            g_Ax_Raw        = 0;
            g_Ay_Raw        = 0;
            g_Az_Raw        = 0;
            g_Ax            = 0;
            g_Ay            = 0;
            g_Az            = 0;
            g_Mx_Raw        = 0;
            g_My_Raw        = 0;
            g_Mz_Raw        = 0;
            g_Mx            = 0;
            g_My            = 0;
            g_Mz            = 0;

            /* Read sensor data */
            Sensor_ReadData(&g_Ax_Raw, &g_Ay_Raw, &g_Az_Raw, &g_Mx_Raw, &g_My_Raw, &g_Mz_Raw);

            /* Average accel value */
            for (i = 1; i < MAX_ACCEL_AVG_COUNT; i++)
            {
                g_Ax_buff[i] = g_Ax_buff[i - 1];
                g_Ay_buff[i] = g_Ay_buff[i - 1];
                g_Az_buff[i] = g_Az_buff[i - 1];
            }

            g_Ax_buff[0] = g_Ax_Raw;
            g_Ay_buff[0] = g_Ay_Raw;
            g_Az_buff[0] = g_Az_Raw;

            for (i = 0; i < MAX_ACCEL_AVG_COUNT; i++)
            {
                g_Ax += (double)g_Ax_buff[i];
                g_Ay += (double)g_Ay_buff[i];
                g_Az += (double)g_Az_buff[i];
            }

            g_Ax /= MAX_ACCEL_AVG_COUNT;
            g_Ay /= MAX_ACCEL_AVG_COUNT;
            g_Az /= MAX_ACCEL_AVG_COUNT;

            if (g_FirstRun)
            {
                g_Mx_LP = g_Mx_Raw;
                g_My_LP = g_My_Raw;
                g_Mz_LP = g_Mz_Raw;
            }

            g_Mx_LP += ((double)g_Mx_Raw - g_Mx_LP) * 0.01;
            g_My_LP += ((double)g_My_Raw - g_My_LP) * 0.01;
            g_Mz_LP += ((double)g_Mz_Raw - g_Mz_LP) * 0.01;

            /* Calculate magnetometer values */
            g_Mx = g_Mx_LP - g_Mx_Offset;
            g_My = g_My_LP - g_My_Offset;
            g_Mz = g_Mz_LP - g_Mz_Offset;

            /* Calculate roll angle g_Roll (-180deg, 180deg) and sin, cos */
            g_Roll   = atan2(g_Ay, g_Az) * RadToDeg;
            sinAngle = sin(g_Roll * DegToRad);
            cosAngle = cos(g_Roll * DegToRad);

            /* De-rotate by roll angle g_Roll */
            By   = g_My * cosAngle - g_Mz * sinAngle;
            g_Mz = g_Mz * cosAngle + g_My * sinAngle;
            g_Az = g_Ay * sinAngle + g_Az * cosAngle;

            /* Calculate pitch angle g_Pitch (-90deg, 90deg) and sin, cos*/
            g_Pitch  = atan2(-g_Ax, g_Az) * RadToDeg;
            sinAngle = sin(g_Pitch * DegToRad);
            cosAngle = cos(g_Pitch * DegToRad);

            /* De-rotate by pitch angle g_Pitch */
            Bx = g_Mx * cosAngle + g_Mz * sinAngle;

            /* Calculate yaw = ecompass angle psi (-180deg, 180deg) */
            g_Yaw = atan2(-By, Bx) * RadToDeg;
            if (g_FirstRun)
            {
                g_Yaw_LP   = g_Yaw;
                g_FirstRun = false;
            }

            g_Yaw_LP += (g_Yaw - g_Yaw_LP) * 0.01;

            if (++loopCounter > 10)
            {
                PRINTF("\r\nCompass Angle: %3.1lf", g_Yaw_LP);
                loopCounter = 0;
            }
        }
    } /* End infinite loops */
}
