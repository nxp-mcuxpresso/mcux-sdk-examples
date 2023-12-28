/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "task.h"

#include "call_gateway.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_adapter_uart.h"
#include "controller_hci_uart.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_lpuart_edma.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_common.h"
#include "fsl_wm8962.h"
#include "fsl_codec_adapter.h"
#include "usb_phy.h"
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
#include "ksdk_mbedtls.h"
#endif /* CONFIG_BT_SMP */
#include "fsl_gpt.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif
#if defined(WIFI_IW612_BOARD_RD_USD)
#define CONTROLLER_RESET_GPIO GPIO3
#define CONTROLLER_RESET_PIN  9U
#endif

/* Select Audio/Video PLL (393.24 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (4U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (16U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ (CLOCK_GetFreq(kCLOCK_AudioPll) / DEMO_SAI1_CLOCK_SOURCE_DIVIDER)

#define DEMO_SAI            SAI1
#define DEMO_CODEC_INSTANCE (1U)

/* DMA */
#define EXAMPLE_DMAMUX_INSTANCE      (0U)
#define EXAMPLE_DMA_INSTANCE         (0U)
#define EXAMPLE_DMA_TX_CHANNEL       (2U)
#define EXAMPLE_DMA_RX_CHANNEL       (1U)
#define EXAMPLE_SAI_CODEC_TX_SOURCE (kDmaRequestMuxSai1Tx)
#define EXAMPLE_SAI_CODEC_RX_SOURCE (kDmaRequestMuxSai1Rx)

/* demo audio data channel */
#define EXAMPLE_CODEC_RX_CHANNEL (kHAL_AudioMonoRight)
#define EXAMPLE_CODEC_TX_CHANNEL (kHAL_AudioMonoLeft)

/* GPT - SyncTimer */
#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
#define SyncTimer_GPT               (GPT2)
#define SyncTimer_GPT_Irq           (GPT2_IRQn)
#define SyncTimer_GPT_ClockRoot     (kCLOCK_Root_Gpt2)
#define SyncTimer_GPT_ClockRoot_Mux (kCLOCK_GPT2_ClockRoot_MuxAudioPllOut)
#define SyncTimer_GPT_ClockRoot_Div (16)
#elif defined(WIFI_IW612_BOARD_RD_USD)
#define SyncTimer_GPT               (GPT3)
#define SyncTimer_GPT_Irq           (GPT3_IRQn)
#define SyncTimer_GPT_ClockRoot     (kCLOCK_Root_Gpt3)
#define SyncTimer_GPT_ClockRoot_Mux (kCLOCK_GPT3_ClockRoot_MuxAudioPllOut)
#define SyncTimer_GPT_ClockRoot_Div (16)
#endif


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_InitHardware(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */
const clock_audio_pll_config_t audioCodecPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

wm8962_config_t wm8962ScoConfig1 = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route =
        {
            .enableLoopBack            = false,
            .leftInputPGASource        = kWM8962_InputPGASourceInput1,
            .leftInputMixerSource      = kWM8962_InputMixerSourceInputPGA,
            .rightInputPGASource       = kWM8962_InputPGASourceInput3,
            .rightInputMixerSource     = kWM8962_InputMixerSourceInputPGA,
            .leftHeadphoneMixerSource  = kWM8962_OutputMixerDisabled,
            .leftHeadphonePGASource    = kWM8962_OutputPGASourceDAC,
            .rightHeadphoneMixerSource = kWM8962_OutputMixerDisabled,
            .rightHeadphonePGASource   = kWM8962_OutputPGASourceDAC,
        },
    .slaveAddress = WM8962_I2C_ADDR,
    .bus          = kWM8962_BusI2S,
    .format       = {.mclk_HZ = 24576000U, .sampleRate = 8000, .bitWidth = kWM8962_AudioBitWidth16bit},
    .masterSlave  = false,
};

codec_config_t boardCodecScoConfig1 = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm8962ScoConfig1};

hal_audio_dma_mux_config_t codecTxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = EXAMPLE_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = EXAMPLE_SAI_CODEC_TX_SOURCE,
};

hal_audio_dma_config_t codecTxDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_DMA_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = (void *)&codecTxDmaMuxConfig,
    .dmaChannelMuxConfig  = NULL,
};

hal_audio_ip_config_t codecTxIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeAsync,
};

hal_audio_config_t codecTxConfig = {
    .dmaConfig         = &codecTxDmaConfig,
    .ipConfig          = (void *)&codecTxIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(DEMO_SAI) / 2U,
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = EXAMPLE_CODEC_TX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_CODEC_INSTANCE,
};

hal_audio_dma_mux_config_t codeRxDmaMuxConfig = {
    .dmaMuxConfig.dmaMuxInstance   = EXAMPLE_DMAMUX_INSTANCE,
    .dmaMuxConfig.dmaRequestSource = EXAMPLE_SAI_CODEC_RX_SOURCE,
};

hal_audio_dma_config_t codecRxDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_DMA_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = (void *)&codeRxDmaMuxConfig,
    .dmaChannelMuxConfig  = NULL,
};

hal_audio_ip_config_t codecRxIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeSync,
};

hal_audio_config_t codecRxConfig = {
    .dmaConfig         = &codecRxDmaConfig,
    .ipConfig          = (void *)&codecRxIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(DEMO_SAI) / 2U,
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = EXAMPLE_CODEC_RX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatI2sClassic,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_CODEC_INSTANCE,
};

#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2) || defined(WIFI_IW612_BOARD_RD_USD)
static volatile uint32_t SyncTimer_Trigger_Counter; /* This used to counter the sync trigger signal. */
static void (*SyncTimer_Trigger_Callback)(uint32_t sync_index, uint64_t bclk_count) = NULL;
static uint32_t SyncTimer_Capture1_Value = 0;
static uint32_t SyncTimer_Pre_Capture2_Value = 0;
static uint64_t SyncTimer_Count_Value = 0;
static uint32_t SyncTimer_Count_To_Bclk_Div = 0;
static uint64_t SyncTimer_Bclk_Value = 0;
#endif


/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate)
{
    CLOCK_DeinitAudioPll();

    if (0U == sampleRate)
    {
        /* Disable MCLK output */
        IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK);
    }
    else
    {
        CLOCK_InitAudioPll(&audioCodecPllConfig);

        /*Clock setting for LPI2C*/
        CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);

        /*Clock setting for SAI1*/
        CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, DEMO_SAI1_CLOCK_SOURCE_SELECT);
        CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, DEMO_SAI1_CLOCK_SOURCE_DIVIDER);

        /* Enable MCLK output */
        IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK;
    }

    return DEMO_SAI_CLK_FREQ;
}


#if defined(BT_THIRD_PARTY_TRANSCEIVER)
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc         = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate  = 1000000u;
    config->runningBaudrate  = 1000000u;
    config->instance         = BOARD_BT_UART_INSTANCE;
    config->enableRxRTS      = 1u;
    config->enableTxCTS      = 1u;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = EXAMPLE_DMA_INSTANCE;
    config->rx_channel       = 4U;
    config->tx_channel       = 5U;
    config->dma_mux_instance = EXAMPLE_DMAMUX_INSTANCE;
    config->rx_request       = kDmaRequestMuxLPUART7Rx;
    config->tx_request       = kDmaRequestMuxLPUART7Tx;
#endif
    return 0;
}

void controller_init(void)
{
}

#elif defined(WIFI_IW612_BOARD_RD_USD)
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc         = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate  = BOARD_BT_UART_BAUDRATE;
    config->runningBaudrate  = BOARD_BT_UART_BAUDRATE;
    config->instance         = BOARD_BT_UART_INSTANCE;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = EXAMPLE_DMA_INSTANCE;
    config->rx_channel       = 4U;
    config->tx_channel       = 5U;
    config->dma_mux_instance = EXAMPLE_DMAMUX_INSTANCE;
    config->rx_request       = kDmaRequestMuxLPUART7Rx;
    config->tx_request       = kDmaRequestMuxLPUART7Tx;
#endif
    config->enableRxRTS      = 1u;
    config->enableTxCTS      = 1u;
    return 0;
}
#elif (defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2) || defined(WIFI_IW416_BOARD_MURATA_1XK_M2) || \
     defined(WIFI_IW612_BOARD_MURATA_2EL_M2))
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc        = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2);
    config->defaultBaudrate = 115200u;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance        = 2;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance     = EXAMPLE_DMA_INSTANCE;
    config->rx_channel       = 4U;
    config->tx_channel       = 5U;
    config->dma_mux_instance = EXAMPLE_DMAMUX_INSTANCE;
    config->rx_request       = kDmaRequestMuxLPUART2Rx;
    config->tx_request       = kDmaRequestMuxLPUART2Tx;
#endif
    config->enableRxRTS = 1u;
    config->enableTxCTS = 1u;
    return 0;
}
#else
#endif

void USB_HostClockInit(void)
{
    uint32_t usbClockFreq;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
    usbClockFreq = 24000000;
    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, usbClockFreq);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, usbClockFreq);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2) || defined(WIFI_IW612_BOARD_RD_USD)
void BOARD_SyncTimer_Init(void (*sync_timer_callback)(uint32_t sync_index, uint64_t bclk_count))
{
    gpt_config_t config;

#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
    /* 2EL use GPT2 as SyncTimer. */
    BOARD_InitGPT2Pins();
#elif defined(WIFI_IW612_BOARD_RD_USD)
    /* RD_USD use GPT3 as SyncTimer. */
    BOARD_InitGPT3Pins();
#endif
    /* Set SyncTimer clock root to Audio_PLL. */
    CLOCK_SetRootClockMux(SyncTimer_GPT_ClockRoot, SyncTimer_GPT_ClockRoot_Mux);
    CLOCK_SetRootClockDiv(SyncTimer_GPT_ClockRoot, SyncTimer_GPT_ClockRoot_Div); /* 393216000 / 16 = 24576000 */

    GPT_GetDefaultConfig(&config);
    config.clockSource     = kGPT_ClockSource_HighFreq; /* GPT2_CLK come from AUDIO PLL. */
    config.divider         = 1U;
    config.enableRunInStop = true;
    config.enableRunInWait = true;
    config.enableRunInDoze = false;
    config.enableRunInDbg  = false;
    config.enableFreeRun   = true;
    config.enableMode      = true;
    GPT_Init(SyncTimer_GPT, &config);

    /* Input Capture 1 capture signal come from SAI FS signal, in order to capture offset between BCLK and FS. */
    GPT_SetInputOperationMode(SyncTimer_GPT, kGPT_InputCapture_Channel1, kGPT_InputOperation_RiseEdge);
    // GPT_EnableInterrupts(SyncTimer_GPT, kGPT_InputCapture1InterruptEnable);
    /* Input Capture 2 capture signal come from Sync INT signal, on order to measure CLK draft. */
    GPT_SetInputOperationMode(SyncTimer_GPT, kGPT_InputCapture_Channel2, kGPT_InputOperation_RiseEdge);
    // GPT_EnableInterrupts(SyncTimer_GPT, kGPT_InputCapture2InterruptEnable);

    EnableIRQWithPriority(SyncTimer_GPT_Irq, 3);

    SyncTimer_Trigger_Callback = sync_timer_callback;
}

void BORAD_SyncTimer_Start(uint32_t sample_rate, uint32_t bits_per_sample)
{
    /* Reset all parameter values. */
    SyncTimer_Trigger_Counter    = 0;
    SyncTimer_Capture1_Value     = 0;
    SyncTimer_Pre_Capture2_Value = 0;
    SyncTimer_Count_Value        = 0;
    SyncTimer_Count_To_Bclk_Div  = CLOCK_GetRootClockFreq(SyncTimer_GPT_ClockRoot) / (sample_rate * bits_per_sample);
    SyncTimer_Bclk_Value         = 0;

    GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture1Flag);
    GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture2Flag);
    GPT_EnableInterrupts(SyncTimer_GPT, kGPT_InputCapture1InterruptEnable);
    GPT_EnableInterrupts(SyncTimer_GPT, kGPT_InputCapture2InterruptEnable);
    GPT_StartTimer(SyncTimer_GPT);
}

void BORAD_SyncTimer_Stop(void)
{
    GPT_StopTimer(SyncTimer_GPT);
}

#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
void GPT2_IRQHandler(void)
#elif defined(WIFI_IW612_BOARD_RD_USD)
void GPT3_IRQHandler(void)
#endif
{
    uint32_t flag;
    uint32_t current_capture2_value;
    uint32_t capture2_value_delta;

    flag = GPT_GetStatusFlags(SyncTimer_GPT, (gpt_status_flag_t)(kGPT_InputCapture1Flag | kGPT_InputCapture2Flag));
    flag &= GPT_GetEnabledInterrupts(SyncTimer_GPT);

    if (flag & kGPT_InputCapture1Flag)
    {
        /* We only need capture the first SAI FS signal to calculate offset. */
        GPT_DisableInterrupts(SyncTimer_GPT, kGPT_InputCapture1InterruptEnable);
        GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture1Flag);
        /* SAI FS trigged BCLK offset. */
        SyncTimer_Capture1_Value = GPT_GetInputCaptureValue(SyncTimer_GPT, kGPT_InputCapture_Channel1);
    }
    if (flag & kGPT_InputCapture2Flag)
    {
        GPT_ClearStatusFlags(SyncTimer_GPT, kGPT_InputCapture2Flag);
        /* Sync INT trigged BCLK count capture. */
        current_capture2_value = GPT_GetInputCaptureValue(SyncTimer_GPT, kGPT_InputCapture_Channel2);
        /* Fix GPT overflow issue. */
        if (current_capture2_value < SyncTimer_Pre_Capture2_Value)
        {
            capture2_value_delta = (uint64_t)0x100000000 + current_capture2_value - SyncTimer_Pre_Capture2_Value;
        }
        else
        {
            capture2_value_delta = current_capture2_value - SyncTimer_Pre_Capture2_Value;
        }
        SyncTimer_Pre_Capture2_Value = current_capture2_value;
        SyncTimer_Count_Value += capture2_value_delta;

        /* Calculate BCLK. */
        /* BCLK should be 0 before the SAI_SW start, because we use AUDIO_PLL as clk source. */
        if (SyncTimer_Capture1_Value > 0U)
        {
            SyncTimer_Bclk_Value = (SyncTimer_Count_Value - SyncTimer_Capture1_Value) / SyncTimer_Count_To_Bclk_Div;
        }
        else
        {
            SyncTimer_Bclk_Value = 0;
        }
        /* invoke callback. */
        if (SyncTimer_Trigger_Callback)
        {
            SyncTimer_Trigger_Callback(SyncTimer_Trigger_Counter, SyncTimer_Bclk_Value);
        }

        /* SyncTimer trigger signal count. */
        SyncTimer_Trigger_Counter++;
    }
}
#endif /* WIFI_IW612_BOARD_MURATA_2EL_M2 || WIFI_IW612_BOARD_RD_USD */


int main(void)
{
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    DMAMUX_Type *dmaMuxBases[] = DMAMUX_BASE_PTRS;
    edma_config_t config;
    DMA_Type *dmaBases[] = DMA_BASE_PTRS;
#endif

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    DMAMUX_Init(dmaMuxBases[EXAMPLE_DMAMUX_INSTANCE]);
    EDMA_GetDefaultConfig(&config);
    EDMA_Init(dmaBases[EXAMPLE_DMA_INSTANCE], &config);
#endif /* HAL_UART_DMA_ENABLE */

#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
    CRYPTO_InitHardware();
#endif /* CONFIG_BT_SMP */

#if defined(WIFI_IW612_BOARD_RD_USD)
    GPIO_PinWrite(CONTROLLER_RESET_GPIO, CONTROLLER_RESET_PIN, 0U);
    SDK_DelayAtLeastUs(10U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    GPIO_PinWrite(CONTROLLER_RESET_GPIO, CONTROLLER_RESET_PIN, 1U);
#endif

    if (xTaskCreate(call_gateway_task, "call_gateway_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("call gateway task creation failed!\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}
