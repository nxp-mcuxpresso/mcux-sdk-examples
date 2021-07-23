/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_smc.h"
#include "fsl_pmc.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "peripherals.h"
#include "board.h"
#include <stdlib.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ADC16_RESULT_REG_ADDR (uint32_t)(&ADC0->R[0]) /* Get adc16 result register address */

#define LED_INIT()   LED_RED_INIT(LOGIC_LED_OFF)
#define LED_TOGGLE() LED_RED_TOGGLE()
#define DEMO_ADC16_SAMPLE_COUNT 16U /* The ADC16 sample count */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ConfigTriggerSource(void);
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
static void EDMA_Configuration(void)
{
    EDMA_SetCallback(&DEMO_eDMA_ADC0_Handle, Edma_Callback, NULL);
    EDMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
                         (void *)g_adc16SampleDataArray, sizeof(uint32_t), sizeof(uint32_t),
                         sizeof(g_adc16SampleDataArray), kEDMA_PeripheralToMemory);
    EDMA_SubmitTransfer(&DEMO_eDMA_ADC0_Handle, &g_transferConfig);
#if defined(FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT) && FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT
    /* Enable async DMA request. */
    EDMA_EnableAsyncRequest(DEMO_EDMA_DMA_BASEADDR, DEMO_EDMA_ADC0_DMA_CHANNEL, true);
#endif /* FSL_FEATURE_EDMA_ASYNCHRO_REQUEST_CHANNEL_COUNT */
    /* Enable transfer. */
    EDMA_StartTransfer(&DEMO_eDMA_ADC0_Handle);
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
    LPTMR_StopTimer(DEMO_LPTMR_PERIPHERAL);
    /* Clear Edma interrupt flag */
    EDMA_ClearChannelStatusFlags(DEMO_EDMA_DMA_BASEADDR, DEMO_EDMA_ADC0_DMA_CHANNEL, kEDMA_InterruptFlag);
    /* Setup transfer */
    EDMA_PrepareTransfer(&g_transferConfig, (void *)ADC16_RESULT_REG_ADDR, sizeof(uint32_t),
                         (void *)g_adc16SampleDataArray, sizeof(uint32_t), sizeof(uint32_t),
                         sizeof(g_adc16SampleDataArray), kEDMA_PeripheralToMemory);
    EDMA_SetTransferConfig(DEMO_EDMA_DMA_BASEADDR, DEMO_EDMA_ADC0_DMA_CHANNEL, &g_transferConfig, NULL);
    /* Enable transfer */
    EDMA_StartTransfer(&DEMO_eDMA_ADC0_Handle);
}

int main(void)
{
    /* Initialize hardware */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitPeripherals();
    BOARD_InitDebugConsole();
    /* Initialize Led */
    LED_INIT();

    PRINTF("ADC LOW POWER ASYNC EDMA PERIPHERAL DEMO\r\n");

    /* Set to allow entering vlps mode */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlp);
    /* Initialize EDMA */
    EDMA_Configuration();
    /* Initialize SIM for ADC hw trigger source selection */
    BOARD_ConfigTriggerSource();

    PRINTF("ADC Full Range: %d\r\n", g_Adc16_16bitFullRange);
    while (1)
    {
        /* Start low power timer */
        LPTMR_StartTimer(DEMO_LPTMR_PERIPHERAL);
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
