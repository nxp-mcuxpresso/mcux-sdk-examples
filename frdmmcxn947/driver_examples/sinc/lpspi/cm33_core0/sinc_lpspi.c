/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_lpspi.h"
#include "fsl_sinc.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Master related */
#define DEMO_LPSPI_MASTER_BASEADDR   (LPSPI1)
#define DEMO_LPSPI_MASTER_IRQN       (LP_FLEXCOMM1_IRQn)
#define DEMO_LPSPI_MASTER_IRQHandler (LP_FLEXCOMM1_IRQHandler)

#define DEMO_LPSPI_MASTER_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define DEMO_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)
#define DEMO_LPSPI_DEFAULT_VALUE           (0x7FU)

#define LPSPI_MASTER_CLK_FREQ CLOCK_GetLPFlexCommClkFreq(1U)

#define DEMO_SINC                 (SINC0)
#define DEMO_SINC_IRQn            (SINC_FILTER_IRQn)
#define DEMO_SINC_IRQ_HANDLER     SINC_FILTER_IRQHandler
#define DEMO_SINC_MOD_CLK_DIVIDER (4UL)
#define DEMO_SINC_OverSampleRatio (139U)
#define TRANSFER_SIZE     64U      /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 5000000U /*! Transfer baudrate - 10M */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_InitLpspi(void);
static void DEMO_InitSinc(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t masterTxData[TRANSFER_SIZE] = {0};

volatile uint32_t masterTxCount;
volatile uint8_t g_masterRxWatermark;
volatile uint8_t g_masterFifoSize;

volatile bool isMasterTransferCompleted = false;

volatile uint32_t tcrReg;
volatile uint32_t sincFifoData;
volatile bool dataReady;
/*******************************************************************************
 * Code
 ******************************************************************************/

void DEMO_LPSPI_MASTER_IRQHandler(void)
{
    /*Write the word to TX register*/
    DEMO_LPSPI_MASTER_BASEADDR->TCR = tcrReg;
    LPSPI_WriteData(DEMO_LPSPI_MASTER_BASEADDR, masterTxData[masterTxCount]);
    ++masterTxCount;

    if (masterTxCount == TRANSFER_SIZE)
    {
        masterTxCount = 0;
    }

    if (dataReady)
    {
        LPSPI_DisableInterrupts(DEMO_LPSPI_MASTER_BASEADDR, kLPSPI_AllInterruptEnable);
    }
    DisableIRQ(DEMO_LPSPI_MASTER_IRQN);
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_SINC_IRQ_HANDLER(void)
{
    if ((SINC_GetInterruptStatus(DEMO_SINC) & kSINC_CH0ConvCompleteIntStatus) != 0UL)
    {
        while (!SINC_CheckChannelResultDataReady(DEMO_SINC, kSINC_Channel0))
            ;
        SINC_LatchChannelDebugProceduce(DEMO_SINC, kSINC_Channel0);
        while (!SINC_CheckChannelDebugDataValid(DEMO_SINC, kSINC_Channel0))
            ;
        sincFifoData = SINC_ReadChannelResultData(DEMO_SINC, kSINC_Channel0);
        dataReady    = true;
        SINC_ClearInterruptStatus(DEMO_SINC, kSINC_CH0ConvCompleteIntStatus);
    }
}

int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to FLEXCOMM1 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom1Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);

    /* Attach FRO_HF to SINC. */
    CLOCK_AttachClk(kFRO_HF_to_SINCFILT);

    /* attach TRACECLKDIV to TRACE */
    CLOCK_SetClkDiv(kCLOCK_DivTraceClk, 2U);
    CLOCK_AttachClk(kTRACE_DIV_to_TRACE);
	
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nSINC LPSPI Example.\r\n");

	DEMO_InitSinc();
    DEMO_InitLpspi();
    while (1)
    {
        PRINTF("\r\nPress any key to trigger sinc conversion!\r\n");
        GETCHAR();
        SINC_AffirmChannelSoftwareTrigger(DEMO_SINC, (1UL << (uint32_t)kSINC_Channel0));
        LPSPI_EnableInterrupts(DEMO_LPSPI_MASTER_BASEADDR,
                               kLPSPI_TxInterruptEnable | kLPSPI_TransferCompleteInterruptEnable);
        EnableIRQ(DEMO_LPSPI_MASTER_IRQN);
        while (!dataReady)
        {
        }
        dataReady = false;
        PRINTF("\r\nSINC Result:0x%X\r\n", sincFifoData);
    }
}

static void DEMO_InitLpspi(void)
{
    uint32_t srcClock_Hz;
    uint32_t i;
    lpspi_which_pcs_t whichPcs;
    uint8_t txWatermark;
    lpspi_master_config_t masterConfig;

    /* Master config. */
    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate                      = TRANSFER_BAUDRATE;
    masterConfig.whichPcs                      = DEMO_LPSPI_MASTER_PCS_FOR_INIT;
    masterConfig.betweenTransferDelayInNanoSec = 0;
    masterConfig.lastSckToPcsDelayInNanoSec    = 0;
    masterConfig.pcsToSckDelayInNanoSec        = 0;

    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(DEMO_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

    /******************Set up master transfer******************/
    /* Set up the transfer data. */
    PRINTF("LPSPI default output value: 0x%x\r\n", DEMO_LPSPI_DEFAULT_VALUE);
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        masterTxData[i] = DEMO_LPSPI_DEFAULT_VALUE;
    }

    isMasterTransferCompleted = false;
    masterTxCount             = 0;
    whichPcs                  = DEMO_LPSPI_MASTER_PCS_FOR_INIT;

    /* The TX and RX FIFO sizes are always the same. */
    g_masterFifoSize = 1;

    /* Set the RX and TX watermarks to reduce the ISR times. */
    if (g_masterFifoSize > 1)
    {
        txWatermark         = 1;
        g_masterRxWatermark = g_masterFifoSize - 2;
    }
    else
    {
        txWatermark         = 0;
        g_masterRxWatermark = 0;
    }

    LPSPI_SetPCSContinous(DEMO_LPSPI_MASTER_BASEADDR, true);
    DEMO_LPSPI_MASTER_BASEADDR->CFGR1 |= LPSPI_CFGR1_NOSTALL_MASK;
    DEMO_LPSPI_MASTER_BASEADDR->TCR |= LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK | LPSPI_TCR_RXMSK_MASK;

    LPSPI_SetFifoWatermarks(DEMO_LPSPI_MASTER_BASEADDR, txWatermark, g_masterRxWatermark);
    LPSPI_Enable(DEMO_LPSPI_MASTER_BASEADDR, true);

    /* Flush FIFO , clear status , disable all the inerrupts. */
    LPSPI_FlushFifo(DEMO_LPSPI_MASTER_BASEADDR, true, true);
    LPSPI_ClearStatusFlags(DEMO_LPSPI_MASTER_BASEADDR, kLPSPI_AllStatusFlag);
    LPSPI_DisableInterrupts(DEMO_LPSPI_MASTER_BASEADDR, kLPSPI_AllInterruptEnable);
    LPSPI_EnableInterrupts(DEMO_LPSPI_MASTER_BASEADDR,
                           kLPSPI_TxInterruptEnable | kLPSPI_TransferCompleteInterruptEnable);

    LPSPI_SelectTransferPCS(DEMO_LPSPI_MASTER_BASEADDR, whichPcs);

    tcrReg = DEMO_LPSPI_MASTER_BASEADDR->TCR;
    tcrReg = DEMO_LPSPI_MASTER_BASEADDR->TCR;

    IRQ_SetPriority(DEMO_LPSPI_MASTER_IRQN, 1U);
}

static void DEMO_InitSinc(void)
{
    sinc_config_t sincConfig;
    sinc_channel_config_t sincChannel0Config;
    sinc_channel_input_option_t sincChannel0InputOption;
    sinc_channel_conv_option_t sincChannel0ConvOption;
    sinc_channel_protection_option_t sincChannel0ProtectionOption;

    sincChannel0InputOption.inputBitFormat = kSINC_InputBit_FormatExternalBitstream;
    sincChannel0InputOption.inputBitDelay  = kSINC_InputBit_DelayDisabled;
    sincChannel0InputOption.inputBitSource = kSINC_InputBit_SourceExternalBitstream;
    sincChannel0InputOption.inputClkEdge   = kSINC_InputClk_EdgeBoth;
    sincChannel0InputOption.inputClkSource = kSINC_InputClk_SourceMclkOut0;

    sincChannel0ConvOption.convMode              = kSINC_ConvMode_Single;
    sincChannel0ConvOption.convTriggerSource     = kSINC_ConvTrig_SoftPosEdge;
    sincChannel0ConvOption.enableChPrimaryFilter = true;
    sincChannel0ConvOption.pfBiasSign            = kSINC_PF_BiasPositive;
    sincChannel0ConvOption.pfHpfAlphaCoeff       = kSINC_PF_HPFAlphaCoeff0;
    sincChannel0ConvOption.pfOrder               = kSINC_PF_FirstOrder;
    sincChannel0ConvOption.pfShiftDirection      = kSINC_PF_ShiftRight;
    sincChannel0ConvOption.u16pfOverSampleRatio  = (DEMO_SINC_OverSampleRatio - 1U);
    sincChannel0ConvOption.u32pfBiasValue        = 0U;
    sincChannel0ConvOption.u8pfShiftBitsNum      = 0U;

    sincChannel0ProtectionOption.bEnableCadBreakSignal = false;
    sincChannel0ProtectionOption.bEnableLmtBreakSignal = false;
    sincChannel0ProtectionOption.bEnableScdBreakSignal = false;
    sincChannel0ProtectionOption.cadLimitThreshold     = kSINC_Cad_Disabled;
    sincChannel0ProtectionOption.limitDetectorMode     = kSINC_Lmt_Disabled;
    sincChannel0ProtectionOption.scdOperateMode        = kSINC_Scd_OperateDisabled;
    sincChannel0ProtectionOption.scdOption             = kSINC_Scd_DetectRepeating0And1;
    sincChannel0ProtectionOption.u32HighLimitThreshold = 0XFFFFFFUL;
    sincChannel0ProtectionOption.u32LowLimitThreshold  = 0x0UL;
    sincChannel0ProtectionOption.u8ScdLimitThreshold   = 2U;
    sincChannel0ProtectionOption.zcdOperateMode        = kSINC_ZCD_Disabled;

    sincChannel0Config.bEnableChannel     = true;
    sincChannel0Config.bEnableFifo        = false;
    sincChannel0Config.bEnablePrimaryDma  = false;
    sincChannel0Config.chConvOption       = &sincChannel0ConvOption;
    sincChannel0Config.chInputOption      = &sincChannel0InputOption;
    sincChannel0Config.chProtectionOption = &sincChannel0ProtectionOption;
    sincChannel0Config.dataFormat         = kSINC_LeftJustifiedUnsigned;
    sincChannel0Config.u8FifoWaterMark    = 1U;

    SINC_GetDefaultConfig(&sincConfig);

    sincConfig.modClkDivider          = DEMO_SINC_MOD_CLK_DIVIDER;
    sincConfig.clockPreDivider        = kSINC_ClkPrescale1;
    sincConfig.channelsConfigArray[0] = &sincChannel0Config;
    sincConfig.enableMaster           = true;
    sincConfig.disableDozeMode        = false;
    sincConfig.disableModClk1Output   = true;
    sincConfig.disableModClk2Output   = true;
    sincConfig.disableModClk0Output   = false;
    SINC_Init(DEMO_SINC, &sincConfig);

    while (!SINC_CheckChannelReadyForConv(DEMO_SINC, kSINC_Channel0))
    {
    }

    SINC_EnableInterrupts(DEMO_SINC, kSINC_CH0ConvCompleteIntEnable);
    SINC_SetChannelDebugOutput(DEMO_SINC, kSINC_Channel0, kSINC_Debug_CicRawData);
    EnableIRQ(DEMO_SINC_IRQn);
    IRQ_SetPriority(DEMO_SINC_IRQn, 0U);
}
