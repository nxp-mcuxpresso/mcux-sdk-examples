/*
 * Copyright 2019, 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i3c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER             I3C1
#define EXAMPLE_I2C_BAUDRATE       400000
#define I3C_MASTER_CLOCK_ROOT      kCLOCK_Root_I3c1
#define I3C_MASTER_CLOCK_GATE      kCLOCK_I3c1
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetIpFreq(I3C_MASTER_CLOCK_ROOT)
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x6A
#define WAIT_TIME                  1000
#define I3C_DATA_LENGTH            1
#define LSM6DSO_WHOAMI_REG_ADDR 0x0FU
#define LSM6DSO_WHOAMI_VALUE 0x6CU

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    i3c_master_config_t masterConfig;
    i3c_master_transfer_t masterXfer;
    i3c_master_daa_baudrate_t daaBaudRate;
    status_t result      = kStatus_Success;
    uint8_t who_am_i_i3c = 0x00;

    /* clang-format off */
    const clock_root_config_t i3cClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };
    /* clang-format on */

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(I3C_MASTER_CLOCK_ROOT, &i3cClkCfg);
    CLOCK_EnableClock(I3C_MASTER_CLOCK_GATE);
    CLOCK_EnableClock(kCLOCK_I3c1);

    PRINTF("\r\nI3C master read sensor data example.\r\n");

    PRINTF("\r\nStart to do I3C master transfer in SDR mode.\r\n");

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = 4000000U;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = 1500000U;
    masterConfig.enableOpenDrainHigh          = false;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);

    /* Reset dynamic address before DAA */
    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress   = 0x7EU; /* Broadcast address */
    masterXfer.subaddress     = 0x06U; /* CCC command RSTDAA */
    masterXfer.subaddressSize = 1U;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.flags          = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse    = kI3C_IbiRespAckMandatory;
    result                    = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    uint8_t addressList[8] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    /* Lsm6dso sensor RM: Address assignment (DAA or ENTDA) must be performed with I2C Fast Mode Plus Timing. When the
       slave is addressed, the I2C slave is disabled and the timing is compatible with I3C specifications. */
    daaBaudRate.sourceClock_Hz   = I3C_MASTER_CLOCK_FREQUENCY;
    daaBaudRate.i3cOpenDrainBaud = 500000;
    daaBaudRate.i3cPushPullBaud  = 1000000;
    result                       = I3C_MasterProcessDAASpecifiedBaudrate(EXAMPLE_MASTER, addressList, 8, &daaBaudRate);
    if (result != kStatus_Success)
    {
        return -1;
    }

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.subaddress     = LSM6DSO_WHOAMI_REG_ADDR;
    masterXfer.subaddressSize = 1;
    masterXfer.slaveAddress = 0x30;
    masterXfer.data         = &who_am_i_i3c;
    masterXfer.dataSize     = I3C_DATA_LENGTH;
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (result != kStatus_Success)
    {
        return -1;
    }

    if (who_am_i_i3c == LSM6DSO_WHOAMI_VALUE)
    {
        PRINTF(
            "\r\nSuccess to read WHO_AM_I register value from LSDM6DSO on board in I3C SDR mode, the value is 0x%02X. "
            "\r\n",
            who_am_i_i3c);
    }

    while (1)
    {
    }
}
