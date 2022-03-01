/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
#include "board.h"
#include "fsl_common.h"
#include "fsl_smc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SMC                 SMC
#define BOARD_ACCEL_I2C_BASEADDR LPI2C0

#define DEMO_ACCEL_I2C_CLOCK_NAME   kCLOCK_Lpi2c0
#define DEMO_ACCEL_I2C_CLOCK_SOURCE kCLOCK_IpSrcSircAsync

#define DEMO_LPI2C_RX_DMA_REQ_SOURCE kDmaRequestMux0LPI2C0Rx
#define DEMO_LPI2C_TX_DMA_REQ_SOURCE kDmaRequestMux0LPI2C0Tx

#define DEMO_LPI2C_CLOCK_FREQUNCY CLOCK_GetIpFreq(DEMO_ACCEL_I2C_CLOCK_NAME)
#define DEMO_LPIT_CLOCK_SOURCE    kCLOCK_IpSrcSircAsync
#define DEMO_LPIT_CLOCK_FREQUNCY  CLOCK_GetIpFreq(kCLOCK_Lpit0)

#define DMAMUX0 DMAMUX
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
static void APP_LPI2C_Init(void);
static void APP_ACCEL_Init(void);
static void APP_ACCEL_WriteReg(uint8_t reg, uint8_t value);
static void APP_DMA_Init(void);
static void APP_LPIT_Init(void);
static void APP_ACCEL_ReadData(void);

static void lpi2c_master_callback(LPI2C_Type *base,
                                  lpi2c_master_handle_t *handle,
                                  status_t completionStatus,
                                  void *param);
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

lpi2c_master_handle_t g_i2c_handle;

/* DMA channels used by i2c */
uint32_t channel0 = 0U;
uint32_t channel1 = 1U;

edma_tcd_t tcds[2];

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_BootClockRUN(void)
{
    const scg_sosc_config_t g_scgSysOscConfig = {.freq        = BOARD_XTAL0_CLK_HZ,
                                                 .enableMode  = kSCG_SysOscEnable | kSCG_SysOscEnableInLowPower,
                                                 .monitorMode = kSCG_SysOscMonitorDisable,
                                                 .div2        = kSCG_AsyncClkDivBy1,
                                                 .workMode    = kSCG_SysOscModeOscLowPower};

    const scg_firc_config_t g_scgFircConfig = {
        .enableMode = kSCG_FircEnable, .div2 = kSCG_AsyncClkDivBy1, .range = kSCG_FircRange48M, .trimConfig = NULL};

    const scg_lpfll_config_t g_scgLpFllConfig = {
        .enableMode = kSCG_LpFllEnable, .div2 = kSCG_AsyncClkDivBy2, .range = kSCG_LpFllRange72M, .trimConfig = NULL};

    const scg_sys_clk_config_t g_sysClkConfigSircSource = {
        .divSlow = kSCG_SysClkDivBy4, .divCore = kSCG_SysClkDivBy1, .src = kSCG_SysClkSrcSirc};

    const scg_sys_clk_config_t g_sysClkConfigNormalRun = {
        .divSlow = kSCG_SysClkDivBy3, .divCore = kSCG_SysClkDivBy1, .src = kSCG_SysClkSrcLpFll};
    const scg_sirc_config_t scgSircConfig = {
        .enableMode = kSCG_SircEnable | kSCG_SircEnableInLowPower | kSCG_SircEnableInStop,
        .div2       = kSCG_AsyncClkDivBy2,
        .range      = kSCG_SircRangeHigh};
    scg_sys_clk_config_t curConfig;

    CLOCK_InitSysOsc(&g_scgSysOscConfig);
    CLOCK_SetXtal0Freq(BOARD_XTAL0_CLK_HZ);

    /* Init Sirc */
    CLOCK_InitSirc(&scgSircConfig);

    /* Change to use SIRC as system clock source to prepare to change FIRCCFG register */
    CLOCK_SetRunModeSysClkConfig(&g_sysClkConfigSircSource);

    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curConfig);
    } while (curConfig.src != g_sysClkConfigSircSource.src);

    /* Init Firc */
    CLOCK_InitFirc(&g_scgFircConfig);

    /* Init LPFLL */
    CLOCK_InitLpFll(&g_scgLpFllConfig);

    /* Use LPFLL as system clock source */
    CLOCK_SetRunModeSysClkConfig(&g_sysClkConfigNormalRun);

    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curConfig);
    } while (curConfig.src != g_sysClkConfigNormalRun.src);

    SystemCoreClock = 72000000U;
}

/* Initialize debug console. */
void APP_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    CLOCK_SetIpSrc(kCLOCK_Lpuart0, kCLOCK_IpSrcSircAsync);
    uartClkSrcFreq = CLOCK_GetIpFreq(kCLOCK_Lpuart0);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

static void lpi2c_master_callback(LPI2C_Type *base,
                                  lpi2c_master_handle_t *handle,
                                  status_t completionStatus,
                                  void *param)
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

static void APP_LPI2C_Init(void)
{
    lpi2c_master_config_t masterConfig;

    /* Initialize the LPI2C master peripheral */
    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = 400000U;
    masterConfig.debugEnable = true;
    masterConfig.enableDoze  = true;
    LPI2C_MasterInit(BOARD_ACCEL_I2C_BASEADDR, &masterConfig, DEMO_LPI2C_CLOCK_FREQUNCY);
    LPI2C_MasterTransferCreateHandle(BOARD_ACCEL_I2C_BASEADDR, &g_i2c_handle, lpi2c_master_callback, NULL);

    /* Enable DMA request */
    LPI2C_MasterEnableDMA(BOARD_ACCEL_I2C_BASEADDR, true, true);
}

/*  ACCEL write register wrap function */
static void APP_ACCEL_WriteReg(uint8_t reg, uint8_t value)
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
    LPI2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_i2c_handle, &masterXfer);

    /* Wait for transfer completed. */
    while ((!g_i2cNakFlag) && (!g_i2cCompletionFlag))
    {
    }
    g_i2cCompletionFlag = false;
    g_i2cNakFlag        = false;
}

/* Init the accelerometer */
static void APP_ACCEL_Init(void)
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
    LPI2C_MasterTransferNonBlocking(BOARD_ACCEL_I2C_BASEADDR, &g_i2c_handle, &masterXfer);

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
    APP_ACCEL_WriteReg(ACCEL_REG_CTRL1, 0x0U);
    /* Configure the data range */
    APP_ACCEL_WriteReg(ACCEL_REG_XYZ_DATA_CFG, 0x01U);
    /* Put ACCEL out of the standby, active to sampling, normal read mode */
    APP_ACCEL_WriteReg(ACCEL_REG_CTRL1, 0xDU);
}

static void APP_DMA_Init(void)
{
    edma_config_t userConfig;

    /* Setup dmamux for LPI2C Tx and Rx */
    DMAMUX_Init(DMAMUX0);
    DMAMUX_SetSource(DMAMUX0, channel0, DEMO_LPI2C_TX_DMA_REQ_SOURCE);
    DMAMUX_EnableChannel(DMAMUX0, channel0);
    DMAMUX_SetSource(DMAMUX0, channel1, DEMO_LPI2C_RX_DMA_REQ_SOURCE);
    DMAMUX_EnableChannel(DMAMUX0, channel1);

    /* Setup DMA */
    EDMA_GetDefaultConfig(&userConfig);
    userConfig.enableHaltOnError = false;
    EDMA_Init(DMA0, &userConfig);

    EDMA_ResetChannel(DMA0, channel0);
    EDMA_ResetChannel(DMA0, channel1);

    /* Enable auto stop request and dma aysnc request */
    EDMA_EnableAsyncRequest(DMA0, channel0, true);
    EDMA_EnableAsyncRequest(DMA0, channel1, true);
    EDMA_EnableAutoStopRequest(DMA0, channel0, true);
    EDMA_EnableAutoStopRequest(DMA0, channel1, true);
}

static void APP_ACCEL_ReadData(void)
{
    edma_transfer_config_t transferConfig;
    edma_tcd_t *linkTcd = NULL;

    /* Get a 32-byte aligned TCD pointer. */
    edma_tcd_t *tcd = (edma_tcd_t *)((uint32_t)(&tcds[1]) & (~0x1FU));

    /* Source and dest address, all 1Bytes(8bit) data width */
    transferConfig.srcAddr          = LPI2C_MasterGetRxFifoAddress(BOARD_ACCEL_I2C_BASEADDR);
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
        EDMA_SetTransferConfig(DMA0, channel1, &transferConfig, NULL);
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
    transferConfig.destAddr         = LPI2C_MasterGetTxFifoAddress(BOARD_ACCEL_I2C_BASEADDR);
    transferConfig.destTransferSize = kEDMA_TransferSize4Bytes;
    /* Offset applied to current address to form next transfer address */
    transferConfig.srcOffset  = 4;
    transferConfig.destOffset = 0;
    /* 4bytes per DMA request */
    transferConfig.minorLoopBytes  = 4;
    transferConfig.majorLoopCounts = sizeof(sendDataCmd) / sizeof(uint32_t);

    EDMA_SetTransferConfig(DMA0, channel0, &transferConfig, linkTcd);

    /* Start dma channel0 transfer */
    EDMA_EnableChannelRequest(DMA0, channel0);

    /* For separate rx/tx DMA requests, enable rx channel */
    if (FSL_FEATURE_LPI2C_HAS_SEPARATE_DMA_RX_TX_REQn(base))
    {
        EDMA_EnableChannelRequest(DMA0, channel1);
    }
}
static void APP_LPIT_Init(void)
{
    lpit_config_t lpitConfig;
    lpit_chnl_params_t chnlSetup;

    /* Init LPIT0 module and enable run in debug and low power mode*/
    LPIT_GetDefaultConfig(&lpitConfig);
    lpitConfig.enableRunInDebug = true;
    lpitConfig.enableRunInDoze  = true;
    LPIT_Init(LPIT0, &lpitConfig);

    chnlSetup.timerMode             = kLPIT_PeriodicCounter;
    chnlSetup.chainChannel          = false;
    chnlSetup.triggerSelect         = kLPIT_Trigger_TimerChn0;
    chnlSetup.triggerSource         = kLPIT_TriggerSource_Internal;
    chnlSetup.enableReloadOnTrigger = false;
    chnlSetup.enableStopOnTimeout   = false;
    chnlSetup.enableStartOnTrigger  = false;
    LPIT_SetupChannel(LPIT0, kLPIT_Chnl_0, &chnlSetup);

    /* Set timer0 period: 500ms */
    LPIT_SetTimerPeriod(LPIT0, kLPIT_Chnl_0, MSEC_TO_COUNT(500U, DEMO_LPIT_CLOCK_FREQUNCY));

    LPIT_EnableInterrupts(LPIT0, LPIT_MIER_TIE0_MASK);
    NVIC_EnableIRQ(LPIT0_IRQn);

    LPIT_StartTimer(LPIT0, kLPIT_Chnl_0);
}

static void APP_EnterVlps(void)
{
    status_t ret = kStatus_Success;
    ret          = SMC_SetPowerModeVlps(DEMO_SMC);
    if (ret == kStatus_SMC_StopAbort)
    {
        PRINTF("Enter VLPS mode aborted!\r\n");
    }
}

/*!
 * @Brief LPIT0 CH0 IRQ handler
 */
void LPIT0_IRQHandler(void)
{
    if (LPIT_GetStatusFlags(LPIT0) & LPIT_MSR_TIF0_MASK)
    {
        /* Check if last receive finished */
        if (EDMA_GetChannelStatusFlags(DMA0, channel0) & 0x1U)
        {
            EDMA_ClearChannelStatusFlags(DMA0, channel0, kEDMA_DoneFlag);
            gDmaDone = true;
        }
        /* Clear interrupt flag.*/
        LPIT_ClearStatusFlags(LPIT0, LPIT_MSR_TIF0_MASK);

        /* Initiate a transfer */
        APP_ACCEL_ReadData();
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief example of LPI2C working in VLPS with async DMA
 *
 * The basic idea of this use case is demonstrate the
 * LPI2C working in VLPS mode without CPU interaction
 * to save power consumption. The working flow is defined as:
 * LPIT (every 500ms) -> LPI2C Tx (send Multi-read request
 * for accelerometer data) -> LPI2C Rx (receive the data from FXOS8700)
 */
int main(void)
{
    int16_t xData, yData, zData;

    gpio_pin_config_t pin_config;
    uint32_t i;

    BOARD_InitBootPins();
    APP_BootClockRUN();
    APP_InitDebugConsole();

    /* Set LPI2C clock source, should work in stop mode */
    CLOCK_SetIpSrc(DEMO_ACCEL_I2C_CLOCK_NAME, DEMO_ACCEL_I2C_CLOCK_SOURCE);
    /* Select the SIRC 8M as LPIT clock, SIRC enabled in stop mode */
    CLOCK_SetIpSrc(kCLOCK_Lpit0, DEMO_LPIT_CLOCK_SOURCE);

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

    /* Init LPI2C for master */
    APP_LPI2C_Init();
    /* Init the FXOS8700 accelerometer */
    APP_ACCEL_Init();
    /* Use LPIT to start DMA for LPI2C Tx every 500ms */
    APP_LPIT_Init();
    /* Init eDMA and DMAMUX for I2C Rx/Tx */
    APP_DMA_Init();
    /* Enable all power mode*/
    SMC_SetPowerModeProtection(DEMO_SMC, kSMC_AllowPowerModeAll);

    PRINTF("\r\nLPI2C_VLPS demo start...\r\n");

    /* Firstly, initiate a transfer */
    APP_ACCEL_ReadData();

    while (1)
    {
        /* Enter VLPS */
        while (!gDmaDone)
        {
            APP_EnterVlps();
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
