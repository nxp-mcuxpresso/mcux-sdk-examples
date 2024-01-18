/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_rgpio.h"
#include "fsl_mipi_dsi.h"
#include "fsl_rm68200.h"
#include "fsl_hx8394.h"
#include "fsl_reset.h"
#include "fsl_upower.h"
#include "fsl_pca6416a.h"
#include "lcdif_support.h"
#include "fsl_debug_console.h"

/*
 * The DPHY bit clock must be fast enough to send out the pixels, it should be
 * larger than:
 *
 *         (Pixel clock * bit per output pixel) / number of MIPI data lane
 *
 * Here the desired DPHY bit clock multiplied by ( 9 / 8 = 1.125) to ensure
 * it is fast enough.
 */
#define DEMO_MIPI_DPHY_BIT_CLK_ENLARGE(origin) (((origin) / 8) * 9)

uint32_t mipiDsiTxEscClkFreq_Hz;
uint32_t mipiDsiDphyBitClkFreq_Hz;
uint32_t mipiDsiDphyRefClkFreq_Hz;
uint32_t mipiDsiDpiClkFreq_Hz;

static void PANEL_PullResetPin(bool pullUp)
{
    if (pullUp)
    {
        RGPIO_PinWrite(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, 1);
    }
    else
    {
        RGPIO_PinWrite(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, 0);
    }
}

static void PANEL_PullPowerPin(bool pullUp)
{
    /* Power pin is connected to high on board. */
}

status_t PANEL_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
}

static mipi_dsi_device_t dsiDevice = {
    .virtualChannel = 0,
    .xferFunc       = PANEL_DSI_Transfer,
};

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)

static const rm68200_resource_t rm68200Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = PANEL_PullResetPin,
    .pullPowerPin = PANEL_PullPowerPin,
};

static display_handle_t rm68200Handle = {
    .resource = &rm68200Resource,
    .ops      = &rm68200_ops,
};

#else

static const hx8394_resource_t hx8394Resource = {
    .dsiDevice    = &dsiDevice,
    .pullResetPin = PANEL_PullResetPin,
    .pullPowerPin = PANEL_PullPowerPin,
};

static display_handle_t hx8394Handle = {
    .resource = &hx8394Resource,
    .ops      = &hx8394_ops,
};

#endif

void BOARD_InitLcdifClock(void)
{
    UPOWER_PowerOnSwitches(kUPOWER_PS_AV_NIC);
    UPOWER_PowerOnMemPart(kUPOWER_MP0_DCNANO_A | kUPOWER_MP0_DCNANO_B, 0U);

    /*
     * DCNano/LCDIF pixel clock is selected by PCC5->PCC_DC_NANO, PCC5_PLAT clock.
     *
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     * Here use the PLL4PDF3DIV2(396MHz) as clock source.
     * For 60Hz frame rate, the RK055AHD091 pixel clock should be 62MHz, set the
     * clock to 66MHz.
     */
    CLOCK_SetIpSrcDiv(kCLOCK_Dcnano, kCLOCK_Pcc5PlatIpSrcPll4Pfd3Div2, 5, 0U);

    /* Enable DCNano/LCDIF */
    CLOCK_EnableClock(kCLOCK_Dcnano);
    /* Release DCNano/LCDIF from reset */
    RESET_PeripheralReset(kRESET_Dcnano);

    mipiDsiDpiClkFreq_Hz = CLOCK_GetDcnanoClkFreq();

    PRINTF("LCDIF pixel clock is: %dHz\r\n", mipiDsiDpiClkFreq_Hz);

    *(volatile uint32_t *)0x2802B044 &= ~0x00000080; /* LPAV alloc to RTD */
    *(volatile uint32_t *)0x2802B04C &= ~0x00000008; /* LCDIF alloc to RTD */
}

static status_t BOARD_InitLcdPanel(void)
{
    status_t status;

    const rgpio_pin_config_t pinConfig = {
        .pinDirection = kRGPIO_DigitalOutput,
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

    RGPIO_PinInit(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, &pinConfig);
    RGPIO_PinInit(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, &pinConfig);

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
    status = RM68200_Init(&rm68200Handle, &displayConfig);
#else
    status = HX8394_Init(&hx8394Handle, &displayConfig);
#endif

    if (status == kStatus_Success)
    {
        RGPIO_PinWrite(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, 1);
    }

    return status;
}

static void BOARD_InitMipiDsiClock(void)
{
    UPOWER_PowerOnMemPart(kUPOWER_MP0_MIPI_DSI, 0);

    /*
     * RxClkEsc larger than 60MHz, TxClkEsc 12 to 20MHz.
     * In the SOC design, RxClkEsc is fixed to 4 x TxClkEsc.
     *
     * Here use PLL4PFD3DIV2 clock (396MHz) as clock source.
     * RxClkEsc = 396MHz / 5 = 79.20MHz
     * TxClkEsc = 396MHz / 5 / 4 = 19.80MHz
     */
    CLOCK_SetIpSrcDiv(kCLOCK_Dsi, kCLOCK_Pcc5PlatIpSrcPll4Pfd3Div2, 4, 0U);
    CLOCK_EnableClock(kCLOCK_Dsi);

    mipiDsiTxEscClkFreq_Hz = CLOCK_GetDsiClkFreq() / 4U;

    PRINTF("MIPI DSI tx_esc_clk frequency: %dHz\r\n", mipiDsiTxEscClkFreq_Hz);

    if ((mipiDsiTxEscClkFreq_Hz < 12000000U) || (mipiDsiTxEscClkFreq_Hz > 20000000U))
    {
        PRINTF("Invalid MIPI DSI tx_esc_clk frequency\r\n");
        while (1)
            ;
    }

    /* DPHY reference clock is OSC clock. */
    mipiDsiDphyRefClkFreq_Hz = CLOCK_GetSysOscFreq();
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

    /* Init the DSI module. */
    DSI_Init(DEMO_MIPI_DSI, &dsiConfig);

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
    mipiDsiDphyBitClkFreq_Hz = mipiDsiDpiClkFreq_Hz * (24 / DEMO_MIPI_DSI_LANE_NUM);

    mipiDsiDphyBitClkFreq_Hz = DEMO_MIPI_DPHY_BIT_CLK_ENLARGE(mipiDsiDphyBitClkFreq_Hz);

    DSI_GetDphyDefaultConfig(&dphyConfig, mipiDsiDphyBitClkFreq_Hz, mipiDsiTxEscClkFreq_Hz);

    /* Init DPHY. */
    mipiDsiDphyBitClkFreq_Hz = DSI_InitDphy(DEMO_MIPI_DSI, &dphyConfig, mipiDsiDphyRefClkFreq_Hz);

    /* Init DPI interface. */
    DSI_SetDpiConfig(DEMO_MIPI_DSI, &dpiConfig, DEMO_MIPI_DSI_LANE_NUM, mipiDsiDpiClkFreq_Hz, mipiDsiDphyBitClkFreq_Hz);

    PRINTF("MIPI DSI DPHY bit clock: %dHz\r\n", mipiDsiDphyBitClkFreq_Hz);
}

status_t BOARD_InitDisplayInterface(void)
{
    *(volatile uint32_t *)0x2802B04C &= ~0x00000010; /* DSI alloc to RTD */

    /* Switch DPI MUX to DCNano/LCDIF. */
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_DPI2_EPDC_DCNANO_MUX_SEL_MASK;

    /* 1. Assert all resets. */
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_RST_ESC_N_MASK;
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_RST_BYTE_N_MASK;
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_RST_DPI_N_MASK;

    RESET_PeripheralReset(kRESET_Dsi);

    /* 2. Setup clock. */
    BOARD_InitMipiDsiClock();

    /* 3. Configures peripheral. */
    BOARD_SetMipiDsiConfig();

    /* 4. Deassert resets. */
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_RST_DPI_N_MASK;
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_RST_BYTE_N_MASK;
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_RST_ESC_N_MASK;

    /* Route MIPI signal to LCD. */
    if (kStatus_Success != PCA6416A_SetPins(&g_pca6416aHandle, (1U << BOARD_PCA6416A_MIPI_SWITCH)))
    {
        PRINTF("ERROR: MIPI_SWITCH pin configure failed\r\n");
    }
    if (kStatus_Success !=
        PCA6416A_SetDirection(&g_pca6416aHandle, (1U << BOARD_PCA6416A_MIPI_SWITCH), kPCA6416A_Output))
    {
        PRINTF("ERROR: MIPI_SWITCH pin configure failed\r\n");
    }

    /* 5. Configure the panel. */
    return BOARD_InitLcdPanel();
}
