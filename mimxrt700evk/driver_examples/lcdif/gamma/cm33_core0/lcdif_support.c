/*
 * Copyright 2023,2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_mipi_dsi.h"
#include "lcdif_support.h"
#if USE_DBI
#if (USE_DBI_PANEL == PANEL_TFT_PROTO_5)
#include "fsl_ssd1963.h"
#include "fsl_lcdif.h"
#else
#include "fsl_rm67162.h"
#endif
#else
#if (USE_MIPI_PANEL == MIPI_PANEL_RK055IQH091)
#include "fsl_rm68191.h"
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
#include "fsl_rm68200.h"
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
#include "fsl_hx8394.h"
#elif (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
#include "fsl_rpi.h"
#endif
#endif /* USE_DBI */
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

uint32_t mipiDsiDpiClkFreq_Hz;

#if ((USE_DBI) && (USE_DBI_PANEL == PANEL_TFT_PROTO_5))

static status_t DEMO_LcdWriteCommand(void *dbiXferHandle, uint32_t command)
{
    LCDIF_Type *lcdifBase = (LCDIF_Type *)dbiXferHandle;

    LCDIF_DbiSendCommand(lcdifBase, 0U, command);

    return kStatus_Success;
}

static status_t DEMO_LcdWriteData(void *dbiXferHandle, void *data, uint32_t len_byte)
{
    LCDIF_Type *lcdifBase = (LCDIF_Type *)dbiXferHandle;

    LCDIF_DbiSendData(lcdifBase, 0U, data, len_byte);

    return kStatus_Success;
}

#else
uint32_t mipiDsiTxEscClkFreq_Hz;
uint32_t mipiDsiDphyBitClkFreq_Hz;

#if ((!USE_DBI) && (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH))
static status_t BOARD_ReadPanelStatus(uint8_t regAddr, uint8_t *value)
{
    return BOARD_I2C_Receive(BOARD_MIPI_PANEL_TOUCH_I2C_BASEADDR, RPI_ADDR, regAddr, 1U, value, 1U);
}

static status_t BOARD_WritePanelRegister(uint8_t regAddr, uint8_t value)
{
    return BOARD_I2C_Send(BOARD_MIPI_PANEL_TOUCH_I2C_BASEADDR, RPI_ADDR, regAddr, 1, &value, 1);
}
#else
static void PANEL_PullResetPin(bool pullUp)
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

static void PANEL_PullPowerPin(bool pullUp)
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

status_t PANEL_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
}
#endif

#if USE_DBI
#if (USE_DBI_PANEL == PANEL_TFT_PROTO_5)
static ssd1963_handle_t lcdHandle;
/* The functions used to drive the panel. */
static dbi_xfer_ops_t s_lcdifDbiOps = {
    .writeCommand          = DEMO_LcdWriteCommand,
    .writeData             = DEMO_LcdWriteData,
    .writeMemory           = NULL,
    .readMemory            = NULL, /* Don't need read in this project. */
    .setMemoryDoneCallback = NULL, /* Write memory is blocking function, don't need callback. */
};
#else
static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = PANEL_DSI_Transfer,
};

static const rm67162_resource_t rm67162Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = PANEL_PullResetPin,
    .pullPowerPin = PANEL_PullPowerPin,
};

static display_handle_t rm67162Handle = {
    .resource = &rm67162Resource,
    .ops      = &rm67162_ops,
};
#endif
#else /* USE_DBI */

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = PANEL_DSI_Transfer,
};

static const rm68200_resource_t rm68200Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = PANEL_PullResetPin,
    .pullPowerPin = PANEL_PullPowerPin,
};

static display_handle_t rm68200Handle = {
    .resource = &rm68200Resource,
    .ops      = &rm68200_ops,
};

#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = PANEL_DSI_Transfer,
};

static const hx8394_resource_t hx8394Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = PANEL_PullResetPin,
    .pullPowerPin = PANEL_PullPowerPin,
};

static display_handle_t hx8394Handle = {
    .resource = &hx8394Resource,
    .ops      = &hx8394_ops,
};

#elif (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = PANEL_DSI_Transfer,
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
    .xferFunc       = PANEL_DSI_Transfer,
};

static const rm68191_resource_t rm68191Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = PANEL_PullResetPin,
    .pullPowerPin = PANEL_PullPowerPin,
};

static display_handle_t rm68191Handle = {
    .resource = &rm68191Resource,
    .ops      = &rm68191_ops,
};
#endif
#endif /* USE_DBI */

void BOARD_InitLcdifClock(void)
{
    POWER_DisablePD(kPDRUNCFG_SHUT_MEDIA_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_APD_LCDIF);
    POWER_DisablePD(kPDRUNCFG_PPD_LCDIF);
    POWER_ApplyPD();

#if !USE_DBI
    /*
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     * Here use the main pll (528MHz) as clock source.
     * Since MIPI DPHY clock use AUDIO pll pfd2 as aource, and its max allowed clock frequency is
     * 532.48 x 18 / 16 = 599.04MHz. To avoid exceed this limit, for RK055AHD091 and RK055MHD091,
     * the frame rate shall be 36.3fps and 37.72fps, which is 37.71MHz pixel clock. For RK055IQH091
     * it's resolution is safe to use 60fps frame rate, which is 35.2mHz pixel clock.
     * For RaspberryPi panel, the frame rate shall be 42fps, which is 19.56MHz pixel clock.
     */
    CLOCK_AttachClk(kMAIN_PLL_PFD2_to_LCDIF);
#if ((USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091) || (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091))
    CLOCK_SetClkDiv(kCLOCK_DivLcdifClk, 14);
#elif (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
    CLOCK_SetClkDiv(kCLOCK_DivLcdifClk, 27);
#else
    CLOCK_SetClkDiv(kCLOCK_DivLcdifClk, 15);
#endif
#endif

    /* Get lcdif pixel clock frequency. */
    mipiDsiDpiClkFreq_Hz = CLOCK_GetLcdifClkFreq();

    CLOCK_EnableClock(kCLOCK_Lcdif);
    RESET_ClearPeripheralReset(kLCDIF_RST_SHIFT_RSTn);
}

#if ((USE_DBI) && (USE_DBI_PANEL == PANEL_TFT_PROTO_5))
status_t BOARD_InitDisplayInterface(void)
{
    status_t status;

    const ssd1963_config_t ssd1963Config = {.pclkFreq_Hz    = DEMO_SSD1963_PCLK_FREQ,
                                            .pixelInterface = SSD1963_DEFAULT_PIXEL_FORMAT_SSD1963,
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

    /* Reset the SSD1963 LCD controller. */
    GPIO_PinWrite(BOARD_SSD1963_RST_GPIO, BOARD_SSD1963_RST_PIN, 0);
    SDK_DelayAtLeastUs(10, SystemCoreClock);   /* Delay 10ns. */
    GPIO_PinWrite(BOARD_SSD1963_RST_GPIO, BOARD_SSD1963_RST_PIN, 1);
    SDK_DelayAtLeastUs(5000, SystemCoreClock); /* Delay 5ms. */

    status = SSD1963_Init(&lcdHandle, &ssd1963Config, &s_lcdifDbiOps, LCDIF, DEMO_SSD1963_XTAL_FREQ);

    if (kStatus_Success != status)
    {
        return status;
    }

    SSD1963_StartDisplay(&lcdHandle);
    SSD1963_SetBackLight(&lcdHandle, 255);

    return kStatus_Success;
}
#else
static status_t BOARD_InitLcdPanel(void)
{
    status_t status;

    const display_config_t displayConfig = {
        .resolution   = FSL_VIDEO_RESOLUTION(DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT),
        .hsw          = DEMO_HSW,
        .hfp          = DEMO_HFP,
        .hbp          = DEMO_HBP,
        .vsw          = DEMO_VSW,
        .vfp          = DEMO_VFP,
        .vbp          = DEMO_VBP,
        .controlFlags = 0,
        .dsiLanes     = DEMO_MIPI_DSI_LANE_NUM,
#if USE_DBI
        .pixelFormat  = DEMO_BUFFER_PIXEL_FORMAT,
#endif
    };

#if ((!USE_DBI) && (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH))
    BOARD_MIPIPanelTouch_I2C_Init();
#else
    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

    GPIO_PinInit(BOARD_MIPI_POWER_GPIO, BOARD_MIPI_POWER_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, &pinConfig);
#endif

#if USE_DBI
    status = RM67162_Init(&rm67162Handle, &displayConfig);
#else
#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
    status = RM68200_Init(&rm68200Handle, &displayConfig);
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
    status = HX8394_Init(&hx8394Handle, &displayConfig);
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055IQH091)
    status = RM68191_Init(&rm68191Handle, &displayConfig);
#elif (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
    status = RPI_Init(&rpiHandle, &displayConfig);
#endif
#endif

#if (USE_MIPI_PANEL != MIPI_PANEL_RASPI_7INCH)
    if (status == kStatus_Success)
    {
        GPIO_PinWrite(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, 1);
    }
#endif

    return status;
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

#if USE_DBI
    /*
     * DPHY supports up to 532.48 x 18 / 16 = 599.04MHz bit clock.
     * When choose the DPHY clock frequency, consider the panel frame rate and
     * resolution.
     *
     * RM67162 controller maximum total bit rate is:
     *  - 500Mbps of 2 data lanes 24-bit data format
     *  - 360Mbps of 2 data lanes 18-bit data format
     *  - 320Mbps of 2 data lanes 16-bit data format
     *
     */
    /* With 279.53MHz source and 16bpp format, a 14 cycle period requires a 279.53MHz / 14 * 16 = 319.46Mhz DPHY clk
       source. Considering the DCS packaging cost, the MIPI DPHY speed shall be SLIGHTLY larger than the DBI interface
       speed. DPHY uses AUDIO_PLL_PFD2 which is 532.48MHz as source, the frequency is 532.48 * 18 / 30 = 319.49MHz. */
    CLOCK_InitAudioPfd(kCLOCK_Pfd2, 30U);
    CLOCK_AttachClk(kAUDIO_PLL_PFD2_to_MIPI_DSI_HOST_PHY);
    CLOCK_SetClkDiv(kCLOCK_DivDphyClk, 1U);
#else
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

    uint8_t div = (uint8_t)((uint64_t)CLOCK_GetAudioPllFreq() * 18U / (uint64_t)mipiDsiDphyBitClkFreq_Hz);

    CLOCK_InitAudioPfd(kCLOCK_Pfd2, div);
    CLOCK_AttachClk(kAUDIO_PLL_PFD2_to_MIPI_DSI_HOST_PHY);
    CLOCK_SetClkDiv(kCLOCK_DivDphyClk, 1);
#endif

    mipiDsiDphyBitClkFreq_Hz = CLOCK_GetMipiDphyClkFreq();
}

static void BOARD_SetMipiDsiConfig(void)
{
    dsi_config_t dsiConfig;
    dsi_dphy_config_t dphyConfig;

#if !USE_DBI
    const dsi_dpi_config_t dpiConfig = {
        .pixelPayloadSize = DEMO_PANEL_WIDTH,
        .dpiColorCoding   = kDSI_Dpi24Bit,
        .pixelPacket      = kDSI_PixelPacket24Bit,
#if (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
        .videoMode        = kDSI_DpiNonBurstWithSyncPulse,
#else
        .videoMode = kDSI_DpiBurst,
#endif
        .bllpMode         = kDSI_DpiBllpLowPower,
        .polarityFlags    = kDSI_DpiVsyncActiveLow | kDSI_DpiHsyncActiveLow,
        .hfp              = DEMO_HFP,
        .hbp              = DEMO_HBP,
        .hsw              = DEMO_HSW,
        .vfp              = DEMO_VFP,
        .vbp              = DEMO_VBP,
        .panelHeight      = DEMO_PANEL_HEIGHT,
        .virtualChannel   = 0
    };
#endif

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
#if (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
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

#if USE_DBI
    DSI_SetDbiPixelFormat(DEMO_MIPI_DSI, kDSI_DbiRGB565);
#else
    /* Init DPI interface. */
    DSI_SetDpiConfig(DEMO_MIPI_DSI, &dpiConfig, DEMO_MIPI_DSI_LANE_NUM, mipiDsiDpiClkFreq_Hz, mipiDsiDphyBitClkFreq_Hz);
#endif
}

status_t BOARD_InitDisplayInterface(void)
{
    /* 1. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 2. Configures peripheral. */
    BOARD_SetMipiDsiConfig();

    /* 3. Configure the panel. */
    return BOARD_InitLcdPanel();
}
#endif