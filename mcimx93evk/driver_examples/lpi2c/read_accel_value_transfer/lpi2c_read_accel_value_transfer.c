/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#include "fsl_iomuxc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPI2C_CLOCK_FREQUENCY    CLOCK_GetIpFreq(LPI2C_MASTER_CLOCK_ROOT)
#define BOARD_ACCEL_I2C_BASEADDR LPI2C1
#define LPI2C_MASTER_CLOCK_ROOT  kCLOCK_Root_Lpi2c1
#define LPI2C_MASTER_CLOCK_GATE  kCLOCK_Lpi2c1
#define I2C_BAUDRATE       100000U
#define FXOS8700_WHOAMI    0xC7U
#define MMA8451_WHOAMI     0x1AU
#define LSM6DSO_WHOAMI     0x6CU
#define ACCEL_STATUS       0x00U
#define ACCEL_XYZ_DATA_CFG 0x0EU
#define ACCEL_CTRL_REG1    0x2AU
/* FXOS8700 and MMA8451 have the same who_am_i register address. */
#define ACCEL_WHOAMI_REG         0x0DU
#define LSM6DSO_WHOAMI_REG       0x0FU
#define ACCEL_READ_TIMES         10U
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
static bool LPI2C_ReadAccelWhoAmI(void);
static bool LPI2C_WriteAccelReg(LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
static bool LPI2C_ReadAccelRegs(
    LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);

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
const uint8_t g_accel_address[SENSOR_MODEL_NUMBERS][4] = {
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

lpi2c_master_handle_t g_m_handle;

uint8_t g_accel_addr_found = 0x00U;
uint8_t g_model            = SENSOR_MODEL_NUMBERS;

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
    uint8_t who_am_i_value         = 0x00;
    uint8_t device_addr_array_size = 0x00;
    bool result                    = false;
    uint8_t i                      = 0U;
    uint8_t model                  = 0;
    uint8_t device_addr_offset     = 0;
    status_t reVal                 = kStatus_Fail;

    lpi2c_master_transfer_t masterXfer;
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress   = g_accel_address[0][0];
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = g_whoami_reg_addr[0];
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &who_am_i_value;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    device_addr_array_size = sizeof(g_accel_address) / sizeof(g_accel_address[0][0]);

    for (i = 0; i < device_addr_array_size; i++)
    {
        model                   = i / sizeof(g_accel_address[0]);
        device_addr_offset      = i % sizeof(g_accel_address[0]);
        masterXfer.slaveAddress = g_accel_address[model][device_addr_offset];
        masterXfer.subaddress   = g_whoami_reg_addr[model];

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
            g_model            = model;
            break;
        }

        /* Wait to make sure the bus is idle. */
        while ((LPI2C_MasterGetStatusFlags(BOARD_ACCEL_I2C_BASEADDR) & (uint32_t)kLPI2C_MasterBusBusyFlag) != 0U)
        {
        }
    }

    if (completionFlag)
    {
        completionFlag = false;
        if (who_am_i_value == FXOS8700_WHOAMI)
        {
            PRINTF("Found an FXOS8700 on board , the device address is 0x%x . \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else if (who_am_i_value == MMA8451_WHOAMI)
        {
            PRINTF("Found an MMA8451 on board , the device address is 0x%x . \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else if (who_am_i_value == LSM6DSO_WHOAMI)
        {
            PRINTF("Found a LSDM6DSO on board, the device address is 0x%02X. \r\n", masterXfer.slaveAddress);
            result = true;
        }
        else
        {
            PRINTF("Found a device, the WhoAmI value is 0x%x\r\n", who_am_i_value);
            PRINTF("It's not MMA8451 or FXOS8700 or LSM6DSO. \r\n");
            PRINTF("The device address is 0x%x. \r\n", masterXfer.slaveAddress);
            result = false;
        }
    }
    else
    {
        PRINTF("\r\n Do not find an accelerometer device ! \r\n");
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

void LPI2C_InitSensor(uint8_t model)
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
        LPI2C_WriteAccelReg(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found, pRegList[i].reg, pRegList[i].val);
    }
}

int main(void)
{
    bool isThereAccel = false;
    lpi2c_master_config_t masterConfig;

    /* clang-format off */
    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };
    /* clang-format on */

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(LPI2C_MASTER_CLOCK_ROOT, &lpi2cClkCfg);
    CLOCK_EnableClock(LPI2C_MASTER_CLOCK_GATE);

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
        uint8_t readBuff[6] = {0};
        int16_t x, y, z;
        uint8_t status0_value = 0;
        uint32_t i            = 0U;
        bool reTrans          = false;

        LPI2C_InitSensor(g_model);

        PRINTF("The accel values:\r\n");
        for (i = 0; i < ACCEL_READ_TIMES; i++)
        {
            status0_value = 0;
            /*  wait for new data are ready. */
            while (status0_value != readSeq[g_model * 2 + READ_STATUS].val)
            {
                reTrans = LPI2C_ReadAccelRegs(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found,
                                              readSeq[g_model * 2 + READ_STATUS].reg, &status0_value,
                                              readSeq[g_model * 2 + READ_STATUS].size);
                if (reTrans == false)
                {
                    PRINTF("F4\n");
                    return -1;
                }
                else
                {
                }
            }

            /*  Multiple-byte Read from acceleration registers */
            reTrans = LPI2C_ReadAccelRegs(BOARD_ACCEL_I2C_BASEADDR, g_accel_addr_found,
                                          readSeq[g_model * 2 + READ_ACCEL_DATA].reg, readBuff,
                                          readSeq[g_model * 2 + READ_ACCEL_DATA].size);
            if (reTrans == false)
            {
                PRINTF("F5\n");
                return -1;
            }

            x = ((int16_t)(((readBuff[0] << 8U) | readBuff[1]))) >> ((g_model == LSM6DSO) ? (0U) : (2U));
            y = ((int16_t)(((readBuff[2] << 8U) | readBuff[3]))) >> ((g_model == LSM6DSO) ? (0U) : (2U));
            z = ((int16_t)(((readBuff[4] << 8U) | readBuff[5]))) >> ((g_model == LSM6DSO) ? (0U) : (2U));

            PRINTF("status_reg = 0x%x , x = %5d , y = %5d , z = %5d \r\n", status0_value, x, y, z);
        }
    }

    PRINTF("\r\nEnd of LPI2C example .\r\n");
    while (1)
    {
    }
}
