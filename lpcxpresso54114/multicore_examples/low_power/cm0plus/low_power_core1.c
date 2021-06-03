/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_mailbox.h"
#include "static_queue.h"
#include "low_power.h"
#include "fsl_i2c.h"
#include "fsl_utick.h"
#include "fsl_clock.h"
#include "fsl_power.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CORE_CLK_FREQ  CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define APP_I2C_MASTER (I2C4)

#define QUEUE_ELEMENT_SIZE  (40U)
#define QUEUE_ELEMENT_COUNT (200U)

#define UTICK_TIME (10000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t volatile g_queue_buffer[QUEUE_ELEMENT_COUNT * QUEUE_ELEMENT_SIZE];
static static_queue_t volatile g_dataBuff;
static struct bmm050_t bmm050;
static struct bma2x2_t bma2x2;
static struct bmi160_t bmi160;
static struct bmp280_t bmp280;
static bool volatile g_readSensorsData = true;
static bool volatile g_flashPowerUp    = true;

/*******************************************************************************
 * Code
 ******************************************************************************/

void MAILBOX_IRQHandler(void)
{
    core_cmd_t cmd = (core_cmd_t)MAILBOX_GetValue(MAILBOX, kMAILBOX_CM0Plus);
    if (cmd == kTurnOffFlash)
    {
        POWER_PowerDownFlash();
        g_flashPowerUp = false;
    }
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM0Plus, 0xffffffffu);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Time delay
 * Function waits desired time with accuracy 1us.
 */
static void timeDelay_us(uint32_t delay)
{
    delay++;
    /* Wait desired time */
    while (delay > 0U)
    {
        if (0 != (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
        {
            delay--;
        }
    }
}

static void timeDelay(uint32_t miliseconds)
{
    timeDelay_us(miliseconds * 1000u);
}

static void timeDelay16(uint16_t miliseconds)
{
    timeDelay_us(miliseconds * 1000u);
}

static void installTimeDelay(void)
{
    /* Disable SysTick timer */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    /* Initialize Reload value to 1us */
    SysTick->LOAD = CORE_CLK_FREQ / 1000000U;
    /* Set clock source to processor clock */
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    /* Enable SysTick timer */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static int8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t cnt)
{
    (void)I2C_MasterStart(APP_I2C_MASTER, addr, kI2C_Write);
    (void)I2C_MasterWriteBlocking(APP_I2C_MASTER, &reg, 1,
                                  (uint32_t)kI2C_TransferNoStartFlag | (uint32_t)kI2C_TransferNoStopFlag);
    (void)I2C_MasterStart(APP_I2C_MASTER, addr, kI2C_Read);
    if (cnt > 0U)
    {
        (void)I2C_MasterReadBlocking(APP_I2C_MASTER, data, cnt, (uint32_t)kI2C_TransferDefaultFlag);
    }
    return 0;
}

static int8_t i2c_write(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t cnt)
{
    (void)I2C_MasterStart(APP_I2C_MASTER, addr, kI2C_Write);
    (void)I2C_MasterWriteBlocking(APP_I2C_MASTER, &reg, 1,
                                  (uint32_t)kI2C_TransferNoStartFlag | (uint32_t)kI2C_TransferNoStopFlag);
    if (cnt > 0U)
    {
        (void)I2C_MasterWriteBlocking(APP_I2C_MASTER, data, cnt,
                                      (uint32_t)kI2C_TransferNoStartFlag | (uint32_t)kI2C_TransferNoStopFlag);
    }
    (void)I2C_MasterStop(APP_I2C_MASTER);
    return 0;
}

static void init_sensors(void)
{
    /* init mag */
    bmm050.bus_write  = i2c_write;
    bmm050.bus_read   = i2c_read;
    bmm050.delay_msec = (void (*)(BMM050_MDELAY_DATA_TYPE))timeDelay;
    bmm050.dev_addr   = 0x12;
    /* Structure used for read the mag xyz data with 32 bit output */
    (void)bmm050_init(&bmm050);
    (void)bmm050_set_functional_state(BMM050_NORMAL_MODE);
    (void)bmm050_set_data_rate(BMM050_DATA_RATE_02HZ);

    /* init accel */
    bma2x2.bus_write  = i2c_write;
    bma2x2.bus_read   = i2c_read;
    bma2x2.delay_msec = (void (*)(BMA2x2_MDELAY_DATA_TYPE))timeDelay;
    bma2x2.dev_addr   = 0x10;
    (void)bma2x2_init(&bma2x2);
    (void)bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
    (void)bma2x2_set_bw(BMA2x2_BW_7_81HZ);

    /* init gyro */
    bmi160.bus_write  = i2c_write;
    bmi160.bus_read   = i2c_read;
    bmi160.delay_msec = (void (*)(BMI160_MDELAY_DATA_TYPE))timeDelay;
    bmi160.dev_addr   = 0x68;
    bmi160_init(&bmi160);
    (void)bmi160_set_command_register(GYRO_MODE_NORMAL);
    timeDelay(1);
    (void)bmi160_set_gyro_output_data_rate(BMI160_GYRO_OUTPUT_DATA_RATE_100HZ);
    (void)bmi160_set_gyro_bw(BMI160_GYRO_NORMAL_MODE);
    (void)bmi160_set_gyro_range(BMI160_GYRO_RANGE_125_DEG_SEC);

    /* bmp280 init */
    bmp280.bus_write  = i2c_write;
    bmp280.bus_read   = i2c_read;
    bmp280.delay_msec = (void (*)(BMP280_MDELAY_DATA_TYPE))timeDelay16;
    bmp280.dev_addr   = 0x76;
    (void)bmp280_init(&bmp280);
    (void)bmp280_set_soft_rst();
    timeDelay(1);
    (void)bmp280_set_power_mode(BMP280_NORMAL_MODE);
    timeDelay(1);
    (void)bmp280_set_work_mode(BMP280_ULTRA_LOW_POWER_MODE);
    (void)bmp280_set_standby_durn(BMP280_STANDBY_TIME_1_MS);
}

static void utick_cb(void)
{
    g_readSensorsData = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    /* attach 12 MHz clock to FLEXCOMM4 (I2C master) */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);

    BOARD_InitBootPins();

    installTimeDelay();

    /* i2c init */
    i2c_master_config_t masterConfig;
    I2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = 380000;

    /* Initialize the LPI2C master peripheral */
    I2C_MasterInit(APP_I2C_MASTER, &masterConfig, CORE_CLK_FREQ);

    /* initialize sensors */
    init_sensors();

    /* Init Mailbox */
    MAILBOX_Init(MAILBOX);

    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

    (void)static_queue_init((static_queue_t *)&g_dataBuff, (uint8_t *)g_queue_buffer, QUEUE_ELEMENT_COUNT,
                            QUEUE_ELEMENT_SIZE);

    /* send to core0 address of shared memory - static queue (ring buffer) */
    MAILBOX_SetValue(MAILBOX, kMAILBOX_CM4, (uint32_t)(&g_dataBuff));

    /* Initialize UTICK */
    UTICK_Init(UTICK0);

    /* Set the UTICK timer to wake up the device from reduced power mode */
    UTICK_SetTick(UTICK0, kUTICK_Repeat, UTICK_TIME, utick_cb);

    sensor_data_t data;

    for (;;)
    {
        (void)bmm050_read_mag_data_XYZ_s32(&data.mag);
        (void)bma2x2_read_accel_xyzt(&data.accel);
        (void)bmi160_read_gyro_xyz(&data.gyro);
        (void)bmp280_read_pressure_temperature((u32 *)&data.press, (s32 *)&data.temp);
        (void)static_queue_add((static_queue_t *)&g_dataBuff, (void *)&data);

        g_readSensorsData = false;

        if (static_queue_size((static_queue_t *)&g_dataBuff) > QUEUE_ELEMENT_COUNT_TRIG)
        {
            MAILBOX_SetValue(MAILBOX, kMAILBOX_CM4, (uint32_t)kProcessData);
        }

        if (!g_flashPowerUp)
        {
            /* Power up flash before waking CM4 */
            POWER_PowerUpFlash();
            timeDelay_us(150);
        }

        /* Send interrupt to CM4 to switch to deep sleep mode */
        MAILBOX_SetValue(MAILBOX, kMAILBOX_CM4, (uint32_t)kGoToDeepSleep);

        if (!g_readSensorsData)
        {
            __WFI();
        }
    }
}
