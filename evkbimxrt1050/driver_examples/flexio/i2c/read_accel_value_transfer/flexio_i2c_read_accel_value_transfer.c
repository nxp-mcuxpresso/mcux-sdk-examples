/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
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
#define BOARD_FLEXIO_BASE FLEXIO2

/* Select USB1 PLL (480 MHz) as flexio clock source */
#define FLEXIO_CLOCK_SELECT (3U)
/* Clock pre divider for flexio clock source */
#define FLEXIO_CLOCK_PRE_DIVIDER (1U)
/* Clock divider for flexio clock source */
#define FLEXIO_CLOCK_DIVIDER (5U)
#define FLEXIO_CLOCK_FREQUENCY \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / (FLEXIO_CLOCK_PRE_DIVIDER + 1U) / (FLEXIO_CLOCK_DIVIDER + 1U))

#define FLEXIO_I2C_SDA_PIN 5U
#define FLEXIO_I2C_SCL_PIN 6U

#define I2C_BAUDRATE       (100000) /* 100K */
#define FXOS8700_WHOAMI    (0xC7U)
#define MMA8451_WHOAMI     (0x1AU)
#define LSM6DSO_WHOAMI     (0x6CU)
#define ACCEL_STATUS       (0x00U)
#define ACCEL_XYZ_DATA_CFG (0x0EU)
#define ACCEL_CTRL_REG1    (0x2AU)
/* FXOS8700 and MMA8451 have the same who_am_i register address. */
#define ACCEL_WHOAMI_REG         (0x0DU)
#define LSM6DSO_WHOAMI_REG       (0x0FU)
#define ACCEL_READ_TIMES         (10U)
#define SENSOR_MODEL_NUMBERS     3U
#define READ_SEQ_COMMAND_NUMBERS 2U

typedef enum _sensor_model
{
    FXOS8700 = 0U,
    MMA8451  = 1U,
    LSM6DSO  = 2U,
} sensor_model;

typedef enum _read_seq_command
{
    READ_STATUS     = 0U,
    READ_ACCEL_DATA = 1U,
} read_seq_command;

/*!
 * @brief This structure defines the Write command List.
 */
typedef struct _regList
{
    uint8_t reg; /* Register Address where the value is wrote to */
    uint8_t val;
    uint8_t size; /* read size from register */
} regList_t;

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

uint8_t g_whoami_reg_addr[SENSOR_MODEL_NUMBERS] = {ACCEL_WHOAMI_REG, ACCEL_WHOAMI_REG, LSM6DSO_WHOAMI_REG};

/*
 * device address:
 * FXOS8700(0x1c, 0x1d, 0x1e, 0x1f),
 * MMA8451(0x1c, 0x1d, 0x1e, 0x1f ),
 * LSM6SDO(0x6a, 0x6b).
 */
const uint8_t g_accel_addr[SENSOR_MODEL_NUMBERS][4] = {
    {0x1CU, 0x1DU, 0x1EU, 0x1FU}, /* FXOS8700 */
    {0x1CU, 0x1DU, 0x1EU, 0x1FU}, /* MMA8451 */
    {0x6AU, 0x6BU, 0x00U, 0x00U}  /* LSM6SDO */
};

/* Each entry in a regWriteList is composed of: register address, value to write, bit-mask to apply to write */
/* For FXOS8700 and MMA8451 */
regList_t FXOS8700AndMMA8451InitSeq[] = {
    /* for FXOS8700 */
    /*  write 0000 0000 = 0x00 to accelerometer control register 1 */
    /*  standby */
    /*  [7-1] = 0000 000 */
    /*  [0]: active=0 */
    {ACCEL_CTRL_REG1, 0x00, 0x01},
    /*  write 0000 0001= 0x01 to XYZ_DATA_CFG register */
    /*  [7]: reserved */
    /*  [6]: reserved */
    /*  [5]: reserved */
    /*  [4]: hpf_out=0 */
    /*  [3]: reserved */
    /*  [2]: reserved */
    /*  [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg/LSB */
    /*  databyte = 0x01; */
    {ACCEL_XYZ_DATA_CFG, 0x01, 0x01},
    /*  write 0000 1101 = 0x0D to accelerometer control register 1 */
    /*  [7-6]: aslp_rate=00 */
    /*  [5-3]: dr=001 for 200Hz data rate (when in hybrid mode) */
    /*  [2]: lnoise=1 for low noise mode */
    /*  [1]: f_read=0 for normal 16 bit reads */
    /*  [0]: active=1 to take the part out of standby and enable sampling */
    /*   databyte = 0x0D; */
    {ACCEL_CTRL_REG1, 0x0d, 0x01},
};

regList_t LSM6DSOInitSeq[] = {
    /* for LSM6DSO */
    /*  write 0000 0001 = 0x01 to CTRL3_C(0x12) */
    /*  software reset */
    {0x12, 0x01, 0x01},

    /*  write 0000 1000 = 0x08 to CTRL4_C(0x13) */
    /*   CTRL4_C[3] = 1,  enable data available */
    {0x13, 0x08, 0x00},

    /*  write 1011 0000 = 0xb0 to CTRL1_XL(0x10) */
    /*   CTRL1_XL[7:4] = 1011,  12.5 Hz(high performance) */
    {0x10, 0xb0, 0x01},
};

/*  SENSOR_MODEL_NUMBERS * READ_SEQ_COMMAND_NUMBERS */
regList_t readSeq[] = {
    /* for FXOS8700 */
    /* read status register */
    {0x00, 0xff, 0x01}, /* READ_STATUS */
    /* read acceleration value from registers */
    {0x01, 0x00, 0x06}, /* READ_ACCEL_DATA */

    /* for MMA8451(same with FXOS8700) */
    /* read status register */
    {0x00, 0xff, 0x01}, /* READ_STATUS */
    /* read acceleration value from registers */
    {0x01, 0x00, 0x06}, /* READ_ACCEL_DATA */

    /* for LSM6DSO */
    /* read STATUS_REG(0x1E) */
    {0x1e, 0x05, 0x01}, /* READ_STATUS */
    /* read acceleration value from OUTX_L_A, OUTX_H_A, OUTY_L_A, OUTY_H_A, OUTZ_L_A, OUTZ_H_A */
    {0x28, 0x00, 0x06}, /* READ_ACCEL_DATA */
};

flexio_i2c_master_handle_t g_m_handle;
FLEXIO_I2C_Type i2cDev;
uint8_t g_device_addr_found  = 0x00;
uint8_t g_model              = SENSOR_MODEL_NUMBERS;
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
    uint8_t who_am_i_value         = 0x00;
    uint8_t device_addr_array_size = 0x00;
    bool find_device               = false;
    bool result                    = false;
    uint8_t i                      = 0;
    uint32_t j                     = 0;
    uint8_t model                  = 0;
    uint8_t device_addr_offset     = 0;

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

    masterXfer.slaveAddress   = g_accel_addr[0][0];
    masterXfer.direction      = kFLEXIO_I2C_Read;
    masterXfer.subaddress     = g_whoami_reg_addr[0];
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &who_am_i_value;
    masterXfer.dataSize       = 1;

    device_addr_array_size = sizeof(g_accel_addr) / sizeof(g_accel_addr[0][0]);

    for (i = 0; i < device_addr_array_size; i++)
    {
        model                   = i / sizeof(g_accel_addr[0]);
        device_addr_offset      = i % sizeof(g_accel_addr[0]);
        masterXfer.slaveAddress = g_accel_addr[model][device_addr_offset];
        masterXfer.subaddress   = g_whoami_reg_addr[model];
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
            completionFlag      = false;
            find_device         = true;
            g_device_addr_found = masterXfer.slaveAddress;
            g_model             = model;
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
            case FXOS8700_WHOAMI:
                PRINTF("Found a FXOS8700 on board, the device address is 0x%02X. \r\n", masterXfer.slaveAddress);
                result = true;
                break;
            case MMA8451_WHOAMI:
                PRINTF("Found a MMA8451 on board, the device address is 0x%02X. \r\n", masterXfer.slaveAddress);
                result = true;
                break;
            case LSM6DSO_WHOAMI:
                PRINTF("Found a LSDM6DSO on board, the device address is 0x%02X. \r\n", masterXfer.slaveAddress);
                result = true;
                break;
            default:

                PRINTF("Found a device, the WhoAmI value is 0x%02X\r\n", who_am_i_value);
                PRINTF("It's not MMA8451 or FXOS8700 or LSM6DSO. \r\n");
                PRINTF("The device address is 0x%02X. \r\n", masterXfer.slaveAddress);
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

void I2C_InitSensor(uint8_t model)
{
    int8_t commandNums  = 0;
    regList_t *pRegList = NULL;
    int8_t i            = 0;

    if (model == FXOS8700 || model == MMA8451)
    {
        commandNums = sizeof(FXOS8700AndMMA8451InitSeq) / sizeof(FXOS8700AndMMA8451InitSeq[0]);
        pRegList    = FXOS8700AndMMA8451InitSeq;
    }
    else if (model == LSM6DSO)
    {
        commandNums = sizeof(LSM6DSOInitSeq) / sizeof(LSM6DSOInitSeq[0]);
        pRegList    = LSM6DSOInitSeq;
    }
    else
    {
        PRINTF("\r\n Failed to initialize sensor\r\n");
        return;
    }

    for (i = 0; i < commandNums; i++)
    {
        I2C_write_accel_reg(&i2cDev, g_device_addr_found, pRegList[i].reg, pRegList[i].val);
    }
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

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Clock setting for Flexio */
    CLOCK_SetMux(kCLOCK_Flexio2Mux, FLEXIO_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, FLEXIO_CLOCK_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Flexio2Div, FLEXIO_CLOCK_DIVIDER);

    PRINTF("\r\nFlexIO I2C example read accelerometer value\r\n");

    isThereAccel = I2C_example_readAccelWhoAmI();

    /*  read the accel xyz value if there is accel device on board */
    if (true == isThereAccel)
    {
        uint8_t readBuff[6] = {0};
        int16_t x, y, z;
        uint8_t status0_value = 0;
        uint32_t i            = 0;

        I2C_InitSensor(g_model);

        PRINTF("The accel values:\r\n");
        for (i = 0; i < ACCEL_READ_TIMES; i++)
        {
            status0_value = 0;
            /*  wait for new data are ready. */
            while (status0_value != readSeq[g_model * 2 + READ_STATUS].val)
            {
                I2C_read_accel_regs(&i2cDev, g_device_addr_found, readSeq[g_model * 2 + READ_STATUS].reg,
                                    &status0_value, readSeq[g_model * 2 + READ_STATUS].size);
            }
            /* Delay 10us for the data to be ready. */
            SDK_DelayAtLeastUs(10, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

            /*  Multiple-byte Read from acceleration registers */
            I2C_read_accel_regs(&i2cDev, g_device_addr_found, readSeq[g_model * 2 + READ_ACCEL_DATA].reg, readBuff,
                                readSeq[g_model * 2 + READ_ACCEL_DATA].size);
            x = ((int16_t)(((readBuff[0] << 8U) | readBuff[1]))) >> ((g_model == LSM6DSO) ? (0U) : (2U));
            y = ((int16_t)(((readBuff[2] << 8U) | readBuff[3]))) >> ((g_model == LSM6DSO) ? (0U) : (2U));
            z = ((int16_t)(((readBuff[4] << 8U) | readBuff[5]))) >> ((g_model == LSM6DSO) ? (0U) : (2U));

            PRINTF("status_reg = 0x%x , x = %5d , y = %5d , z = %5d \r\n", status0_value, x, y, z);
        }
    }

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
