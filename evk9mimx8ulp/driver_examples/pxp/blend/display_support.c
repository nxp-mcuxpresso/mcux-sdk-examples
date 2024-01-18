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
#include "display_support.h"
#if APP_DISPLAY_EXTERNAL_CONVERTOR
#include "fsl_it6161.h"
#endif
#include "fsl_debug_console.h"

#include "fsl_dc_fb_lcdif.h"

uint32_t mipiDsiTxEscClkFreq_Hz;
uint32_t mipiDsiDphyBitClkFreq_Hz;
uint32_t mipiDsiDphyRefClkFreq_Hz;
uint32_t mipiDsiDpiClkFreq_Hz;

dc_fb_lcdif_handle_t s_dcFbLcdifHandle = {0}; /* The handle must be initialized to 0. */

enum _driver_ic_model
{
    RM68200,
    HX8394,
#if APP_DISPLAY_EXTERNAL_CONVERTOR
    IT6161, /* MIPI_DSI to HDMI bridge IC */
#endif
};

static enum _driver_ic_model whichDrvIcModel = HX8394;

const display_common_cfg s_displayCommonCfg[] = {
    /* The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate. */

    /*
     * for drive ic rm68200(The parameters is from linux driver "drivers/gpu/drm/panel/panel-raydium-rm68200.c"), 720 x
     * 1280@25Hz (1280 + 5 + 12 + 12) * (720 + 9 + 48 + 48) * 25 = 27 MHz
     */
    {
        .width  = DEMO_PANEL_WIDTH,
        .height = DEMO_PANEL_HEIGHT,
        .hsw    = 9,
        .hfp    = 48,
        .hbp    = 48,
        .vsw    = 5,
        .vfp    = 12,
        .vbp    = 12,
        .clock  = 27000, /* pixel clock in KHz, 27000 KHz = 27 MHz */
    },

    /*
     * for drive ic hx8394(The parameters is from linux driver "drivers/gpu/drm/panel/panel-rocktech-hx8394f.c"), 720 x
     * 1280@24Hz (1280 + 7 + 16 + 16) * (720 + 10 + 52 + 52) * 24 = 27 MHz
     */
    {
        .width  = DEMO_PANEL_WIDTH,
        .height = DEMO_PANEL_HEIGHT,
        .hsw    = 10,
        .hfp    = 52,
        .hbp    = 52,
        .vsw    = 7,
        .vfp    = 16,
        .vbp    = 16,
        .clock  = 27000, /* pixel clock in KHz, 27000 KHz = 27 MHz */
    },

#if APP_DISPLAY_EXTERNAL_CONVERTOR
    /*
     * for hdmi monitor
     * MIPI_DSI <-> MIPI_DSI to HDMI bridge(IT6161) <-> HDMI Monitor
     * Only support 720 x 480@59Hz
     * (480 + 6 + 9 + 30) * (720 + 62 + 16 + 60) * 59 = 27 MHz
     * These parameters from linux (drivers/gpu/drm/drm_edid.c, edid_cea_modes_1[], From CEA/CTA-861 spec.)
     */
    {
        .width  = DEMO_PANEL_WIDTH,
        .height = DEMO_PANEL_HEIGHT,
        .hsw    = 62,
        .hfp    = 16,
        .hbp    = 60,
        .vsw    = 6,
        .vfp    = 9,
        .vbp    = 30,
        .clock  = 27000, /* pixel clock in KHz, 27000 KHz = 27 MHz */
    },
#endif
};

dc_fb_lcdif_config_t s_dcFbLcdifConfig = {
    .lcdif         = DEMO_LCDIF,
    .polarityFlags = DEMO_POL_FLAGS,
    .outputFormat  = kLCDIF_Output24Bit,
    .width         = DEMO_PANEL_WIDTH,
    .height        = DEMO_PANEL_HEIGHT,
};

const dc_fb_t g_dc = {
    .ops     = &g_dcFbOpsLcdif,
    .prvData = &s_dcFbLcdifHandle,
    .config  = &s_dcFbLcdifConfig,
};

static dsi_dpi_config_t dpiConfig = {.pixelPayloadSize = DEMO_PANEL_WIDTH,
                                     .dpiColorCoding   = kDSI_Dpi24Bit,
                                     .pixelPacket      = kDSI_PixelPacket24Bit,
                                     .videoMode        = DEMO_DPI_VIDEO_MODE,
                                     .bllpMode         = kDSI_DpiBllpLowPower,
                                     .polarityFlags    = kDSI_DpiVsyncActiveLow | kDSI_DpiHsyncActiveLow,
                                     .virtualChannel   = 0};

status_t PANEL_DSI_Transfer(dsi_transfer_t *xfer)
{
    return DSI_TransferBlocking(DEMO_MIPI_DSI, xfer);
}

#if !APP_DISPLAY_EXTERNAL_CONVERTOR
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
static void IT6161_PullResetPin(bool pullUp)
{
    /* do something to reset the ic it6161 */
}

static it6161_resource_t it6161Resource = {
    .i2cAddr        = BOARD_IT6161_I2C_ADDR,
    .pullResetPin   = IT6161_PullResetPin,
    .i2cSendFunc    = BOARD_Display_I2C_Send,
    .i2cReceiveFunc = BOARD_Display_I2C_Receive,
};

display_handle_t it6161Handle = {
    .resource = &it6161Resource,
    .ops      = &it6161_ops,
};
#endif

#if APP_DISPLAY_EXTERNAL_CONVERTOR
static RGPIO_Type *const gpios[] = RGPIO_BASE_PTRS;

static void APP_HandleGPIOHandler(uint8_t gpioIdx)
{
    RGPIO_Type *gpio = gpios[gpioIdx];

    if (APP_GPIO_IDX(APP_IT6161_INT_PIN) == gpioIdx &&
        (1U << APP_PIN_IDX(APP_IT6161_INT_PIN)) & RGPIO_GetPinsInterruptFlags(gpio, APP_IT6161_INT_PIN_INT_SEL))
    {
        RGPIO_ClearPinsInterruptFlags(gpio, APP_IT6161_INT_PIN_INT_SEL, 1U << APP_PIN_IDX(APP_IT6161_INT_PIN));
        DisableIRQ(APP_IT6161_INT_PIN_IRQ_NUM);
        IT6161_Interrupt(&it6161Handle);
        EnableIRQ(APP_IT6161_INT_PIN_IRQ_NUM);
    }
}

void GPIOA_INT0_IRQHandler(void)
{
    APP_HandleGPIOHandler(0U);
}

void GPIOA_INT1_IRQHandler(void)
{
    APP_HandleGPIOHandler(0U);
}
#endif

display_config_t displayConfig = {
    .resolution   = FSL_VIDEO_RESOLUTION(DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT),
    .controlFlags = 0,
    .dsiLanes     = DEMO_MIPI_DSI_LANE_NUM,
};

void BOARD_InitLcdifClock(void)
{
    UPOWER_PowerOnSwitches(kUPOWER_PS_AV_NIC);
    UPOWER_PowerOnMemPart(kUPOWER_MP0_DCNANO_A | kUPOWER_MP0_DCNANO_B | kUPOWER_MP0_PXP, 0U);

    /*
     * DCNano/LCDIF pixel clock is selected by PCC5->PCC_DC_NANO, PCC5_PLAT clock.
     *
     * The pixel clock is (height + VSW + VFP + VBP) * (width + HSW + HFP + HBP) * frame rate.
     *
     * Use the PLL4PDF0DIV1(297 MHz) as clock source of DCNANO(LCDIF), pixel clock
     * of DCNANO(LCDIF) should be 27 MHz, set the clock to 66MHz, PLL4PFD0 output frequency = PLL4 * (18 / N) MHz = 528
     * * (18 / 32)= = 297 MHz (N = FRAC) PLL4PFD0DIV1 output frequency = 297 MHz / (10 + 1) = 27 MHz (10 is clock
     * division of pll4pfd0 clock). pixel clock of DCNANO(LCDIF) = 27 MHz / (0 + 1) = 27 MHz (0 is PCC_DC_NANO[PCD])
     * FRO24CLK --(24MHz)--> pll4 --(528 MHz)--> pll4pfd3 --(297MHz)--> pll4fd3div2 --(27MHz)--> PCC_DC_NANO --(27
     * MHz)-->DCNANO(LCDIF) -> MIPI DSI -> hdmi bridge(such as: IT6161) -> HDMI monitor
     */
    /* set pfd0div1 of pll4 to generate 27 MHz pixel clock rate for 720 x 480p@59Hz(for hdmi monitor),
     * 720x1280@25Hz(for mipi panel RK055AHD091, rm68200)
     * 720x1280@24Hz(for mipi panel RK055MHD091, hx8394)
     */
    const cgc_pll4_config_t g_cgcPll4Config = {.enableMode = kCGC_PllEnable,
                                               .div1       = 0U,
                                               .pfd0Div1   = DEMO_PLL4_PFD0DIV1,
                                               .pfd0Div2   = 0U,
                                               .pfd1Div1   = 0U,
                                               .pfd1Div2   = 0U,
                                               .pfd2Div1   = 0U,
                                               .pfd2Div2   = 0U,
                                               .pfd3Div1   = 0U,
                                               .pfd3Div2   = 0U,
                                               .src        = kCGC_PllSrcSysOsc,
                                               .mult       = kCGC_Pll4Mult22,
                                               .num        = 578,
                                               .denom      = 1000};
    CLOCK_InitPll4(&g_cgcPll4Config);
    CLOCK_EnablePll4PfdClkout(kCGC_PllPfd0Clk,
                              DEMO_PLL4_PFD0); /* pll4pfd0 output frequency is (528 MHz * 18) / 32 = 297 MHz */
    CGC_LPAV->PLL4DIV_PFD_0 = (g_cgcPll4Config.pfd0Div1 > 0 ?
                                   CGC_LPAV_PLL4DIV_PFD_0_DIV1(g_cgcPll4Config.pfd0Div1 - 1) :
                                   CGC_LPAV_PLL4DIV_PFD_0_DIV1HALT_MASK); /* pll4pfd0div1 output frequency is (528 MHz *
                                                                             18) / 32 / 11 = 297 / 11 = 27 MHz */
    CLOCK_SetIpSrcDiv(kCLOCK_Dcnano, DEMO_DCNANO_CLK_SRC, DEMO_DCNANO_CLK_DIV, 0U);

    /* Enable DCNano/LCDIF */
    CLOCK_EnableClock(kCLOCK_Dcnano);
    /* Release DCNano/LCDIF from reset */
    RESET_PeripheralReset(kRESET_Dcnano);

    mipiDsiDpiClkFreq_Hz = CLOCK_GetDcnanoClkFreq();

    PRINTF("LCDIF pixel clock is: %dHz\r\n", mipiDsiDpiClkFreq_Hz);

    SIM_SEC->SYSCTRL0 &= ~SIM_SEC_SYSCTRL0_LPAV_MASTER_CTRL(1);                   /* Allocate LPAV to RTD */
    SIM_SEC->LPAV_MASTER_ALLOC_CTRL &= ~SIM_SEC_LPAV_MASTER_ALLOC_CTRL_DCNANO(1); /* Allocate LCDIF(DCNANO) to RTD */
}

#if !APP_DISPLAY_EXTERNAL_CONVERTOR
static void BOARD_DetectLcdDriveIc(void)
{
    /*
     * Currently cannot read data from the new panel(DEMO_PANEL_RK055MHD091, drive ic is hx8394)
     * So drop it. we'll revert the patch to support dynamic detect drive ic after the issue is resolved by panel
     * vendor.
     */
#if 0
    uint8_t rxData[5]  = {0};
    int32_t rxDataSize = 5;

    /* Power on. */
    PANEL_PullPowerPin(true);
    VIDEO_DelayMs(1);

    /* Perform reset. */
    PANEL_PullResetPin(false);
    VIDEO_DelayMs(1);
    PANEL_PullResetPin(true);
    VIDEO_DelayMs(100);

    /* check supplier ID code, Module ID */
    MIPI_DSI_ReadCMD(&dsiDevice, kMIPI_DCS_ReadDDBStart, rxData, &rxDataSize);
    if (!memcmp(rxData, RM68200_DDB_START, sizeof(RM68200_DDB_START)))
    {
        whichDrvIcModel = RM68200;
    }
    else
    {
        whichDrvIcModel = HX8394;
    }
#endif
#if (DEMO_PANEL_RK055AHD091 == DEMO_PANEL)
    whichDrvIcModel = RM68200;
#elif (DEMO_PANEL_RK055MHD091 == DEMO_PANEL)
    whichDrvIcModel = HX8394;
#endif
}

static status_t BOARD_InitLcdDriveIc(void)
{
    status_t status;

    displayConfig.hsw = s_displayCommonCfg[whichDrvIcModel].hsw;
    displayConfig.hfp = s_displayCommonCfg[whichDrvIcModel].hfp;
    displayConfig.hbp = s_displayCommonCfg[whichDrvIcModel].hbp;
    displayConfig.vsw = s_displayCommonCfg[whichDrvIcModel].vsw;
    displayConfig.vfp = s_displayCommonCfg[whichDrvIcModel].vfp;
    displayConfig.vbp = s_displayCommonCfg[whichDrvIcModel].vbp;

    if (whichDrvIcModel == RM68200)
    {
        status = RM68200_Init(&rm68200Handle, &displayConfig);
    }
    else
    {
        status = HX8394_Init(&hx8394Handle, &displayConfig);
    }

    if (status == kStatus_Success)
    {
        RGPIO_PinWrite(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, 1);
    }

    return status;
}
#else
status_t BOARD_InitMipiToHdmiBridgeIc(void)
{
    uint8_t gpioIdx               = APP_GPIO_IDX(APP_IT6161_INT_PIN);
    uint8_t pinIdx                = APP_PIN_IDX(APP_IT6161_INT_PIN);
    rgpio_pin_config_t gpioConfig = {
        kRGPIO_DigitalInput,
        0U,
    };

    /* Init the resource. */
    BOARD_Display_I2C_Init();

    displayConfig.hsw = s_displayCommonCfg[whichDrvIcModel].hsw;
    displayConfig.hfp = s_displayCommonCfg[whichDrvIcModel].hfp;
    displayConfig.hbp = s_displayCommonCfg[whichDrvIcModel].hbp;
    displayConfig.vsw = s_displayCommonCfg[whichDrvIcModel].vsw;
    displayConfig.vfp = s_displayCommonCfg[whichDrvIcModel].vfp;
    displayConfig.vbp = s_displayCommonCfg[whichDrvIcModel].vbp;

    IT6161_Init(&it6161Handle, &displayConfig);

    /* Init gpio for INT pin of IT6161(INT pin is connected to PTA19 of iMX8ULP) */
    RGPIO_PinInit(gpios[gpioIdx], pinIdx, &gpioConfig); /* Config gpio as input */
    RGPIO_SetPinInterruptConfig(gpios[gpioIdx], pinIdx, APP_IT6161_INT_PIN_INT_SEL, APP_IT6161_INT_PIN_INT_CFG);
    NVIC_ClearPendingIRQ(APP_IT6161_INT_PIN_IRQ_NUM);
    /* Enable interrupt for GPIO. */
    NVIC_SetPriority(APP_IT6161_INT_PIN_IRQ_NUM, APP_IT6161_INT_PIN_IRQ_PRIO);
    EnableIRQ(APP_IT6161_INT_PIN_IRQ_NUM);

    return kStatus_Success;
}
#endif

status_t BOARD_InitLcdPanel(void)
{
    status_t status;

#if !APP_DISPLAY_EXTERNAL_CONVERTOR
    status = BOARD_InitLcdDriveIc();
#else
    status = BOARD_InitMipiToHdmiBridgeIc();
#endif

    return status;
}

static void BOARD_InitMipiDsiClock(void)
{
    UPOWER_PowerOnMemPart(kUPOWER_MP0_MIPI_DSI, 0);

    /*
     * RxClkEsc larger than 60MHz, TxClkEsc 12 to 20MHz.
     * In the SOC design, RxClkEsc is fixed to 4 x TxClkEsc.
     *
     * Here use PLL4PFD3DIV2 clock (396 MHz) as clock source.
     * RxClkEsc = 396 MHz / 5 = 79.20MHz
     * TxClkEsc = 396 MHz / 5 / 4 = 19.80 MHz
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

static void BOARD_SetMipiDpiConfig(void)
{
    /* Init DPI interface. */
    dpiConfig.hsw         = s_displayCommonCfg[whichDrvIcModel].hsw;
    dpiConfig.hfp         = s_displayCommonCfg[whichDrvIcModel].hfp;
    dpiConfig.hbp         = s_displayCommonCfg[whichDrvIcModel].hbp;
    dpiConfig.vfp         = s_displayCommonCfg[whichDrvIcModel].vfp;
    dpiConfig.vbp         = s_displayCommonCfg[whichDrvIcModel].vbp;
    dpiConfig.panelHeight = s_displayCommonCfg[whichDrvIcModel].height;
    DSI_SetDpiConfig(DEMO_MIPI_DSI, &dpiConfig, DEMO_MIPI_DSI_LANE_NUM, mipiDsiDpiClkFreq_Hz, mipiDsiDphyBitClkFreq_Hz);
}

static void BOARD_SetMipiDsiConfig(void)
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

    /* Init the DSI module. */
    DSI_Init(DEMO_MIPI_DSI, &dsiConfig);

    /* Init DPHY.
     *
     * Note that the DSI output pixel is 24bit per pixel.
     */
    mipiDsiDphyBitClkFreq_Hz = mipiDsiDpiClkFreq_Hz * (24 / DEMO_MIPI_DSI_LANE_NUM);
    DSI_GetDphyDefaultConfig(&dphyConfig, mipiDsiDphyBitClkFreq_Hz, mipiDsiTxEscClkFreq_Hz);

    /* Init DPHY. */
    mipiDsiDphyBitClkFreq_Hz = DSI_InitDphy(DEMO_MIPI_DSI, &dphyConfig, mipiDsiDphyRefClkFreq_Hz);

    PRINTF("MIPI DSI DPHY bit clock: %dHz\r\n", mipiDsiDphyBitClkFreq_Hz);
}

/*
 * Route MIPI signal of CPU to LCD module or HDMI
 * MipiToLcdOrHdmi:
 *     true, route MIPI signal of CPU to LCD module
 *     false, route MIPI signal of CPU to HDMI
 */
void BOARD_RouteMipiToLcdOrHdmi(bool MipiToLcdOrHdmi)
{
    status_t (*func)(pca6416a_handle_t *, uint16_t) = MipiToLcdOrHdmi ? (PCA6416A_SetPins) : (PCA6416A_ClearPins);

    if (kStatus_Success != func(&g_pca6416aHandle, 1 << BOARD_PCA6416A_MIPI_SWITCH))
    {
        PRINTF("ERROR: MIPI_SWITCH pin configure failed\r\n");
    }

    if (kStatus_Success !=
        PCA6416A_SetDirection(&g_pca6416aHandle, (1 << BOARD_PCA6416A_MIPI_SWITCH), kPCA6416A_Output))
    {
        PRINTF("ERROR: MIPI_SWITCH pin configure failed\r\n");
    }
}

#if !APP_DISPLAY_EXTERNAL_CONVERTOR
static void BOARD_InitMipiPins(void)
{
    const rgpio_pin_config_t pinConfig = {
        .pinDirection = kRGPIO_DigitalOutput,
        .outputLogic  = 0,
    };

    RGPIO_PinInit(BOARD_MIPI_RST_GPIO, BOARD_MIPI_RST_PIN, &pinConfig);
    RGPIO_PinInit(BOARD_MIPI_BL_GPIO, BOARD_MIPI_BL_PIN, &pinConfig);
}
#endif

status_t BOARD_InitDisplayInterface(void)
{
    SIM_SEC->LPAV_MASTER_ALLOC_CTRL &=
        ~SIM_SEC_LPAV_MASTER_ALLOC_CTRL_MIPI_DSI(1); /* Allocate MIPI_DSI to Real Time Domain(RTD) */

    /* 1. Switch DPI MUX to DCNano/LCDIF. */
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_DPI2_EPDC_DCNANO_MUX_SEL_MASK;

    /* 2. Assert all resets. */
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_RST_ESC_N_MASK;
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_RST_BYTE_N_MASK;
    SIM_LPAV->SYSCTRL0 &= ~SIM_LPAV_SYSCTRL0_DSI_RST_DPI_N_MASK;

    RESET_PeripheralReset(kRESET_Dsi);

    /* Always use normal mode(full mode) for Type-4 display */
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_CM(1);

    /* 3. Setup MIPI DSI controller clock. */
    BOARD_InitMipiDsiClock();

    /* 4. Route MIPI signal to LCD. */
#if APP_DISPLAY_EXTERNAL_CONVERTOR
    BOARD_RouteMipiToLcdOrHdmi(false);
    whichDrvIcModel = IT6161;
#else
    BOARD_RouteMipiToLcdOrHdmi(true);
#endif

    /* 5. Deassert resets. */
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_RST_DPI_N_MASK;
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_RST_BYTE_N_MASK;
    SIM_LPAV->SYSCTRL0 |= SIM_LPAV_SYSCTRL0_DSI_RST_ESC_N_MASK;

    /* 6. Configures peripheral. */
    BOARD_SetMipiDsiConfig();

#if !APP_DISPLAY_EXTERNAL_CONVERTOR
    /* 7. Check lcd drive ic */
    BOARD_InitMipiPins();
    BOARD_DetectLcdDriveIc();
#endif

    ((dc_fb_lcdif_config_t *)g_dc.config)->width  = s_displayCommonCfg[whichDrvIcModel].width;
    ((dc_fb_lcdif_config_t *)g_dc.config)->height = s_displayCommonCfg[whichDrvIcModel].height;
    ((dc_fb_lcdif_config_t *)g_dc.config)->hsw    = s_displayCommonCfg[whichDrvIcModel].hsw;
    ((dc_fb_lcdif_config_t *)g_dc.config)->hfp    = s_displayCommonCfg[whichDrvIcModel].hfp;
    ((dc_fb_lcdif_config_t *)g_dc.config)->hbp    = s_displayCommonCfg[whichDrvIcModel].hbp;
    ((dc_fb_lcdif_config_t *)g_dc.config)->vsw    = s_displayCommonCfg[whichDrvIcModel].vsw;
    ((dc_fb_lcdif_config_t *)g_dc.config)->vfp    = s_displayCommonCfg[whichDrvIcModel].vfp;
    ((dc_fb_lcdif_config_t *)g_dc.config)->vbp    = s_displayCommonCfg[whichDrvIcModel].vbp;
    /* 8. Configure DSI DPI. */
    BOARD_SetMipiDpiConfig();

    /* 9. Configure the Lcd panel. */
    return BOARD_InitLcdPanel();
}
void DCNano_IRQHandler(void)
{
    DC_FB_LCDIF_IRQHandler(&g_dc);
}

status_t BOARD_PrepareDisplayController(void)
{
    status_t status;

    BOARD_InitLcdifClock();

    status = BOARD_InitDisplayInterface();
    if (kStatus_Success == status)
    {
        NVIC_ClearPendingIRQ(DCNano_IRQn);
        NVIC_SetPriority(DCNano_IRQn, 3);
        EnableIRQ(DCNano_IRQn);
    }

    return 0;
}
