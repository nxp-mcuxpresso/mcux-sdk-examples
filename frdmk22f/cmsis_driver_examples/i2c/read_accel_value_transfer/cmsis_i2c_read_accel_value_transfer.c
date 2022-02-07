/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017,2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "Driver_I2C.h"
#include "fsl_i2c_cmsis.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER Driver_I2C0

#define I2C_RELEASE_SDA_PORT  PORTB
#define I2C_RELEASE_SCL_PORT  PORTB
#define I2C_RELEASE_SDA_GPIO  GPIOB
#define I2C_RELEASE_SDA_PIN   3U
#define I2C_RELEASE_SCL_GPIO  GPIOB
#define I2C_RELEASE_SCL_PIN   2U
#define I2C_RELEASE_BUS_COUNT 100U
/* MMA8491 does not have WHO_AM_I, DATA_CFG amd CTRL registers */
#if !(defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491)
#define FXOS8700_WHOAMI    0xC7U
#define MMA8451_WHOAMI     0x1AU
#define MMA8652_WHOAMI     0x4AU
#define ACCEL_XYZ_DATA_CFG 0x0EU
#define ACCEL_CTRL_REG1    0x2AU
/* FXOS8700 and MMA8451 have the same who_am_i register address. */
#define ACCEL_WHOAMI_REG 0x0DU
#endif
#define ACCEL_STATUS     0x00U
#define ACCEL_READ_TIMES 10U
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
#if !(defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491)
static bool I2C_ReadAccelWhoAmI(void);
static bool I2C_WriteAccelReg(uint8_t device_addr, uint8_t reg_addr, uint8_t value);
#endif
static bool I2C_ReadAccelRegs(uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491
uint8_t g_mma8491_addr = 0x55U;
#else
/* FXOS8700, MMA8652 and MMA8451 device address */
const uint8_t g_accel_address[] = {0x1CU, 0x1DU, 0x1EU, 0x1FU};
#endif
uint8_t g_accel_addr_found   = 0x00;
volatile bool completionFlag = false;
volatile bool nakFlag        = false;

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
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux        = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic  = 1U;
    CLOCK_EnableClock(kCLOCK_PortB); /* Port B Clock Gate Control: Clock enabled */
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SCL_PIN, &i2c_pin_config);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SDA_PIN, &i2c_pin_config);

    GPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    GPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

uint32_t I2C0_GetFreq(void)
{
    return CLOCK_GetFreq(I2C0_CLK_SRC);
}

void I2C_MasterSignalEvent_t(uint32_t event)
{
    if (event == ARM_I2C_EVENT_TRANSFER_DONE)
    {
        completionFlag = true;
    }
    if (event == ARM_I2C_EVENT_ADDRESS_NACK)
    {
        nakFlag = true;
    }
}

static void I2C_InitModule(void)
{
    /*
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableStopHold = false;
     * masterConfig.glitchFilterWidth = 0U;
     * masterConfig.enableMaster = true;
     */
    EXAMPLE_I2C_MASTER.Initialize(I2C_MasterSignalEvent_t);
    EXAMPLE_I2C_MASTER.PowerControl(ARM_POWER_FULL);
    EXAMPLE_I2C_MASTER.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST_PLUS);
}

#if !(defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491)
static bool I2C_ReadAccelWhoAmI(void)
{
    /*
    How to read the device who_am_I value ?
    Start + Device_address_Write , who_am_I_register;
    Repeart_Start + Device_address_Read , who_am_I_value.
    */
    uint8_t who_am_i_reg          = ACCEL_WHOAMI_REG;
    uint8_t who_am_i_value        = 0x00;
    uint8_t accel_addr_array_size = 0x00;
    bool find_device              = false;
    uint8_t i                     = 0;

    I2C_InitModule();

    accel_addr_array_size = sizeof(g_accel_address) / sizeof(g_accel_address[0]);

    for (i = 0; i < accel_addr_array_size; i++)
    {
        EXAMPLE_I2C_MASTER.MasterTransmit(g_accel_address[i], &who_am_i_reg, 1, true);

        /*  wait for transfer completed. */
        while ((!nakFlag) && (!completionFlag))
        {
        }

        nakFlag = false;

        if (completionFlag == true)
        {
            completionFlag     = false;
            find_device        = true;
            g_accel_addr_found = g_accel_address[i];
            break;
        }
    }

    if (find_device == true)
    {
        EXAMPLE_I2C_MASTER.MasterReceive(g_accel_addr_found, &who_am_i_value, 1, false);

        /*  wait for transfer completed. */
        while ((!nakFlag) && (!completionFlag))
        {
        }

        nakFlag = false;

        if (completionFlag == true)
        {
            completionFlag = false;
            if (who_am_i_value == FXOS8700_WHOAMI)
            {
                PRINTF("Found an FXOS8700 on board , the device address is 0x%x . \r\n", g_accel_addr_found);
                return true;
            }
            else if (who_am_i_value == MMA8451_WHOAMI)
            {
                PRINTF("Found an MMA8451 on board , the device address is 0x%x . \r\n", g_accel_addr_found);
                return true;
            }
            else
            {
                PRINTF("Found a device, the WhoAmI value is 0x%x\r\n", who_am_i_value);
                PRINTF("It's not MMA8451 or FXOS8700. \r\n");
                PRINTF("The device address is 0x%x. \r\n", g_accel_addr_found);
                return false;
            }
        }
        else
        {
            PRINTF("Not a successful i2c communication \r\n");
            return false;
        }
    }
    else
    {
        PRINTF("\r\n Do not find an accelerometer device ! \r\n");
        return false;
    }
}

static bool I2C_WriteAccelReg(uint8_t device_addr, uint8_t reg_addr, uint8_t value)
{
    uint8_t writedata[2] = {reg_addr, value};

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    EXAMPLE_I2C_MASTER.MasterTransmit(device_addr, writedata, 2, false);

    /*  wait for transfer completed. */
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;

    if (completionFlag == true)
    {
        completionFlag = false;
        return true;
    }
    else
    {
        return false;
    }
}
#endif

static bool I2C_ReadAccelRegs(uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize)
{
    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    EXAMPLE_I2C_MASTER.MasterTransmit(device_addr, &reg_addr, 1, false);
    while ((!nakFlag) && (!completionFlag))
    {
    }
    nakFlag        = false;
    completionFlag = false;
    EXAMPLE_I2C_MASTER.MasterReceive(device_addr, rxBuff, rxSize, false);

    /*  wait for transfer completed. */
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;

    if (completionFlag == true)
    {
        completionFlag = false;
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    bool isThereAccel = false;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_I2C_ReleaseBus();
    BOARD_InitDebugConsole();
    PRINTF("\r\nI2C example -- Read Accelerometer Value\r\n");

#if defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491
    uint8_t status0_value                = 0;
    gpio_pin_config_t tilt_enable_config = {.pinDirection = kGPIO_DigitalOutput, .outputLogic = 0U};

    /* Enable sensor */
    GPIO_PinInit(BOARD_TILT_ENABLE_GPIO, BOARD_TILT_ENABLE_GPIO_PIN, &tilt_enable_config);

    I2C_InitModule();

    /* If using MMA8491, then perform get status operation to g_mma8491_addr to check whether it is welded on board. */
    isThereAccel = I2C_ReadAccelRegs(g_mma8491_addr, ACCEL_STATUS, &status0_value, 1);

    if (isThereAccel)
    {
        PRINTF("Found MMA8491 on board, the device address is 0x%x. \r\n", g_mma8491_addr);
        g_accel_addr_found = g_mma8491_addr;
    }
    else
    {
        PRINTF("\r\nDo not find an accelerometer device ! \r\n");
    }
#else
    /* For other sensors, check the type of the sensor */
    isThereAccel = I2C_ReadAccelWhoAmI();
#endif

    /*  read the accel xyz value if there is accel device on board */
    if (true == isThereAccel)
    {
        uint8_t readBuff[7];
        int16_t x, y, z;
        uint8_t status0_value = 0;
        uint32_t i            = 0U;
#if !(defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491)
        uint8_t databyte  = 0;
        uint8_t write_reg = 0;

        /*  please refer to the "example FXOS8700CQ Driver Code" in FXOS8700 datasheet. */
        /*  write 0000 0000 = 0x00 to accelerometer control register 1 */
        /*  standby */
        /*  [7-1] = 0000 000 */
        /*  [0]: active=0 */
        write_reg = ACCEL_CTRL_REG1;
        databyte  = 0;
        I2C_WriteAccelReg(g_accel_addr_found, write_reg, databyte);

        /*  write 0000 0001= 0x01 to XYZ_DATA_CFG register */
        /*  [7]: reserved */
        /*  [6]: reserved */
        /*  [5]: reserved */
        /*  [4]: hpf_out=0 */
        /*  [3]: reserved */
        /*  [2]: reserved */
        /*  [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg/LSB */
        /*  databyte = 0x01; */
        write_reg = ACCEL_XYZ_DATA_CFG;
        databyte  = 0x01;
        I2C_WriteAccelReg(g_accel_addr_found, write_reg, databyte);

        /*  write 0000 1101 = 0x0D to accelerometer control register 1 */
        /*  [7-6]: aslp_rate=00 */
        /*  [5-3]: dr=001 for 200Hz data rate (when in hybrid mode) */
        /*  [2]: lnoise=1 for low noise mode */
        /*  [1]: f_read=0 for normal 16 bit reads */
        /*  [0]: active=1 to take the part out of standby and enable sampling */
        /*   databyte = 0x0D; */
        write_reg = ACCEL_CTRL_REG1;
        databyte  = 0x0d;
        I2C_WriteAccelReg(g_accel_addr_found, write_reg, databyte);
#endif
        PRINTF("The accel values:\r\n");
        for (i = 0; i < ACCEL_READ_TIMES; i++)
        {
#if defined(I2C_ACCEL_MMA8491) && I2C_ACCEL_MMA8491
            /* Disable and re-enable sensor to get another reading */
            GPIO_PortToggle(BOARD_TILT_ENABLE_GPIO, 1U << BOARD_TILT_ENABLE_GPIO_PIN);
            GPIO_PortToggle(BOARD_TILT_ENABLE_GPIO, 1U << BOARD_TILT_ENABLE_GPIO_PIN);
            status0_value = 0;
            while (status0_value != 0xfU)
#else
            while (status0_value != 0xffU)
#endif
            {
                I2C_ReadAccelRegs(g_accel_addr_found, ACCEL_STATUS, &status0_value, 1);
            }

            /*  Multiple-byte Read from STATUS (0x00) register */
            I2C_ReadAccelRegs(g_accel_addr_found, ACCEL_STATUS, readBuff, 7);

            status0_value = readBuff[0];
            x             = ((int16_t)(((readBuff[1] * 256U) | readBuff[2]))) / 4U;
            y             = ((int16_t)(((readBuff[3] * 256U) | readBuff[4]))) / 4U;
            z             = ((int16_t)(((readBuff[5] * 256U) | readBuff[6]))) / 4U;

            PRINTF("status_reg = 0x%x , x = %5d , y = %5d , z = %5d \r\n", status0_value, x, y, z);
        }
    }

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
