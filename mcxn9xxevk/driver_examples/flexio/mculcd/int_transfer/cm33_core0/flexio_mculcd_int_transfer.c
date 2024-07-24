/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_flexio_mculcd.h"
#include "fsl_debug_console.h"

#if BOARD_LCD_S035
#include "fsl_st7796s.h"
#else
#include "fsl_ssd1963.h"
#endif

#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_PANEL_WIDTH  800U
#define DEMO_PANEL_HEIGHT 480U

/* Macros for the LCD controller. */
#define DEMO_SSD1963_XTAL_FREQ     10000000U
#define DEMO_SSD1963_PCLK_FREQ     30000000U
#define DEMO_SSD1963_HSW           48U
#define DEMO_SSD1963_HFP           40U
#define DEMO_SSD1963_HBP           0U
#define DEMO_SSD1963_VSW           3U
#define DEMO_SSD1963_VFP           13U
#define DEMO_SSD1963_VBP           18U
#define DEMO_SSD1963_POLARITY_FLAG 0U
#define DEMO_SSD1963_RST_GPIO      GPIO4
#define DEMO_SSD1963_RST_PIN       7
#define DEMO_SSD1963_CS_GPIO       GPIO0
#define DEMO_SSD1963_CS_PIN        12
#define DEMO_SSD1963_RS_GPIO       GPIO0
#define DEMO_SSD1963_RS_PIN        7

#define DEMO_FLEXIO              FLEXIO0
#define DEMO_FLEXIO_CLOCK_FREQ   CLOCK_GetFlexioClkFreq()
#define DEMO_FLEXIO_BAUDRATE_BPS 160000000U /* 16-bit data bus, baud rate on each pin is 1MHz. */

/* Macros for FlexIO shifter, timer, and pins. */
#define DEMO_FLEXIO_WR_PIN           1
#define DEMO_FLEXIO_RD_PIN           0
#define DEMO_FLEXIO_DATA_PIN_START   16
#define DEMO_FLEXIO_TX_START_SHIFTER 0
#define DEMO_FLEXIO_RX_START_SHIFTER 0
#define DEMO_FLEXIO_TX_END_SHIFTER   7
#define DEMO_FLEXIO_RX_END_SHIFTER   7
#define DEMO_FLEXIO_TIMER            0


/* Buffer of the LCD send data. */
#define DEMO_BUFFER_WIDTH  (DEMO_PANEL_WIDTH / 2U)
#define DEMO_BUFFER_HEIGHT (DEMO_PANEL_HEIGHT / 2U)

/* Color for RGB565 */
typedef uint16_t demo_pixel_t;
#define DEMO_COLOR_RED   0xF800U
#define DEMO_COLOR_GREEN 0x07E0U
#define DEMO_COLOR_BLUE  0x001FU
#define DEMO_COLOR_WHITE 0xFFFFU
#define DEMO_COLOR_BLACK 0x0000U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_SetCSPin(bool set);
void BOARD_SetRSPin(bool set);
void BOARD_SetResetPin(bool set);
static void DEMO_InitFlexioMcuLcd(void);

static void DEMO_InitPanel(void);

static void DEMO_StartPanel(void);

static void DEMO_DrawWindow(uint16_t startX,
                            uint16_t startY,
                            uint16_t endX,
                            uint16_t endY,
                            const uint8_t *data,
                            uint32_t dataLen,
                            uint32_t delay);

static void DEMO_DrawMultiWindows(demo_pixel_t color, uint32_t intervalMs);

static status_t DEMO_LcdWriteCommand(void *dbiXferHandle, uint32_t command);

static status_t DEMO_LcdWriteData(void *dbiXferHandle, void *data, uint32_t len_byte);

static status_t DEMO_LcdWriteMemory(void *dbiXferHandle, uint32_t command, const void *data, uint32_t len_byte);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* The FlexIO MCU LCD device. */
static FLEXIO_MCULCD_Type flexioLcdDev = {
    .flexioBase          = DEMO_FLEXIO,
    .busType             = kFLEXIO_MCULCD_8080,
    .dataPinStartIndex   = DEMO_FLEXIO_DATA_PIN_START,
    .ENWRPinIndex        = DEMO_FLEXIO_WR_PIN,
    .RDPinIndex          = DEMO_FLEXIO_RD_PIN,
    .txShifterStartIndex = DEMO_FLEXIO_TX_START_SHIFTER,
    .txShifterEndIndex   = DEMO_FLEXIO_TX_END_SHIFTER,
    .rxShifterStartIndex = DEMO_FLEXIO_RX_START_SHIFTER,
    .rxShifterEndIndex   = DEMO_FLEXIO_RX_END_SHIFTER,
    .timerIndex          = DEMO_FLEXIO_TIMER,
    .setCSPin            = BOARD_SetCSPin,
    .setRSPin            = BOARD_SetRSPin,
    .setRDWRPin          = NULL /* Not used in 8080 mode. */
};

/* The functions used to drive the panel. */
static dbi_xfer_ops_t s_flexioDbiOps = {
    .writeCommand          = DEMO_LcdWriteCommand,
    .writeData             = DEMO_LcdWriteData,
    .writeMemory           = DEMO_LcdWriteMemory,
    .readMemory            = NULL, /* Don't need read in this project. */
    .setMemoryDoneCallback = NULL, /* This used for upper layer to register callback.
                                      If the writeMemory is a non-blocking(or async)
                                      function, when write finished, the registered
                                      callback function will be called.
                                      In this project, to be simple, DEMO_LcdWriteMemory
                                      blocks to wait the write done, it is not async
                                      implementation, so don't need to set callback.
                                    */
};

#if BOARD_LCD_S035
static st7796s_handle_t lcdHandle;
#else
static ssd1963_handle_t lcdHandle;
#endif
static flexio_mculcd_handle_t s_flexioLcdHandle;
static demo_pixel_t s_lcdBuffer[DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH];
static volatile bool s_transferDone = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_SetCSPin(bool set)
{
    GPIO_PinWrite(DEMO_SSD1963_CS_GPIO, DEMO_SSD1963_CS_PIN, (uint8_t)set);
}

void BOARD_SetRSPin(bool set)
{
    GPIO_PinWrite(DEMO_SSD1963_RS_GPIO, DEMO_SSD1963_RS_PIN, (uint8_t)set);
}

void BOARD_SetResetPin(bool set)
{
    GPIO_PinWrite(DEMO_SSD1963_RST_GPIO, DEMO_SSD1963_RST_PIN, (uint8_t)set);
}
static void DEMO_FLEXIO_TransferCallback(FLEXIO_MCULCD_Type *base,
                                         flexio_mculcd_handle_t *handle,
                                         status_t status,
                                         void *userData)
{
    s_transferDone = true;
}

static status_t DEMO_LcdWriteCommand(void *dbiXferHandle, uint32_t command)
{
    /* dbiXferHandle is actually s_flexioLcdHandle, set by SSD1963_Init 4th parameter. */

    /* Here use blocking way to send command, because command is short. */
    FLEXIO_MCULCD_Type *flexioLCD = &flexioLcdDev;

    FLEXIO_MCULCD_StartTransfer(flexioLCD);
    FLEXIO_MCULCD_WriteCommandBlocking(flexioLCD, command);
    FLEXIO_MCULCD_StopTransfer(flexioLCD);

    return kStatus_Success;
}

static status_t DEMO_LcdWriteData(void *dbiXferHandle, void *data, uint32_t len_byte)
{
    /* dbiXferHandle is actually s_flexioLcdHandle, set by SSD1963_Init 4th parameter. */

    /*
     * Here use blocking way to write data, this function is generally
     * used to send command parameter, the data length is short.
     */
    FLEXIO_MCULCD_Type *flexioLCD = &flexioLcdDev;

    FLEXIO_MCULCD_StartTransfer(flexioLCD);
    FLEXIO_MCULCD_WriteDataArrayBlocking(flexioLCD, data, len_byte);
    FLEXIO_MCULCD_StopTransfer(flexioLCD);

    return kStatus_Success;
}

static status_t DEMO_LcdWriteMemory(void *dbiXferHandle, uint32_t command, const void *data, uint32_t len_byte)
{
    status_t status;

    /* dbiXferHandle is actually s_flexioLcdHandle, set by SSD1963_Init 4th parameter. */
    flexio_mculcd_handle_t *flexioLcdHandle = (flexio_mculcd_handle_t *)dbiXferHandle;

    FLEXIO_MCULCD_Type *flexioLCD = &flexioLcdDev;

    flexio_mculcd_transfer_t xfer = {
        .command             = command,
        .mode                = kFLEXIO_MCULCD_WriteArray,
        .dataAddrOrSameValue = (uint32_t)data,
        .dataSize            = len_byte,
    };

    s_transferDone = false;
    status         = FLEXIO_MCULCD_TransferNonBlocking(flexioLCD, flexioLcdHandle, &xfer);

    if (kStatus_Success != status)
    {
        PRINTF("Write Memory Failed\r\n");
        while (1)
        {
        }
    }
    else
    {
        /* Wait for transfer done. */
        while (!s_transferDone)
        {
        }
    }

    return status;
}

static void DEMO_InitFlexioMcuLcd(void)
{
    status_t status;

    flexio_mculcd_config_t flexioMcuLcdConfig;

    /* Initialize the flexio MCU LCD. */
    /*
     * flexioMcuLcdConfig.enable = true;
     * flexioMcuLcdConfig.enableInDoze = false;
     * flexioMcuLcdConfig.enableInDebug = true;
     * flexioMcuLcdConfig.enableFastAccess = true;
     * flexioMcuLcdConfig.baudRate_Bps = 96000000U;
     */
    FLEXIO_MCULCD_GetDefaultConfig(&flexioMcuLcdConfig);
    flexioMcuLcdConfig.baudRate_Bps = DEMO_FLEXIO_BAUDRATE_BPS;

    status = FLEXIO_MCULCD_Init(&flexioLcdDev, &flexioMcuLcdConfig, DEMO_FLEXIO_CLOCK_FREQ);

    if (kStatus_Success != status)
    {
        PRINTF("FlexIO MCULCD init failed\r\n");
        while (1)
            ;
    }

    status = FLEXIO_MCULCD_TransferCreateHandle(&flexioLcdDev, &s_flexioLcdHandle, DEMO_FLEXIO_TransferCallback, NULL);

    if (kStatus_Success != status)
    {
        PRINTF("FlexIO MCULCD handle creation failed\r\n");
        while (1)
            ;
    }
}

static void DEMO_InitPanel(void)
{
    status_t status;

#if BOARD_LCD_S035
    const st7796s_config_t st7796sConfig = {.driverPreset    = kST7796S_DriverPresetLCDPARS035,
                                            .pixelFormat     = kST7796S_PixelFormatRGB565,
                                            .orientationMode = kST7796S_Orientation270,
                                            .teConfig        = kST7796S_TEDisabled,
                                            .invertDisplay   = true,
                                            .flipDisplay     = true,
                                            .bgrFilter       = true};
#else
    const ssd1963_config_t ssd1963Config = {
        .pclkFreq_Hz = DEMO_SSD1963_PCLK_FREQ,
#if SSD1963_DATA_WITDH == 16
        .pixelInterface = kSSD1963_PixelInterface16Bit565,
#else
        .pixelInterface = kSSD1963_PixelInterface8BitBGR888,
#endif
        .panelDataWidth = kSSD1963_PanelData24Bit,
        .polarityFlags  = DEMO_SSD1963_POLARITY_FLAG,
        .panelWidth     = DEMO_PANEL_WIDTH,
        .panelHeight    = DEMO_PANEL_HEIGHT,
        .hsw            = DEMO_SSD1963_HSW,
        .hfp            = DEMO_SSD1963_HFP,
        .hbp            = DEMO_SSD1963_HBP,
        .vsw            = DEMO_SSD1963_VSW,
        .vfp            = DEMO_SSD1963_VFP,
        .vbp            = DEMO_SSD1963_VBP
    };
#endif

    /* Reset the SSD1963 LCD controller. */
    BOARD_SetResetPin(false);
    SDK_DelayAtLeastUs(1, SystemCoreClock); /* Delay 10ns. */
    BOARD_SetResetPin(true);
    SDK_DelayAtLeastUs(5000, SystemCoreClock); /* Delay 5ms. */

#if BOARD_LCD_S035
    status = ST7796S_Init(&lcdHandle, &st7796sConfig, &s_flexioDbiOps, &s_flexioLcdHandle);
#else
    status = SSD1963_Init(&lcdHandle, &ssd1963Config, &s_flexioDbiOps, &s_flexioLcdHandle, DEMO_SSD1963_XTAL_FREQ);
#endif

    if (kStatus_Success != status)
    {
        PRINTF("Panel initialization failed\r\n");
        while (1)
        {
        }
    }
}

static void DEMO_StartPanel(void)
{
#if BOARD_LCD_S035
    ST7796S_EnableDisplay(&lcdHandle, true);
#else
    SSD1963_StartDisplay(&lcdHandle);
    SSD1963_SetBackLight(&lcdHandle, 255);
#endif
}

/*
 * Draw one window with pixel array, and delay some time after draw done.
 */
static void DEMO_DrawWindow(uint16_t startX,
                            uint16_t startY,
                            uint16_t endX,
                            uint16_t endY,
                            const uint8_t *data,
                            uint32_t dataLen,
                            uint32_t delayMsAfterDraw)
{
#if BOARD_LCD_S035
    ST7796S_SelectArea(&lcdHandle, startX, startY, endX, endY);
    ST7796S_WritePixels(&lcdHandle, (uint16_t *)data, dataLen / 2);
#else
    SSD1963_SelectArea(&lcdHandle, startX, startY, endX, endY);
    SSD1963_WriteMemory(&lcdHandle, data, dataLen);
#endif

    if (delayMsAfterDraw > 0)
    {
        SDK_DelayAtLeastUs(delayMsAfterDraw * 1000, SystemCoreClock);
    }
}

/*
 * Draw multiple windows with specified color, can delay some time each window draw finished.
 * In this example, the whole screen is devided in to 4 windows, this function fills the
 * windows one by one.
 */
static void DEMO_DrawMultiWindows(demo_pixel_t color, uint32_t intervalMs)
{
    /* Fill buffer with pixel. */
    for (uint32_t y = 0; y < DEMO_BUFFER_HEIGHT; y++)
    {
        for (uint32_t x = 0; x < DEMO_BUFFER_WIDTH; x++)
        {
            s_lcdBuffer[y][x] = color;
        }
    }

    /* Region 0. */
    DEMO_DrawWindow(0, 0, DEMO_BUFFER_WIDTH - 1, DEMO_BUFFER_HEIGHT - 1, (const uint8_t *)s_lcdBuffer,
                    sizeof(s_lcdBuffer), intervalMs);

    /* Region 1. */
    DEMO_DrawWindow(DEMO_BUFFER_WIDTH, 0, 2 * DEMO_BUFFER_WIDTH - 1, DEMO_BUFFER_HEIGHT - 1,
                    (const uint8_t *)s_lcdBuffer, sizeof(s_lcdBuffer), intervalMs);

    /* Region 2. */
    DEMO_DrawWindow(0, DEMO_BUFFER_HEIGHT, DEMO_BUFFER_WIDTH - 1, 2 * DEMO_BUFFER_HEIGHT - 1,
                    (const uint8_t *)s_lcdBuffer, sizeof(s_lcdBuffer), intervalMs);

    /* Region 3. */
    DEMO_DrawWindow(DEMO_BUFFER_WIDTH, DEMO_BUFFER_HEIGHT, 2 * DEMO_BUFFER_WIDTH - 1, 2 * DEMO_BUFFER_HEIGHT - 1,
                    (const uint8_t *)s_lcdBuffer, sizeof(s_lcdBuffer), intervalMs);
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t colorIdx                = 0;
    const demo_pixel_t colorTable[] = {
        DEMO_COLOR_RED,
        DEMO_COLOR_GREEN,
        DEMO_COLOR_BLUE,
        DEMO_COLOR_WHITE,
    };

    const gpio_pin_config_t resetPinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1,
    };

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, 1u);
    CLOCK_AttachClk(kPLL0_to_FLEXIO);

    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio2);
    CLOCK_EnableClock(kCLOCK_Gpio4);

    /* Set SSD1963 CS, RS, and reset pin to output. */
    GPIO_PinInit(DEMO_SSD1963_RST_GPIO, DEMO_SSD1963_RST_PIN, &resetPinConfig);
    GPIO_PinInit(DEMO_SSD1963_CS_GPIO, DEMO_SSD1963_CS_PIN, &resetPinConfig);
    GPIO_PinInit(DEMO_SSD1963_RS_GPIO, DEMO_SSD1963_RS_PIN, &resetPinConfig);

    DEMO_InitFlexioMcuLcd();

    DEMO_InitPanel();

    /*
     * First fill the panel with black, then start show. Otherwise when start show,
     * the panel memory is random value.
     */
    DEMO_DrawMultiWindows(DEMO_COLOR_BLACK, 0);

    DEMO_StartPanel();

    while (1)
    {
        DEMO_DrawMultiWindows(colorTable[colorIdx], 500);

        colorIdx++;
        if (colorIdx >= ARRAY_SIZE(colorTable))
        {
            colorIdx = 0U;
        }
    }
}
