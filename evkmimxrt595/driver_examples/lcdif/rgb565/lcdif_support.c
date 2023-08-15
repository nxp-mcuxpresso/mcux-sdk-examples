/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_mipi_dsi.h"
#include "lcdif_support.h"
#if (USE_MIPI_PANEL == MIPI_PANEL_RK055IQH091)
#include "fsl_rm68191.h"
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
#include "fsl_rm68200.h"
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
#include "fsl_hx8394.h"
#endif

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

uint32_t mipiDsiTxEscClkFreq_Hz;
uint32_t mipiDsiDphyBitClkFreq_Hz;
uint32_t mipiDsiDpiClkFreq_Hz;

static void PANEL_PullResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_RST_PORT, BOARD_MIPI_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_RST_PORT, BOARD_MIPI_RST_PIN, 0);
    }
}

static void PANEL_PullPowerPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_POWER_PORT, BOARD_MIPI_POWER_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_POWER_PORT, BOARD_MIPI_POWER_PIN, 0);
    }
}

status_t PANEL_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
}

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

void BOARD_InitLcdifClock(void)
{
    POWER_DisablePD(kPDRUNCFG_APD_DCNANO_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_DCNANO_SRAM);
    POWER_ApplyPD();

    /*
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     * Here use the aux0 pll (396MHz) as clock source.
     * For 60Hz frame rate, the RK055IQH091 pixel clock should be 36MHz.
     * the RK055AHD091 pixel clock should be 62MHz.
     */
    CLOCK_AttachClk(kAUX0_PLL_to_DCPIXEL_CLK);
#if ((USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091) || (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091))
    CLOCK_SetClkDiv(kCLOCK_DivDcPixelClk, 7);
#else
    CLOCK_SetClkDiv(kCLOCK_DivDcPixelClk, 11);
#endif

    mipiDsiDpiClkFreq_Hz = CLOCK_GetDcPixelClkFreq();

    CLOCK_EnableClock(kCLOCK_DisplayCtrl);
    RESET_ClearPeripheralReset(kDISP_CTRL_RST_SHIFT_RSTn);

    CLOCK_EnableClock(kCLOCK_AxiSwitch);
    RESET_ClearPeripheralReset(kAXI_SWITCH_RST_SHIFT_RSTn);
}

static status_t BOARD_InitLcdPanel(void)
{
    status_t status;

    const gpio_pin_config_t pinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

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
    };

    GPIO_PinInit(GPIO, BOARD_MIPI_POWER_PORT, BOARD_MIPI_POWER_PIN, &pinConfig);
    GPIO_PinInit(GPIO, BOARD_MIPI_RST_PORT, BOARD_MIPI_RST_PIN, &pinConfig);
    GPIO_PinInit(GPIO, BOARD_MIPI_BL_PORT, BOARD_MIPI_BL_PIN, &pinConfig);

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
    status = RM68200_Init(&rm68200Handle, &displayConfig);
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
    status = HX8394_Init(&hx8394Handle, &displayConfig);
#else
    status = RM68191_Init(&rm68191Handle, &displayConfig);
#endif

    if (status == kStatus_Success)
    {
        GPIO_PinWrite(GPIO, BOARD_MIPI_BL_PORT, BOARD_MIPI_BL_PIN, 1);
    }

    return status;
}

static void BOARD_InitMipiDsiClock(void)
{
    POWER_DisablePD(kPDRUNCFG_APD_MIPIDSI_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_MIPIDSI_SRAM);
    POWER_DisablePD(kPDRUNCFG_PD_MIPIDSI);
    POWER_ApplyPD();

    /* RxClkEsc max 60MHz, TxClkEsc 12 to 20MHz. */
    CLOCK_AttachClk(kFRO_DIV1_to_MIPI_DPHYESC_CLK);
    /* RxClkEsc = 192MHz / 4 = 48MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscRxClk, 4);
    /* TxClkEsc = 192MHz / 4 / 3 = 16MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivDphyEscTxClk, 3);
    mipiDsiTxEscClkFreq_Hz = CLOCK_GetMipiDphyEscTxClkFreq();

    /* The DPHY bit clock must be fast enough to send out the pixels, it should be
     * larger than:
     *
     *         (Pixel clock * bit per output pixel) / number of MIPI data lane
     *
     * DPHY supports up to 895.1MHz bit clock. The MIPI panel supports up to 850MHz bit clock.
     */
    /* Use AUX1 PLL clock.
     * AUX1 PLL clock is system pll clock * 18 / pfd.
     * system pll clock is 528MHz defined in clock_config.c
     */
    CLOCK_AttachClk(kAUX1_PLL_to_MIPI_DPHY_CLK);
#if ((USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091) || (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091))
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 12);
#else
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 18);
#endif
    CLOCK_SetClkDiv(kCLOCK_DivDphyClk, 1);
    mipiDsiDphyBitClkFreq_Hz = CLOCK_GetMipiDphyClkFreq();
}

static void BOARD_SetMipiDsiConfig(void)
{
    dsi_config_t dsiConfig;
    dsi_dphy_config_t dphyConfig;

    const dsi_dpi_config_t dpiConfig = {.pixelPayloadSize = DEMO_PANEL_WIDTH,
                                        .dpiColorCoding   = kDSI_Dpi24Bit,
                                        .pixelPacket      = kDSI_PixelPacket24Bit,
                                        .videoMode        = kDSI_DpiBurst,
                                        .bllpMode         = kDSI_DpiBllpLowPower,
                                        .polarityFlags    = kDSI_DpiVsyncActiveLow | kDSI_DpiHsyncActiveLow,
                                        .hfp              = DEMO_HFP,
                                        .hbp              = DEMO_HBP,
                                        .hsw              = DEMO_HSW,
                                        .vfp              = DEMO_VFP,
                                        .vbp              = DEMO_VBP,
                                        .panelHeight      = DEMO_PANEL_HEIGHT,
                                        .virtualChannel   = 0};

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

    /* Init DPI interface. */
    DSI_SetDpiConfig(DEMO_MIPI_DSI, &dpiConfig, DEMO_MIPI_DSI_LANE_NUM, mipiDsiDpiClkFreq_Hz, mipiDsiDphyBitClkFreq_Hz);
}

status_t BOARD_InitDisplayInterface(void)
{
    /* 1. Assert MIPI DPHY reset. */
    RESET_SetPeripheralReset(kMIPI_DSI_PHY_RST_SHIFT_RSTn);

    /* 2. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 3. Configures peripheral. */
    RESET_ClearPeripheralReset(kMIPI_DSI_CTRL_RST_SHIFT_RSTn);
    BOARD_SetMipiDsiConfig();

    /* 4. Deassert reset. */
    RESET_ClearPeripheralReset(kMIPI_DSI_PHY_RST_SHIFT_RSTn);

    /* 5. Configure the panel. */
    return BOARD_InitLcdPanel();
}
