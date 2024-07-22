/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPI2C_CLOCK_FREQUENCY CLOCK_GetIpFreq(kCLOCK_Lpi2c1)
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
static bool LPI2C_ReadAccelWhoAmI(void);
static bool LPI2C_WriteAccelReg(LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
static bool LPI2C_ReadAccelRegs(
    LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* FXLS8965AF device address */
const uint8_t g_accel_address[] = {0x18U, 0x19U, 0x20U, 0x21U};
lpi2c_master_handle_t g_m_handle;
uint8_t g_accel_addr_found   = 0x00U;
volatile bool completionFlag = false;
volatile bool nakFlag        = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        completionFlag = true;
    }
    /* Signal transfer success when received success status. */
    if (status == kStatus_LPI2C_Nak)
    {
        nakFlag = true;
    }
}

static bool LPI2C_ReadAccelWhoAmI(void)
{
    /*
    How to read the device who_am_I value ?
    Start + Device_address_Write , who_am_I_register;
    Repeart_Start + Device_address_Read , who_am_I_value.
    */
    uint8_t who_am_i_reg          = ACCEL_WHOAMI_REG;
    uint8_t who_am_i_value        = 0x00;
    uint8_t accel_addr_array_size = 0x00;
    bool result                   = false;
    uint8_t i                     = 0U;
    status_t reVal                = kStatus_Fail;

    lpi2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress   = g_accel_address[0];
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = who_am_i_reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &who_am_i_value;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    accel_addr_array_size = sizeof(g_accel_address) / sizeof(g_accel_address[0]);

    for (i = 0; i < accel_addr_array_size; i++)
    {
        masterXfer.slaveAddress = g_accel_address[i];

        reVal = LPI2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, &masterXfer);
        if (reVal != kStatus_Success)
        {
            continue;
        }
        /*  wait for transfer completed. */
        while ((!nakFlag) && (!completionFlag))
        {
        }

        nakFlag = false;

        if (completionFlag == true)
        {
            g_accel_addr_found = masterXfer.slaveAddress;
            break;
        }

        /* Delay at least one clock cycle to make sure the bus is idle. */
        SDK_DelayAtLeastUs(1000000UL / I2C_BAUDRATE, SystemCoreClock);
    }

    if (completionFlag)
    {
        completionFlag = false;
        if (FXLS8965_WHOAMI == who_am_i_value)
        {
            PRINTF("Found an FXLS8965 on board , the device address is 0x%x.\r\n", masterXfer.slaveAddress);
            result = true;
        }
        else if (FXLS8964_WHOAMI == who_am_i_value)
        {
            PRINTF("Found an FXLS8964 on board , the device address is 0x%x . \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else if (FXLS8967_WHOAMI == who_am_i_value)
        {
            PRINTF("Found an FXLS8967 on board , the device address is 0x%x . \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else if (FXLS8962_WHOAMI == who_am_i_value)
        {
            PRINTF("Found an FXLS8962 on board , the device address is 0x%x . \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else if (FXLS8974_WHOAMI == who_am_i_value)
        {
            PRINTF("Found an FXLS8974 on board , the device address is 0x%x . \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else
        {
            PRINTF("Found a device, the WhoAmI value is 0x%x\r\n", who_am_i_value);
            PRINTF("It's not FXLS896x.\r\n");
            PRINTF("The device address is 0x%x. \r\n", masterXfer.slaveAddress);
            result = false;
        }
    }
    else
    {
        PRINTF("\r\nDo not find an accelerometer device!\r\n");
        result = false;
    }
    return result;
}

static bool LPI2C_WriteAccelReg(LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value)
{
    lpi2c_master_transfer_t masterXfer;
    status_t reVal = kStatus_Fail;

    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress   = device_addr;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &value;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    reVal = LPI2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return false;
    }

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

static bool LPI2C_ReadAccelRegs(
    LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize)
{
    lpi2c_master_transfer_t masterXfer;
    status_t reVal = kStatus_Fail;

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = device_addr;
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = rxSize;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    reVal = LPI2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return false;
    }
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

static bool LPI2C_SetAccelMode(LPI2C_Type *base, uint8_t device_addr, accel_mode_t mode)
{
    uint8_t ctrlReg1;
    bool ret = false;

    ret = LPI2C_ReadAccelRegs(BOARD_ACCEL_I2C_BASEADDR, device_addr, ACCEL_SENS_CONFIG1_REG, &ctrlReg1, 1);
    if (true != ret)
    {
        return false;
    }

    ctrlReg1 = (ctrlReg1 & (~ACCEL_SENS_CONFIG1_ACTIVE_MASK)) | mode;
    ret      = LPI2C_WriteAccelReg(BOARD_ACCEL_I2C_BASEADDR, device_addr, ACCEL_SENS_CONFIG1_REG, ctrlReg1);

    return ret;
}

int main(void)
{
    bool isThereAccel = false;
    lpi2c_master_config_t masterConfig;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(kCLOCK_Lpi2c1, kCLOCK_IpSrcFro192M);
    CLOCK_SetIpSrcDiv(kCLOCK_Lpi2c1, kSCG_SysClkDivBy16);

    PRINTF("\r\nLPI2C example -- Read Accelerometer Value\r\n");

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
     * masterConfig.baudRate_Hz = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    LPI2C_MasterGetDefaultConfig(&masterConfig);

    masterConfig.baudRate_Hz = I2C_BAUDRATE;

    LPI2C_MasterInit(BOARD_ACCEL_I2C_BASEADDR, &masterConfig, LPI2C_CLOCK_FREQUENCY);
    LPI2C_MasterTransferCreateHandle(BOARD_ACCEL_I2C_BASEADDR, &g_m_handle, lpi2c_master_callback, NULL);
    isThereAccel = LPI2C_ReadAccelWhoAmI();

    /*  read the accel xyz value if there is accel device on board */
    if (true == isThereAccel)
    {
        uint8_t readBuff[6];
        int16_t x, y, z;
        uint32_t i   = 0U;
        bool reTrans = false;

        reTrans = LPI2C_SetAccelMode(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found, STANDBY);
        if (reTrans == false)
        {
            PRINTF("Accel enter standby mode failed\r\n");
            return -1;
        }
        reTrans = LPI2C_WriteAccelReg(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found, ACCEL_SENS_CONFIG3, WAKE_ODR_100HZ);
        if (reTrans == false)
        {
            PRINTF("Accel write config register failed\r\n");
            return -1;
        }
        reTrans = LPI2C_SetAccelMode(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found, ACTIVE);
        if (reTrans == false)
        {
            PRINTF("Accel enter active mode failed\r\n");
            return -1;
        }

        PRINTF("The accel values:\r\n");
        for (i = 0; i < ACCEL_READ_TIMES; i++)
        {
            /*  Multiple-byte Read from OUT_X_LSB (0x04) register */
            reTrans =
                LPI2C_ReadAccelRegs(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found, ACCEL_OUT_X_LSB_REG, readBuff, 6);
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

    PRINTF("\r\nEnd of LPI2C example.\r\n");
    while (1)
    {
    }
}
