/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
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
#include "fsl_edma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC16_BASEADDR      ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_CHANNEL       26U                     /* Temperature sensor. */
#define ADC16_RESULT_REG_ADDR    (uint32_t)(&ADC0->R[0]) /* Get adc16 result register address */

#define DEMO_DMA_BASEADDR   DMA0
#define DEMO_DMA_IRQ_ID     DMA0_DMA16_IRQn
#define DEMO_DMA_CHANNEL    0U
#define DEMO_DMA_ADC_SOURCE kDmaRequestMux0ADC0

#define DEMO_DMAMUX_BASEADDR DMAMUX0

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
 * @brief Initialize the EDMA for async mode.
 *
 */
static void EDMA_Configuration(void);

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

/*!
 * @brief Callback function for EDMA.
 *
 */
static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t g_adc16SampleDataArray[DEMO_ADC16_SAMPLE_COUNT]; /* ADC value array */
static uint32_t g_avgADCValue = 0U;                              /* Average ADC value */
edma_handle_t g_EDMA_Handle;                                     /* Edma handler */
edma_transfer_config_t g_transferConfig;                         /* Edma transfer config */
const uint32_t g_Adc16_16bitFullRange = 65536U;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ConfigTriggerSource(void)
{
    /* Configure SIM for ADC hw trigger source selection */
    SIM->SOPT7 |= SIM_SOPT7_ADC0TRGSEL(14) | SIM_SOPT7_ADC0ALTTRGEN(1);
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

static void EDMA_Configuration(void)
{
    edma_config_t userConfig;

    /* Configure DMAMUX */
    DMAMUX_Init(DEMO_DMAMUX_BASEADDR);
    DMAMUX_SetSource(DEMO_DMAMUX_BASEADDR, DEMO_DMA_CHANNEL, DEMO_DMA_ADC_SOURCE); /* Map ADC source to channel 0 */
    DMAMUX_EnableChannel(DEMO_DMAMUX_BASEADDR, DEMO_DMA_CHANNEL);

    EDMA_GetDefaultConfig(&userConfig);
    EDMA_Init(DEMO_DMA_BASEADDR, &userConfig);
    EDMA_CreateHandle(&g_EDMA_Handle, DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL);
    EDMA_SetCallback(&g_EDMA_Handle, Edma_Callback, NULL);
    EDMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
                         (void *)g_adc16SampleDataArray, sizeof(uint32_t), sizeof(uint32_t),
                         sizeof(g_adc16SampleDataArray), kEDMA_PeripheralToMemory);
    EDMA_SubmitTransfer(&g_EDMA_Handle, &g_transferConfig);
    /* Enable interrupt when transfer is done. */
    EDMA_EnableChannelInterrupts(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, kEDMA_MajorInterruptEnable);
#if defined(FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT) && FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT
    /* Enable async DMA request. */
    EDMA_EnableAsyncRequest(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, true);
#endif /* FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT */
    /* Enable transfer. */
    EDMA_StartTransfer(&g_EDMA_Handle);
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

static void Edma_Callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
    /* Stop trigger */
    LPTMR_StopTimer(DEMO_LPTMR_BASE);
    /* Clear Edma interrupt flag */
    EDMA_ClearChannelStatusFlags(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, kEDMA_InterruptFlag);
    /* Setup transfer */
    EDMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
                         (void *)g_adc16SampleDataArray, sizeof(uint32_t), sizeof(uint32_t),
                         sizeof(g_adc16SampleDataArray), kEDMA_PeripheralToMemory);
    EDMA_SetTransferConfig(DEMO_DMA_BASEADDR, DEMO_DMA_CHANNEL, &g_transferConfig, NULL);
    /* Enable transfer */
    EDMA_StartTransfer(&g_EDMA_Handle);
}

int main(void)
{
    /* Initialize hardware */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    /* Initialize Led */
    LED_INIT();

    PRINTF("ADC LOW POWER ASYNC EDMA DEMO\r\n");

    /* Set to allow entering vlps mode */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlp);
    /* Initialize ADC */
    ADC16_Configuration();
    /* Initialize EDMA */
    EDMA_Configuration();
    /* Initialize the HW trigger source */
    LPTMR_Configuration();
    /* Initialize SIM for ADC hw trigger source selection */
    BOARD_ConfigTriggerSource();

    PRINTF("ADC Full Range: %d\r\n", g_Adc16_16bitFullRange);
    while (1)
    {
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
