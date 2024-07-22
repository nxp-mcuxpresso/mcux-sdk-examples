/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_flexio_i2c_master.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FLEXIO_BASE      FLEXIO0
#define FLEXIO_I2C_SDA_PIN     17U
#define FLEXIO_I2C_SCL_PIN     16U
#define FLEXIO_CLOCK_FREQUENCY CLOCK_GetIpFreq(kCLOCK_Flexio0)
#define I2C_BAUDRATE 100000U

#define FXLS8962_WHOAMI 0x62U
#define FXLS8964_WHOAMI 0x84U
#define FXLS8965_WHOAMI 0x65U
#define FXLS8967_WHOAMI 0x87U
#define FXLS8974_WHOAMI 0x86U

#define ACCEL_READ_TIMES 10U

#define ACCEL_OUT_X_LSB_REG    0x04U
#define ACCEL_WHOAMI_REG       0x13U
#define ACCEL_SENS_CONFIG1_REG 0x15U
#define ACCEL_SENS_CONFIG3     0x17U
#define ACCEL_SENS_CONFIG4     0x18U
#define ACCEL_INT_EN           0x20U
#define ACCEL_INT_PIN_SEL      0x21U

#define ACCEL_SENS_CONFIG1_ACTIVE_MASK ((uint8_t)0x01)

/*!
 * @brief accel mode
 */
typedef enum accel_mode
{
    STANDBY = 0, /*!<STANDBY Mode*/
    ACTIVE  = 1, /*!<Active  Mode.*/
} accel_mode_t;

typedef enum accel_odr
{
    WAKE_ODR_3200HZ   = 0x00,
    WAKE_ODR_1600HZ   = 0x10,
    WAKE_ODR_800HZ    = 0x20,
    WAKE_ODR_400HZ    = 0x30,
    WAKE_ODR_200HZ    = 0x40,
    WAKE_ODR_100HZ    = 0x50,
    WAKE_ODR_50HZ     = 0x60,
    WAKE_ODR_25HZ     = 0x70,
    WAKE_ODR_12_5HZ   = 0x80,
    WAKE_ODR_6_25HZ   = 0x90,
    WAKE_ODR_3_125HZ  = 0xa0,
    WAKE_ODR_1_563HZ  = 0xb0,
    WAKE_ODR_0_781HZ  = 0xc0,
    SLEEP_ODR_3200HZ  = 0x00,
    SLEEP_ODR_1600HZ  = 0x01,
    SLEEP_ODR_800HZ   = 0x02,
    SLEEP_ODR_400HZ   = 0x03,
    SLEEP_ODR_200HZ   = 0x04,
    SLEEP_ODR_100HZ   = 0x05,
    SLEEP_ODR_50HZ    = 0x06,
    SLEEP_ODR_25HZ    = 0x07,
    SLEEP_ODR_12_5HZ  = 0x08,
    SLEEP_ODR_6_25HZ  = 0x09,
    SLEEP_ODR_3_125HZ = 0x0a,
    SLEEP_ODR_1_563HZ = 0x0b,
    SLEEP_ODR_0_781HZ = 0x0c
} accel_odr_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static bool I2C_example_readAccelWhoAmI(void);
static bool I2C_write_accel_reg(FLEXIO_I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
static bool I2C_read_accel_regs(
    FLEXIO_I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* FXLS8965AF device address */
const uint8_t g_accel_address[] = {0x18U, 0x19U, 0x20U, 0x21U};

flexio_i2c_master_handle_t g_m_handle;
FLEXIO_I2C_Type i2cDev;
uint8_t g_accel_addr_found   = 0x00;
volatile bool completionFlag = false;
volatile bool nakFlag        = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void flexio_i2c_master_callback(FLEXIO_I2C_Type *base,
                                       flexio_i2c_master_handle_t *handle,
                                       status_t status,
                                       void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        completionFlag = true;
    }
    /* Signal transfer success when received success status. */
    if (status == kStatus_FLEXIO_I2C_Nak)
    {
        nakFlag = true;
    }
}

static bool I2C_example_readAccelWhoAmI(void)
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
    bool result                   = false;
    uint8_t i                     = 0;
    uint32_t j                    = 0;

    flexio_i2c_master_config_t masterConfig;

    /*
     * masterConfig.enableMaster = true;
     * masterConfig.enableInDoze = false;
     * masterConfig.enableInDebug = true;
     * masterConfig.enableFastAccess = false;
     * masterConfig.baudRate_Bps = 100000U;
     */
    FLEXIO_I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    if (FLEXIO_I2C_MasterInit(&i2cDev, &masterConfig, FLEXIO_CLOCK_FREQUENCY) != kStatus_Success)
    {
        PRINTF("FlexIO clock frequency exceeded upper range. \r\n");
        return false;
    }

    FLEXIO_I2C_MasterTransferCreateHandle(&i2cDev, &g_m_handle, flexio_i2c_master_callback, NULL);

    flexio_i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress   = g_accel_address[0];
    masterXfer.direction      = kFLEXIO_I2C_Read;
    masterXfer.subaddress     = who_am_i_reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &who_am_i_value;
    masterXfer.dataSize       = 1;

    accel_addr_array_size = sizeof(g_accel_address) / sizeof(g_accel_address[0]);

    for (i = 0; i < accel_addr_array_size; i++)
    {
        masterXfer.slaveAddress = g_accel_address[i];
        completionFlag          = false;
        FLEXIO_I2C_MasterTransferNonBlocking(&i2cDev, &g_m_handle, &masterXfer);

        /*  wait for transfer completed. */
        while ((nakFlag == false) && (completionFlag == false))
        {
        }
        if (nakFlag == true)
        {
            nakFlag = false;
            for (j = 0; j < 0x1FFF; j++)
            {
                __NOP();
            }
            continue;
        }
        if (completionFlag == true)
        {
            completionFlag     = false;
            find_device        = true;
            g_accel_addr_found = masterXfer.slaveAddress;
            break;
        }
        for (j = 0; j < 0xFFF; j++)
        {
            __NOP();
        }
    }

    if (find_device == true)
    {
        switch (who_am_i_value)
        {
            case FXLS8965_WHOAMI:
                PRINTF("Found a FXLS8965 on board, the device address is 0x%02X.\r\n", masterXfer.slaveAddress);
                result = true;
                break;
            case FXLS8964_WHOAMI:
                PRINTF("Found a FXLS8964 on board, the device address is 0x%02X.\r\n", masterXfer.slaveAddress);
                result = true;
                break;
            case FXLS8967_WHOAMI:
                PRINTF("Found a FXLS8967 on board, the device address is 0x%02X.\r\n", masterXfer.slaveAddress);
                result = true;
                break;
            case FXLS8962_WHOAMI:
                PRINTF("Found a FXLS8962 on board, the device address is 0x%02X.\r\n", masterXfer.slaveAddress);
                result = true;
                break;
            case FXLS8974_WHOAMI:
                PRINTF("Found a FXLS8974 on board, the device address is 0x%02X.\r\n", masterXfer.slaveAddress);
                result = true;
                break;
            default:
                PRINTF("Found a device, the WhoAmI value is 0x%02X\r\n", who_am_i_value);
                PRINTF("It's not FXLS896x.\r\n");
                PRINTF("The device address is 0x%02X.\r\n", masterXfer.slaveAddress);
                result = false;
                break;
        }

        return result;
    }
    else
    {
        PRINTF("Not a successful i2c communication\r\n");
        return false;
    }
}

static bool I2C_write_accel_reg(FLEXIO_I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value)
{
    flexio_i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress   = device_addr;
    masterXfer.direction      = kFLEXIO_I2C_Write;
    masterXfer.subaddress     = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &value;
    masterXfer.dataSize       = 1;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    FLEXIO_I2C_MasterTransferNonBlocking(&i2cDev, &g_m_handle, &masterXfer);

    /*  Wait for transfer completed. */
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

static bool I2C_read_accel_regs(
    FLEXIO_I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize)
{
    flexio_i2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = device_addr;
    masterXfer.direction      = kFLEXIO_I2C_Read;
    masterXfer.subaddress     = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = rxSize;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    FLEXIO_I2C_MasterTransferNonBlocking(&i2cDev, &g_m_handle, &masterXfer);

    /*  Wait for transfer completed. */
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

static bool I2C_set_accel_mode(FLEXIO_I2C_Type *base, uint8_t device_addr, accel_mode_t mode)
{
    uint8_t ctrlReg1;
    bool ret = false;

    ret = I2C_read_accel_regs(base, device_addr, ACCEL_SENS_CONFIG1_REG, &ctrlReg1, 1);
    if (true != ret)
    {
        return false;
    }

    ctrlReg1 = (ctrlReg1 & (~ACCEL_SENS_CONFIG1_ACTIVE_MASK)) | mode;
    ret      = I2C_write_accel_reg(base, device_addr, ACCEL_SENS_CONFIG1_REG, ctrlReg1);

    return ret;
}

/*!
 * @brief Main function
 */
int main(void)
{
    bool isThereAccel = false;

    /*do hardware configuration*/
    i2cDev.flexioBase      = BOARD_FLEXIO_BASE;
    i2cDev.SDAPinIndex     = FLEXIO_I2C_SDA_PIN;
    i2cDev.SCLPinIndex     = FLEXIO_I2C_SCL_PIN;
    i2cDev.shifterIndex[0] = 0U;
    i2cDev.shifterIndex[1] = 1U;
    i2cDev.timerIndex[0]   = 0U;
    i2cDev.timerIndex[1]   = 1U;
    i2cDev.timerIndex[2]   = 2U;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Set MRCC FLEXIO0 selection */
    CLOCK_SetIpSrc(kCLOCK_Flexio0, kCLOCK_IpSrcFro192M);
    /* Set MRCC FLEXIO0 fraction divider */
    CLOCK_SetIpSrcDiv(kCLOCK_Flexio0, kSCG_SysClkDivBy6);

    PRINTF("\r\nFlexIO I2C example read accelerometer value\r\n");

    isThereAccel = I2C_example_readAccelWhoAmI();

    /*  read the accel xyz value if there is accel device on board */
    if (true == isThereAccel)
    {
        uint8_t readBuff[7];
        int16_t x, y, z;
        uint32_t i   = 0;
        bool reTrans = false;

        reTrans = I2C_set_accel_mode(&i2cDev, g_accel_addr_found, STANDBY);
        if (reTrans == false)
        {
            PRINTF("Accel enter standby mode failed\r\n");
            return -1;
        }
        reTrans = I2C_write_accel_reg(&i2cDev, g_accel_addr_found, ACCEL_SENS_CONFIG3, WAKE_ODR_100HZ);
        if (reTrans == false)
        {
            PRINTF("Accel write config register failed\r\n");
            return -1;
        }
        reTrans = I2C_set_accel_mode(&i2cDev, g_accel_addr_found, ACTIVE);
        if (reTrans == false)
        {
            PRINTF("Accel enter active mode failed\r\n");
            return -1;
        }

        PRINTF("The accel values:\r\n");
        for (i = 0; i < ACCEL_READ_TIMES; i++)
        {
            /*  Multiple-byte Read from OUT_X_LSB (0x04) register */
            reTrans = I2C_read_accel_regs(&i2cDev, g_accel_addr_found, ACCEL_OUT_X_LSB_REG, readBuff, 6);

            if (reTrans == false)
            {
                PRINTF("Read accel value failed\r\n");
                return -1;
            }

            x = ((int16_t)(((readBuff[1] << 8U) | readBuff[0])));
            y = ((int16_t)(((readBuff[3] << 8U) | readBuff[2])));
            z = ((int16_t)(((readBuff[5] << 8U) | readBuff[4])));

            PRINTF("x = %5d , y = %5d , z = %5d\r\n", x, y, z);
        }
    }

    PRINTF("\r\nEnd of I2C example.\r\n");
    while (1)
    {
    }
}
