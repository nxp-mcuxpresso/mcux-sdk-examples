/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i3c.h"
#include "fsl_icm42688p.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER             I3C0
#define EXAMPLE_I2C_BAUDRATE       400000
#define EXAMPLE_I3C_OD_BAUDRATE    3125000U
#define EXAMPLE_I3C_PP_BAUDRATE    6250000U
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_TIME_OUT_INDEX         100000000U
#define SENSOR_MIPI_VENDOR_ID      0x235U

#ifndef EXAMPLE_I2C_BAUDRATE
#define EXAMPLE_I2C_BAUDRATE 400000
#endif
#ifndef EXAMPLE_I3C_OD_BAUDRATE
#define EXAMPLE_I3C_OD_BAUDRATE 6250000
#endif
#ifndef EXAMPLE_I3C_PP_BAUDRATE
#define EXAMPLE_I3C_PP_BAUDRATE 12500000
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void i3c_master_ibi_callback(I3C_Type *base,
                                    i3c_master_handle_t *handle,
                                    i3c_ibi_type_t ibiType,
                                    i3c_ibi_state_t ibiState);
static void i3c_master_callback(I3C_Type *base, i3c_master_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
i3c_master_handle_t g_i3c_m_handle;
uint8_t g_ibiBuff[10U];
static uint8_t g_ibiUserBuff[10U];
static uint8_t g_ibiUserBuffUsed            = 0;
static volatile bool g_masterCompletionFlag = false;
static volatile bool g_ibiWonFlag           = false;
static volatile status_t g_completionStatus = kStatus_Success;
icm42688p_handle_t icmp42688p_handle;
/*******************************************************************************
 * Code
 ******************************************************************************/
status_t I3C_WriteSensor(uint8_t deviceAddress, uint8_t regAddress, uint8_t *regData, size_t dataSize)
{
    i3c_master_transfer_t masterXfer;
    status_t result        = kStatus_Success;
    g_ibiWonFlag           = false;
    g_masterCompletionFlag = false;
    g_completionStatus     = kStatus_Success;

    memset(&masterXfer, 0, sizeof(masterXfer));

    /* Set register data */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.subaddress     = regAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = regData;
    masterXfer.dataSize       = dataSize;
    masterXfer.flags          = kI3C_TransferDefaultFlag;

    result = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    uint32_t timeout = 0U;
    /* Wait for transfer completed. */
    while ((!g_ibiWonFlag) && (!g_masterCompletionFlag))
    {
        timeout++;
        if ((g_completionStatus != kStatus_Success) || (timeout > I3C_TIME_OUT_INDEX))
        {
            break;
        }
        __NOP();
    }
    result = g_completionStatus;
    if (timeout > I3C_TIME_OUT_INDEX)
    {
        result = kStatus_Timeout;
    }

    return result;
}

status_t I3C_ReadSensor(uint8_t deviceAddress, uint8_t regAddress, uint8_t *regData, size_t dataSize)
{
    i3c_master_transfer_t masterXfer;
    status_t result        = kStatus_Success;
    g_ibiWonFlag           = false;
    g_masterCompletionFlag = false;
    g_completionStatus     = kStatus_Success;

    memset(&masterXfer, 0, sizeof(masterXfer));

    /* Set register data */
    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kI3C_Read;
    masterXfer.busType        = kI3C_TypeI3CSdr;
    masterXfer.subaddress     = regAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = regData;
    masterXfer.dataSize       = dataSize;
    masterXfer.flags          = kI3C_TransferDefaultFlag;

    result = I3C_MasterTransferNonBlocking(EXAMPLE_MASTER, &g_i3c_m_handle, &masterXfer);

    uint32_t timeout = 0U;
    /* Wait for transfer completed. */
    while ((!g_ibiWonFlag) && (!g_masterCompletionFlag))
    {
        timeout++;
        if ((g_completionStatus != kStatus_Success) || (timeout > I3C_TIME_OUT_INDEX))
        {
            break;
        }
        __NOP();
    }
    result = g_completionStatus;
    if (timeout > I3C_TIME_OUT_INDEX)
    {
        result = kStatus_Timeout;
    }

    return result;
}

static void i3c_master_ibi_callback(I3C_Type *base,
                                    i3c_master_handle_t *handle,
                                    i3c_ibi_type_t ibiType,
                                    i3c_ibi_state_t ibiState)
{
    switch (ibiType)
    {
        case kI3C_IbiNormal:
            if (ibiState == kI3C_IbiDataBuffNeed)
            {
                handle->ibiBuff = g_ibiBuff;
            }
            else
            {
                memcpy(g_ibiUserBuff, (void *)handle->ibiBuff, handle->ibiPayloadSize);
                g_ibiUserBuffUsed = handle->ibiPayloadSize;
            }
            break;

        default:
            assert(false);
            break;
    }
}

static void i3c_master_callback(I3C_Type *base, i3c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_masterCompletionFlag = true;
    }

    if (status == kStatus_I3C_IBIWon)
    {
        g_ibiWonFlag = true;
    }

    g_completionStatus = status;
}

static const i3c_master_transfer_callback_t masterCallback = {
    .slave2Master = NULL, .ibiCallback = i3c_master_ibi_callback, .transferComplete = i3c_master_callback};

/*!
 * @brief Main function
 */
int main(void)
{
    status_t result = kStatus_Success;
    uint8_t addressList[3] = {0x08, 0x09, 0x0A};
    uint8_t slaveAddr = 0;
    i3c_master_config_t masterConfig;
    i3c_device_info_t *devList;
    uint8_t devCount;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, false);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Attach main clock to I3C, 150MHz / 6 = 25MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 6U, false);
    CLOCK_AttachClk(kMAIN_CLK_to_I3CFCLK);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI3C master read sensor data example.\r\n");

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = EXAMPLE_I3C_PP_BAUDRATE;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = EXAMPLE_I3C_OD_BAUDRATE;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);

    I3C_MasterTransferCreateHandle(EXAMPLE_MASTER, &g_i3c_m_handle, &masterCallback, NULL);

    PRINTF("\r\nI3C master do dynamic address assignment to the sensor slave.\r\n");

    /* Reset dynamic address. */
    result = I3C_WriteSensor(0x7E, 0x06, NULL, 0);
    if (result != kStatus_Success)
    {
        return -1;
    }

    result = I3C_MasterProcessDAA(EXAMPLE_MASTER, addressList, sizeof(addressList));
    if (result != kStatus_Success)
    {
        return -1;
    }

    devList = I3C_MasterGetDeviceListAfterDAA(EXAMPLE_MASTER, &devCount);
    for (uint8_t devIndex = 0; devIndex < devCount; devIndex++)
    {
        if (devList[devIndex].vendorID == SENSOR_MIPI_VENDOR_ID)
        {
            slaveAddr = devList[devIndex].dynamicAddr;
            break;
        }
    }

    if (slaveAddr == 0U)
    {
        PRINTF("I3C master dynamic address assignment failed.\r\n");
        return -1;
    }
    PRINTF("I3C master dynamic address assignment done, sensor address: 0x%2x.\r\n", slaveAddr);

    icm42688p_config_t sensorConfig;
    sensorConfig.Sensor_WriteTransfer = (sensor_write_transfer_func_t)I3C_WriteSensor;
    sensorConfig.Sensor_ReadTransfer  = (sensor_read_transfer_func_t)I3C_ReadSensor;
    sensorConfig.sensorAddress        = slaveAddr;
    sensorConfig.isReset              = true;

    result = ICM42688P_Init(&icmp42688p_handle, &sensorConfig);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSensor reset failed.\r\n");
        return -1;
    }

    PRINTF("\r\nSensor reset is done, re-assgin dynamic address.\r\n");

    /* Reset dynamic address. */
    result = I3C_WriteSensor(0x7E, 0x06, NULL, 0);
    if (result != kStatus_Success)
    {
        return -1;
    }

    result = I3C_MasterProcessDAA(EXAMPLE_MASTER, addressList, sizeof(addressList));
    if (result != kStatus_Success)
    {
        return -1;
    }

    devList = I3C_MasterGetDeviceListAfterDAA(EXAMPLE_MASTER, &devCount);
    slaveAddr = 0;
    for (uint8_t devIndex = 0; devIndex < devCount; devIndex++)
    {
        if (devList[devIndex].vendorID == SENSOR_MIPI_VENDOR_ID)
        {
            slaveAddr = devList[devIndex].dynamicAddr;
            break;
        }
    }

    if (slaveAddr == 0U)
    {
        PRINTF("I3C master dynamic address assignment failed.\r\n");
        return -1;
    }
    PRINTF("I3C master dynamic address assignment done, sensor address: 0x%2x.\r\n", slaveAddr);

    i3c_register_ibi_addr_t ibiRecord = {.address = {slaveAddr}, .ibiHasPayload = true};
    I3C_MasterRegisterIBI(EXAMPLE_MASTER, &ibiRecord);

    result = ICM42688P_EnableSensors(&icmp42688p_handle);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSensor enable failed.\r\n");
        return -1;
    }

    result = ICM42688P_ConfigureTapDetectIBI(&icmp42688p_handle);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nEnable TAP detect IBI failed.\r\n");
        return -1;
    }

    uint8_t bankSel = 0;
    /* Select bank 0 to read sensor data.*/
    result = ICM42688P_WriteReg(&icmp42688p_handle, BANK_SEL, &bankSel, 1);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSelect sensor bank 0 failure.\r\n");
        return -1;
    }

    /* Wait for 1ms for FIFO data ready with 1KHz sensor sample rate. */
    SDK_DelayAtLeastUs(1000, SystemCoreClock);

    icm42688p_sensor_data_t sensorData = {0};
    while (!g_ibiWonFlag)
    {
        result = ICM42688P_ReadSensorData(&icmp42688p_handle, &sensorData);
        if (result != kStatus_Success)
        {
            if (result == kStatus_NoData)
            {
                /* Wait for 1ms for FIFO data ready with 1KHz sensor sample rate. */
                SDK_DelayAtLeastUs(1000, SystemCoreClock);
                continue;
            }
            PRINTF("\r\nRead sensor data failed.\r\n");
            return -1;
        }
        PRINTF("\r\nSensor Data: ACCEL X %d, Y %d, Z %d; GYRO X %d, Y %d, Z %d.", sensorData.accelDataX,
               sensorData.accelDataY, sensorData.accelDataZ, sensorData.gyroDataX, sensorData.gyroDataY,
               sensorData.gyroDataZ);
        SDK_DelayAtLeastUs(50000, SystemCoreClock);
    }

    while (1)
    {
        if (g_ibiWonFlag)
        {
            PRINTF("\r\nReceived slave IBI request.");
            for (uint8_t count = 0; count < g_ibiUserBuffUsed; count++)
            {
                PRINTF(" Data 0x%x.", g_ibiUserBuff[count]);
            }
            g_ibiWonFlag = false;
        }
    }
}
