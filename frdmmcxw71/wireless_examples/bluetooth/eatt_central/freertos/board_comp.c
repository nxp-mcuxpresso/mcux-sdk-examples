/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
 * Include
 ************************************************************************************/
#include "pin_mux.h"
#include "board_comp.h"
#include "app.h"
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)) || (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
#include "fsl_adapter_gpio.h"
#endif
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c >= 1)
#include "fsl_component_serial_manager.h"
#endif
#if defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)
#include "fsl_component_led.h"
#endif
#if defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)
#include "fsl_component_button.h"
#endif

#if (defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 1)) && defined(gDebugConsoleEnable_d) && \
    (gDebugConsoleEnable_d == 1)
#error second serial manager Instance can not be used if DebugConsole enabled, (sharing the same UART instance)
#endif

#if (defined(gAppOtaExternalStorage_c) && (gAppOtaExternalStorage_c > 0))
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
#if (!defined(gAppRequireRgbLed_c) || (gAppRequireRgbLed_c == 0))
/* Even if the BSP supports 2 LEDs the Application may want one LED only but
 * that LED may RGB or monochrome.
 * Here, if the external flash is required, we can only use the RGDB LED.
 */
#error "Monochrome LED use is incompatible with external Flash PB0 conflict"
#endif /* gAppRequireRgbLed_c */
#endif
#endif /* gAppOtaExternalStorage_c */
/************************************************************************************
*************************************************************************************
* Private type definitions and macros
*************************************************************************************
************************************************************************************/
#ifndef SERIAL_MANAGER_RING_BUFFER_SIZE
#define SERIAL_MANAGER_RING_BUFFER_SIZE (128U)
#endif

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c >= 1)

static uint8_t s_ringBuffer1[SERIAL_MANAGER_RING_BUFFER_SIZE];
static uint8_t s_ringBuffer2[SERIAL_MANAGER_RING_BUFFER_SIZE];

#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
static serial_port_uart_dma_config_t uartDmaConfig1 = {
    .instance     = BOARD_APP_UART_INSTANCE,
    .baudRate     = BOARD_APP_UART_BAUDRATE,
    .parityMode   = kSerialManager_UartParityDisabled,
    .stopBitCount = kSerialManager_UartOneStopBit,
    .enableRx     = 1,
    .enableTx     = 1,
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0) && (BOARD_APP_UART_INSTANCE == 0U))
    .enableRxRTS = 1,
    .enableTxCTS = 1,
#endif
    .dma_instance = 0,
    .rx_channel   = 0,
    .tx_channel   = 1,
};

static const serial_manager_config_t s_serialManagerConfig1 = {
    .type           = BOARD_APP_UART_TYPE,
    .ringBuffer     = &s_ringBuffer1[0],
    .ringBufferSize = SERIAL_MANAGER_RING_BUFFER_SIZE,
    .portConfig     = (serial_port_uart_config_t *)&uartDmaConfig1,
};

static serial_port_uart_dma_config_t uartDmaConfig2 = {

    .instance     = BOARD_APP2_UART_INSTANCE,
    .baudRate     = BOARD_APP2_UART_BAUDRATE,
    .parityMode   = kSerialManager_UartParityDisabled,
    .stopBitCount = kSerialManager_UartOneStopBit,
    .enableRx     = 1,
    .enableTx     = 1,
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0) && (BOARD_APP_UART_INSTANCE == 0U))
    .enableRxRTS = 1,
    .enableTxCTS = 1,
#endif
    .dma_instance = 0,
    .rx_channel   = 0,
    .tx_channel   = 1,
};

static const serial_manager_config_t s_serialManagerConfig2 = {
    .type           = BOARD_APP_UART_TYPE,
    .ringBuffer     = &s_ringBuffer2[0],
    .ringBufferSize = SERIAL_MANAGER_RING_BUFFER_SIZE,
    .portConfig     = (serial_port_uart_config_t *)&uartDmaConfig2,
};
#else
static serial_port_uart_config_t uartConfig1 = {
    .instance     = BOARD_APP_UART_INSTANCE,
    .baudRate     = BOARD_APP_UART_BAUDRATE,
    .parityMode   = kSerialManager_UartParityDisabled,
    .stopBitCount = kSerialManager_UartOneStopBit,
    .enableRx     = 1,
    .enableTx     = 1,
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0) && (BOARD_APP_UART_INSTANCE == 0U))
    .enableRxRTS  = 1,
    .enableTxCTS  = 1,
#endif
};

static const serial_manager_config_t s_serialManagerConfig1 = {
    .type           = BOARD_APP_UART_TYPE,
    .ringBuffer     = &s_ringBuffer1[0],
    .ringBufferSize = SERIAL_MANAGER_RING_BUFFER_SIZE,
    .portConfig     = (serial_port_uart_config_t *)&uartConfig1,
};

static serial_port_uart_config_t uartConfig2 = {
    .instance     = BOARD_APP2_UART_INSTANCE,
    .baudRate     = BOARD_APP2_UART_BAUDRATE,
    .parityMode   = kSerialManager_UartParityDisabled,
    .stopBitCount = kSerialManager_UartOneStopBit,
    .enableRx     = 1,
    .enableTx     = 1,
#if (defined(gBoardUseUart0HwFlowControl) && (gBoardUseUart0HwFlowControl > 0) && (BOARD_APP2_UART_INSTANCE == 0U))
    .enableRxRTS  = 1,
    .enableTxCTS  = 1,
#endif
};

static const serial_manager_config_t s_serialManagerConfig2 = {
    .type           = BOARD_APP_UART_TYPE,
    .ringBuffer     = &s_ringBuffer2[0],
    .ringBufferSize = SERIAL_MANAGER_RING_BUFFER_SIZE,
    .portConfig     = (serial_port_uart_config_t *)&uartConfig2,
};
#endif

#endif /* defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c >= 1) */

#if defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)

/*LED(B1) pin configuration*/
static const led_config_t g_monoLedConfig = {
    .type = kLED_TypeMonochrome,
    .ledMonochrome =
        {
            .monochromePin =
                {
                    .dimmingEnable = 0,
                    .gpio =
                        {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                            kHAL_GpioDirectionOut,
#endif
                            BOARD_MONOCHROME_GPIO_PIN_DEFAULT_STATE,
                            BOARD_MONOCHROME_GPIO_PORT_INSTANCE,
                            BOARD_MONOCHROME_GPIO_PIN,
                        },
                },
        },
};

static const led_config_t g_RgbLedConfig = {
    .type = kLED_TypeRgb,
    .ledRgb =
        {
            .redPin =
                {
                    .dimmingEnable = 0,
                    .gpio =
                        {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                            .direction = kHAL_GpioDirectionOut,
#endif
                            .level = BOARD_RGB_RED_GPIO_PIN_DEFAULT_STATE,
                            .port  = BOARD_RGB_RED_GPIO_PORT_INSTANCE,
                            .pin   = BOARD_RGB_RED_GPIO_PIN,
                        },
                },
            .greenPin =
                {
                    .dimmingEnable = 0,
                    .gpio =
                        {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                            .direction = kHAL_GpioDirectionOut,
#endif
                            .level = BOARD_RGB_GREEN_GPIO_PIN_DEFAULT_STATE,
                            .port  = BOARD_RGB_GREEN_GPIO_PORT_INSTANCE,
                            .pin   = BOARD_RGB_GREEN_GPIO_PIN,
                        },
                },
            .bluePin =
                {
                    .dimmingEnable = 0,
                    .gpio =
                        {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                            .direction = kHAL_GpioDirectionOut,
#endif
                            .level = BOARD_RGB_BLUE_GPIO_PIN_DEFAULT_STATE,
                            .port  = BOARD_RGB_BLUE_GPIO_PORT_INSTANCE,
                            .pin   = BOARD_RGB_BLUE_GPIO_PIN,
                        },
                },
        },
};

static const led_config_t g_RedLedConfig = {
    .type = kLED_TypeMonochrome,
    .ledMonochrome =
        {
            .monochromePin =
                {
                    .dimmingEnable = 0,
                    .gpio =
                        {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                            kHAL_GpioDirectionOut,
#endif
                            BOARD_RED_GPIO_PIN_DEFAULT_STATE,
                            BOARD_RED_GPIO_PORT_INSTANCE,
                            BOARD_RED_GPIO_PIN,
                        },
                },
        },
};

static const led_config_t g_YellowLedConfig = {
    .type = kLED_TypeMonochrome,
    .ledMonochrome =
        {
            .monochromePin =
                {
                    .dimmingEnable = 0,
                    .gpio =
                        {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                            kHAL_GpioDirectionOut,
#endif
                            BOARD_YELLOW_GPIO_PIN_DEFAULT_STATE,
                            BOARD_YELLOW_GPIO_PORT_INSTANCE,
                            BOARD_YELLOW_GPIO_PIN,
                        },
                },
        },
};

#endif /* defined(gAppLedCnt_c) && (gAppLedCnt_c > 0) */

#if defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)

/*Button pins configuration*/
static const button_config_t g_button0Config = {
    .gpio =
        {
            .pinStateDefault = BOARD_BUTTON0_GPIO_PIN_DEFAULT_STATE,
            .port            = BOARD_BUTTON0_GPIO_PORT_INSTANCE,
            .pin             = BOARD_BUTTON0_GPIO_PIN,
        },
};

static const button_config_t g_button1Config = {
    .gpio =
        {
            .pinStateDefault = BOARD_BUTTON1_GPIO_PIN_DEFAULT_STATE,
            .port            = BOARD_BUTTON1_GPIO_PORT_INSTANCE,
            .pin             = BOARD_BUTTON1_GPIO_PIN,
        },
};

#endif /* defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0) */

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c >= 1)

/* Initialize the serial manager */
void BOARD_InitSerialManager(serial_handle_t serialManagerHandle)
{
    serial_manager_status_t ret;

    BOARD_InitAppConsole();
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    uartDmaConfig1.clockRate = CLOCK_GetIpFreq(BOARD_APP_UART_CLK);
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    dma_mux_configure_t dma_mux;
    dma_mux.dma_dmamux_configure.dma_mux_instance = 0;
    dma_mux.dma_dmamux_configure.rx_request       = BOARD_APP_UART_DMAREQMUX_RX;
    dma_mux.dma_dmamux_configure.tx_request       = BOARD_APP_UART_DMAREQMUX_TX;
    uartDmaConfig1.dma_mux_configure              = &dma_mux;
#endif
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    dma_channel_mux_configure_t dma_channel_mux;
    dma_channel_mux.dma_dmamux_configure.dma_tx_channel_mux = BOARD_APP_UART_DMAREQ_TX;
    dma_channel_mux.dma_dmamux_configure.dma_rx_channel_mux = BOARD_APP_UART_DMAREQ_RX;
    uartDmaConfig1.dma_channel_mux_configure                = &dma_channel_mux;
#endif
#else
#if !defined(FPGA_SUPPORT) || (FPGA_SUPPORT == 0)
    uartConfig1.clockRate = CLOCK_GetIpFreq(BOARD_APP_UART_CLK);
#else
    uartConfig1.clockRate = 16000000;
#endif
#endif
    /* Init Serial Manager */
    ret = SerialManager_Init((serial_handle_t)serialManagerHandle, &s_serialManagerConfig1);
    assert(kStatus_SerialManager_Success == ret);
    (void)ret;
}

/* Initialize the second serial manager */
void BOARD_InitSerialManager2(serial_handle_t serialManagerHandle)
{
    serial_manager_status_t ret;

    /* Second instance of serial manager uses the UART for Debug Console */
    BOARD_InitApp2Console();
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    uartDmaConfig2.clockRate = CLOCK_GetIpFreq(BOARD_APP2_UART_CLK);
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    dma_mux_configure_t dma_mux;
    dma_mux.dma_dmamux_configure.dma_mux_instance = 0;
    dma_mux.dma_dmamux_configure.rx_request       = BOARD_APP2_UART_DMAREQMUX_RX;
    dma_mux.dma_dmamux_configure.tx_request       = BOARD_APP2_UART_DMAREQMUX_TX;
    uartDmaConfig2.dma_mux_configure              = &dma_mux;
#endif
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    dma_channel_mux_configure_t dma_channel_mux;
    dma_channel_mux.dma_dmamux_configure.dma_tx_channel_mux = BOARD_APP2_UART_DMAREQ_TX;
    dma_channel_mux.dma_dmamux_configure.dma_rx_channel_mux = BOARD_APP2_UART_DMAREQ_RX;
    uartDmaConfig2.dma_channel_mux_configure                = &dma_channel_mux;
#endif
#else
    uartConfig2.clockRate = CLOCK_GetIpFreq(BOARD_APP2_UART_CLK);
#endif

    /* Init Serial Manager */
    ret = SerialManager_Init((serial_handle_t)serialManagerHandle, &s_serialManagerConfig2);
    assert(kStatus_SerialManager_Success == ret);
    (void)ret;
}

/* DeInit Serial Manager before going lowpower when wake domain is in DS  (device DS3 or lower)*/
void BOARD_UninitSerialManager(void)
{
    /* disable clock and pins */
    BOARD_UninitAppConsole();
}

/* Initialize the serial manager */
void BOARD_ReinitSerialManager(serial_handle_t serialManagerHandle)
{
    serial_manager_status_t ret;

    /* Init LPUART clock and pins */
    BOARD_InitAppConsole();

    ret = SerialManager_ExitLowpower((serial_handle_t)serialManagerHandle);
    assert(kStatus_SerialManager_Success == ret);
    (void)ret;
}

/* DeInit Serial Manager before going lowpower when wake domain is in DS  (device DS3 or lower)*/
void BOARD_UninitSerialManager2(void)
{
    /* disable clock and pins */
    BOARD_UninitApp2Console();
}
/* Change the serial manager BaudRate*/
void BOARD_ChangeSerialManagerBaudRate(serial_handle_t serialManagerHandle, uint32_t baudrate)
{
    serial_manager_status_t ret;

    ret = SerialManager_Deinit((serial_handle_t)serialManagerHandle);
    assert(kStatus_SerialManager_Success == ret);
    (void)ret;
#if (defined(HAL_UART_DMA_ENABLE) && (HAL_UART_DMA_ENABLE > 0U))
    uartDmaConfig1.baudRate = baudrate;
#else
    uartConfig1.baudRate  = baudrate;
#endif
    BOARD_InitSerialManager((serial_handle_t)serialManagerHandle);
}

/* Initialize the serial manager */
void BOARD_ReinitSerialManager2(serial_handle_t serialManagerHandle)
{
    serial_manager_status_t ret;

    /* Init LPUART clock and pins */
    BOARD_InitApp2Console();

    ret = SerialManager_ExitLowpower((serial_handle_t)serialManagerHandle);
    assert(kStatus_SerialManager_Success == ret);
    (void)ret;
}

#endif /* defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c >= 1) */

#if defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)

/*Initialize LED*/
void BOARD_InitMonochromeLed(led_handle_t ledHandle)
{
    led_status_t ret;

    BOARD_InitPinLED1();

    ret = LED_Init(ledHandle, &g_monoLedConfig);
    assert(kStatus_LED_Success == ret);
    (void)ret;
}

void BOARD_UnInitMonochromeLed(led_handle_t ledHandle)
{
    led_status_t ret;
    ret = LED_TurnOnOff(ledHandle, 0U);
    assert(kStatus_LED_Success == ret);
    (void)ret;
    BOARD_UnInitPinLED1();
}

/*Initialize LED*/
void BOARD_InitRgbLed(led_handle_t ledHandle)
{
    led_status_t ret;

    BOARD_InitPinLED2();
    BOARD_InitPinLED3();
    BOARD_InitPinLED4();

    ret = LED_Init(ledHandle, &g_RgbLedConfig);
    assert(kStatus_LED_Success == ret);
    (void)ret;
}

void BOARD_UnInitRgbLed(led_handle_t ledHandle)
{
    led_status_t ret = LED_TurnOnOff(ledHandle, 0U);
    assert(kStatus_LED_Success == ret);
    (void)ret;
    BOARD_UnInitPinLED2();
    BOARD_UnInitPinLED3();
    BOARD_UnInitPinLED4();
}

#endif /* defined(gAppLedCnt_c) && (gAppLedCnt_c > 0) */

#if defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)

/*Initialize Button*/
void BOARD_InitButton0(button_handle_t buttonHandle)
{
    button_status_t ret;
    button_config_t button_config = g_button0Config;
    /* Init Pin mux */
    BOARD_InitPinButton0();

    /* Init button module and Gpio module */
    ret = BUTTON_Init(buttonHandle, &button_config);
    assert(ret == kStatus_BUTTON_Success);
    (void)ret;
}

void BOARD_Button0ExitPowerDown(button_handle_t buttonHandle)
{
    button_status_t ret;

    /* Restore pin mux config */
    BOARD_InitPinButton0();

    ret = BUTTON_ExitLowpower(buttonHandle);
    assert(ret == kStatus_BUTTON_Success);
    (void)ret;
}

void BOARD_InitButton1(button_handle_t buttonHandle)
{
    button_status_t ret;
    button_config_t button_config = g_button1Config;
    /* Init Pin mux */
    BOARD_InitPinButton1();

    /* Init button module and Gpio module */
    ret = BUTTON_Init(buttonHandle, &button_config);
    assert(ret == kStatus_BUTTON_Success);
    (void)ret;
}

void BOARD_Button1ExitPowerDown(button_handle_t buttonHandle)
{
    button_status_t ret;

    /* Restore pin mux config */
    BOARD_InitPinButton1();

    ret = BUTTON_ExitLowpower(buttonHandle);
    assert(ret == kStatus_BUTTON_Success);
    (void)ret;
}

#endif /* defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0) */

/************************************************************************************
*************************************************************************************
* Localization board specific functions
*************************************************************************************
************************************************************************************/

#if defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)

/*Initialize LED*/
void BOARD_InitRedLed(led_handle_t ledHandle)
{
    led_status_t ret;

    BOARD_InitPinLED1();

    ret = LED_Init(ledHandle, &g_RedLedConfig);
    assert(kStatus_LED_Success == ret);
    (void)ret;
}

void BOARD_UnInitRedLed(led_handle_t ledHandle)
{
    led_status_t ret;
    ret = LED_TurnOnOff(ledHandle, 0U);
    assert(kStatus_LED_Success == ret);
    (void)ret;
    BOARD_UnInitPinLED1();
}

/*Initialize LED*/
void BOARD_InitYellowLed(led_handle_t ledHandle)
{
    led_status_t ret;

    BOARD_InitPinLED2();

    ret = LED_Init(ledHandle, &g_YellowLedConfig);
    assert(kStatus_LED_Success == ret);
    (void)ret;
}

void BOARD_UnInitYellowLed(led_handle_t ledHandle)
{
    led_status_t ret;
    ret = LED_TurnOnOff(ledHandle, 0U);
    assert(kStatus_LED_Success == ret);
    (void)ret;
    BOARD_UnInitPinLED2();
}

#endif /* defined(gAppLedCnt_c) && (gAppLedCnt_c > 0) */