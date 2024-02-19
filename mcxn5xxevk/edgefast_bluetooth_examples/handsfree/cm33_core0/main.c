/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include <porting.h>
#include <string.h>
#include <errno/errno.h>
#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/hfp_hf.h>
#include "FreeRTOS.h"
#include "task.h"

#include "app_handsfree.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_clock.h"
#include "controller_hci_uart.h"
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
#include "els_mbedtls.h"
#endif /* CONFIG_BT_SMP */
#include "fsl_lpflexcomm.h"
#include "fsl_codec_common.h"
#include "fsl_dialog7212.h"
#include "fsl_codec_adapter.h"
#include "fsl_adapter_audio.h"
#include "fsl_sai.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_phy.h"
#include "usb.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* M.2 UART DMA */
#define LPUART_TX_DMA_CHANNEL       0U
#define LPUART_RX_DMA_CHANNEL       1U
#define DEMO_LPUART_TX_EDMA_CHANNEL kDma0RequestMuxLpFlexcomm2Tx
#define DEMO_LPUART_RX_EDMA_CHANNEL kDma0RequestMuxLpFlexcomm2Rx
#define EXAMPLE_LPUART_DMA_INDEX    0U

/* M.2 SCO SAI0 */
#define SCO_SAI               SAI0
#define DEMO_SCO_INSTANCE     (0U)

/* Codec SAI1 */
#define CODEC_SAI             SAI1
#define DEMO_CODEC_INSTANCE   (1U)
#define DEMO_SAI_CLK_FREQ     CLOCK_GetSaiClkFreq(1U)

/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate16KHz)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH (kHAL_AudioWordWidth16bits)

/* DMA */
#define EXAMPLE_DMA_INSTANCE         (0U)
#define EXAMPLE_MICBUF_TX_CHANNEL    (5U)
#define EXAMPLE_MICBUF_RX_CHANNEL    (4U)
#define EXAMPLE_SPKBUF_TX_CHANNEL    (3U)
#define EXAMPLE_SPKBUF_RX_CHANNEL    (2U)
#define EXAMPLE_SAI_MICBUF_TX_SOURCE (kDma0RequestMuxSai0Tx)
#define EXAMPLE_SAI_MICBUF_RX_SOURCE (kDma0RequestMuxSai1Rx)
#define EXAMPLE_SAI_SPKBUF_TX_SOURCE (kDma0RequestMuxSai1Tx)
#define EXAMPLE_SAI_SPKBUF_RX_SOURCE (kDma0RequestMuxSai0Rx)

/* demo audio data channel */
#define DEMO_MICBUF_TX_CHANNEL (kHAL_AudioMono)
#define DEMO_MICBUF_RX_CHANNEL (kHAL_AudioMonoRight)
#define DEMO_SPKBUF_TX_CHANNEL (kHAL_AudioMonoLeft)
#define DEMO_SPKBUF_RX_CHANNEL (kHAL_AudioMono)

/* USB */
#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_InitHardware(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
da7212_config_t da7212Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusDSPMode,
    .format       = {.mclk_HZ = 24576000U, .sampleRate = 8000, .bitWidth = DEMO_AUDIO_BIT_WIDTH},
    .sysClkSource = kDA7212_SysClkSourceMCLK,
    .inputSource  = kDA7212_Input_MIC1_Dig,
    .isMaster     = false,
};
codec_config_t boardCodecScoConfig = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

da7212_config_t da7212Config1 = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusI2S,
    .format       = {.mclk_HZ = 24576000U, .sampleRate = 8000, .bitWidth = DEMO_AUDIO_BIT_WIDTH},
    .sysClkSource = kDA7212_SysClkSourceMCLK,
    .inputSource  = kDA7212_Input_MIC1_Dig,
    .isMaster     = false,
};
codec_config_t boardCodecScoConfig1 = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

hal_audio_dma_channel_mux_config_t txSpeakerDmaChannelMuxConfig = {
    .dmaChannelMuxConfig.dmaRequestSource = EXAMPLE_SAI_SPKBUF_TX_SOURCE,
};

hal_audio_dma_config_t txSpeakerDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_SPKBUF_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = (void *)&txSpeakerDmaChannelMuxConfig,
};

hal_audio_ip_config_t txSpeakerIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeAsync,
};

hal_audio_config_t txSpeakerConfig = {
    .dmaConfig         = &txSpeakerDmaConfig,
    .ipConfig          = (void *)&txSpeakerIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(CODEC_SAI) / 2U,
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_SPKBUF_TX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeB,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_CODEC_INSTANCE,
};

hal_audio_dma_channel_mux_config_t rxMicDmaChannelMuxConfig = {
    .dmaChannelMuxConfig.dmaRequestSource = EXAMPLE_SAI_MICBUF_RX_SOURCE,
};

hal_audio_dma_config_t rxMicDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_MICBUF_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = (void *)&rxMicDmaChannelMuxConfig,
};

hal_audio_ip_config_t rxMicIpConfig = {
    .sai.lineMask = 1U << 0U,
    .sai.syncMode = kHAL_AudioSaiModeSync,
};

hal_audio_config_t rxMicConfig = {
    .dmaConfig         = &rxMicDmaConfig,
    .ipConfig          = (void *)&rxMicIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(CODEC_SAI) / 2U,
    .msaterSlave       = kHAL_AudioMaster,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_MICBUF_RX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeB,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_CODEC_INSTANCE,
};

hal_audio_dma_channel_mux_config_t txMicDmaChannelMuxConfig = {
    .dmaChannelMuxConfig.dmaRequestSource = EXAMPLE_SAI_MICBUF_TX_SOURCE,
};

hal_audio_dma_config_t txMicDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_MICBUF_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = (void *)&txMicDmaChannelMuxConfig,
};

hal_audio_ip_config_t txMicIpConfig = {
    .sai.lineMask = 1U << 1U,
#if defined(PCM_MODE_CONFIG_TX_CLK_SYNC)
    .sai.syncMode = kHAL_AudioSaiModeSync,
#else
    .sai.syncMode = kHAL_AudioSaiModeAsync,
#endif
};

hal_audio_config_t txMicConfig = {
    .dmaConfig         = &txMicDmaConfig,
    .ipConfig          = (void *)&txMicIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(SCO_SAI) / 2U,
    .msaterSlave       = kHAL_AudioSlave,
    .bclkPolarity      = kHAL_AudioSampleOnFallingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthOneBitClk,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_MICBUF_TX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeA,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_SCO_INSTANCE,
};

hal_audio_dma_channel_mux_config_t rxSpeakerDmaChannelMuxConfig = {
    .dmaChannelMuxConfig.dmaRequestSource = EXAMPLE_SAI_SPKBUF_RX_SOURCE,
};

hal_audio_dma_config_t rxSpeakerDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_SPKBUF_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = (void *)&rxSpeakerDmaChannelMuxConfig,
};

hal_audio_ip_config_t rxSpeakerIpConfig = {
    .sai.lineMask = 1U << 1U,
#if defined(PCM_MODE_CONFIG_TX_CLK_SYNC)
    .sai.syncMode = kHAL_AudioSaiModeAsync,
#else
    .sai.syncMode = kHAL_AudioSaiModeSync,
#endif
};

hal_audio_config_t rxSpeakerConfig = {
    .dmaConfig         = &rxSpeakerDmaConfig,
    .ipConfig          = (void *)&rxSpeakerIpConfig,
    .srcClock_Hz       = 0,
    .sampleRate_Hz     = 0,
    .fifoWatermark     = FSL_FEATURE_SAI_FIFO_COUNTn(SCO_SAI) / 2U,
    .msaterSlave       = kHAL_AudioSlave,
    .bclkPolarity      = kHAL_AudioSampleOnFallingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthOneBitClk,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_SPKBUF_RX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeA,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_SCO_INSTANCE,
};

sai_master_clock_t mclkConfig;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate)
{
    if(sampleRate == 48000U)
    {
        /* < Set up PLL1 */
        const pll_setup_t pll1Setup1 = {
          .pllctrl = SCG_APLLCTRL_SOURCE(1U) | SCG_APLLCTRL_SELI(7U) | SCG_APLLCTRL_SELP(3U),
          .pllndiv = SCG_APLLNDIV_NDIV(1U),
          .pllpdiv = SCG_APLLPDIV_PDIV(4U),
          .pllmdiv = SCG_APLLMDIV_MDIV(8U),
          .pllRate = 48000000U
        };
        CLOCK_SetPLL1Freq(&pll1Setup1);
        CLOCK_SetClkDiv(kCLOCK_DivPLL1Clk0, 1U);
    }
    else if(sampleRate == 44100U)
    {
        /* do nothing, 44100 is the default value */
    }
  
    da7212Config.format.sampleRate = sampleRate;
    da7212Config.format.mclk_HZ    = DEMO_SAI_CLK_FREQ;
    
    return DEMO_SAI_CLK_FREQ;
}

void BOARD_MASTER_CLOCK_CONFIG(void)
{
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = 12288000U;
    mclkConfig.mclkSourceClkHz = 12288000U;
    SAI_SetMasterClockConfig(CODEC_SAI, &mclkConfig);
}


#if (defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2) || defined(WIFI_IW416_BOARD_MURATA_1XK_M2))
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc        = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate = 115200u;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance        = BOARD_BT_UART_INSTANCE;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    config->dma_instance = EXAMPLE_LPUART_DMA_INDEX;
    config->rx_channel   = LPUART_RX_DMA_CHANNEL;
    config->tx_channel   = LPUART_TX_DMA_CHANNEL;
#if (defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && (FSL_FEATURE_EDMA_HAS_CHANNEL_MUX > 0U))
    config->rx_request = DEMO_LPUART_RX_EDMA_CHANNEL;
    config->tx_request = DEMO_LPUART_TX_EDMA_CHANNEL;
#endif
#endif
    config->enableRxRTS = 1u;
    config->enableTxCTS = 1u;
    return 0;
}
#endif

void USB_HostClockInit(void)
{
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif

#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    SPC0->ACTIVE_VDELAY = 0x0500;
    /* Change the power DCDC to 1.8v (By deafult, DCDC is 1.8V), CORELDO to 1.1v (By deafult, CORELDO is 1.0V) */
    SPC0->ACTIVE_CFG &= ~SPC_ACTIVE_CFG_CORELDO_VDD_DS_MASK;
    SPC0->ACTIVE_CFG |= SPC_ACTIVE_CFG_DCDC_VDD_LVL(0x3) | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(0x3) |
                        SPC_ACTIVE_CFG_SYSLDO_VDD_DS_MASK | SPC_ACTIVE_CFG_DCDC_VDD_DS(0x2u);
    /* Wait until it is done */
    while (SPC0->SC & SPC_SC_BUSY_MASK)
        ;
    if (0u == (SCG0->LDOCSR & SCG_LDOCSR_LDOEN_MASK))
    {
        SCG0->TRIM_LOCK = 0x5a5a0001U;
        SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
        /* wait LDO ready */
        while (0U == (SCG0->LDOCSR & SCG_LDOCSR_VOUT_OK_MASK))
            ;
    }
    SYSCON->AHBCLKCTRLSET[2] |= SYSCON_AHBCLKCTRL2_USB_HS_MASK | SYSCON_AHBCLKCTRL2_USB_HS_PHY_MASK;
    SCG0->SOSCCFG &= ~(SCG_SOSCCFG_RANGE_MASK | SCG_SOSCCFG_EREFS_MASK);
    /* xtal = 20 ~ 30MHz */
    SCG0->SOSCCFG = (1U << SCG_SOSCCFG_RANGE_SHIFT) | (1U << SCG_SOSCCFG_EREFS_SHIFT);
    SCG0->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
    while (1)
    {
        if (SCG0->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)
        {
            break;
        }
    }
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK | SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT_MASK;
    CLOCK_EnableClock(kCLOCK_UsbHs);
    CLOCK_EnableClock(kCLOCK_UsbHsPhy);
    CLOCK_EnableUsbhsPhyPllClock(kCLOCK_Usbphy480M, 24000000U);
    CLOCK_EnableUsbhsClock();
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    CLOCK_AttachClk(kCLK_48M_to_USB0);
    CLOCK_EnableClock(kCLOCK_Usb0Ram);
    CLOCK_EnableClock(kCLOCK_Usb0Fs);
    CLOCK_EnableUsbfsClock();
#endif
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
#endif /* USB_HOST_CONFIG_EHCI */
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    uint8_t usbHOSTKhciIrq[] = USBFS_IRQS;
    irqNumber                = usbHOSTKhciIrq[CONTROLLER_ID - kUSB_ControllerKhci0];
#endif /* USB_HOST_CONFIG_KHCI */

/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    USB_HostEhciTaskFunction(param);
#endif
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    USB_HostKhciTaskFunction(param);
#endif
}

int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to FLEXCOMM2 (M.2) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(BOARD_BT_UART_CLK_ATTACH);
    
    CLOCK_EnableClock(kCLOCK_Scg);
    CLOCK_SetupFROHFClocking(48000000U);
    /* < Set up PLL1 */
    const pll_setup_t pll1Setup = {.pllctrl = SCG_SPLLCTRL_SOURCE(1U) | SCG_SPLLCTRL_SELI(3U) | SCG_SPLLCTRL_SELP(1U),
                                   .pllndiv = SCG_SPLLNDIV_NDIV(25U),
                                   .pllpdiv = SCG_SPLLPDIV_PDIV(10U),
                                   .pllmdiv = SCG_SPLLMDIV_MDIV(256U),
                                   .pllRate = 24576000U};
    CLOCK_SetPLL1Freq(&pll1Setup);
    CLOCK_SetClkDiv(kCLOCK_DivPLL1Clk0, 1U);

    /* attach PLL1 to SAI1 */
    CLOCK_SetClkDiv(kCLOCK_DivSai1Clk, 1u);
    CLOCK_AttachClk(kPLL1_CLK0_to_SAI1);
    
    /* attach PLL1 to SAI0 */
    CLOCK_SetClkDiv(kCLOCK_DivSai0Clk, 1u);
    CLOCK_AttachClk(kPLL1_CLK0_to_SAI0);
    
    /* attach FRO HF to USDHC */
    CLOCK_SetClkDiv(kCLOCK_DivUSdhcClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_USDHC);

    /* Enables the clock for GPIO0 */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for GPIO2 */
    CLOCK_EnableClock(kCLOCK_Gpio2);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enables the clock for LPFlexComm2 */
#if defined(LPFLEXCOMM_INIT_NOT_USED_IN_DRIVER) && LPFLEXCOMM_INIT_NOT_USED_IN_DRIVER
    LP_FLEXCOMM_Init(BOARD_BT_UART_INSTANCE, LP_FLEXCOMM_PERIPH_LPI2CAndLPUART);
#endif /* LPFLEXCOMM_INIT_NOT_USED_IN_DRIVER */

#if (defined(FSL_FEATURE_SAI_HAS_MCR) && (FSL_FEATURE_SAI_HAS_MCR)) || \
    (defined(FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) && (FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER))
    (void)CLOCK_EnableClock(kCLOCK_Sai1);
    /* master clock configurations */
    BOARD_MASTER_CLOCK_CONFIG();
#endif

    // XCACHE_DisableCache(XCACHE_PS);
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
    CRYPTO_InitHardware();
#endif /* CONFIG_BT_SMP */

    if (xTaskCreate(peripheral_hfp_hf_task, "peripheral_hfp_hf_task", configMINIMAL_STACK_SIZE * 8, NULL,
                    tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("pherial hrs task creation failed!\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}
