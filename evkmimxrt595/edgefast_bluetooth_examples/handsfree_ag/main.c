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
#include <bluetooth/hfp_ag.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app_handsfree_ag.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_dma.h"
#include "fsl_wm8904.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_adapter_uart.h"
#include "controller_hci_uart.h"
#include "fsl_power.h"
#include "usb_host_config.h"
#include "usb_phy.h"
#include "usb_host.h"
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
#include "ksdk_mbedtls.h"
#endif /* CONFIG_BT_SMP */
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

/* I2S */
/*board send data to wireless module*/
#define DEMO_MICBUF_TX_INSTANCE (6U)
/*board receive data from codec*/
#define DEMO_MICBUF_RX_INSTANCE (1U)
/*board send data to codec*/
#define DEMO_SPKBUF_TX_INSTANCE (3U)
/*board receive data from wireless module*/
#define DEMO_SPKBUF_RX_INSTANCE (7U)

/* DMA */
/*DMA channel number, refer to DMA Controller chapter in refenece mannual*/
#define EXAMPLE_DMA_INSTANCE      (0U)
#define EXAMPLE_MICBUF_TX_CHANNEL (13U)
#define EXAMPLE_MICBUF_RX_CHANNEL (2U)
#define EXAMPLE_SPKBUF_TX_CHANNEL (7U)
#define EXAMPLE_SPKBUF_RX_CHANNEL (14U)

/* demo audio data channel */
#define DEMO_MICBUF_TX_CHANNEL (kHAL_AudioMono)
#define DEMO_MICBUF_RX_CHANNEL (kHAL_AudioMonoRight)
#define DEMO_SPKBUF_TX_CHANNEL (kHAL_AudioMonoLeft)
#define DEMO_SPKBUF_RX_CHANNEL (kHAL_AudioMono)

#define APP_DEBUG_UART_TYPE     kSerialPort_Uart
#define APP_DEBUG_UART_BASEADDR (uint32_t) USART12
#define APP_DEBUG_UART_INSTANCE 12U
#define APP_DEBUG_UART_CLK_FREQ CLOCK_GetFlexcommClkFreq(12)
#define APP_DEBUG_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){12U, kCLOCK_FrgPllDiv, 255U, 0U}) /*!< Select FRG0 mux as frg_pll */
#define APP_DEBUG_UART_CLK_ATTACH kFRG_to_FLEXCOMM12
#define APP_UART_IRQ_HANDLER      FLEXCOMM12_IRQHandler
#define APP_UART_IRQ              FLEXCOMM12_IRQn
#define APP_DEBUG_UART_BAUDRATE   115200

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void BOARD_InitHardware(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*
 * AUDIO PLL setting: Frequency = Fref * (MULT + NUM / DENOM)
 *                              = 24 * (22 + 5040/27000)
 *                              = 532.48MHz
 */
/*setting for multiple of 8Khz,such as 48Khz/16Khz/32KHz*/
const clock_audio_pll_config_t audioPllConfig = {
    .audio_pll_src  = kCLOCK_AudioPllXtalIn, /* OSC clock */
    .numerator      = 5040,                  /* Numerator of the Audio PLL fractional loop divider is null */
    .denominator    = 27000,                 /* Denominator of the Audio PLL fractional loop divider is null */
    .audio_pll_mult = kCLOCK_AudioPllMult22  /* Divide by 22 */
};

wm8904_config_t wm8904ScoConfig = {
    .i2cConfig          = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 0},
    .recordSource       = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolPCMB,
    .format             = {.sampleRate = kWM8904_SampleRate8kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = 24576000,
    .master             = true,
};
codec_config_t boardCodecScoConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904ScoConfig};

wm8904_config_t wm8904ScoConfig1 = {
    .i2cConfig          = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 0},
    .recordSource       = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate8kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = 24576000,
    .master             = true,
};
codec_config_t boardCodecScoConfig1 = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904ScoConfig1};

hal_audio_dma_config_t txSpeakerDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_SPKBUF_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = NULL,
};

hal_audio_config_t txSpeakerConfig = {
    .dmaConfig         = &txSpeakerDmaConfig,
    .ipConfig          = NULL,
    .srcClock_Hz       = 24576000,
    .sampleRate_Hz     = (uint32_t)kHAL_AudioSampleRate8KHz,
    .frameLength       = 32,
    .fifoWatermark     = 0,
    .msaterSlave       = kHAL_AudioSlave,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_SPKBUF_TX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeB,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_SPKBUF_TX_INSTANCE,
};

hal_audio_dma_config_t rxMicDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_MICBUF_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = NULL,
};

hal_audio_config_t rxMicConfig = {
    .dmaConfig         = &rxMicDmaConfig,
    .ipConfig          = NULL,
    .srcClock_Hz       = 24576000,
    .sampleRate_Hz     = (uint32_t)kHAL_AudioSampleRate8KHz,
    .frameLength       = 32,
    .fifoWatermark     = 0,
    .msaterSlave       = kHAL_AudioSlave,
    .bclkPolarity      = kHAL_AudioSampleOnRisingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_MICBUF_RX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeB,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_MICBUF_RX_INSTANCE,
};

hal_audio_dma_config_t txMicDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_MICBUF_TX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = NULL,
};

hal_audio_config_t txMicConfig = {
    .dmaConfig     = &txMicDmaConfig,
    .ipConfig      = NULL,
    .srcClock_Hz   = 24576000,
    .sampleRate_Hz = (uint32_t)kHAL_AudioSampleRate8KHz,
#if defined(WIFI_88W8987_BOARD_AW_CM358MA) || defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2)
    .frameLength = 22, /* Here is 22 because the bt module will generate 22 bits clock after one clock WS. */
#elif defined(WIFI_IW416_BOARD_AW_AM510MA) || defined(WIFI_IW416_BOARD_MURATA_1XK_M2)
    .frameLength = 256, /* Here is 256 because the bt module will generate 256 bits clock after one clock WS. */
#else
#endif
    .fifoWatermark     = 0,
    .msaterSlave       = kHAL_AudioSlave,
    .bclkPolarity      = kHAL_AudioSampleOnFallingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthOneBitClk,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_MICBUF_TX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeA,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_MICBUF_TX_INSTANCE,
};

hal_audio_dma_config_t rxSpeakerDmaConfig = {
    .instance             = EXAMPLE_DMA_INSTANCE,
    .channel              = EXAMPLE_SPKBUF_RX_CHANNEL,
    .enablePreemption     = false,
    .enablePreemptAbility = false,
    .priority             = kHAL_AudioDmaChannelPriorityDefault,
    .dmaMuxConfig         = NULL,
    .dmaChannelMuxConfig  = NULL,
};

hal_audio_config_t rxSpeakerConfig = {
    .dmaConfig     = &rxSpeakerDmaConfig,
    .ipConfig      = NULL,
    .srcClock_Hz   = 24576000,
    .sampleRate_Hz = (uint32_t)kHAL_AudioSampleRate8KHz,
#if defined(WIFI_88W8987_BOARD_AW_CM358MA)
    .frameLength = 22, /* Here is 22 because the bt module will generate 22 bits clock after one clock WS. */
#elif defined(WIFI_IW416_BOARD_AW_AM510MA) || defined(WIFI_IW416_BOARD_AW_AM510MA)
    .frameLength = 256, /* Here is 256 because the bt module will generate 256 bits clock after one clock WS. */
#else
#endif
    .fifoWatermark     = 0,
    .msaterSlave       = kHAL_AudioSlave,
    .bclkPolarity      = kHAL_AudioSampleOnFallingEdge,
    .frameSyncWidth    = kHAL_AudioFrameSyncWidthOneBitClk,
    .frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
    .lineChannels      = DEMO_SPKBUF_RX_CHANNEL,
    .dataFormat        = kHAL_AudioDataFormatDspModeA,
    .bitWidth          = (uint8_t)kHAL_AudioWordWidth16bits,
    .instance          = DEMO_SPKBUF_RX_INSTANCE,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_InitAppDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    /* attach FRG0 clock to FLEXCOMM12 (debug console) */
    CLOCK_SetFRGClock(APP_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(APP_DEBUG_UART_CLK_ATTACH);

    uartClkSrcFreq = APP_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(APP_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, APP_DEBUG_UART_TYPE, uartClkSrcFreq);
}
uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate)
{
    CLOCK_DeinitAudioPll();
    if (0U == sampleRate)
    {
        /* Disable MCLK output */
        SYSCTL1->MCLKPINDIR &= ~SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;
    }
    else
    {
        CLOCK_InitAudioPll(&audioPllConfig);
        CLOCK_InitAudioPfd(kCLOCK_Pfd0, 26);         /* Enable Audio PLL clock */
        CLOCK_SetClkDiv(kCLOCK_DivAudioPllClk, 15U); /* Set AUDIOPLLCLKDIV divider to value 15 */

        /* Attach main clock to I3C, 396MHz / 16 = 24.75MHz. */
        CLOCK_AttachClk(kMAIN_CLK_to_I3C_CLK);
        CLOCK_SetClkDiv(kCLOCK_DivI3cClk, 16);

        /* attach AUDIO PLL clock to FLEXCOMM1 (I2S1) */
        CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM1);
        /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
        CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM3);
        /* attach AUDIO PLL clock to FLEXCOMM6 (I2S6) */
        CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM6);
        /* attach AUDIO PLL clock to FLEXCOMM7 (I2S7) */
        CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM7);

        /* attach AUDIO PLL clock to MCLK (AudioPll * (18 / 26) / 15 / 1 = 24.576MHz / 22.5792MHz) */
        CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
        CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 1);
        SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

        /* Set shared signal set 0: SCK, WS from Flexcomm1 */
        SYSCTL1->SHAREDCTRLSET[0] = SYSCTL1_SHAREDCTRLSET_SHAREDSCKSEL(1) | SYSCTL1_SHAREDCTRLSET_SHAREDWSSEL(1);
        /* Set flexcomm3 SCK, WS from shared signal set 0 */
        SYSCTL1->FCCTRLSEL[3] = SYSCTL1_FCCTRLSEL_SCKINSEL(1) | SYSCTL1_FCCTRLSEL_WSINSEL(1);

        /* Set shared signal set 1: SCK, WS from Flexcomm6 */
        SYSCTL1->SHAREDCTRLSET[1] = SYSCTL1_SHAREDCTRLSET_SHAREDSCKSEL(6) | SYSCTL1_SHAREDCTRLSET_SHAREDWSSEL(6);
        /* Set flexcomm7 SCK, WS from shared signal set 1 */
        SYSCTL1->FCCTRLSEL[7] = SYSCTL1_FCCTRLSEL_SCKINSEL(2) | SYSCTL1_FCCTRLSEL_WSINSEL(2);

        switch (sampleRate)
        {
            case 8000:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate8kHz;
                break;
            case 11025:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate11025Hz;
                break;
            case 12000:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate12kHz;
                break;
            case 16000:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate16kHz;
                break;
            case 22050:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate22050Hz;
                break;
            case 24000:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate24kHz;
                break;
            case 32000:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate32kHz;
                break;
            case 44100:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate44100Hz;
                break;
            case 48000:
                wm8904ScoConfig.format.sampleRate = kWM8904_SampleRate48kHz;
                break;
            default:
                /* codec does not support this sample rate. */
                break;
        }

        wm8904ScoConfig.i2cConfig.codecI2CSourceClock  = CLOCK_GetI3cClkFreq();
        wm8904ScoConfig.mclk_HZ                        = CLOCK_GetMclkClkFreq();
        wm8904ScoConfig1.format.sampleRate             = wm8904ScoConfig.format.sampleRate;
        wm8904ScoConfig1.i2cConfig.codecI2CSourceClock = wm8904ScoConfig.i2cConfig.codecI2CSourceClock;
        wm8904ScoConfig1.mclk_HZ                       = wm8904ScoConfig.mclk_HZ;
    }

    return CLOCK_GetMclkClkFreq();
}

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < 100; i++)
    {
        __NOP();
    }
}

void BOARD_I3C_ReleaseBus(void)
{
    uint8_t i = 0;

    GPIO_PortInit(GPIO, 2);
    BOARD_InitI3CPinsAsGPIO();

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(GPIO, 2, 30, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(GPIO, 2, 29, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(GPIO, 2, 30, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(GPIO, 2, 29, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(GPIO, 2, 29, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIO, 2, 30, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIO, 2, 29, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(GPIO, 2, 30, 1U);
    i2c_release_bus_delay();
}
#if defined(WIFI_88W8987_BOARD_AW_CM358MA) || defined(WIFI_IW416_BOARD_MURATA_1XK_M2) || \
    defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2)
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
    config->enableRxRTS     = 1u;
    config->enableTxCTS     = 1u;
    return 0;
}
#elif (defined(WIFI_IW416_BOARD_AW_AM510MA))
int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config)
{
    if (NULL == config)
    {
        return -1;
    }
    config->clockSrc = BOARD_BT_UART_CLK_FREQ;
    config->defaultBaudrate = BOARD_BT_UART_BAUDRATE;
    config->runningBaudrate = BOARD_BT_UART_BAUDRATE;
    config->instance = BOARD_BT_UART_INSTANCE;
    config->enableRxRTS = 1u;
    config->enableTxCTS = 1u;
    return 0;
}
#else
#endif

void USB_HostClockInit(void)
{
    uint8_t usbClockDiv = 1;
    uint32_t usbClockFreq;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    /* Make sure USDHC ram buffer and usb1 phy has power up */
    POWER_DisablePD(kPDRUNCFG_APD_USBHS_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_USBHS_SRAM);
    POWER_ApplyPD();

    RESET_PeripheralReset(kUSBHS_PHY_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSBHS_DEVICE_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSBHS_HOST_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSBHS_SRAM_RST_SHIFT_RSTn);

    /* enable usb ip clock */
    CLOCK_EnableUsbHs0HostClock(kOSC_CLK_to_USB_CLK, usbClockDiv);
    /* save usb ip clock freq*/
    usbClockFreq = g_xtalFreq / usbClockDiv;
    CLOCK_SetClkDiv(kCLOCK_DivPfc1Clk, 4);
    /* enable usb ram clock */
    CLOCK_EnableClock(kCLOCK_UsbhsSram);
    /* enable USB PHY PLL clock, the phy bus clock (480MHz) source is same with USB IP */
    CLOCK_EnableUsbHs0PhyPllClock(kOSC_CLK_to_USB_CLK, usbClockFreq);

    /* USB PHY initialization */
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL_SYS_CLK_HZ, &phyConfig);

#if ((defined FSL_FEATURE_USBHSH_USB_RAM) && (FSL_FEATURE_USBHSH_USB_RAM > 0U))
    for (int i = 0; i < (FSL_FEATURE_USBHSH_USB_RAM >> 2); i++)
    {
        ((uint32_t *)FSL_FEATURE_USBHSH_USB_RAM_BASE_ADDRESS)[i] = 0U;
    }
#endif

    /* enable usb1 device clock */
    CLOCK_EnableClock(kCLOCK_UsbhsDevice);
    USBHSH->PORTMODE &= ~USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /*  Wait until dev_needclk de-asserts */
    while (SYSCTL0->USB0CLKSTAT & SYSCTL0_USB0CLKSTAT_DEV_NEED_CLKST_MASK)
    {
        __ASM("nop");
    }
    /* disable usb1 device clock */
    CLOCK_DisableClock(kCLOCK_UsbhsDevice);
}
void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHSH_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerIp3516Hs0];
    /* USB_HOST_CONFIG_EHCI */

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);

    EnableIRQ((IRQn_Type)irqNumber);
}


int main(void)
{
    DMA_Type *dmaBases[] = DMA_BASE_PTRS;
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    BOARD_I3C_ReleaseBus();
    BOARD_InitI3CPins();

    APP_InitAppDebugConsole();

    /* Set FlexSPI clock: source AUX0_PLL, divide by 4 */
    BOARD_SetFlexspiClock(FLEXSPI0, 2U, 4U);

    /* attach FRG0 clock to FLEXCOMM5(debug console) */
    CLOCK_SetFRGClock(BOARD_BT_UART_FRG_CLK);
    CLOCK_AttachClk(BOARD_BT_UART_CLK_ATTACH);

    DMA_Init(dmaBases[EXAMPLE_DMA_INSTANCE]);

#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)))
    CRYPTO_InitHardware();
#endif /* CONFIG_BT_SMP */

    if (xTaskCreate(peripheral_hfp_ag_task, "peripheral_hfp_ag_task", configMINIMAL_STACK_SIZE * 8, NULL,
                    tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("pherial hfp ag task creation failed!\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}
