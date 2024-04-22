/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_edma.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_qtmr.h"

#include "fsl_ele_base_api.h"
#include "fsl_trdc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* When CM33 set TRDC, CM7 must NOT require TRDC ownership from ELE */
#define CM33_SET_TRDC 0U

#define ELE_TRDC_AON_ID    0x74
#define ELE_TRDC_WAKEUP_ID 0x78
#define ELE_CORE_CM33_ID   0x1
#define ELE_CORE_CM7_ID    0x2

/*
 * Set ELE_STICK_FAILED_STS to 0 when ELE status check is not required,
 * which is useful when debug reset, where the core has already get the
 * TRDC ownership at first time and ELE is not able to release TRDC
 * ownership again for the following TRDC ownership request.
 */
#define ELE_STICK_FAILED_STS 1

#if ELE_STICK_FAILED_STS
#define ELE_IS_FAILED(x) (x != kStatus_Success)
#else
#define ELE_IS_FAILED(x) false
#endif
/* The QTMR instance/channel used for board */
#define BOARD_QTMR_BASEADDR              TMR4
#define BOARD_QTMR_INPUT_CAPTURE_CHANNEL kQTMR_Channel_0
#define BOARD_QTMR_PWM_CHANNEL           kQTMR_Channel_1
#define QTMR_PWM_OUTPUT_FREQUENCY        50000
#define QTMR_DUTYCYCLE_PERCENT           50
#define QTMR_CounterInputPin             kQTMR_Counter0InputPin

/* QTMR Clock source divider for Ipg clock source, the value of two macros below should be aligned. */
#define QTMR_PRIMARY_SOURCE       (kQTMR_ClockDivide_8)
#define QTMR_CLOCK_SOURCE_DIVIDER (8U)

/* Get source clock for QTMR driver */
#define QTMR_SOURCE_CLOCK (CLOCK_GetRootClockFreq(kCLOCK_Root_Bus_Wakeup) / QTMR_CLOCK_SOURCE_DIVIDER)

#define EXAMPLE_QTMR_DMA (DMA4)

#define QTMR_EDMA_REQUEST_CAPT_SOURCE  kDma4RequestMuxQTIMER4CaptTimer0
#define QTMR_EDMA_REQUEST_CMPLD_SOURCE kDma4RequestMuxQTIMER4Cmpld1Timer0Cmpld2Timer1

#define BOARD_SetEDMAConfig(config)                                              \
    {                                                                            \
        static edma_channel_config_t channelConfig = {                           \
            .enableMasterIDReplication = true,                                   \
            .securityLevel             = kEDMA_ChannelSecurityLevelSecure,       \
            .protectionLevel           = kEDMA_ChannelProtectionLevelPrivileged, \
        };                                                                       \
        config.enableMasterIdReplication = true;                                 \
        config.channelConfig[0]          = &channelConfig;                       \
    }


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_handle_t g_EDMA_Handle;
volatile bool g_Transfer_Done                                 = false;
AT_NONCACHEABLE_SECTION_INIT(volatile uint16_t g_Cmpld1Value) = 0U;
AT_NONCACHEABLE_SECTION_INIT(volatile uint16_t g_Cmpld2Value) = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void TRDC_EDMA4_ResetPermissions(void)
{
    uint8_t i, j;

    /* Set the master domain access configuration for eDMA4 */
    trdc_non_processor_domain_assignment_t edma4Assignment;

    (void)memset(&edma4Assignment, 0, sizeof(edma4Assignment));
    edma4Assignment.domainId       = 0x7U;
    edma4Assignment.privilegeAttr  = kTRDC_MasterPrivilege;
    edma4Assignment.secureAttr     = kTRDC_MasterSecure;
    edma4Assignment.bypassDomainId = true;
    edma4Assignment.lock           = false;
    TRDC_SetNonProcessorDomainAssignment(TRDC2, kTRDC2_MasterDMA4, &edma4Assignment);

    /* Enable all access modes for MBC and MRC of TRDCA and TRDCW */
    trdc_hardware_config_t hwConfig;
    trdc_memory_access_control_config_t memAccessConfig;

    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));
    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;

    TRDC_GetHardwareConfig(TRDC2, &hwConfig);
    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }
}

void BOARD_SetDMA4Permission(void)
{
#if !(defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U))

    status_t sts;

    /* Get ELE FW status */
    do
    {
        uint32_t ele_fw_sts;
        sts = ELE_BaseAPI_GetFwStatus(MU_RT_S3MUA, &ele_fw_sts);
    } while (sts != kStatus_Success);

    /* Release TRDC A to CM7 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_AON_ID, ELE_CORE_CM7_ID);
    } while (ELE_IS_FAILED(sts));

    /* Release TRDC W to CM7 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_WAKEUP_ID, ELE_CORE_CM7_ID);
    } while (ELE_IS_FAILED(sts));

    TRDC_EDMA4_ResetPermissions();

#endif /* !(defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U)) */
}

/* User callback function for EDMA transfer. */
void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
    }
}

status_t QTMR_SetCmpldValue(uint32_t pwmFreqHz, uint8_t dutyCyclePercent, int32_t srcClock_Hz)
{
    uint32_t periodCount, highCount, lowCount;
    periodCount = (srcClock_Hz / pwmFreqHz);
    if (dutyCyclePercent > 100U)
    {
        /* Invalid dutycycle */
        return kStatus_Fail;
    }
    highCount = (periodCount * dutyCyclePercent) / 100U;
    lowCount  = periodCount - highCount;

    if (highCount > 0U)
    {
        highCount -= 1U;
    }
    if (lowCount > 0U)
    {
        lowCount -= 1U;
    }

    /* This should not be a 16-bit overflow value. If it is, change to a larger divider for clock source. */
    assert(highCount <= 0xFFFFU);
    assert(lowCount <= 0xFFFFU);

    g_Cmpld1Value = (uint16_t)lowCount;
    g_Cmpld2Value = (uint16_t)highCount;
    return kStatus_Success;
}

AT_NONCACHEABLE_SECTION_INIT(uint16_t captValue) = 0;

/*!
 * @brief Main function
 */
int main(void)
{
    qtmr_config_t qtmrConfig;
    edma_config_t userConfig;
    edma_transfer_config_t transferConfig;
    uint8_t updatedDutycycle = 50U;
    uint8_t getCharValue     = 0U;
    uint32_t timeCapt        = 0;
    uint32_t count           = 0;
    uint32_t counterClock    = 0;

    /* Board pin, clock, debug console init */
   
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_SetDMA4Permission();

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* DMAMUX init */
    DMAMUX_Init(EXAMPLE_QTMR_DMA_MUX);
    DMAMUX_SetSource(EXAMPLE_QTMR_DMA_MUX, 0, QTMR_EDMA_REQUEST_CAPT_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_QTMR_DMA_MUX, 0);
#endif

    /* EDMA init */
    /*
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableContinuousLinkMode = false;
     * userConfig.enableDebugMode = false;
     */

    EDMA_GetDefaultConfig(&userConfig);
#if defined(BOARD_SetEDMAConfig)
    BOARD_SetEDMAConfig(userConfig);
#endif

    EDMA_Init(EXAMPLE_QTMR_DMA, &userConfig);
    EDMA_CreateHandle(&g_EDMA_Handle, EXAMPLE_QTMR_DMA, 0);
    EDMA_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_QTMR_DMA, 0, QTMR_EDMA_REQUEST_CAPT_SOURCE);
#endif

    /*
     * qtmrConfig.debugMode = kQTMR_RunNormalInDebug;
     * qtmrConfig.enableExternalForce = false;
     * qtmrConfig.enableMasterMode = false;
     * qtmrConfig.faultFilterCount = 0;
     * qtmrConfig.faultFilterPeriod = 0;
     * qtmrConfig.primarySource = kQTMR_ClockDivide_2;
     * qtmrConfig.secondarySource = kQTMR_Counter0InputPin;
     */
    QTMR_GetDefaultConfig(&qtmrConfig);

    PRINTF("\r\n****Input capture dma example start.****\n");
    PRINTF("\r\n****Provide a signal input to the QTMR pin****\n");

    /* Initial the input channel. */
    qtmrConfig.primarySource = QTMR_PRIMARY_SOURCE;
    QTMR_Init(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL, &qtmrConfig);

    /* Setup the input capture */
    QTMR_SetupInputCapture(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL, QTMR_CounterInputPin, false, true,
                           kQTMR_RisingEdge);

    /* Enable the input edge flag DMA*/
    QTMR_EnableDma(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL, kQTMR_InputEdgeFlagDmaEnable);
    EDMA_PrepareTransfer(&transferConfig,
                         (uint16_t *)&BOARD_QTMR_BASEADDR->CHANNEL[BOARD_QTMR_INPUT_CAPTURE_CHANNEL].CAPT, 2,
                         &captValue, 2, 2, 2, kEDMA_PeripheralToMemory);
    EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);

    /* Start the input capture channel to count on rising edge of the primary source clock */
    QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL, kQTMR_PriSrcRiseEdge);

    /* Wait input Edge*/
    while (count < 5 || timeCapt == 0)
    {
        while (!(QTMR_GetStatus(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL) & kQTMR_EdgeFlag))
        {
        }
        QTMR_ClearStatusFlags(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL, kQTMR_EdgeFlag);
        count++;
        timeCapt = BOARD_QTMR_BASEADDR->CHANNEL[BOARD_QTMR_INPUT_CAPTURE_CHANNEL].CAPT;
    }

    counterClock = QTMR_SOURCE_CLOCK / 1000U;

    EDMA_StartTransfer(&g_EDMA_Handle);

    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    PRINTF("\r\nCaptured Period time=%d us\n", timeCapt * 1000U / counterClock);
    QTMR_DisableDma(BOARD_QTMR_BASEADDR, BOARD_QTMR_INPUT_CAPTURE_CHANNEL, kQTMR_InputEdgeFlagDmaEnable);

#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    DMAMUX_DisableChannel(EXAMPLE_QTMR_DMA_MUX, 0);
    DMAMUX_SetSource(EXAMPLE_QTMR_DMA_MUX, 0, QTMR_EDMA_REQUEST_CMPLD_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_QTMR_DMA_MUX, 0);
#endif

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_QTMR_DMA, 0, 0);
    EDMA_SetChannelMux(EXAMPLE_QTMR_DMA, 0, QTMR_EDMA_REQUEST_CMPLD_SOURCE);
#endif

    PRINTF("\r\n****Output pwm dma example.****\n");
    PRINTF("\r\n*********Make sure to connect an oscilloscope.*********\n");
    PRINTF("\r\n****A 50%% duty cycle PWM wave is observed on an oscilloscope.****\n");

    /* Initial the output channel. */
    qtmrConfig.primarySource = QTMR_PRIMARY_SOURCE;
    QTMR_Init(BOARD_QTMR_BASEADDR, BOARD_QTMR_PWM_CHANNEL, &qtmrConfig);

    /* Generate a 50Khz PWM signal with 50% dutycycle by default */
    QTMR_SetupPwm(BOARD_QTMR_BASEADDR, BOARD_QTMR_PWM_CHANNEL, QTMR_PWM_OUTPUT_FREQUENCY, QTMR_DUTYCYCLE_PERCENT, false,
                  QTMR_SOURCE_CLOCK);
    /* Enable comparator preload register 1 DMA */
    QTMR_EnableDma(BOARD_QTMR_BASEADDR, BOARD_QTMR_PWM_CHANNEL, kQTMR_ComparatorPreload1DmaEnable);
    /* Enable comparator preload register 2 DMA */
    QTMR_EnableDma(BOARD_QTMR_BASEADDR, BOARD_QTMR_PWM_CHANNEL, kQTMR_ComparatorPreload2DmaEnable);
    /* Start the counter */
    QTMR_StartTimer(BOARD_QTMR_BASEADDR, BOARD_QTMR_PWM_CHANNEL, kQTMR_PriSrcRiseEdge);
    while (1)
    {
        g_Transfer_Done = false;
        do
        {
            PRINTF("\r\nPlease enter a value to update the Duty cycle:\r\n");
            PRINTF("Note: The range of value is 1 to 9.\r\n");
            PRINTF("For example: If enter '5', the duty cycle will be set to 50 percent.\r\n");
            PRINTF("Value:");
            getCharValue = GETCHAR() - 0x30U;
            PRINTF("%d", getCharValue);
            PRINTF("\r\n");
        } while ((getCharValue > 9U) || (getCharValue == 0U));

        updatedDutycycle = getCharValue * 10U;
        QTMR_SetCmpldValue(QTMR_PWM_OUTPUT_FREQUENCY, updatedDutycycle, QTMR_SOURCE_CLOCK);
        EDMA_PrepareTransfer(&transferConfig, (uint16_t *)&g_Cmpld1Value, 2,
                             (uint16_t *)&BOARD_QTMR_BASEADDR->CHANNEL[BOARD_QTMR_PWM_CHANNEL].CMPLD1, 2, 2, 2,
                             kEDMA_MemoryToPeripheral);
        EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
        EDMA_StartTransfer(&g_EDMA_Handle);
        /* Wait for EDMA transfer finish */
        while (g_Transfer_Done != true)
        {
        }
        g_Transfer_Done = false;
        EDMA_PrepareTransfer(&transferConfig, (uint16_t *)&g_Cmpld2Value, 2,
                             (uint16_t *)&BOARD_QTMR_BASEADDR->CHANNEL[BOARD_QTMR_PWM_CHANNEL].CMPLD2, 2, 2, 2,
                             kEDMA_MemoryToPeripheral);
        EDMA_SubmitTransfer(&g_EDMA_Handle, &transferConfig);
        EDMA_StartTransfer(&g_EDMA_Handle);
        /* Wait for EDMA transfer finish */
        while (g_Transfer_Done != true)
        {
        }
        PRINTF("The duty cycle was successfully updated!\r\n");
    }
}
