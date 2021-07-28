/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpi2c.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_lpit.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_smc.h"
#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SMC SMC
/* Accelerometer I2C address*/
#define ACCEL_I2C_ADDR 0x1CU
/* Accelerometer Device identification register read value*/
#define ACCEL_WHO_AM_I 0xC7U
/* Accelerometer Reset PIN */
#define ACCEL_RESET_GPIO (GPIOB)
#define ACCEL_RESET_PIN  (9U)
/* FXOS8700 and MMA8451 have the same register address */
#define ACCEL_REG_OUT_X_MSB    0x01
#define ACCEL_REG_WHO_AM_I     0x0D
#define ACCEL_REG_CTRL1        0x2A
#define ACCEL_REG_CTRL2        0x2B
#define ACCEL_REG_XYZ_DATA_CFG 0x0E

#define LPI2C_READ           1
#define LPI2C_WRITE          0
#define LPI2C_CMD_TX         0
#define LPI2C_CMD_RX         1
#define LPI2C_CMD_STOP       2
#define LPI2C_CMD_START_SEND 4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_ACCEL_Init(void);
static void DEMO_ACCEL_WriteReg(uint8_t reg, uint8_t value);
static void DEMO_ACCEL_ReadData(void);

void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *param);
/*******************************************************************************
 * Variables
 *******************************************************************************/
static uint8_t recvData[6];

static uint32_t sendDataCmd[] = {
    (LPI2C_CMD_START_SEND << 8) | (ACCEL_I2C_ADDR << 1) | LPI2C_WRITE, /* Device address*/
    (LPI2C_CMD_TX << 8) | ACCEL_REG_OUT_X_MSB,                         /* Sub-address */
    (LPI2C_CMD_START_SEND << 8) | (ACCEL_I2C_ADDR << 1) | LPI2C_READ,
    (LPI2C_CMD_RX << 8) | (sizeof(recvData) - 1), /* Receive 6bytes: OUT_X_MSB ~ OUT_Z_LSB, 6 regs*/
    (LPI2C_CMD_STOP << 8) | 0                     /* Stop */
};

static volatile bool gDmaDone            = false;
static volatile bool g_i2cCompletionFlag = false;
static volatile bool g_i2cNakFlag        = false;

edma_tcd_t tcds[2];

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void DEMO_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    CLOCK_SetIpSrc(kCLOCK_Lpuart1, kCLOCK_IpSrcSircAsync);
    uartClkSrcFreq = CLOCK_GetIpFreq(kCLOCK_Lpuart1);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t completionStatus, void *param)
{
    /* Signal transfer success when received success status. */
    if (completionStatus == kStatus_Success)
    {
        g_i2cCompletionFlag = true;
    }
    /* Signal transfer success when received success status. */
    if (completionStatus == kStatus_LPI2C_Nak)
    {
        g_i2cNakFlag = true;
    }
}

/*  ACCEL write register wrap function */
static void DEMO_ACCEL_WriteReg(uint8_t reg, uint8_t value)
{
    lpi2c_master_transfer_t masterXfer;

    memset(&masterXfer, 0, sizeof(lpi2c_master_transfer_t));
    masterXfer.slaveAddress   = ACCEL_I2C_ADDR;
    masterXfer.direction      = kLPI2C_Write;
    masterXfer.subaddress     = reg;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &value;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;

    /* Send master data to slave */
    LPI2C_MasterTransferNonBlocking(DEMO_LPI2C_PERIPHERAL, &DEMO_LPI2C_masterHandle, &masterXfer);

    /* Wait for transfer completed. */
    while ((!g_i2cNakFlag) && (!g_i2cCompletionFlag))
    {
    }
    g_i2cCompletionFlag = false;
    g_i2cNakFlag        = false;
}

/* Init the accelerometer */
static void DEMO_ACCEL_Init(void)
{
    uint8_t rxData;
    lpi2c_master_transfer_t masterXfer;

    memset(&masterXfer, 0, sizeof(lpi2c_master_transfer_t));
    masterXfer.slaveAddress   = ACCEL_I2C_ADDR;
    masterXfer.direction      = kLPI2C_Read;
    masterXfer.subaddress     = ACCEL_REG_WHO_AM_I;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = &rxData;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kLPI2C_TransferDefaultFlag;
    LPI2C_MasterTransferNonBlocking(DEMO_LPI2C_PERIPHERAL, &DEMO_LPI2C_masterHandle, &masterXfer);

    /* Wait for transfer completed. */
    while ((!g_i2cNakFlag) && (!g_i2cCompletionFlag))
    {
    }
    g_i2cCompletionFlag = false;
    g_i2cNakFlag        = false;

    if (rxData != ACCEL_WHO_AM_I)
    {
        PRINTF("Cannot find the Accelerometer chip\r\n");
        assert(false);
    }

    /* Put ACCEL into standby mode */
    DEMO_ACCEL_WriteReg(ACCEL_REG_CTRL1, 0x0U);
    /* Configure the data range */
    DEMO_ACCEL_WriteReg(ACCEL_REG_XYZ_DATA_CFG, 0x01U);
    /* Put ACCEL out of the standby, active to sampling, normal read mode */
    DEMO_ACCEL_WriteReg(ACCEL_REG_CTRL1, 0xDU);
}

static void DEMO_ACCEL_ReadData(void)
{
    edma_transfer_config_t transferConfig;
    edma_tcd_t *linkTcd = NULL;

    /* Get a 32-byte aligned TCD pointer. */
    edma_tcd_t *tcd = (edma_tcd_t *)((uint32_t)(&tcds[1]) & (~0x1FU));

    /* Source and dest address, all 1Bytes(8bit) data width */
    transferConfig.srcAddr          = LPI2C_MasterGetRxFifoAddress(DEMO_LPI2C_PERIPHERAL);
    transferConfig.srcTransferSize  = kEDMA_TransferSize1Bytes;
    transferConfig.destAddr         = (uint32_t)recvData;
    transferConfig.destTransferSize = kEDMA_TransferSize1Bytes;
    /* Offset applied to current address to form next transfer address */
    transferConfig.srcOffset  = 0;
    transferConfig.destOffset = 1;
    /* 1bytes per DMA request */
    transferConfig.minorLoopBytes  = 1;
    transferConfig.majorLoopCounts = sizeof(recvData);

    /*
     * For chips have separate I2C dma request, use two channel to send command and receive data.
     * For others shared dma request, use one edma channel to send command and the same channel to
     * receive data from i2c,  in this case receive is linked after send.
     */
    if (FSL_FEATURE_LPI2C_HAS_SEPARATE_DMA_RX_TX_REQn(base))
    {
        /* Put this receive transfer on its own DMA channel. */
        EDMA_SetTransferConfig(DEMO_DMA_DMA_BASEADDR, DEMO_DMA_CH1_DMA_CHANNEL, &transferConfig, NULL);
    }
    else
    {
        /* Create a software TCD, which will be chained after send data */
        EDMA_TcdReset(tcd);
        EDMA_TcdSetTransferConfig(tcd, &transferConfig, NULL);
        linkTcd = tcd;
    }

    /* Source and dest address, all 4Bytes(32bit) data width */
    transferConfig.srcAddr          = (uint32_t)sendDataCmd;
    transferConfig.srcTransferSize  = kEDMA_TransferSize4Bytes;
    transferConfig.destAddr         = LPI2C_MasterGetTxFifoAddress(DEMO_LPI2C_PERIPHERAL);
    transferConfig.destTransferSize = kEDMA_TransferSize4Bytes;
    /* Offset applied to current address to form next transfer address */
    transferConfig.srcOffset  = 4;
    transferConfig.destOffset = 0;
    /* 4bytes per DMA request */
    transferConfig.minorLoopBytes  = 4;
    transferConfig.majorLoopCounts = sizeof(sendDataCmd) / sizeof(uint32_t);

    EDMA_SetTransferConfig(DEMO_DMA_DMA_BASEADDR, DEMO_DMA_CH0_DMA_CHANNEL, &transferConfig, linkTcd);

    /* Start dma DEMO_DMA_CH0_DMA_CHANNEL transfer */
    EDMA_EnableChannelRequest(DEMO_DMA_DMA_BASEADDR, DEMO_DMA_CH0_DMA_CHANNEL);

    /* For separate rx/tx DMA requests, enable rx channel */
    if (FSL_FEATURE_LPI2C_HAS_SEPARATE_DMA_RX_TX_REQn(base))
    {
        EDMA_EnableChannelRequest(DEMO_DMA_DMA_BASEADDR, DEMO_DMA_CH1_DMA_CHANNEL);
    }
}

static void DEMO_EnterVlps(void)
{
    status_t ret = kStatus_Success;
    ret          = SMC_SetPowerModeVlps(DEMO_SMC);
    if (ret == kStatus_SMC_StopAbort)
    {
        PRINTF("Enter VLPS mode aborted!\r\n");
    }
}

/*!
 * @Brief LPIT IRQ handler
 */
void LPIT_IRQHandler(void)
{
    if (LPIT_GetStatusFlags(DEMO_LPIT_PERIPHERAL) & LPIT_MSR_TIF0_MASK)
    {
        /* Check if last receive finished */
        if (EDMA_GetChannelStatusFlags(DEMO_DMA_DMA_BASEADDR, DEMO_DMA_CH0_DMA_CHANNEL) & 0x1U)
        {
            EDMA_ClearChannelStatusFlags(DEMO_DMA_DMA_BASEADDR, DEMO_DMA_CH0_DMA_CHANNEL, kEDMA_DoneFlag);
            gDmaDone = true;
        }
        /* Clear interrupt flag.*/
        LPIT_ClearStatusFlags(DEMO_LPIT_PERIPHERAL, LPIT_MSR_TIF0_MASK);

        /* Initiate a transfer */
        DEMO_ACCEL_ReadData();
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief example of LPI2C working in VLPS with async DMA
 *
 * The basic idea of this use case is demonstrate the
 * LPI2C working in VLPS mode without CPU interaction
 * to save power consumption. The working flow is defined as:
 * LPIT -> LPI2C Tx (send Multi-read request for accelerometer
 * data) -> LPI2C Rx (receive the data from FXOS8700)
 */
int main(void)
{
    int16_t xData, yData, zData;

    gpio_pin_config_t pin_config;
    uint32_t i;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    DEMO_InitDebugConsole();

    /* Reset sensor by reset pin*/
    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic  = 1;
    GPIO_PinInit(ACCEL_RESET_GPIO, ACCEL_RESET_PIN, &pin_config);
    GPIO_PinWrite(ACCEL_RESET_GPIO, ACCEL_RESET_PIN, 1);
    /* Delay to ensure reliable sensor reset */
    for (i = 0; i < SystemCoreClock / 1000U; i++)
    {
        __NOP();
    }
    GPIO_PinWrite(ACCEL_RESET_GPIO, ACCEL_RESET_PIN, 0);

    /* Delay to wait sensor stable after reset */
    for (i = 0; i < SystemCoreClock / 100U; i++)
    {
        __NOP();
    }

    BOARD_InitBootPeripherals();

    /* Init the FXOS8700 accelerometer */
    DEMO_ACCEL_Init();

    /* Enable all power mode*/
    SMC_SetPowerModeProtection(DEMO_SMC, kSMC_AllowPowerModeAll);

    PRINTF("\r\nLPI2C_VLPS demo start...\r\n");

    /* Firstly, initiate a transfer */
    DEMO_ACCEL_ReadData();

    while (1)
    {
        /* Enter VLPS */
        while (!gDmaDone)
        {
            DEMO_EnterVlps();
        }
        gDmaDone = false;

        /* Calculate X, Y, Z value */
        xData = ((int16_t)(((recvData[0] * 255U) | recvData[1]))) / 4;
        yData = ((int16_t)(((recvData[2] * 255U) | recvData[3]))) / 4;
        zData = ((int16_t)(((recvData[4] * 255U) | recvData[5]))) / 4;

        /* Output data*/
        PRINTF("X:%5d,  Y:%5d,  Z:%5d\r\n", xData, yData, zData);
    }
}
