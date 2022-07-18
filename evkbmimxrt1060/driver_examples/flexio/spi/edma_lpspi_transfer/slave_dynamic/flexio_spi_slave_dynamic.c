/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_flexio_spi_edma.h"
#include "pin_mux.h"
#include "board.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*Master related*/
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 400000U /*! Transfer baudrate - 400k */

#define MASTER_LPSPI_BASEADDR   (LPSPI1)
#define MASTER_LPSPI_IRQ_HANDLE (LPSPI1_DriverIRQHandler)
#define MASTER_LPSPI_IRQN       (LPSPI1_IRQn)

#define MASTER_LPSPI_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define MASTER_LPSPI_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define MASTER_LPSPI_CLOCK_SELECT (1U)
/* Clock divider for lpspi clock source */
#define MASTER_LPSPI_CLOCK_DIVIDER   (7U)
#define MASTER_LPSPI_CLOCK_FREQUENCY (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (MASTER_LPSPI_CLOCK_DIVIDER + 1U))

/*Slave related*/
#define SLAVE_FLEXIO_SPI_BASEADDR   (FLEXIO2)
#define FLEXIO_SPI_SOUT_PIN         6U
#define FLEXIO_SPI_SIN_PIN          7U
#define FLEXIO_SPI_CLK_PIN          5U
#define FLEXIO_SPI_PCS_PIN          8U
#define FLEXIO_SPI_CS_MONITOR_TIMER 2U

/* Select USB1 PLL (480 MHz) as flexio clock source */
#define SLAVE_FLEXIO_SPI_CLOCK_SELECT (3U)
/* Clock divider for flexio clock source */
#define SLAVE_FLEXIO_SPI_CLOCK_PRE_DIVIDER (4U)
/* Clock divider for flexio clock source */
#define SLAVE_FLEXIO_SPI_CLOCK_DIVIDER (7U)

#define FLEXIO_DMA_REQUEST_SOURCE_BASE        (kDmaRequestMuxFlexIO2Request0Request1)
#define EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR    DMAMUX
#define EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR DMA0
#define FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL       (0U)
#define FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL       (1U)
#define FLEXIO_TX_SHIFTER_INDEX               0U
#define FLEXIO_RX_SHIFTER_INDEX               2U
#define EXAMPLE_TX_DMA_SOURCE                 (kDmaRequestMuxFlexIO2Request0Request1)
#define EXAMPLE_RX_DMA_SOURCE                 (kDmaRequestMuxFlexIO2Request2Request3)
#define EXAMPLE_FLEXIO_IRQHandler             FLEXIO2_IRQHandler
#define EXAMPLE_FLEXIO_IRQ                    FLEXIO2_IRQn


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* LPSPI user callback */
void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(uint8_t masterTxData[TRANSFER_SIZE]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t masterRxData[TRANSFER_SIZE]) = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveTxData[TRANSFER_SIZE])  = {0U};
AT_NONCACHEABLE_SECTION_INIT(uint8_t slaveRxData[TRANSFER_SIZE])  = {0U};

lpspi_master_handle_t g_m_handle;
FLEXIO_SPI_Type spiDev;
flexio_spi_slave_edma_handle_t g_s_handle;

edma_handle_t txHandle;
edma_handle_t rxHandle;

volatile bool isSlaveTransferCompleted  = false;
volatile bool isMasterTransferCompleted = false;
bool isMasterIrqInIntmux                = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        __NOP();
    }
    isMasterTransferCompleted = true;
}

void EXAMPLE_FLEXIO_IRQHandler()
{
    size_t dataCount;

    FLEXIO_ClearTimerStatusFlags(SLAVE_FLEXIO_SPI_BASEADDR, 1U << FLEXIO_SPI_CS_MONITOR_TIMER);
    FLEXIO_SPI_SlaveTransferGetCountEDMA(&spiDev, &g_s_handle, &dataCount);
    /* When slave working in CS continous mode with CPHA = 0, extra byte will be loaded when CS re-asserts. */
    PRINTF("Slave received %d bytes of data\r\n", dataCount - 1U);
    FLEXIO_SPI_SlaveTransferAbortEDMA(&spiDev, &g_s_handle);

    isSlaveTransferCompleted = true;
    /* Disable the monitor timer, in case any noise on bus causes the timer to fire. */
    FLEXIO_DisableTimerStatusInterrupts(SLAVE_FLEXIO_SPI_BASEADDR, 1U << FLEXIO_SPI_CS_MONITOR_TIMER);
}

void EXAMPLE_SetupCsMonitor(void)
{
    /* Use timer FLEXIO_SPI_CS_MONITOR_TIMER to monitor CS pin. The timer enables on CS pin falling edge and
       decrement on pin input. The compare value is 0 which means the compare event occur when CS pin rising. */
    flexio_timer_config_t timerConfig;

    timerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_PININPUT(FLEXIO_SPI_PCS_PIN);
    timerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveHigh;
    timerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    timerConfig.pinConfig       = kFLEXIO_PinConfigOutputDisabled;
    timerConfig.pinSelect       = FLEXIO_SPI_PCS_PIN;
    timerConfig.pinPolarity     = kFLEXIO_PinActiveLow;
    timerConfig.timerMode       = kFLEXIO_TimerModeSingle16Bit;
    timerConfig.timerOutput     = kFLEXIO_TimerOutputOneNotAffectedByReset;
    timerConfig.timerDecrement  = kFLEXIO_TimerDecSrcOnPinInputShiftPinInput;
    timerConfig.timerReset      = kFLEXIO_TimerResetNever;
    timerConfig.timerDisable    = kFLEXIO_TimerDisableOnTimerCompare;
    timerConfig.timerEnable     = kFLEXIO_TimerEnableOnPinRisingEdge;
    timerConfig.timerCompare    = 0U;
    timerConfig.timerStop       = kFLEXIO_TimerStopBitDisabled;
    timerConfig.timerStart      = kFLEXIO_TimerStartBitDisabled;

    FLEXIO_SetTimerConfig(SLAVE_FLEXIO_SPI_BASEADDR, FLEXIO_SPI_CS_MONITOR_TIMER, &timerConfig);
    FLEXIO_ClearTimerStatusFlags(SLAVE_FLEXIO_SPI_BASEADDR, 1U << FLEXIO_SPI_CS_MONITOR_TIMER);
    EnableIRQ(EXAMPLE_FLEXIO_IRQ);
}

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Clock setting for Flexio */
    CLOCK_SetMux(kCLOCK_Flexio2Mux, SLAVE_FLEXIO_SPI_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, SLAVE_FLEXIO_SPI_CLOCK_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Flexio2Div, SLAVE_FLEXIO_SPI_CLOCK_DIVIDER);

    /* Clock setting for Lpspi */
    CLOCK_SetMux(kCLOCK_LpspiMux, MASTER_LPSPI_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, MASTER_LPSPI_CLOCK_DIVIDER);

    PRINTF("LPSPI Master interrupt - FLEXIO SPI Slave edma dynamic transfer example start.\r\n");
    PRINTF("This example use one lpspi instance as master and one flexio spi slave on one board.\r\n");
    PRINTF("Master uses interrupt and slave uses edma way.\r\n");
    PRINTF("Master transfers indefinite amount of data to slave, and the CS signal remains low during transfer.\r\n");
    PRINTF(
        "After master finishes the transfer and re-asserts the CS signal, interrupt is triggered to let user "
        "know.\r\n");
    PRINTF("Slave must configure the transfer size larger than master's.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
    PRINTF("LPSPI_master -- FLEXIO_SPI_slave   \r\n");
    PRINTF("   CLK      --    CLK  \r\n");
    PRINTF("   PCS      --    PCS  \r\n");
    PRINTF("   SOUT     --    SIN  \r\n");
    PRINTF("   SIN      --    SOUT \r\n");

    uint32_t errorCount;
    uint32_t i;
    lpspi_master_config_t masterConfig;
    flexio_spi_slave_config_t slaveConfig;
    lpspi_transfer_t masterXfer;
    flexio_spi_transfer_t slaveXfer;
    edma_config_t config;

    /*Master config*/
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate = TRANSFER_BAUDRATE;
    masterConfig.whichPcs = MASTER_LPSPI_PCS_FOR_INIT;

    LPSPI_MasterInit(MASTER_LPSPI_BASEADDR, &masterConfig, MASTER_LPSPI_CLOCK_FREQUENCY);

    /* Slave config */
    FLEXIO_SPI_SlaveGetDefaultConfig(&slaveConfig);

    spiDev.flexioBase      = SLAVE_FLEXIO_SPI_BASEADDR;
    spiDev.SDOPinIndex     = FLEXIO_SPI_SOUT_PIN;
    spiDev.SDIPinIndex     = FLEXIO_SPI_SIN_PIN;
    spiDev.SCKPinIndex     = FLEXIO_SPI_CLK_PIN;
    spiDev.CSnPinIndex     = FLEXIO_SPI_PCS_PIN;
    spiDev.shifterIndex[0] = FLEXIO_TX_SHIFTER_INDEX;
    spiDev.shifterIndex[1] = FLEXIO_RX_SHIFTER_INDEX;
    spiDev.timerIndex[0]   = 0U;
    FLEXIO_SPI_SlaveInit(&spiDev, &slaveConfig);

    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        masterTxData[i] = i % 256U;
        masterRxData[i] = 0U;

        slaveTxData[i] = ~masterTxData[i];
        slaveRxData[i] = 0U;
    }

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* Init DMAMUX */
    DMAMUX_Init(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR);

    /* Set channel for FLEXIO */
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL, EXAMPLE_TX_DMA_SOURCE);
    DMAMUX_SetSource(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL, EXAMPLE_RX_DMA_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL);
    DMAMUX_EnableChannel(EXAMPLE_FLEXIO_SPI_DMAMUX_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL);
#endif

    /* Init the EDMA module */
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, &config);

    EDMA_CreateHandle(&txHandle, EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL);
    EDMA_CreateHandle(&rxHandle, EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL);

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_TX_DMA_LPSPI_CHANNEL, EXAMPLE_TX_DMA_SOURCE);
    EDMA_SetChannelMux(EXAMPLE_FLEXIO_SPI_DMA_LPSPI_BASEADDR, FLEXIO_SPI_RX_DMA_LPSPI_CHANNEL, EXAMPLE_RX_DMA_SOURCE);
#endif

    /* Set up master and slave handle */
    FLEXIO_SPI_SlaveTransferCreateHandleEDMA(&spiDev, &g_s_handle, NULL, NULL, &txHandle, &rxHandle);
    LPSPI_MasterTransferCreateHandle(MASTER_LPSPI_BASEADDR, &g_m_handle, LPSPI_MasterUserCallback, NULL);

    /* Set up the monitor of CS pin. */
    EXAMPLE_SetupCsMonitor();

    /* Set slave ready to trasnfer TRANSFER_SIZE byte of data */
    slaveXfer.txData   = slaveTxData;
    slaveXfer.rxData   = slaveRxData;
    slaveXfer.dataSize = TRANSFER_SIZE;
    slaveXfer.flags    = kFLEXIO_SPI_8bitMsb | kFLEXIO_SPI_csContinuous;

    FLEXIO_SPI_SlaveTransferEDMA(&spiDev, &g_s_handle, &slaveXfer);
    FLEXIO_EnableTimerStatusInterrupts(SLAVE_FLEXIO_SPI_BASEADDR, 1U << FLEXIO_SPI_CS_MONITOR_TIMER);

    /* Set master to transfer first half of TRANSFER_SIZE byte of data. */
    masterXfer.txData      = masterTxData;
    masterXfer.rxData      = masterRxData;
    masterXfer.dataSize    = TRANSFER_SIZE / 2U;
    masterXfer.configFlags = MASTER_LPSPI_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous;

    LPSPI_MasterTransferNonBlocking(MASTER_LPSPI_BASEADDR, &g_m_handle, &masterXfer);

    /* Wait transfer complete. */
    while (!(isSlaveTransferCompleted && isMasterTransferCompleted))
    {
    }

    isSlaveTransferCompleted  = false;
    isMasterTransferCompleted = false;

    /* Set slave ready to trasnfer TRANSFER_SIZE byte of data */
    slaveXfer.txData = &slaveTxData[TRANSFER_SIZE / 2U];
    slaveXfer.rxData = &slaveRxData[TRANSFER_SIZE / 2U];
    FLEXIO_SPI_SlaveTransferEDMA(&spiDev, &g_s_handle, &slaveXfer);
    FLEXIO_EnableTimerStatusInterrupts(SLAVE_FLEXIO_SPI_BASEADDR, 1U << FLEXIO_SPI_CS_MONITOR_TIMER);

    /* Set master to transfer the second half of TRANSFER_SIZE byte of data. */
    masterXfer.txData = &masterTxData[TRANSFER_SIZE / 2U];
    masterXfer.rxData = &masterRxData[TRANSFER_SIZE / 2U];

    LPSPI_MasterTransferNonBlocking(MASTER_LPSPI_BASEADDR, &g_m_handle, &masterXfer);

    /* Wait transfer complete. */
    while (!(isSlaveTransferCompleted && isMasterTransferCompleted))
    {
    }

    errorCount = 0U;
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        if (slaveTxData[i] != masterRxData[i])
        {
            errorCount++;
        }

        if (masterTxData[i] != slaveRxData[i])
        {
            errorCount++;
        }
    }
    if (errorCount == 0U)
    {
        PRINTF("LPSPI master <-> FLEXIO SPI slave transfer all data matched!\r\n");
    }
    else
    {
        PRINTF("Error occurred in LPSPI master <-> FLEXIO SPI slave transfer!\r\n");
    }

    LPSPI_Deinit(MASTER_LPSPI_BASEADDR);
    FLEXIO_SPI_SlaveDeinit(&spiDev);

    PRINTF("\r\nEnd of Example. \r\n");

    while (1)
    {
    }
}
