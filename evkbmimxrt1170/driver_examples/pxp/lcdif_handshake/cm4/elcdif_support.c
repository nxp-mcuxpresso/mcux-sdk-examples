/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_gpio.h"
#include "fsl_mipi_dsi.h"
#include "fsl_rm68191.h"
#include "fsl_rm68200.h"
#include "fsl_hx8394.h"
#include "elcdif_support.h"

uint32_t mipiDsiTxEscClkFreq_Hz;
uint32_t mipiDsiDphyBitClkFreq_Hz;
uint32_t mipiDsiDphyRefClkFreq_Hz;
uint32_t mipiDsiDpiClkFreq_Hz;
extern void APP_LCDIF_IRQHandler(void);

const MIPI_DSI_Type g_mipiDsi = {
    .host = DSI_HOST,
    .apb  = DSI_HOST_APB_PKT_IF,
    .dpi  = DSI_HOST_DPI_INTFC,
    .dphy = DSI_HOST_DPHY_INTFC,
};

static void PANEL_PullResetPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_RST_GPIO, BOARD_MIPI_PANEL_RST_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_RST_GPIO, BOARD_MIPI_PANEL_RST_PIN, 0);
    }
}

/* From the schematic, the power pin is pinned to high. */
static void PANEL_PullPowerPin(bool pullUp)
{
    if (pullUp)
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_POWER_GPIO, BOARD_MIPI_PANEL_POWER_PIN, 1);
    }
    else
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_POWER_GPIO, BOARD_MIPI_PANEL_POWER_PIN, 0);
    }
}

status_t PANEL_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(APP_MIPI_DSI, xfer);
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
    /*
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     *
     * Use PLL_528 as clock source.
     *
     * For ELCDIF handshake case, the pixel clock could not be set to high,
     * otherwise the PXP could not feed ELCDIF fast enough.
     * Here the pixel clock is set to 36MHz, then the RK055IQH091 frame rate is
     * 60Hz, the RK055AHD091 frame rate is 35Hz.
     */
    const clock_root_config_t lcdifClockConfig = {
        .clockOff = false,
        .mux      = 4, /*!< PLL_528. */
        .div      = 15,
    };

    CLOCK_SetRootClock(kCLOCK_Root_Lcdif, &lcdifClockConfig);

    mipiDsiDpiClkFreq_Hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Lcdif);
}

static status_t BOARD_InitLcdPanel(void)
{
    status_t status;

    const gpio_pin_config_t pinConfig = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    const display_config_t displayConfig = {
        .resolution   = FSL_VIDEO_RESOLUTION(APP_PANEL_WIDTH, APP_PANEL_HEIGHT),
        .hsw          = APP_HSW,
        .hfp          = APP_HFP,
        .hbp          = APP_HBP,
        .vsw          = APP_VSW,
        .vfp          = APP_VFP,
        .vbp          = APP_VBP,
        .controlFlags = 0,
        .dsiLanes     = APP_MIPI_DSI_LANE_NUM,
    };

    GPIO_PinInit(BOARD_MIPI_PANEL_POWER_GPIO, BOARD_MIPI_PANEL_POWER_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_PANEL_BL_GPIO, BOARD_MIPI_PANEL_BL_PIN, &pinConfig);
    GPIO_PinInit(BOARD_MIPI_PANEL_RST_GPIO, BOARD_MIPI_PANEL_RST_PIN, &pinConfig);

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
    status = RM68200_Init(&rm68200Handle, &displayConfig);
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
    status = HX8394_Init(&hx8394Handle, &displayConfig);
#else
    status = RM68191_Init(&rm68191Handle, &displayConfig);
#endif

    if (status == kStatus_Success)
    {
        GPIO_PinWrite(BOARD_MIPI_PANEL_BL_GPIO, BOARD_MIPI_PANEL_BL_PIN, 1);
    }

    return status;
}

static void BOARD_InitMipiDsiClock(void)
{
    uint32_t mipiDsiEscClkFreq_Hz;

    /* RxClkEsc max 60MHz, TxClkEsc 12 to 20MHz. */
    /* RxClkEsc = 528MHz / 11 = 48MHz. */
    /* TxClkEsc = 528MHz / 11 / 3 = 16MHz. */
    const clock_root_config_t mipiEscClockConfig = {
        .clockOff = false,
        .mux      = 4, /*!< PLL_528. */
        .div      = 11,
    };

    CLOCK_SetRootClock(kCLOCK_Root_Mipi_Esc, &mipiEscClockConfig);

    mipiDsiEscClkFreq_Hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Mipi_Esc);

    const clock_group_config_t mipiEscClockGroupConfig = {
        .clockOff = false, .resetDiv = 2, .div0 = 2, /* TX esc clock. */
    };

    CLOCK_SetGroupConfig(kCLOCK_Group_MipiDsi, &mipiEscClockGroupConfig);

    mipiDsiTxEscClkFreq_Hz = mipiDsiEscClkFreq_Hz / 3;

    /* DPHY reference clock, use OSC 24MHz clock. */
    const clock_root_config_t mipiDphyRefClockConfig = {
        .clockOff = false,
        .mux      = 1, /*!< OSC_24M. */
        .div      = 1,
    };

    CLOCK_SetRootClock(kCLOCK_Root_Mipi_Ref, &mipiDphyRefClockConfig);

    mipiDsiDphyRefClkFreq_Hz = CLOCK_GetRootClockFreq(kCLOCK_Root_Mipi_Ref);
}

static void BOARD_SetMipiDsiConfig(void)
{
    dsi_config_t dsiConfig;
    dsi_dphy_config_t dphyConfig;

    const dsi_dpi_config_t dpiConfig = {.pixelPayloadSize = APP_PANEL_WIDTH,
                                        .dpiColorCoding   = kDSI_Dpi24Bit,
                                        .pixelPacket      = kDSI_PixelPacket24Bit,
                                        .videoMode        = kDSI_DpiBurst,
                                        .bllpMode         = kDSI_DpiBllpLowPower,
                                        .polarityFlags    = kDSI_DpiVsyncActiveLow | kDSI_DpiHsyncActiveLow,
                                        .hfp              = APP_HFP,
                                        .hbp              = APP_HBP,
                                        .hsw              = APP_HSW,
                                        .vfp              = APP_VFP,
                                        .vbp              = APP_VBP,
                                        .panelHeight      = APP_PANEL_HEIGHT,
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
    dsiConfig.numLanes       = APP_MIPI_DSI_LANE_NUM;
    dsiConfig.autoInsertEoTp = true;

    /* Init the DSI module. */
    DSI_Init(APP_MIPI_DSI, &dsiConfig);

    /* Init DPHY.
     *
     * The DPHY bit clock must be fast enough to send out the pixels, it should be
     * larger than:
     *
     *         (Pixel clock * bit per output pixel) / number of MIPI data lane
     *
     * Here the desired DPHY bit clock multiplied by ( 9 / 8 = 1.125) to ensure
     * it is fast enough.
     *
     * Note that the DSI output pixel is 24bit per pixel.
     */
    mipiDsiDphyBitClkFreq_Hz = mipiDsiDpiClkFreq_Hz * (24 / APP_MIPI_DSI_LANE_NUM);

    mipiDsiDphyBitClkFreq_Hz = APP_MIPI_DPHY_BIT_CLK_ENLARGE(mipiDsiDphyBitClkFreq_Hz);

    DSI_GetDphyDefaultConfig(&dphyConfig, mipiDsiDphyBitClkFreq_Hz, mipiDsiTxEscClkFreq_Hz);

    mipiDsiDphyBitClkFreq_Hz = DSI_InitDphy(APP_MIPI_DSI, &dphyConfig, mipiDsiDphyRefClkFreq_Hz);

    /* Init DPI interface. */
    DSI_SetDpiConfig(APP_MIPI_DSI, &dpiConfig, APP_MIPI_DSI_LANE_NUM, mipiDsiDpiClkFreq_Hz, mipiDsiDphyBitClkFreq_Hz);
}

status_t BOARD_InitDisplayInterface(void)
{
    /* LCDIF output to MIPI DSI. */
    CLOCK_EnableClock(kCLOCK_Video_Mux);
    VIDEO_MUX->VID_MUX_CTRL.CLR = VIDEO_MUX_VID_MUX_CTRL_MIPI_DSI_SEL_MASK;

    /* 1. Power on and isolation off. */
    PGMC_BPC4->BPC_POWER_CTRL |= (PGMC_BPC_BPC_POWER_CTRL_PSW_ON_SOFT_MASK | PGMC_BPC_BPC_POWER_CTRL_ISO_OFF_SOFT_MASK);

    /* 2. Assert MIPI reset. */
    IOMUXC_GPR->GPR62 &=
        ~(IOMUXC_GPR_GPR62_MIPI_DSI_PCLK_SOFT_RESET_N_MASK | IOMUXC_GPR_GPR62_MIPI_DSI_ESC_SOFT_RESET_N_MASK |
          IOMUXC_GPR_GPR62_MIPI_DSI_BYTE_SOFT_RESET_N_MASK | IOMUXC_GPR_GPR62_MIPI_DSI_DPI_SOFT_RESET_N_MASK);

    /* 3. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 4. Deassert PCLK and ESC reset. */
    IOMUXC_GPR->GPR62 |=
        (IOMUXC_GPR_GPR62_MIPI_DSI_PCLK_SOFT_RESET_N_MASK | IOMUXC_GPR_GPR62_MIPI_DSI_ESC_SOFT_RESET_N_MASK);

    /* 5. Configures peripheral. */
    BOARD_SetMipiDsiConfig();

    /* 6. Deassert BYTE and DBI reset. */
    IOMUXC_GPR->GPR62 |=
        (IOMUXC_GPR_GPR62_MIPI_DSI_BYTE_SOFT_RESET_N_MASK | IOMUXC_GPR_GPR62_MIPI_DSI_DPI_SOFT_RESET_N_MASK);

    /* 7. Configure the panel. */
    return BOARD_InitLcdPanel();
}
