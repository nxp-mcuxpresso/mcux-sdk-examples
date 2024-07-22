/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_flexio_mculcd.h"
#include "fsl_debug_console.h"
#include "fsl_st7796s.h"
#include "fsl_gpio.h"
#include "fsl_smartdma.h"
#include "fsl_smartdma_mcxn.h"
#include "fsl_smartdma_prv.h"
#include "fsl_inputmux.h"
#include "fsl_ov7670.h"
#include "fsl_clock.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_BUFFER_WIDTH  DEMO_PANEL_WIDTH
#define DEMO_BUFFER_HEIGHT DEMO_PANEL_HEIGHT

#define DEMO_CAMERA_RESOLUTION kVIDEO_ResolutionVGA /* 640*480 */

#define DEMO_SMARTDMA_API kSMARTDMA_CameraWholeFrame320_480

#define DEMO_LCD_RST_GPIO GPIO4
#define DEMO_LCD_RST_PIN  7U
#define DEMO_LCD_CS_GPIO  GPIO0
#define DEMO_LCD_CS_PIN   12U
#define DEMO_LCD_RS_GPIO  GPIO0
#define DEMO_LCD_RS_PIN   7U

#define DEMO_FLEXIO              FLEXIO0
#define DEMO_FLEXIO_CLOCK_FREQ   CLOCK_GetFlexioClkFreq()
#define DEMO_FLEXIO_BAUDRATE_BPS 160000000U /* 16-bit data bus, baud rate on each pin is 1MHz. */

/* Macros for FlexIO shifter, timer, and pins. */
#define DEMO_FLEXIO_WR_PIN           1U
#define DEMO_FLEXIO_RD_PIN           0U
#define DEMO_FLEXIO_DATA_PIN_START   16U
#define DEMO_FLEXIO_TX_START_SHIFTER 0U
#define DEMO_FLEXIO_RX_START_SHIFTER 0U
#define DEMO_FLEXIO_TX_END_SHIFTER   7U
#define DEMO_FLEXIO_RX_END_SHIFTER   7U
#define DEMO_FLEXIO_TIMER            0U
#define DEMO_PANEL_WIDTH  480U
#define DEMO_PANEL_HEIGHT 320U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_SetCSPin(bool set);

void BOARD_SetRSPin(bool set);

void BOARD_SetResetPin(bool set);

static void DEMO_InitFlexioMcuLcd(void);

static void DEMO_InitPanel(void);

static void DEMO_DrawWindow(
    uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, const uint8_t *data, uint32_t pixelLen);

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
    .setMemoryDoneCallback = NULL, /* Write memory is blocking function, don' need callback. */
};

/* Panel handle. */
static st7796s_handle_t lcdHandle;

/* Camera resource and handle */
const ov7670_resource_t resource = {
    .i2cSendFunc    = BOARD_Camera_I2C_SendSCCB,
    .i2cReceiveFunc = BOARD_Camera_I2C_ReceiveSCCB,
    .xclock         = kOV7670_InputClock24MHZ,
};

camera_device_handle_t handle = {
    .resource = (void *)&resource,
    .ops      = &ov7670_ops,
};

static volatile bool g_camera_complete_flag = false;
static uint16_t g_camera_buffer[DEMO_BUFFER_WIDTH * DEMO_BUFFER_HEIGHT];
volatile uint8_t g_samrtdma_stack[32] = {0};
/*******************************************************************************
 * Code
 ******************************************************************************/
static void SDMA_CompleteCallback(void *param)
{
    g_camera_complete_flag = true;
}

void BOARD_SetCSPin(bool set)
{
    GPIO_PinWrite(DEMO_LCD_CS_GPIO, DEMO_LCD_CS_PIN, (uint8_t)set);
}

void BOARD_SetRSPin(bool set)
{
    GPIO_PinWrite(DEMO_LCD_RS_GPIO, DEMO_LCD_RS_PIN, (uint8_t)set);
}

void BOARD_SetResetPin(bool set)
{
    GPIO_PinWrite(DEMO_LCD_RST_GPIO, DEMO_LCD_RST_PIN, (uint8_t)set);
}

static status_t DEMO_LcdWriteCommand(void *dbiXferHandle, uint32_t command)
{
    /* dbiXferHandle is actually flexioLcdDev, set by LCD_Init 4th parameter. */
    FLEXIO_MCULCD_Type *flexioLCD = (FLEXIO_MCULCD_Type *)dbiXferHandle;

    FLEXIO_MCULCD_StartTransfer(flexioLCD);
    FLEXIO_MCULCD_WriteCommandBlocking(flexioLCD, command);
    FLEXIO_MCULCD_StopTransfer(flexioLCD);

    return kStatus_Success;
}

static status_t DEMO_LcdWriteData(void *dbiXferHandle, void *data, uint32_t len_byte)
{
    /* dbiXferHandle is actually flexioLcdDev, set by LCD_Init 4th parameter. */
    FLEXIO_MCULCD_Type *flexioLCD = (FLEXIO_MCULCD_Type *)dbiXferHandle;

    FLEXIO_MCULCD_StartTransfer(flexioLCD);
    FLEXIO_MCULCD_WriteDataArrayBlocking(flexioLCD, data, len_byte);
    FLEXIO_MCULCD_StopTransfer(flexioLCD);

    return kStatus_Success;
}

static status_t DEMO_LcdWriteMemory(void *dbiXferHandle, uint32_t command, const void *data, uint32_t len_byte)
{
    /* dbiXferHandle is actually flexioLcdDev, set by LCD_Init 4th parameter. */
    FLEXIO_MCULCD_Type *flexioLCD = (FLEXIO_MCULCD_Type *)dbiXferHandle;

    FLEXIO_MCULCD_StartTransfer(flexioLCD);
    FLEXIO_MCULCD_WriteCommandBlocking(flexioLCD, command);
    FLEXIO_MCULCD_WriteDataArrayBlocking(flexioLCD, data, len_byte);
    FLEXIO_MCULCD_StopTransfer(flexioLCD);

    return kStatus_Success;
}

static void DEMO_InitFlexioMcuLcd(void)
{
    status_t status;

    flexio_mculcd_config_t flexioMcuLcdConfig;

    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1,
    };

    /* Set LCD CS, RS, and reset pin to output. */
    GPIO_PinInit(DEMO_LCD_RST_GPIO, DEMO_LCD_RST_PIN, &pinConfig);
    GPIO_PinInit(DEMO_LCD_CS_GPIO, DEMO_LCD_CS_PIN, &pinConfig);
    GPIO_PinInit(DEMO_LCD_RS_GPIO, DEMO_LCD_RS_PIN, &pinConfig);

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
}

static void DEMO_InitPanel(void)
{
    status_t status;

    const st7796s_config_t st7796sConfig = {.driverPreset    = kST7796S_DriverPresetLCDPARS035,
                                            .pixelFormat     = kST7796S_PixelFormatRGB565,
                                            .orientationMode = kST7796S_Orientation180,
                                            .teConfig        = kST7796S_TEDisabled,
                                            .invertDisplay   = true,
                                            .flipDisplay     = true,
                                            .bgrFilter       = true};

    /* Reset the LCD LCD controller. */
    BOARD_SetResetPin(false);
    SDK_DelayAtLeastUs(1, SystemCoreClock);    /* Delay 10ns. */
    BOARD_SetResetPin(true);
    SDK_DelayAtLeastUs(5000, SystemCoreClock); /* Delay 5ms. */

    status = ST7796S_Init(&lcdHandle, &st7796sConfig, &s_flexioDbiOps, &flexioLcdDev);

    if (kStatus_Success != status)
    {
        PRINTF("Panel initialization failed\r\n");
        while (1)
        {
        }
    }

    ST7796S_EnableDisplay(&lcdHandle, true);
}

/*
 * Draw one window with pixel array, and delay some time after draw done.
 */
static void DEMO_DrawWindow(
    uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, const uint8_t *data, uint32_t pixelLen)
{
    ST7796S_SelectArea(&lcdHandle, startX, startY, endX, endY);
    ST7796S_WritePixels(&lcdHandle, (uint16_t *)data, pixelLen);
}

static void DEMO_InitCamera(void)
{
    /* Init ov7670 module with default setting. */
    camera_config_t camconfig = {
        .pixelFormat                = kVIDEO_PixelFormatRGB565,
        .resolution                 = DEMO_CAMERA_RESOLUTION,
        .framePerSec                = 30,
        .interface                  = kCAMERA_InterfaceGatedClock,
        .frameBufferLinePitch_Bytes = 0, /* Not used. */
        .controlFlags               = 0, /* Not used. */
        .bytesPerPixel              = 0, /* Not used. */
        .mipiChannel                = 0, /* Not used. */
        .csiLanes                   = 0, /* Not used. */
    };

    BOARD_Camera_I2C_Init();
    CAMERA_DEVICE_Init(&handle, &camconfig);
}

static void DEMO_InitSmartDma(void)
{
    static smartdma_camera_param_t smartdmaParam;

    /* Clear camera buffer. */
    memset((void *)g_camera_buffer, 0, sizeof(g_camera_buffer));

    /* Init smartdma for camera. */
    SMARTDMA_InitWithoutFirmware();
    SMARTDMA_InstallFirmware(SMARTDMA_CAMERA_MEM_ADDR, s_smartdmaCameraFirmware, SMARTDMA_CAMERA_FIRMWARE_SIZE);
    SMARTDMA_InstallCallback(SDMA_CompleteCallback, NULL); /* Set camera call back. */
    NVIC_EnableIRQ(SMARTDMA_IRQn);
    NVIC_SetPriority(SMARTDMA_IRQn, 3);

    /* Boot smartdma. */
    smartdmaParam.smartdma_stack = (uint32_t *)g_samrtdma_stack;
    smartdmaParam.p_buffer       = (uint32_t *)g_camera_buffer;
    /* Make sure the frame size that the firmware fetches is smaller than or equal to the camera resolution.
       In this case it is half of the camera resolution. */
    SMARTDMA_Boot(DEMO_SMARTDMA_API, &smartdmaParam, 0x2);
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Set the LDO_CORE VDD regulator to 1.0 V voltage level */
    spc_active_mode_core_ldo_option_t ldoOpt = {
        .CoreLDOVoltage       = kSPC_CoreLDO_MidDriveVoltage,
        .CoreLDODriveStrength = kSPC_CoreLDO_NormalDriveStrength,
    };
    SPC_SetActiveModeCoreLDORegulatorConfig(SPC0, &ldoOpt);

    /* Set the DCDC VDD regulator to 1.0 V voltage level */
    spc_active_mode_dcdc_option_t dcdcOpt = {
        .DCDCVoltage       = kSPC_DCDC_MidVoltage,
        .DCDCDriveStrength = kSPC_DCDC_NormalDriveStrength,
    };
    SPC_SetActiveModeDCDCRegulatorConfig(SPC0, &dcdcOpt);

    /* Enable clock for PCLK. */
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);
    CLOCK_SetClkDiv(kCLOCK_DivClkOut, 25U);

    /* Enable flexio clock */
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, 1u);
    CLOCK_AttachClk(kPLL0_to_FLEXIO);

    /* Enable RST/RS/CS pins' GPIO clock. */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio2);
    CLOCK_EnableClock(kCLOCK_Gpio4);

    /* Init camera I2C clock. */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);
    CLOCK_EnableClock(kCLOCK_LPFlexComm7);
    CLOCK_EnableClock(kCLOCK_LPI2c7);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 1u);

    /* Attach camera signals. P0_4/P0_11/P0_5 is set to EZH_CAMERA_VSYNC/EZH_CAMERA_HSYNC/EZH_CAMERA_PCLK. */
    INPUTMUX_Init(INPUTMUX0);
    INPUTMUX_AttachSignal(INPUTMUX0, 0, kINPUTMUX_GpioPort0Pin4ToSmartDma);
    INPUTMUX_AttachSignal(INPUTMUX0, 1, kINPUTMUX_GpioPort0Pin11ToSmartDma);
    INPUTMUX_AttachSignal(INPUTMUX0, 2, kINPUTMUX_GpioPort0Pin5ToSmartDma);
    /* Turn off clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX0);

    PRINTF("SmartDma camera and flexio mculcd demo start.\r\n");
    PRINTF("Make sure the frame size that the SmartDma fetches is smaller than or equal to the camera resolution.\r\n");

    DEMO_InitCamera();
    DEMO_InitFlexioMcuLcd();
    DEMO_InitPanel();
    DEMO_InitSmartDma();

    /* Send left half of frame of all black pixel. */
    DEMO_DrawWindow(0U, 0U, DEMO_PANEL_HEIGHT - 1U, DEMO_PANEL_WIDTH / 2U - 1U, (const uint8_t *)g_camera_buffer,
                    DEMO_PANEL_WIDTH * DEMO_PANEL_HEIGHT / 2U);

    /* Send right half of frame of all black pixel. */
    DEMO_DrawWindow(0U, DEMO_PANEL_WIDTH / 2U, DEMO_PANEL_HEIGHT - 1U, DEMO_PANEL_WIDTH - 1U,
                    (const uint8_t *)g_camera_buffer, DEMO_PANEL_WIDTH * DEMO_PANEL_HEIGHT / 2U);

    while (1)
    {
        while (!g_camera_complete_flag)
        {
        }
        g_camera_complete_flag = false;
        DEMO_DrawWindow(0U, 0U, DEMO_BUFFER_HEIGHT - 1U, DEMO_BUFFER_WIDTH - 1U, (const uint8_t *)g_camera_buffer,
                        DEMO_BUFFER_WIDTH * DEMO_BUFFER_HEIGHT);
    }
}
