/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_smc.h"
#include "fsl_pmc.h"
#include "fsl_adc16.h"
#include "fsl_dmamux.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lptmr.h"
#include <stdlib.h>
#include "fsl_dma.h"

#include "fsl_xbar.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC16_BASEADDR      ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_CHANNEL       26U
#define ADC16_RESULT_REG_ADDR    (uint32_t)(&ADC0->R[0]) /* Get ADC16 result register address */

#define DEMO_DMA_BASEADDR         DMA0
#define DEMO_DMA_IRQ_ID           DMA0_IRQn
#define DEMO_DMA_IRQ_HANDLER_FUNC DMA0_IRQHandler
#define DEMO_DMA_CHANNEL          0U
#define DEMO_DMA_ADC_SOURCE       kDmaRequestMux0SarADC

#define DEMO_DMAMUX_BASEADDR DMAMUX

#define DEMO_LPTMR_BASE LPTMR0

#define LED_INIT()   LED_RED_INIT(LOGIC_LED_OFF)
#define LED_TOGGLE() LED_RED_TOGGLE()

#define DEMO_LPTMR_COMPARE_VALUE 100U /* Low Power Timer interrupt time in miliseconds */
#define DEMO_ADC16_SAMPLE_COUNT  16U  /* The ADC16 sample count */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ConfigTriggerSource(void);
/*!
 * @brief Initialize the LPTimer.
 *
 */
static void LPTMR_Configuration(void);

/*!
 * @brief Initialize the ADCx for HW trigger.
 *
 */
static void ADC16_Configuration(void);

/*!
 * @brief Process ADC values.
 *
 */
static void ProcessSampleData(void);

/*!
 * @brief Initialize the DMA for async mode.
 *
 */
static void DMA_Configuration(void);

/*!
 * @brief Wait clock stable.
 *
 */
static void APP_WaitClockStable(void);

/*!
 * @brief Delay a bit.
 *
 */
static void Delay(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t g_adc16SampleDataArray[DEMO_ADC16_SAMPLE_COUNT]; /* ADC value array */
static uint32_t g_avgADCValue = 0U;                              /* Average ADC value */
dma_handle_t g_DMA_Handle;                                       /* Dma handler */
dma_transfer_config_t g_transferConfig;                          /* Dma transfer config */
const uint32_t g_Adc16_16bitFullRange = 65536U;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ConfigTriggerSource(void)
{
    /* Configure SIM for ADC hw trigger source selection */
    SIM->CTRL_REG |= 0x8U;

    /* Init xbar module. */
    XBAR_Init(XBAR);
    /* Configure the XBAR signal connections. */
    XBAR_SetSignalsConnection(XBAR, kXBAR_InputLptmr0Output, kXBAR_OutputAdcTrgA);
}
/* Enable the trigger source of LPTimer */
static void LPTMR_Configuration(void)
{
    lptmr_config_t lptmrUserConfig;

    LPTMR_GetDefaultConfig(&lptmrUserConfig);
    /* Init LPTimer driver */
    LPTMR_Init(DEMO_LPTMR_BASE, &lptmrUserConfig);

    /* Set the LPTimer period */
    LPTMR_SetTimerPeriod(DEMO_LPTMR_BASE, DEMO_LPTMR_COMPARE_VALUE);
}

static void ADC16_Configuration(void)
{
    adc16_config_t adcUserConfig;
    adc16_channel_config_t adcChnConfig;
    /*
     * Initialization ADC for
     * 16bit resolution, interrupt mode, hw trigger enabled.
     * normal convert speed, VREFH/L as reference,
     * disable continuous convert mode.
     */
    ADC16_GetDefaultConfig(&adcUserConfig);
    adcUserConfig.resolution                 = kADC16_Resolution16Bit;
    adcUserConfig.enableContinuousConversion = false;
    adcUserConfig.clockSource                = kADC16_ClockSourceAsynchronousClock;
    adcUserConfig.enableAsynchronousClock    = true;

    adcUserConfig.longSampleMode = kADC16_LongSampleCycle24;
    adcUserConfig.enableLowPower = true;
#if ((defined BOARD_ADC_USE_ALT_VREF) && BOARD_ADC_USE_ALT_VREF)
    adcUserConfig.referenceVoltageSource = kADC16_ReferenceVoltageSourceValt;
#endif
    ADC16_Init(DEMO_ADC16_BASEADDR, &adcUserConfig);

#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION
    /* Auto calibration */
    if (kStatus_Success == ADC16_DoAutoCalibration(DEMO_ADC16_BASEADDR))
    {
        PRINTF("ADC16_DoAutoCalibration() Done.\r\n");
    }
    else
    {
        PRINTF("ADC16_DoAutoCalibration() Failed.\r\n");
    }
#endif

    adcChnConfig.channelNumber = DEMO_ADC16_CHANNEL;
#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
    adcChnConfig.enableDifferentialConversion = false;
#endif
    adcChnConfig.enableInterruptOnConversionCompleted = false;
    /* Configure channel 0 */
    ADC16_SetChannelConfig(DEMO_ADC16_BASEADDR, DEMO_ADC16_CHANNEL_GROUP, &adcChnConfig);
    /* Enable hardware trigger  */
    ADC16_EnableHardwareTrigger(DEMO_ADC16_BASEADDR, true);
    /* Enable DMA */
    ADC16_EnableDMA(DEMO_ADC16_BASEADDR, true);
}

static void DMA_Configuration(void)
{
    /* Configure DMAMUX */
    DMAMUX_Init(DEMO_DMAMUX_BASEADDR);
    DMAMUX_SetSource(DEMO_DMAMUX_BASEADDR, DEMO_DMA_CHANNEL, DEMO_DMA_ADC_SOURCE); /* Map ADC source to channel 0 */
    DMAMUX_EnableChannel(DEMO_DMAMUX_BASEADDR, DEMO_DMA_CHANNEL);

    DMA_Init(DEMO_DMA_BASEADDR);
    DMA_CreateHandle(&g_DMA_Handle, DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL);
    DMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
                        (void *)g_adc16SampleDataArray, sizeof(uint32_t), sizeof(g_adc16SampleDataArray),
                        kDMA_PeripheralToMemory);
    /* Setup transfer */
    DMA_SetTransferConfig(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, &g_transferConfig);
    /* Enable interrupt when transfer is done. */
    DMA_EnableInterrupts(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL);
    /* Enable async DMA request. */
    DMA_EnableAsyncRequest(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, true);
    /* Forces a single read/write transfer per request. */
    DMA_EnableCycleSteal(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, true);
    /* Enable transfer. */
    DMA_StartTransfer(&g_DMA_Handle);
    /* Enable IRQ. */
    NVIC_EnableIRQ(DEMO_DMA_IRQ_ID);
}

static void ProcessSampleData(void)
{
    uint32_t i = 0U;

    g_avgADCValue = 0;
    /* Get average adc value */
    for (i = 0; i < DEMO_ADC16_SAMPLE_COUNT; i++)
    {
        g_avgADCValue += g_adc16SampleDataArray[i];
    }
    g_avgADCValue = g_avgADCValue / DEMO_ADC16_SAMPLE_COUNT;

    /* Reset old value */
    for (i = 0; i < DEMO_ADC16_SAMPLE_COUNT; i++)
    {
        g_adc16SampleDataArray[i] = 0U;
    }
}

static void APP_WaitClockStable(void)
{
    /* Delay a bit */
    Delay();
#if defined(FSL_FEATURE_MCG_HAS_PLL) && (FSL_FEATURE_MCG_HAS_PLL)
    /* Set to PEE mode clock */
    if (kMCG_ModePBE == CLOCK_GetMode())
    {
        /* Wait for PLL lock. */
        while (!(kMCG_Pll0LockFlag & CLOCK_GetStatusFlags()))
        {
        }
        CLOCK_SetPeeMode();
    }
#endif
}

static void Delay(void)
{
    uint32_t i = 0;

    for (i = 0; i < 6000; i++)
    {
        __NOP();
    }
}

void DEMO_DMA_IRQ_HANDLER_FUNC(void)
{
    /* Stop trigger */
    LPTMR_StopTimer(DEMO_LPTMR_BASE);
    /* Clear transaction done interrupt flag */
    DMA_ClearChannelStatusFlags(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, kDMA_TransactionsDoneFlag);
    /* Setup transfer */
    DMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
                        (void *)g_adc16SampleDataArray, sizeof(uint32_t), sizeof(g_adc16SampleDataArray),
                        kDMA_PeripheralToMemory);
    DMA_SetTransferConfig(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, &g_transferConfig);
    SDK_ISR_EXIT_BARRIER;
}

int main(void)
{
    /* Initialize hardware */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Initialize Led */
    LED_INIT();

    PRINTF("ADC LOW POWER ASYNC DMA DEMO\r\n");

    /* Set to allow entering vlps mode */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlp);
    /* Initialize ADC */
    ADC16_Configuration();
    /* Initialize DMA */
    DMA_Configuration();
    /* Initialize the HW trigger source */
    LPTMR_Configuration();
    /* Initialize SIM for ADC hw trigger source selection */
    BOARD_ConfigTriggerSource();

    PRINTF("ADC Full Range: %d\r\n", g_Adc16_16bitFullRange);
    while (1)
    {
        PRINTF("Please press any key to trigger ADC conversion.\r\n");
        (void)GETCHAR();

        /* Start low power timer */
        LPTMR_StartTimer(DEMO_LPTMR_BASE);
        /* Enter to Very Low Power Stop Mode */
        SMC_SetPowerModeVlps(SMC);
        /* Wait clock stable after wake up */
        APP_WaitClockStable();
        /* Deinit debug console */
        DbgConsole_Deinit();
        /* Init debug console after wake up */
        BOARD_InitDebugConsole();
        /* Toggle led */
        LED_TOGGLE();
        /* Process ADC value */
        ProcessSampleData();

        PRINTF("ADC value: %d\r\n", g_avgADCValue);

        /* Wait for data in uart fifo flushed */
        Delay();
    }
}
