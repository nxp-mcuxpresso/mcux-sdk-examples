/*
 * Copyright (c) 2023,2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "display_support.h"
#include "fsl_gpio.h"
#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)
#include "fsl_dc_fb_dbi.h"
#include "fsl_ssd1963.h"
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
#include "fsl_dbi_flexio_edma.h"
#include "fsl_edma.h"
#else
#include "fsl_dbi_lcdif.h"
#endif
#elif ((DEMO_PANEL_RK055AHD091 == DEMO_PANEL) || (DEMO_PANEL_RK055IQH091 == DEMO_PANEL) || \
       (DEMO_PANEL_RK055MHD091 == DEMO_PANEL) || (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL))
#include "fsl_dc_fb_lcdif.h"
#include "fsl_xspi.h"
#if (DEMO_PANEL_RK055AHD091 == DEMO_PANEL)
#include "fsl_rm68200.h"
#elif (DEMO_PANEL_RK055IQH091 == DEMO_PANEL)
#include "fsl_rm68191.h"
#elif (DEMO_PANEL_RK055MHD091 == DEMO_PANEL)
#include "fsl_hx8394.h"
#elif (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
#include "fsl_rpi.h"
#endif
#include "fsl_mipi_dsi.h"
#else
#if RM67162_USE_LCDIF
#include "fsl_dc_fb_dbi.h"
#include "fsl_dbi_lcdif.h"
#else
#include "fsl_dc_fb_dsi_cmd.h"
#endif
#include "fsl_rm67162.h"
#include "fsl_mipi_dsi.h"
#endif
#include "fsl_power.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * PANEL_TFT_PROTO_5 SSD1963 controller
 */

/* SSD1963 XTAL_IN clock frequency, decided by the LCD board, don't change. */
#define DEMO_SSD1963_XTAL_FREQ 10000000U
/* Configures the SSD1963 output PCLK frequency, for 60Hz frame rate,
 * PCLK = (800 + 48 + 40 + 0) * (480 + 3 + 13 + 18) * 60 */
#define DEMO_SSD1963_PCLK_FREQ     30000000U
#define DEMO_SSD1963_HSW           48U
#define DEMO_SSD1963_HFP           40U
#define DEMO_SSD1963_HBP           0U
#define DEMO_SSD1963_VSW           3U
#define DEMO_SSD1963_VFP           13U
#define DEMO_SSD1963_VBP           18U
#define DEMO_SSD1963_POLARITY_FLAG 0U

#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)

/* Macros for FLEXIO and clock. */
#define DEMO_SSD1963_FLEXIO            FLEXIO
#define DEMO_SSD1963_FLEXIO_CLOCK_FREQ CLOCK_GetFlexioClkFreq()
#define DEMO_SSD1963_FLEXIO_BAUDRATE_BPS                             \
    (DEMO_SSD1963_FLEXIO_CLOCK_FREQ * FLEXIO_MCULCD_DATA_BUS_WIDTH / \
     2) /* FLEXIO_MCULCD_DATA_BUS_WIDTH defined in compiler symbols */

/* Macros for LCD DMA. */
#define DEMO_SSD1963_DMA                   DMA0
#define DEMO_SSD1963_FLEXIO_TX_DMA_CHANNEL 0 /* Match the DEMO_SSD1963_FLEXIO_TX_START_SHIFTER */

/* Macros for FlexIO shifter, timer, and pins. */
#define DEMO_SSD1963_FLEXIO_WR_PIN           4
#define DEMO_SSD1963_FLEXIO_RD_PIN           3
#define DEMO_SSD1963_FLEXIO_DATA_PIN_START   6
#define DEMO_SSD1963_FLEXIO_TX_START_SHIFTER 0
#define DEMO_SSD1963_FLEXIO_RX_START_SHIFTER 0
#define DEMO_SSD1963_FLEXIO_TX_END_SHIFTER   7
#define DEMO_SSD1963_FLEXIO_RX_END_SHIFTER   7
#define DEMO_SSD1963_FLEXIO_TIMER            0

/* Macros for FLEIO EDMA. */
#define DEMO_FLEXIO_TX_DMA_REQUEST kDmaRequestMuxFlexIO0ShiftRegister0Request

#else /* SSD1963_DRIVEN_BY_LCDIF */

/* Macros for LCDIF and interrupt. */
#define DEMO_SSD1963_LCDIF      LCDIF
#define DEMO_SSD1963_LCDIF_IRQn LCDIF_IRQn

#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
static void BOARD_SetCSPin(bool set);
static void BOARD_SetRSPin(bool set);
static void BOARD_InitFlexioClock(void);
static status_t BOARD_InitFlexioLCD(void);
static void BOARD_InitEDMA(void);
#else /* SSD1963_DRIVEN_BY_LCDIF */
static void BOARD_InitLcdifPowerReset(void);
static void BOARD_InitLcdif(void);
#endif
static void BOARD_ResetSSD1963(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dc_fb_dbi_handle_t s_dcDbiHandle;

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsDbi,
    .prvData = &s_dcDbiHandle,
    .config  = NULL,
};

const ssd1963_config_t ssd1963Config = {.pclkFreq_Hz    = DEMO_SSD1963_PCLK_FREQ,
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
                                        .pixelInterface = kSSD1963_BGR888,
#else
                                        .pixelInterface = kSSD1963_RGB888,
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
                                        .vbp            = DEMO_SSD1963_VBP};

#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)

static edma_handle_t s_edmaTxHandle;

static flexio_mculcd_edma_handle_t s_flexioHandle;

/* The FlexIO MCU LCD device. */
static FLEXIO_MCULCD_Type flexioLcdDev = {
    .flexioBase          = DEMO_SSD1963_FLEXIO,
    .busType             = kFLEXIO_MCULCD_8080,
    .dataPinStartIndex   = DEMO_SSD1963_FLEXIO_DATA_PIN_START,
    .ENWRPinIndex        = DEMO_SSD1963_FLEXIO_WR_PIN,
    .RDPinIndex          = DEMO_SSD1963_FLEXIO_RD_PIN,
    .txShifterStartIndex = DEMO_SSD1963_FLEXIO_TX_START_SHIFTER,
    .txShifterEndIndex   = DEMO_SSD1963_FLEXIO_TX_END_SHIFTER,
    .rxShifterStartIndex = DEMO_SSD1963_FLEXIO_RX_START_SHIFTER,
    .rxShifterEndIndex   = DEMO_SSD1963_FLEXIO_RX_END_SHIFTER,
    .timerIndex          = DEMO_SSD1963_FLEXIO_TIMER,
    .setCSPin            = BOARD_SetCSPin,
    .setRSPin            = BOARD_SetRSPin,
    .setRDWRPin          = NULL /* Not used in 8080 mode. */
};
#else                           /* SSD1963_DRIVEN_BY_LCDIF */

static dbi_lcdif_prv_data_t s_lcdifPrvData;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
static void BOARD_InitEDMA(void)
{
    edma_config_t edmaConfig;

    POWER_DisablePD(kPDRUNCFG_APD_DMA0_1_PKC_ETF);
    POWER_DisablePD(kPDRUNCFG_PPD_DMA0_1_PKC_ETF);
    POWER_ApplyPD();

    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Dma0);

    /* Init and create EDMA handles. */
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(DEMO_SSD1963_DMA, &edmaConfig);
    EDMA_CreateHandle(&s_edmaTxHandle, DEMO_SSD1963_DMA, DEMO_SSD1963_FLEXIO_TX_DMA_CHANNEL);
    EDMA_SetChannelMux(DEMO_SSD1963_DMA, DEMO_SSD1963_FLEXIO_TX_DMA_CHANNEL, DEMO_FLEXIO_TX_DMA_REQUEST);
    EDMA_EnableRequest(DEMO_SSD1963_DMA, DEMO_FLEXIO_TX_DMA_REQUEST);

    NVIC_SetPriority(EDMA0_CH0_IRQn, 3);
}

static void BOARD_InitFlexioClock(void)
{
    CLOCK_AttachClk(kFRO1_DIV1_to_FLEXIO);
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, 4U);
    CLOCK_EnableClock(kCLOCK_Flexio);

    RESET_ClearPeripheralReset(kFLEXIO0_RST_SHIFT_RSTn);
}

static status_t BOARD_InitFlexioLCD(void)
{
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
    flexioMcuLcdConfig.enableFastAccess = false;
    flexioMcuLcdConfig.baudRate_Bps     = DEMO_SSD1963_FLEXIO_BAUDRATE_BPS;

    return FLEXIO_MCULCD_Init(&flexioLcdDev, &flexioMcuLcdConfig, DEMO_SSD1963_FLEXIO_CLOCK_FREQ);
}

status_t BOARD_PrepareDisplayController(void)
{
    status_t status;

    /* 1. Initialize FLEXIO and DMA. */
    BOARD_InitFlexioClock();

    status = BOARD_InitFlexioLCD();
    if (kStatus_Success != status)
    {
        return status;
    }

    BOARD_InitEDMA();

    /* 2. Create the DBI XFER interface. */
    status =
        DBI_FLEXIO_EDMA_CreateHandle(&(s_dcDbiHandle.dbiIface), &flexioLcdDev, &s_flexioHandle, &s_edmaTxHandle, NULL);
    if (kStatus_Success != status)
    {
        return status;
    }

    /* 3. Initialize panel. */
    BOARD_ResetSSD1963();

    SSD1963_Init(&(s_dcDbiHandle.dbiIface), &ssd1963Config, DEMO_SSD1963_XTAL_FREQ);
    SSD1963_SetBackLight(&(s_dcDbiHandle.dbiIface), 255);
    
    return kStatus_Success;
}

static void BOARD_SetCSPin(bool set)
{
    GPIO_PinWrite(BOARD_SSD1963_CS_GPIO, BOARD_SSD1963_CS_PIN, (uint8_t)set);
}

static void BOARD_SetRSPin(bool set)
{
    GPIO_PinWrite(BOARD_SSD1963_RS_GPIO, BOARD_SSD1963_RS_PIN, (uint8_t)set);
}

#else /* SSD1963_DRIVEN_BY_LCDIF */

static void BOARD_InitLcdifPowerReset(void)
{
    POWER_DisablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_APD_LCDIF);
    POWER_DisablePD(kPDRUNCFG_PPD_LCDIF);
    POWER_ApplyPD();

    /* LCDIF DBI mode uses MEDIA MAIN clock as source, no need to enable LCDIF(pixel) clock. */
    RESET_ClearPeripheralReset(kLCDIF_RST_SHIFT_RSTn);
}

static void BOARD_InitLcdif(void)
{
    lcdif_panel_config_t config;
    lcdif_dbi_config_t dbiConfig;

    /* DBI configurations. */
    LCDIF_DbiModeGetDefaultConfig(&dbiConfig);
    dbiConfig.acTimeUnit      = 0;
#if (defined(FSL_FEATURE_LCDIF_HAS_DBIX_POLARITY) && FSL_FEATURE_LCDIF_HAS_DBIX_POLARITY)
    dbiConfig.reversePolarity = true;
#endif
    dbiConfig.writeWRPeriod   = 6;
    dbiConfig.format          = kLCDIF_DbiOutD8RGB888;
    dbiConfig.type            = kLCDIF_DbiTypeB;
    dbiConfig.writeCSAssert   = 1;
    dbiConfig.writeCSDeassert = 4;
    dbiConfig.writeWRAssert   = (dbiConfig.writeWRPeriod - 1U) / 2U; /* Asset at the middle. */
    dbiConfig.writeWRDeassert = (dbiConfig.writeWRPeriod - 1U);      /* Deassert at the end */

    LCDIF_Init(DEMO_SSD1963_LCDIF);
    LCDIF_DbiModeSetConfig(DEMO_SSD1963_LCDIF, 0, &dbiConfig);

    LCDIF_PanelGetDefaultConfig(&config);
    LCDIF_SetPanelConfig(DEMO_SSD1963_LCDIF, 0, &config);

    NVIC_ClearPendingIRQ(DEMO_SSD1963_LCDIF_IRQn);
    NVIC_SetPriority(DEMO_SSD1963_LCDIF_IRQn, 3);
    NVIC_EnableIRQ(DEMO_SSD1963_LCDIF_IRQn);
}

status_t BOARD_PrepareDisplayController(void)
{
    /* 1. Initialize LCDIF. */
    BOARD_InitLcdifPowerReset();

    BOARD_InitLcdif();

    /* 2. Create the LCDIF DBI XFER interface. */
    DBI_LCDIF_InitController(&(s_dcDbiHandle.dbiIface), &s_lcdifPrvData, DEMO_SSD1963_LCDIF, NULL);

    /* 3. Initialize panel. */
    BOARD_ResetSSD1963();

    SSD1963_Init(&(s_dcDbiHandle.dbiIface), &ssd1963Config, DEMO_SSD1963_XTAL_FREQ);

    SSD1963_SetBackLight(&(s_dcDbiHandle.dbiIface), 255);

    return kStatus_Success;
}

void LCDIF_IRQHandler(void)
{
    DBI_LCDIF_IRQHandler(&(s_dcDbiHandle.dbiIface));
}

#endif

static void BOARD_ResetSSD1963(void)
{
    const gpio_pin_config_t resetPinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1,
    };

    /* Set SSD1963 CS, RS, and reset pin to output. */
    GPIO_PinInit(BOARD_SSD1963_RST_GPIO, BOARD_SSD1963_RST_PIN, &resetPinConfig);
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
    /* FLEXIO needs external CS and RS pins. */
    GPIO_PinInit(BOARD_SSD1963_CS_GPIO, BOARD_SSD1963_CS_PIN, &resetPinConfig);
    GPIO_PinInit(BOARD_SSD1963_RS_GPIO, BOARD_SSD1963_RS_PIN, &resetPinConfig);
#endif

    /* Reset the SSD1963 LCD controller. */
    GPIO_PinWrite(BOARD_SSD1963_RST_GPIO, BOARD_SSD1963_RST_PIN, 0);
    VIDEO_DelayMs(1); /* Delay 1ms (10ns required). */
    GPIO_PinWrite(BOARD_SSD1963_RST_GPIO, BOARD_SSD1963_RST_PIN, 1);
    VIDEO_DelayMs(5); /* Delay 5ms. */
}

#elif ((DEMO_PANEL_RK055AHD091 == DEMO_PANEL) || (DEMO_PANEL_RK055IQH091 == DEMO_PANEL) || \
       (DEMO_PANEL_RK055MHD091 == DEMO_PANEL) || (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL))

#if BOARD_ENABLE_PSRAM_CACHE
/*
 * When PSRAM cache enabled and frame buffer placed in PSRAM, LCDIF reads the
 * cache first, then reads PSRAM if cache missed. Generally the frame buffer
 * is large, cache miss happens frequently, so the LCDIF read performance is low.
 *
 * There are three solutions:
 * 1. Disables the PSRAM cache, LCDIF reads from PSRAM directly.
 * 2. Slow down the frame rate, or use smaller pixel format, or use small resolution.
 * 3. Don't place frame buffer in PSRAM.
 *
 * In the example, solution 1 is used.
 */
#error Please read the comment about PSRAM cache and display refresh rate here
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * RK055AHD091 panel
 */

#if (DEMO_PANEL_RK055AHD091 == DEMO_PANEL)
#define DEMO_LCDIF_HSW 8
#define DEMO_LCDIF_HFP 32
#define DEMO_LCDIF_HBP 32
#define DEMO_LCDIF_VSW 2
#define DEMO_LCDIF_VFP 16
#define DEMO_LCDIF_VBP 14

#elif (DEMO_PANEL_RK055IQH091 == DEMO_PANEL)

#define DEMO_LCDIF_HSW 2
#define DEMO_LCDIF_HFP 32
#define DEMO_LCDIF_HBP 30
#define DEMO_LCDIF_VSW 2
#define DEMO_LCDIF_VFP 16
#define DEMO_LCDIF_VBP 14

#elif (DEMO_PANEL_RK055MHD091 == DEMO_PANEL)

#define DEMO_LCDIF_HSW 6
#define DEMO_LCDIF_HFP 12
#define DEMO_LCDIF_HBP 24
#define DEMO_LCDIF_VSW 2
#define DEMO_LCDIF_VFP 16
#define DEMO_LCDIF_VBP 14

#elif (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)

#define DEMO_LCDIF_HSW 20
#define DEMO_LCDIF_HFP 70
#define DEMO_LCDIF_HBP 23
#define DEMO_LCDIF_VSW 2
#define DEMO_LCDIF_VFP 7
#define DEMO_LCDIF_VBP 21

#endif

#define DEMO_LCDIF_POL_FLAGS \
    (kLCDIF_DataEnableActiveHigh | kLCDIF_VsyncActiveLow | kLCDIF_HsyncActiveLow | kLCDIF_DriveDataOnRisingClkEdge)

#define DEMO_LCDIF    LCDIF

/* Definitions for MIPI. */
#define DEMO_MIPI_DSI MIPI_DSI_HOST
#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
#define DEMO_MIPI_DSI_LANE_NUM 1
#else
#define DEMO_MIPI_DSI_LANE_NUM 2
#endif
#define DEMO_MIPI_DSI_BIT_PER_PIXEL            24

/* Here the desired DPHY bit clock multiplied by ( 9 / 8 = 1.125) to ensure
 * it is fast enough.
 */
#define DEMO_MIPI_DPHY_BIT_CLK_ENLARGE(origin) (((origin) / 8) * 9)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
static status_t BOARD_ReadPanelStatus(uint8_t regAddr, uint8_t *status);
static status_t BOARD_WritePanelRegister(uint8_t regAddr, uint8_t value);
#else
static void BOARD_PullPanelResetPin(bool pullUp);
static void BOARD_PullPanelPowerPin(bool pullUp);
#endif
static void BOARD_InitLcdifClock(void);
static void BOARD_InitMipiDsiClock(void);
static status_t BOARD_DSI_Transfer(dsi_transfer_t *xfer);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t mipiDsiTxEscClkFreq_Hz;
static uint32_t mipiDsiDphyBitClkFreq_Hz;
static uint32_t mipiDsiDpiClkFreq_Hz;

#if (DEMO_PANEL == DEMO_PANEL_RK055AHD091)

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = BOARD_DSI_Transfer,
};

static const rm68200_resource_t rm68200Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = BOARD_PullPanelResetPin,
    .pullPowerPin = BOARD_PullPanelPowerPin,
};

static display_handle_t rm68200Handle = {
    .resource = &rm68200Resource,
    .ops      = &rm68200_ops,
};

#elif (DEMO_PANEL == DEMO_PANEL_RK055MHD091)

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = BOARD_DSI_Transfer,
};

static const hx8394_resource_t hx8394Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = BOARD_PullPanelResetPin,
    .pullPowerPin = BOARD_PullPanelPowerPin,
};

static display_handle_t hx8394Handle = {
    .resource = &hx8394Resource,
    .ops      = &hx8394_ops,
};

#elif (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = BOARD_DSI_Transfer,
};

static const rpi_resource_t rpiResource = {
    .dsiDevice     = &dsiDevice,
    .readStatus    = &BOARD_ReadPanelStatus,
    .writeRegister = &BOARD_WritePanelRegister,
};

static display_handle_t rpiHandle = {
    .resource = &rpiResource,
    .ops      = &rpi_ops,
};

#else

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = BOARD_DSI_Transfer,
};

static const rm68191_resource_t rm68191Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = BOARD_PullPanelResetPin,
    .pullPowerPin = BOARD_PullPanelPowerPin,
};

static display_handle_t rm68191Handle = {
    .resource = &rm68191Resource,
    .ops      = &rm68191_ops,
};

#endif

static dc_fb_lcdif_handle_t s_dcFbLcdifHandle;

static const dc_fb_lcdif_config_t s_dcFbLcdifConfig = {
    .lcdif         = DEMO_LCDIF,
    .width         = DEMO_PANEL_WIDTH,
    .height        = DEMO_PANEL_HEIGHT,
    .hsw           = DEMO_LCDIF_HSW,
    .hfp           = DEMO_LCDIF_HFP,
    .hbp           = DEMO_LCDIF_HBP,
    .vsw           = DEMO_LCDIF_VSW,
    .vfp           = DEMO_LCDIF_VFP,
    .vbp           = DEMO_LCDIF_VBP,
    .polarityFlags = DEMO_LCDIF_POL_FLAGS,
    .outputFormat  = kLCDIF_Output24Bit,
};

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsLcdif,
    .prvData = &s_dcFbLcdifHandle,
    .config  = &s_dcFbLcdifConfig,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
static status_t BOARD_ReadPanelStatus(uint8_t regAddr, uint8_t *value)
{
    return BOARD_I2C_Receive(BOARD_MIPI_PANEL_TOUCH_I2C_BASEADDR, RPI_ADDR, regAddr, 1U, value, 1U);
}

static status_t BOARD_WritePanelRegister(uint8_t regAddr, uint8_t value)
{
    return BOARD_I2C_Send(BOARD_MIPI_PANEL_TOUCH_I2C_BASEADDR, RPI_ADDR, regAddr, 1, &value, 1);
}
#else
static void BOARD_PullPanelResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, 0);
    }
}

static void BOARD_PullPanelPowerPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, 0);
    }
}
#endif

static status_t BOARD_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
}

static void BOARD_InitLcdifClock(void)
{
    POWER_DisablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_APD_LCDIF);
    POWER_DisablePD(kPDRUNCFG_PPD_LCDIF);
    POWER_ApplyPD();

    /*
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     * Here use the main pll (528MHz) as clock source.
     * Since MIPI DPHY clock use AUDIO pll pfd2 as aource, and its max allowed clock frequency is
     * 532.48 x 18 / 16 = 599.04MHz. To avoid exceed this limit, for RK055AHD091 and RK055MHD091,
     * the frame rate shall be 28.23fps and 28.93fps, which is 29.33MHz pixel clock. For RK055IQH091
     * it's resolution is safe to use 60fps frame rate, which is 35.2mHz pixel clock.
     * For RaspberryPi panel, the frame rate shall be 42fps, which is 19.56MHz pixel clock.
     */
    CLOCK_AttachClk(kMAIN_PLL_PFD2_to_LCDIF);
#if ((DEMO_PANEL == DEMO_PANEL_RK055AHD091) || (DEMO_PANEL == DEMO_PANEL_RK055MHD091))
    CLOCK_SetClkDiv(kCLOCK_DivLcdifClk, 18);
#elif (DEMO_PANEL == DEMO_PANEL_RASPI_7INCH)
    CLOCK_SetClkDiv(kCLOCK_DivLcdifClk, 27);
#else
    CLOCK_SetClkDiv(kCLOCK_DivLcdifClk, 15);
#endif

    /* Get lcdif pixel clock frequency. */
    mipiDsiDpiClkFreq_Hz = CLOCK_GetLcdifClkFreq();

    CLOCK_EnableClock(kCLOCK_Lcdif);
    RESET_ClearPeripheralReset(kLCDIF_RST_SHIFT_RSTn);
}

static void BOARD_InitMipiDsiClock(void)
{
    uint8_t div;

    POWER_DisablePD(kPDRUNCFG_PPD_MIPIDSI);
    POWER_DisablePD(kPDRUNCFG_APD_MIPIDSI);
    POWER_DisablePD(kPDRUNCFG_PD_VDD2_MIPI);
    POWER_ApplyPD();

    /* Use PLL PFD1 as clock source, 396m. */
    CLOCK_AttachClk(kMAIN_PLL_PFD1_to_MIPI_DPHYESC_CLK);
    /* RxClkEsc min 60MHz, TxClkEsc 12 to 20MHz. */
    /* RxClkEsc = 396MHz / 6 = 66MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscRxClk, 6);
    /* TxClkEsc = 396MHz / 6 / 4 = 16.5MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscTxClk, 4);

    mipiDsiTxEscClkFreq_Hz = CLOCK_GetMipiDphyEscTxClkFreq();

    /* The DPHY bit clock must be fast enough to send out the pixels, it should be
     * larger than:
     *
     *         (Pixel clock * bit per output pixel) / number of MIPI data lane
     *
     * DPHY uses AUDIO pll pfd2 as aource, and its max allowed clock frequency is
     * 532.48 x 18 / 16 = 599.04MHz. The MIPI panel supports up to 850MHz bit clock.
     */
    mipiDsiDphyBitClkFreq_Hz = mipiDsiDpiClkFreq_Hz * DEMO_MIPI_DSI_BIT_PER_PIXEL / DEMO_MIPI_DSI_LANE_NUM;
    mipiDsiDphyBitClkFreq_Hz = DEMO_MIPI_DPHY_BIT_CLK_ENLARGE(mipiDsiDphyBitClkFreq_Hz);

    div = (uint8_t)((uint64_t)CLOCK_GetAudioPllFreq() * 18U / (uint64_t)mipiDsiDphyBitClkFreq_Hz);

    CLOCK_InitAudioPfd(kCLOCK_Pfd2, div);

    CLOCK_AttachClk(kAUDIO_PLL_PFD2_to_MIPI_DSI_HOST_PHY);

    CLOCK_SetClkDiv(kCLOCK_DivDphyClk, 1);

    mipiDsiDphyBitClkFreq_Hz = CLOCK_GetMipiDphyClkFreq();
}

static status_t BOARD_InitLcdPanel(void)
{
    status_t status;

    const display_config_t displayConfig = {
        .resolution   = FSL_VIDEO_RESOLUTION(DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT),
        .hsw          = DEMO_LCDIF_HSW,
        .hfp          = DEMO_LCDIF_HFP,
        .hbp          = DEMO_LCDIF_HBP,
        .vsw          = DEMO_LCDIF_VSW,
        .vfp          = DEMO_LCDIF_VFP,
        .vbp          = DEMO_LCDIF_VBP,
        .controlFlags = 0,
        .dsiLanes     = DEMO_MIPI_DSI_LANE_NUM,
    };

#if (DEMO_PANEL == DEMO_PANEL_RASPI_7INCH)
    BOARD_MIPIPanelTouch_I2C_Init();
#else
    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

    GPIO_PinInit(GPIO0, 10, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, &pinConfig);
#endif

#if (DEMO_PANEL == DEMO_PANEL_RK055AHD091)
    status = RM68200_Init(&rm68200Handle, &displayConfig);
#elif (DEMO_PANEL == DEMO_PANEL_RK055MHD091)
    status = HX8394_Init(&hx8394Handle, &displayConfig);
#elif (DEMO_PANEL == DEMO_PANEL_RK055IQH091)
    status = RM68191_Init(&rm68191Handle, &displayConfig);
#elif (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
    status = RPI_Init(&rpiHandle, &displayConfig);
#endif

#if (DEMO_PANEL != DEMO_PANEL_RASPI_7INCH)
    if (status == kStatus_Success)
    {
        GPIO_PinWrite(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, 1);
        GPIO_PinWrite(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, 1);
    }
#endif

    return status;
}

static void BOARD_InitMipiDsiConfig(void)
{
    dsi_config_t dsiConfig;
    dsi_dphy_config_t dphyConfig;

    const dsi_dpi_config_t dpiConfig = {
        .pixelPayloadSize = DEMO_PANEL_WIDTH,
        .dpiColorCoding   = kDSI_Dpi24Bit,
        .pixelPacket      = kDSI_PixelPacket24Bit,
#if (DEMO_PANEL == DEMO_PANEL_RASPI_7INCH)
        .videoMode        = kDSI_DpiNonBurstWithSyncPulse,
#else
        .videoMode = kDSI_DpiBurst,
#endif
        .bllpMode         = kDSI_DpiBllpLowPower,
        .polarityFlags    = kDSI_DpiVsyncActiveLow | kDSI_DpiHsyncActiveLow,
        .hfp              = DEMO_LCDIF_HFP,
        .hbp              = DEMO_LCDIF_HBP,
        .hsw              = DEMO_LCDIF_HSW,
        .vfp              = DEMO_LCDIF_VFP,
        .vbp              = DEMO_LCDIF_VBP,
        .panelHeight      = DEMO_PANEL_HEIGHT,
        .virtualChannel   = 0
    };

    /*
     * dsiConfig.numLanes = 4;
     * dsiConfig.enableNonContinuousHsClk = false;
     * dsiConfig.autoInsertEoTp = true;
     * dsiConfig.numExtraEoTp = 0;
     * dsiConfig.htxTo_ByteClk = 0;
     * dsiConfig.lrxHostTo_ByteClk = 0;
     * dsiConfig.btaTo_ByteClk = 0;
     */
    DSI_GetDefaultConfig(&dsiConfig);
    dsiConfig.numLanes                 = DEMO_MIPI_DSI_LANE_NUM;
#if (DEMO_PANEL == DEMO_PANEL_RASPI_7INCH)
    dsiConfig.autoInsertEoTp           = false;
    dsiConfig.enableNonContinuousHsClk = false;
#else
    dsiConfig.autoInsertEoTp = true;
#endif

    DSI_GetDphyDefaultConfig(&dphyConfig, mipiDsiDphyBitClkFreq_Hz, mipiDsiTxEscClkFreq_Hz);

    /* Init the DSI module. */
    DSI_Init(DEMO_MIPI_DSI, &dsiConfig);

    /* Init DPHY. There is not DPHY PLL, the ref clock is not used. */
    DSI_InitDphy(DEMO_MIPI_DSI, &dphyConfig, 0);

    /* Init DPI interface. */
    DSI_SetDpiConfig(DEMO_MIPI_DSI, &dpiConfig, DEMO_MIPI_DSI_LANE_NUM, mipiDsiDpiClkFreq_Hz, mipiDsiDphyBitClkFreq_Hz);
}

status_t BOARD_InitDisplayInterface(void)
{
    /* 1. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 2. Configures peripheral. */
    BOARD_InitMipiDsiConfig();

    /* 3. Configure the panel. */
    return BOARD_InitLcdPanel();
}

void LCDIF_IRQHandler(void)
{
    DC_FB_LCDIF_IRQHandler(&g_dc);
}

/*
 * With the default configuration, when frame buffer is placed in PSRAM,
 * LCDIF might underflow if some other masters are writing to PSRAM.
 * This function improve the LCDIF bandwidth to make sure it doesn't underflow.
 */
static void BOARD_ImproveLcdifBandwidth(void)
{
/* Allocate 2K AHB buffer for LCDIF. */
#define XSPI_LCDIF_AHB_BUF_SIZE (2 * 1024)

    xspi_ahbBuffer_config_t lcdifBufferConfig = {
        .bufferSize            = XSPI_LCDIF_AHB_BUF_SIZE / 8,
        .enaPri.enablePriority = true,
        .masterId              = 0x1dU, /* LCDIF */
        .ptrSubBuffer0Config   = NULL,
        .ptrSubBuffer1Config   = NULL,
        .ptrSubBuffer2Config   = NULL,
        .ptrSubBuffer3Config   = NULL,
    };
    xspi_ahbBuffer_config_t buffer1Config = {
        .bufferSize            = 0,
        .enaPri.enablePriority = false,
        .masterId              = 0xFFU, /* Not used. */
        .ptrSubBuffer0Config   = NULL,
        .ptrSubBuffer1Config   = NULL,
        .ptrSubBuffer2Config   = NULL,
        .ptrSubBuffer3Config   = NULL,
    };
    xspi_ahbBuffer_config_t buffer2Config = {
        .bufferSize            = 0,
        .enaPri.enablePriority = false,
        .masterId              = 0xFFU, /* Not used. */
        .ptrSubBuffer0Config   = NULL,
        .ptrSubBuffer1Config   = NULL,
        .ptrSubBuffer2Config   = NULL,
        .ptrSubBuffer3Config   = NULL,
    };
    xspi_ahbBuffer_config_t buffer3Config = {
        .bufferSize = ((4 * 1024) - XSPI_LCDIF_AHB_BUF_SIZE) / 8, /* Totally 4K, Left part shared by other masters */
        .enaPri.enableAllMaster = true,
        .masterId               = 0x0U,
        .ptrSubBuffer0Config    = NULL,
        .ptrSubBuffer1Config    = NULL,
        .ptrSubBuffer2Config    = NULL,
        .ptrSubBuffer3Config    = NULL,
    };

    if (XSPI_SetAhbBufferConfig(XSPI2, &lcdifBufferConfig, &buffer1Config, &buffer2Config, &buffer3Config) !=
        kStatus_Success)
    {
        PRINTF("XSPI AHB buffer reconfiguration failed.\r\n");
        while (1)
        {
        }
    }

    /* Let LCDIF read has higher priority from MEDIA AXI_SWITCH */
    NIC_MEDIA1->ASIB[7].READ_QOS = NIC_READ_QOS_READ_QOS(1);
}

status_t BOARD_PrepareDisplayController(void)
{
    status_t status;

    BOARD_ImproveLcdifBandwidth();

    BOARD_InitLcdifClock();

    status = BOARD_InitDisplayInterface();

    if (kStatus_Success == status)
    {
        NVIC_SetPriority(LCDIF_IRQn, 3);
        EnableIRQ(LCDIF_IRQn);
    }

    return kStatus_Success;
}

#elif (DEMO_PANEL_RM67162 == DEMO_PANEL)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * RM67162 panel
 */

/* Definitions for MIPI. */
#define DEMO_MIPI_DSI          MIPI_DSI_HOST
#define DEMO_MIPI_DSI_LANE_NUM 1

#if RM67162_USE_LCDIF

/* Macros for LCDIF and interrupt. */
#define DEMO_SSD1963_LCDIF      LCDIF
#define DEMO_SSD1963_LCDIF_IRQn LCDIF_IRQn

#else /* RM67162_USE_LCDIF = 0 */

#define DEMO_MIPI_DSI_IRQn MIPI_IRQn

/*
 * The max TX array size:
 *
 * 1. One byte in FIFO is reserved for DSC command
 * 2. One pixel should not be split to two transfer.
 */
#define DEMO_DSI_TX_ARRAY_MAX \
    (((FSL_DSI_TX_MAX_PAYLOAD_BYTE - 1U) / DEMO_BUFFER_BYTE_PER_PIXEL) * DEMO_BUFFER_BYTE_PER_PIXEL)

typedef struct _dsi_mem_write_ctx
{
    volatile bool onGoing;
    const uint8_t *txData;
    uint32_t leftByteLen;
    uint8_t dscCmd;
} dsi_mem_write_ctx_t;

#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void BOARD_PullPanelResetPin(bool pullUp);
static void BOARD_PullPanelPowerPin(bool pullUp);
static void BOARD_InitMipiDsiClock(void);
static void BOARD_InitMipiDsi(void);
static status_t BOARD_DSI_Transfer(dsi_transfer_t *xfer);
static void BOARD_InitMipiPanelTEPin(void);

#if RM67162_USE_LCDIF
static void BOARD_InitLcdifPowerReset(void);
static void BOARD_InitLcdif(void);
#else /* RM67162_USE_LCDIF = 0 */
static status_t BOARD_DSI_MemWrite(uint8_t virtualChannel, const uint8_t *data, uint32_t length);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t mipiDsiTxEscClkFreq_Hz;
static uint32_t mipiDsiDphyBitClkFreq_Hz;

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = BOARD_DSI_Transfer,
#if !RM67162_USE_LCDIF
    .memWriteFunc   = BOARD_DSI_MemWrite,
#endif
};

static const rm67162_resource_t rm67162Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = BOARD_PullPanelResetPin,
    .pullPowerPin = BOARD_PullPanelPowerPin,
};

static display_handle_t rm67162Handle = {
    .resource = &rm67162Resource,
    .ops      = &rm67162_ops,
};

#if RM67162_USE_LCDIF

static display_config_t displayConfig = {
    .resolution   = FSL_VIDEO_RESOLUTION(DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT),
    .hsw          = 0,
    .hfp          = 0,
    .hbp          = 0,
    .vsw          = 0,
    .vfp          = 0,
    .vbp          = 0,
    .controlFlags = 0,
    .dsiLanes     = DEMO_MIPI_DSI_LANE_NUM,
    .pixelFormat  = kVIDEO_PixelFormatRGB565,
};

static dbi_lcdif_prv_data_t s_lcdifPrvData;

static dc_fb_dbi_handle_t s_dcDbiHandle;

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsDbi,
    .prvData = &s_dcDbiHandle,
    .config  = NULL,
};

#else /* RM67162_USE_LCDIF = 0 */

static dsi_mem_write_ctx_t s_dsiMemWriteCtx;
static dsi_transfer_t s_dsiMemWriteXfer = {0};
static dsi_handle_t s_dsiDriverHandle;
static uint8_t s_dsiMemWriteTmpArray[DEMO_DSI_TX_ARRAY_MAX];

const dc_fb_dsi_cmd_config_t s_panelConfig = {
    .commonConfig =
        {
            .resolution   = FSL_VIDEO_RESOLUTION(DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT),
            .hsw          = 0,
            .hfp          = 0,
            .hbp          = 0,
            .vsw          = 0,
            .vfp          = 0,
            .vbp          = 0,
            .controlFlags = 0,
            .dsiLanes     = DEMO_MIPI_DSI_LANE_NUM,
            .pixelFormat  = DEMO_BUFFER_PIXEL_FORMAT,
        },
    .useTEPin = true,
};

static dc_fb_dsi_cmd_handle_t s_dcFbDsiCmdHandle = {
    .dsiDevice   = &dsiDevice,
    .panelHandle = &rm67162Handle,
};

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsDsiCmd,
    .prvData = &s_dcFbDsiCmdHandle,
    .config  = &s_panelConfig,
};

#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

static void BOARD_PullPanelResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, 0);
    }
}

static void BOARD_PullPanelPowerPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, 0);
    }
}

static status_t BOARD_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
}

static void BOARD_InitMipiDsiClock(void)
{
    POWER_DisablePD(kPDRUNCFG_PPD_MIPIDSI);
    POWER_DisablePD(kPDRUNCFG_APD_MIPIDSI);
    POWER_DisablePD(kPDRUNCFG_PD_VDD2_MIPI);
    POWER_ApplyPD();

    /* Use PLL PFD1 as clock source, 396m. */
    CLOCK_AttachClk(kMAIN_PLL_PFD1_to_MIPI_DPHYESC_CLK);
    /* RxClkEsc min 60MHz, TxClkEsc 12 to 20MHz. */
    /* RxClkEsc = 396MHz / 6 = 66MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscRxClk, 6);
    /* TxClkEsc = 396MHz / 6 / 4 = 16.5MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscTxClk, 4);

    mipiDsiTxEscClkFreq_Hz = CLOCK_GetMipiDphyEscTxClkFreq();

    /* When using LCDIF, with 279.53MHz DBI source clock and 16bpp format, a 14 cycle period requires a 279.53MHz / 14 *
       16 = 319.46Mhz DPHY clk source. Considering the DCS packaging cost, the MIPI DPHY speed shall be ***SLIGHTLY***
       larger than the DBI interface speed. DPHY uses AUDIO_PLL_PFD2 which is 532.48MHz as source, the frequency is
       532.48 * 18 / 30 = 319.49MHz, which meets the requirement. */
    CLOCK_InitAudioPfd(kCLOCK_Pfd2, 30);

    CLOCK_AttachClk(kAUDIO_PLL_PFD2_to_MIPI_DSI_HOST_PHY);

    CLOCK_SetClkDiv(kCLOCK_DivDphyClk, 1);
    mipiDsiDphyBitClkFreq_Hz = CLOCK_GetMipiDphyClkFreq();
}

static void BOARD_InitMipiDsi(void)
{
    dsi_config_t dsiConfig;
    dsi_dphy_config_t dphyConfig;

    /*
     * dsiConfig.numLanes = 4;
     * dsiConfig.enableNonContinuousHsClk = false;
     * dsiConfig.autoInsertEoTp = true;
     * dsiConfig.numExtraEoTp = 0;
     * dsiConfig.htxTo_ByteClk = 0;
     * dsiConfig.lrxHostTo_ByteClk = 0;
     * dsiConfig.btaTo_ByteClk = 0;
     */
    DSI_GetDefaultConfig(&dsiConfig);
    dsiConfig.numLanes       = DEMO_MIPI_DSI_LANE_NUM;
    dsiConfig.autoInsertEoTp = true;

    DSI_GetDphyDefaultConfig(&dphyConfig, mipiDsiDphyBitClkFreq_Hz, mipiDsiTxEscClkFreq_Hz);

    /* Init the DSI module. */
    DSI_Init(DEMO_MIPI_DSI, &dsiConfig);

    /* Init DPHY. There is not DPHY PLL, the ref clock is not used. */
    DSI_InitDphy(DEMO_MIPI_DSI, &dphyConfig, 0);

#if RM67162_USE_LCDIF
    DSI_SetDbiPixelFormat(DEMO_MIPI_DSI, kDSI_DbiRGB565);
#endif
}

static status_t BOARD_InitLcdPanel(void)
{
    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

    GPIO_PinInit(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, &pinConfig);

    BOARD_InitMipiPanelTEPin();

    return kStatus_Success;
}

static void BOARD_InitMipiPanelTEPin(void)
{
    const gpio_pin_config_t tePinConfig = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic  = 0,
    };

    /*
     * TE pin configure method:
     *
     * The TE pin interrupt is like this:
     *
     *            VSYNC
     *         +--------+
     *         |        |
     *         |        |
     * --------+        +----------------
     *
     * 1. If one frame send time is shorter than one frame refresh time, then set
     *    TE pin interrupt at the start of VSYNC.
     * 2. If one frame send time is longer than one frame refresh time, and shorter
     *    than two frames refresh time, then set TE pin interrupt at the end of VSYNC.
     * 3. If one frame send time is longer than two frame refresh time, tearing effect
     *    could not be removed.
     *
     * For RM67162 @60Hz frame rate, frame refresh time is 16.7 ms. After test,
     * one frame send time is shorter than one frame refresh time. So TE interrupt is
     * set to start of VSYNC.
     */

    GPIO_PinInit(BOARD_MIPI_TE_GPIO, BOARD_MIPI_TE_PIN, &tePinConfig);

    GPIO_SetPinInterruptConfig(BOARD_MIPI_TE_GPIO, BOARD_MIPI_TE_PIN, kGPIO_InterruptRisingEdge);
    GPIO_SetPinInterruptChannel(BOARD_MIPI_TE_GPIO, BOARD_MIPI_TE_PIN, kGPIO_InterruptOutput0);

    NVIC_SetPriority(BOARD_MIPI_TE_GPIO_IRQn, 3);
    EnableIRQ(BOARD_MIPI_TE_GPIO_IRQn);
}

#if RM67162_USE_LCDIF

static void BOARD_InitLcdifPowerReset(void)
{
    POWER_DisablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_APD_LCDIF);
    POWER_DisablePD(kPDRUNCFG_PPD_LCDIF);
    POWER_ApplyPD();

    CLOCK_EnableClock(kCLOCK_Lcdif);
    RESET_ClearPeripheralReset(kLCDIF_RST_SHIFT_RSTn);
}

static void BOARD_InitLcdif(void)
{
    lcdif_dbi_config_t dbiConfig;
    lcdif_panel_config_t config;

    /* DBI uses MEDIA MAIN clock. Change the PFD2 divide to 17. Then the DBI source frequency shall be 528 * 18 / 17 / 2
     * = 279.53MHz. */
    CLOCK_InitMainPfd(kCLOCK_Pfd2, 17);
    CLOCK_SetClkDiv(kCLOCK_DivMediaMainClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD2_to_MEDIA_MAIN);

    /* DBI configurations. */
    LCDIF_DbiModeGetDefaultConfig(&dbiConfig);
    dbiConfig.acTimeUnit      = 0;
#if (defined(FSL_FEATURE_LCDIF_HAS_DBIX_POLARITY) && FSL_FEATURE_LCDIF_HAS_DBIX_POLARITY)
    dbiConfig.reversePolarity = true;
#endif
    dbiConfig.writeWRPeriod   = 14U;
    /* With 279.53MHz source and 16bpp format, a 14 cycle period requires a 279.53MHz / 14 * 16 = 319.46Mhz DPHY clk
     * source. */
    dbiConfig.format          = kLCDIF_DbiOutD16RGB565;
    dbiConfig.type            = kLCDIF_DbiTypeB;
    dbiConfig.writeCSAssert   = 1;
    dbiConfig.writeCSDeassert = 4;
    dbiConfig.writeWRAssert   = (dbiConfig.writeWRPeriod - 1U) / 2U; /* Asset at the middle. */
    dbiConfig.writeWRDeassert = (dbiConfig.writeWRPeriod - 1U);      /* Deassert at the end */

    LCDIF_Init(DEMO_SSD1963_LCDIF);
    LCDIF_DbiModeSetConfig(DEMO_SSD1963_LCDIF, 0, &dbiConfig);

    LCDIF_PanelGetDefaultConfig(&config);
    LCDIF_SetPanelConfig(DEMO_SSD1963_LCDIF, 0, &config);

    NVIC_ClearPendingIRQ(DEMO_SSD1963_LCDIF_IRQn);
    NVIC_SetPriority(DEMO_SSD1963_LCDIF_IRQn, 3);
    NVIC_EnableIRQ(DEMO_SSD1963_LCDIF_IRQn);
}

status_t BOARD_PrepareDisplayController(void)
{
    /* 1. Initialize LCDIF and MIPI-DSI. */
    /* Initialize clock, power and reset. */
    BOARD_InitMipiDsiClock();

    BOARD_InitLcdifPowerReset();

    /* Configures peripheral. */
    BOARD_InitMipiDsi();

    BOARD_InitLcdif();

    /* 2. Create the LCDIF DBI XFER interface. */
    DBI_LCDIF_InitController(&(s_dcDbiHandle.dbiIface), &s_lcdifPrvData, DEMO_SSD1963_LCDIF, DEMO_MIPI_DSI);

    /* 3. Initialize the panel. */
    BOARD_InitLcdPanel();

    return DISPLAY_Init(&rm67162Handle, &displayConfig);
}

void LCDIF_IRQHandler(void)
{
    DBI_LCDIF_IRQHandler(&(s_dcDbiHandle.dbiIface));
}

/* Smart panel TE pin IRQ handler. */
void BOARD_DisplayTEPinHandler(void)
{
    DC_FB_DBI_TE_IRQHandler(&g_dc);
}

#else /* RM67162_USE_LCDIF = 0 */

static status_t BOARD_DsiMemWriteSendChunck(void)
{
    uint32_t curSendLen;
    uint32_t i;

    curSendLen =
        DEMO_DSI_TX_ARRAY_MAX > s_dsiMemWriteCtx.leftByteLen ? s_dsiMemWriteCtx.leftByteLen : DEMO_DSI_TX_ARRAY_MAX;

    s_dsiMemWriteXfer.txDataType = kDSI_TxDataDcsLongWr;
    s_dsiMemWriteXfer.dscCmd     = s_dsiMemWriteCtx.dscCmd;
    s_dsiMemWriteXfer.txData     = s_dsiMemWriteTmpArray;
    s_dsiMemWriteXfer.txDataSize = curSendLen;

    /* For each pixel, the MIPI DSI sends out low byte first, but according to
     * the MIPI DSC spec, the high byte should be send first, so swap the pixel byte
     * first.
     */
#if (DEMO_RM67162_BUFFER_FORMAT == DEMO_RM67162_BUFFER_RGB565)
    for (i = 0; i < curSendLen; i += 2)
    {
        s_dsiMemWriteTmpArray[i]     = *(s_dsiMemWriteCtx.txData + 1);
        s_dsiMemWriteTmpArray[i + 1] = *(s_dsiMemWriteCtx.txData);

        s_dsiMemWriteCtx.txData += 2;
    }
#else
    for (i = 0; i < curSendLen; i += 3)
    {
        s_dsiMemWriteTmpArray[i]     = *(s_dsiMemWriteCtx.txData + 2);
        s_dsiMemWriteTmpArray[i + 1] = *(s_dsiMemWriteCtx.txData + 1);
        s_dsiMemWriteTmpArray[i + 2] = *(s_dsiMemWriteCtx.txData);

        s_dsiMemWriteCtx.txData += 3;
    }
#endif

    s_dsiMemWriteCtx.leftByteLen -= curSendLen;
    s_dsiMemWriteCtx.dscCmd = kMIPI_DCS_WriteMemoryContinue;

    return DSI_TransferNonBlocking(DEMO_MIPI_DSI, &s_dsiDriverHandle, &s_dsiMemWriteXfer);
}

static void BOARD_DsiMemWriteCallback(MIPI_DSI_HOST_Type *base, dsi_handle_t *handle, status_t status, void *userData)
{
    if ((kStatus_Success == status) && (s_dsiMemWriteCtx.leftByteLen > 0))
    {
        status = BOARD_DsiMemWriteSendChunck();

        if (kStatus_Success == status)
        {
            return;
        }
    }

    s_dsiMemWriteCtx.onGoing = false;
    MIPI_DSI_MemoryDoneDriverCallback(status, &dsiDevice);
}

static status_t BOARD_DSI_MemWrite(uint8_t virtualChannel, const uint8_t *data, uint32_t length)
{
    status_t status;

    if (s_dsiMemWriteCtx.onGoing)
    {
        return kStatus_Fail;
    }

    s_dsiMemWriteXfer.virtualChannel = virtualChannel;
    s_dsiMemWriteXfer.flags          = kDSI_TransferUseHighSpeed;
    s_dsiMemWriteXfer.sendDscCmd     = true;

    s_dsiMemWriteCtx.onGoing     = true;
    s_dsiMemWriteCtx.txData      = data;
    s_dsiMemWriteCtx.leftByteLen = length;
    s_dsiMemWriteCtx.dscCmd      = kMIPI_DCS_WriteMemoryStart;

    status = BOARD_DsiMemWriteSendChunck();

    if (status != kStatus_Success)
    {
        /* Memory write does not start actually. */
        s_dsiMemWriteCtx.onGoing = false;
    }

    return status;
}

/* Smart panel TE pin IRQ handler. */
void BOARD_DisplayTEPinHandler(void)
{
    DC_FB_DSI_CMD_TE_IRQHandler(&g_dc);
}

status_t BOARD_InitDisplayInterface(void)
{
    /* 1. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 2. Configures peripheral. */
    BOARD_InitMipiDsi();

    /* 3. Configure the panel. */
    return BOARD_InitLcdPanel();
}

status_t BOARD_PrepareDisplayController(void)
{
    status_t status;

    status = BOARD_InitDisplayInterface();

    if (kStatus_Success == status)
    {
        /*
         * Suggest setting to low priority. Because a new DSI transfer is prepared
         * in the callback BOARD_DsiMemWriteCallback, so the core spends more time
         * in ISR. Setting the low priority, then the important ISR won't be blocked.
         */
        NVIC_SetPriority(DEMO_MIPI_DSI_IRQn, 6);
    }
    else
    {
        return status;
    }

    memset(&s_dsiMemWriteCtx, 0, sizeof(dsi_mem_write_ctx_t));

    /* Create the MIPI DSI trasnfer handle for non-blocking data trasnfer. */
    return DSI_TransferCreateHandle(DEMO_MIPI_DSI, &s_dsiDriverHandle, BOARD_DsiMemWriteCallback, NULL);
}

#endif /* RM67162_USE_LCDIF */

#endif
