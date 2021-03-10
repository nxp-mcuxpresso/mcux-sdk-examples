/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_adc.h"
#include "fsl_clock.h"
#include "fsl_power.h"
#include "fsl_dma.h"
#include "fsl_inputmux.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC_BASE                  ADC0
#define DEMO_ADC_SAMPLE_CHANNEL_NUMBER 7U
#define DEMO_ADC_CLOCK_DIVIDER         1U
#define DEMO_DMA_ADC_CHANNEL   0U
#define DEMO_DMA_BASE          DMA0
#define DMA_DESCRIPTOR_NUM     2U
#define DEMO_ADC_DATA_REG_ADDR (uint32_t)(&(DEMO_ADC_BASE->DAT[DEMO_ADC_SAMPLE_CHANNEL_NUMBER]))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void ADC_Configuration(void);
static void DMA_Configfuation(void);
static void NVIC_Configuration(void);

dma_handle_t g_DmaHandleStruct;      /* Handler structure for using DMA. */
uint32_t g_AdcConvResult[1];         /* Keep the ADC conversion resulut moved from ADC data register by DMA. */
volatile bool g_DmaTransferDoneFlag; /* Flag of DMA transfer done trigger by ADC conversion. */
/* DMA descripter table used for ping-pong mode. */
DMA_ALLOCATE_LINK_DESCRIPTORS(s_dma_table, DMA_DESCRIPTOR_NUM);
const uint32_t g_XferConfig =
    DMA_CHANNEL_XFER(true,                          /* Reload link descriptor after current exhaust, */
                     true,                          /* Clear trigger status. */
                     true,                          /* Enable interruptA. */
                     false,                         /* Not enable interruptB. */
                     sizeof(uint32_t),              /* Dma transfer width. */
                     kDMA_AddressInterleave0xWidth, /* Dma source address no interleave  */
                     kDMA_AddressInterleave0xWidth, /* Dma destination address no interleave  */
                     sizeof(uint32_t)               /* Dma transfer byte. */
    );
const uint32_t g_Adc_12bitFullRange = 4096U;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void ADC_ClockPower_Configuration(void)
{
    /* SYSCON power. */
    POWER_DisablePD(kPDRUNCFG_PD_ADC0);     /* Power on the ADC converter. */
    POWER_DisablePD(kPDRUNCFG_PD_VD7_ENA);  /* Power on the analog power supply. */
    POWER_DisablePD(kPDRUNCFG_PD_VREFP_SW); /* Power on the reference voltage source. */
    POWER_DisablePD(kPDRUNCFG_PD_TEMPS);    /* Power on the temperature sensor. */

    CLOCK_EnableClock(kCLOCK_Adc0); /* SYSCON->AHBCLKCTRL[0] |= SYSCON_AHBCLKCTRL_ADC0_MASK; */
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize board hardware. */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Enable the power and clock for ADC. */
    ADC_ClockPower_Configuration();
    PRINTF("ADC DMA example.\r\n");

    /* Configure peripherals. */
    NVIC_Configuration();
    DMA_Configfuation();

#if !(defined(FSL_FEATURE_ADC_HAS_NO_CALIB_FUNC) && FSL_FEATURE_ADC_HAS_NO_CALIB_FUNC)
    uint32_t frequency = 0U;
    /* Calibration after power up. */
#if defined(FSL_FEATURE_ADC_HAS_CALIB_REG) && FSL_FEATURE_ADC_HAS_CALIB_REG
    DEMO_ADC_BASE->CTRL |= ADC_CTRL_BYPASSCAL_MASK;
    frequency = CLOCK_GetFreq(kCLOCK_BusClk);
    if (true == ADC_DoOffsetCalibration(DEMO_ADC_BASE, frequency))
#else
#if defined(SYSCON_ADCCLKDIV_DIV_MASK)
    frequency = CLOCK_GetFreq(DEMO_ADC_CLOCK_SOURCE) / CLOCK_GetClkDivider(kCLOCK_DivAdcClk);
#else
    frequency = CLOCK_GetFreq(DEMO_ADC_CLOCK_SOURCE);
#endif /* SYSCON_ADCCLKDIV_DIV_MASK */
    if (true == ADC_DoSelfCalibration(DEMO_ADC_BASE, frequency))
#endif /* FSL_FEATURE_ADC_HAS_CALIB_REG */
    {
        PRINTF("ADC Calibration Done.\r\n");
    }
    else
    {
        PRINTF("ADC Calibration Failed.\r\n");
    }
#endif /* FSL_FEATURE_ADC_HAS_NO_CALIB_FUNC */

    ADC_Configuration();
    PRINTF("Configuration Done.\r\n");
#if defined(FSL_FEATURE_ADC_HAS_CTRL_RESOL) & FSL_FEATURE_ADC_HAS_CTRL_RESOL
    PRINTF("ADC Full Range: %d\r\n", g_Adc_12bitFullRange);
#endif /* FSL_FEATURE_ADC_HAS_CTRL_RESOL */
    PRINTF("Type in any key to trigger the conversion ...\r\n");

    while (1)
    {
        /* Get the input from terminal and trigger the converter by software. */
        GETCHAR();

        g_DmaTransferDoneFlag = false;
        ADC_DoSoftwareTriggerConvSeqA(DEMO_ADC_BASE); /* Trigger the ADC and start the conversion. */

        /* Wait for the converter & transfer to be done. */
        while (!g_DmaTransferDoneFlag)
        {
        }
        PRINTF("Conversion word : 0x%X\r\n", g_AdcConvResult[0]);
        PRINTF("Conversion value: %d\r\n", (g_AdcConvResult[0] & ADC_DAT_RESULT_MASK) >> ADC_DAT_RESULT_SHIFT);
        PRINTF("\r\n");
    }
}

static void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(DMA0_IRQn);
}

static void ADC_Configuration(void)
{
    adc_config_t adcConfigStruct;
    adc_conv_seq_config_t adcConvSeqConfigStruct;
    adc_result_info_t adcResultInfoStruct;

/* Configure the converter. */
#if defined(FSL_FEATURE_ADC_HAS_CTRL_ASYNMODE) & FSL_FEATURE_ADC_HAS_CTRL_ASYNMODE
    adcConfigStruct.clockMode = kADC_ClockSynchronousMode; /* Using sync clock source. */
#endif                                                     /* FSL_FEATURE_ADC_HAS_CTRL_ASYNMODE */
    adcConfigStruct.clockDividerNumber = DEMO_ADC_CLOCK_DIVIDER;
#if defined(FSL_FEATURE_ADC_HAS_CTRL_RESOL) & FSL_FEATURE_ADC_HAS_CTRL_RESOL
    adcConfigStruct.resolution = kADC_Resolution12bit;
#endif /* FSL_FEATURE_ADC_HAS_CTRL_RESOL */
#if defined(FSL_FEATURE_ADC_HAS_CTRL_BYPASSCAL) & FSL_FEATURE_ADC_HAS_CTRL_BYPASSCAL
    adcConfigStruct.enableBypassCalibration = false;
#endif /* FSL_FEATURE_ADC_HAS_CTRL_BYPASSCAL */
#if defined(FSL_FEATURE_ADC_HAS_CTRL_TSAMP) & FSL_FEATURE_ADC_HAS_CTRL_TSAMP
    adcConfigStruct.sampleTimeNumber = 0U;
#endif /* FSL_FEATURE_ADC_HAS_CTRL_TSAMP */
#if defined(FSL_FEATURE_ADC_HAS_CTRL_LPWRMODE) & FSL_FEATURE_ADC_HAS_CTRL_LPWRMODE
    adcConfigStruct.enableLowPowerMode = false;
#endif /* FSL_FEATURE_ADC_HAS_CTRL_LPWRMODE */
#if defined(FSL_FEATURE_ADC_HAS_TRIM_REG) & FSL_FEATURE_ADC_HAS_TRIM_REG
    adcConfigStruct.voltageRange = kADC_HighVoltageRange;
#endif /* FSL_FEATURE_ADC_HAS_TRIM_REG */
    ADC_Init(DEMO_ADC_BASE, &adcConfigStruct);

#if !(defined(FSL_FEATURE_ADC_HAS_NO_INSEL) && FSL_FEATURE_ADC_HAS_NO_INSEL)
    /* Use the temperature sensor input to channel 0. */
    ADC_EnableTemperatureSensor(DEMO_ADC_BASE, true);
#endif /* FSL_FEATURE_ADC_HAS_NO_INSEL. */

    /* Enable channel DEMO_ADC_SAMPLE_CHANNEL_NUMBER's conversion in Sequence A. */
    adcConvSeqConfigStruct.channelMask =
        (1U << DEMO_ADC_SAMPLE_CHANNEL_NUMBER); /* Includes channel DEMO_ADC_SAMPLE_CHANNEL_NUMBER. */
    adcConvSeqConfigStruct.triggerMask      = 0U;
    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = false;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachSequence; /* Enable the interrupt/DMA trigger. */
    ADC_SetConvSeqAConfig(DEMO_ADC_BASE, &adcConvSeqConfigStruct);
    ADC_EnableConvSeqA(DEMO_ADC_BASE, true); /* Enable the conversion sequence A. */

    /* Clear the result register. */
    ADC_DoSoftwareTriggerConvSeqA(DEMO_ADC_BASE);
    while (!ADC_GetChannelConversionResult(DEMO_ADC_BASE, DEMO_ADC_SAMPLE_CHANNEL_NUMBER, &adcResultInfoStruct))
    {
    }
    ADC_GetConvSeqAGlobalConversionResult(DEMO_ADC_BASE, &adcResultInfoStruct);

    /* Enable DMA trigger when Seq A conversion done. */
    ADC_EnableInterrupts(DEMO_ADC_BASE, kADC_ConvSeqAInterruptEnable);
}

/* Software ISR for DMA transfer done. */
void DEMO_DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (true == transferDone)
    {
        g_DmaTransferDoneFlag = true;
    }
}

static void DMA_Configfuation(void)
{
    dma_channel_config_t dmaChannelConfigStruct;
    dma_channel_trigger_t dmaChannelTriggerStruct;

    /* Init DMA. This must be set before INPUTMUX_Init() for DMA peripheral reset will clear the mux setting. */
    DMA_Init(DEMO_DMA_BASE);

    /* Configure DMAMUX. */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, DEMO_DMA_ADC_CHANNEL, kINPUTMUX_Adc0SeqaIrqToDma);

    /* Configure DMA. */
    DMA_EnableChannel(DEMO_DMA_BASE, DEMO_DMA_ADC_CHANNEL);
    DMA_CreateHandle(&g_DmaHandleStruct, DEMO_DMA_BASE, DEMO_DMA_ADC_CHANNEL);
    DMA_SetCallback(&g_DmaHandleStruct, DEMO_DMA_Callback, NULL);
    /*
     * Configure the DMA trigger:
     * The DATAVALID of ADC will trigger the interrupt. This signal is also for thie DMA triger, which is changed 0 ->
     * 1.
     */
    dmaChannelTriggerStruct.burst = kDMA_EdgeBurstTransfer1;
    dmaChannelTriggerStruct.type  = kDMA_RisingEdgeTrigger;
    dmaChannelTriggerStruct.wrap  = kDMA_NoWrap;

    /* Prepare and submit the transfer. */
    DMA_PrepareChannelTransfer(&dmaChannelConfigStruct,              /* DMA channel transfer configurationstructure. */
                               (void *)DEMO_ADC_DATA_REG_ADDR,       /* DMA transfer source address. */
                               (void *)g_AdcConvResult,              /* DMA transfer destination address. */
                               g_XferConfig,                         /* Xfer configuration */
                               kDMA_MemoryToMemory,                  /* DMA transfer type. */
                               &dmaChannelTriggerStruct,             /* DMA channel trigger configurations. */
                               (dma_descriptor_t *)&(s_dma_table[0]) /* Address of next descriptor. */
    );
    DMA_SubmitChannelTransfer(&g_DmaHandleStruct, &dmaChannelConfigStruct);

    /* Set two DMA descripters to use ping-pong mode.  */
    DMA_SetupDescriptor(&(s_dma_table[0]), g_XferConfig, (void *)DEMO_ADC_DATA_REG_ADDR, (void *)g_AdcConvResult,
                        (dma_descriptor_t *)&(s_dma_table[1]));
    DMA_SetupDescriptor(&(s_dma_table[1]), g_XferConfig, (void *)DEMO_ADC_DATA_REG_ADDR, (void *)g_AdcConvResult,
                        (dma_descriptor_t *)&(s_dma_table[0]));
}
