/*
 * Copyright (c) 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lcdif.h"
#include "lcdif_support.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_BYTE_PER_PIXEL         4U
#define DEMO_IMG_HEIGHT             DEMO_PANEL_HEIGHT / 2U
#define DEMO_IMG_WIDTH              DEMO_PANEL_WIDTH / 2U
#define DEMO_MAKE_COLOR(a, r, g, b) (((a) << 24U) | ((r) << 16U) | ((g) << 8U) | ((b) << 0U))
#define DEMO_COLOR_RED              DEMO_MAKE_COLOR(0xFF, 0xFF, 0, 0)
#define DEMO_COLOR_BLUE             DEMO_MAKE_COLOR(0xFF, 0, 0, 0xFF)

#define DEMO_IMG_BYTES_PER_LINE LCDIF_ALIGN_ADDR((DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL), LCDIF_FB_ALIGN)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_srcFrameBufferAddr                           = DEMO_FB0_ADDR;
static uint32_t s_dstFrameBufferAddr                           = DEMO_FB1_ADDR;
static uint32_t s_bufferWidth                                  = DEMO_IMG_WIDTH / 4U * 3U;
static uint32_t s_bufferHeight                                 = DEMO_IMG_HEIGHT / 4U * 3U;
AT_NONCACHEABLE_SECTION_INIT(static volatile bool s_frameDone) = false;
AT_NONCACHEABLE_SECTION_INIT(static uint32_t loops)            = 0U;
static lcdif_porter_duff_blend_mode_t porterDuffModeArray[]    = {
    kLCDIF_PorterDuffSrc,   kLCDIF_PorterDuffAtop,   kLCDIF_PorterDuffOver,    kLCDIF_PorterDuffIn,
    kLCDIF_PorterDuffOut,   kLCDIF_PorterDuffDst,    kLCDIF_PorterDuffDstAtop, kLCDIF_PorterDuffDstOver,
    kLCDIF_PorterDuffDstIn, kLCDIF_PorterDuffDstOut, kLCDIF_PorterDuffPlus,    kLCDIF_PorterDuffXor,
    kLCDIF_PorterDuffClear};
static char *porterDuffNameArray[] = {"PorterDuffSrc",   "PorterDuffAtop",   "PorterDuffOver",    "PorterDuffIn",
                                      "PorterDuffOut",   "PorterDuffDst",    "PorterDuffDstAtop", "PorterDuffDstOver",
                                      "PorterDuffDstIn", "PorterDuffDstOut", "PorterDuffPlus",    "PorterDuffXor",
                                      "PorterDuffClear"};

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = LCDIF_GetAndClearInterruptPendingFlags(DEMO_LCDIF);

    if (0 != (intStatus & kLCDIF_Display0FrameDoneInterrupt))
    {
        s_frameDone = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_FillFrameBuffer(void)
{
    uint32_t i, j;
    uint32_t(*srcFb)[DEMO_IMG_WIDTH] = (void *)s_srcFrameBufferAddr;
    uint32_t(*dstFb)[DEMO_IMG_WIDTH] = (void *)s_dstFrameBufferAddr;

    memset(srcFb, 0, DEMO_IMG_WIDTH * DEMO_IMG_HEIGHT * 4);
    memset(dstFb, 0, DEMO_IMG_WIDTH * DEMO_IMG_HEIGHT * 4);

    /* Draw a solid red rectangle at the top left of the screen, then the rest is transparent. */
    for (i = 0; i < DEMO_IMG_HEIGHT; i++)
    {
        for (j = 0; j < DEMO_IMG_WIDTH; j++)
        {
            if ((i <= s_bufferHeight) && (j <= s_bufferWidth))
            {
                ((uint32_t *)(((uint8_t *)srcFb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = DEMO_COLOR_RED;
            }
            else
            {
                ((uint32_t *)(((uint8_t *)srcFb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = 0U;
            }
        }
    }

    /* Draw a solid blue rectangle at the bottom right of the screen, then the rest is transparent. */
    for (i = 0; i < DEMO_IMG_HEIGHT; i++)
    {
        for (j = 0; j < DEMO_IMG_WIDTH; j++)
        {
            if ((i >= (s_bufferHeight / 3U)) && (j >= (s_bufferWidth / 3U)))
            {
                ((uint32_t *)(((uint8_t *)dstFb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = DEMO_COLOR_BLUE;
            }
            else
            {
                ((uint32_t *)(((uint8_t *)dstFb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = 0U;
            }
        }
    }
}

void DEMO_LCDIF_Init(void)
{
    const lcdif_dpi_config_t dpiConfig = {
        .panelWidth    = DEMO_PANEL_WIDTH,
        .panelHeight   = DEMO_PANEL_HEIGHT,
        .hsw           = DEMO_HSW,
        .hfp           = DEMO_HFP,
        .hbp           = DEMO_HBP,
        .vsw           = DEMO_VSW,
        .vfp           = DEMO_VFP,
        .vbp           = DEMO_VBP,
        .polarityFlags = DEMO_POL_FLAGS,
        .format        = kLCDIF_Output24Bit,
    };
    lcdif_panel_config_t config;

    LCDIF_Init(DEMO_LCDIF);

    LCDIF_DpiModeSetConfig(DEMO_LCDIF, 0, &dpiConfig);

    LCDIF_PanelGetDefaultConfig(&config);

    LCDIF_SetPanelConfig(DEMO_LCDIF, 0, &config);

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    NVIC_EnableIRQ(DEMO_LCDIF_IRQn);

    LCDIF_EnableInterrupts(DEMO_LCDIF, kLCDIF_Display0FrameDoneInterrupt);
}

void DEMO_LCDIF_Porterduff(void)
{
    AT_NONCACHEABLE_SECTION_INIT(static lcdif_fb_config_t dstFbConfig) = {0};
    AT_NONCACHEABLE_SECTION_INIT(static lcdif_fb_config_t srcFbConfig) = {0};

    (void)memset(&dstFbConfig, 0, sizeof(dstFbConfig));
    (void)memset(&srcFbConfig, 0, sizeof(srcFbConfig));

    DEMO_FillFrameBuffer();

    /* Set the video layer config, it performs as the destination layer. */
    LCDIF_SetFrameBufferStride(DEMO_LCDIF, 0, DEMO_IMG_BYTES_PER_LINE);
    LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_dstFrameBufferAddr);

    dstFbConfig.enable          = true;
    dstFbConfig.inOrder         = kLCDIF_PixelInputOrderARGB;
    dstFbConfig.format          = kLCDIF_PixelFormatARGB8888;
    dstFbConfig.alpha.enable    = false;
    dstFbConfig.colorkey.enable = false;
    dstFbConfig.topLeftX        = 0U;
    dstFbConfig.topLeftY        = 0U;
    dstFbConfig.width           = DEMO_IMG_WIDTH;
    dstFbConfig.height          = DEMO_IMG_HEIGHT;
    dstFbConfig.rotateFlipMode  = kLCDIF_Rotate0;

    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &dstFbConfig);

    /* Set the overlay layer 0 config, it performs as the source layer. */
    LCDIF_SetOverlayLayerStride(DEMO_LCDIF, 0, DEMO_IMG_BYTES_PER_LINE, 0);
    LCDIF_SetOverlayLayerAddr(DEMO_LCDIF, 0, (uint32_t)s_srcFrameBufferAddr, 0);

    srcFbConfig.enable          = true;
    srcFbConfig.inOrder         = kLCDIF_PixelInputOrderARGB;
    srcFbConfig.format          = kLCDIF_PixelFormatARGB8888;
    srcFbConfig.alpha.enable    = true;
    srcFbConfig.colorkey.enable = false;
    srcFbConfig.topLeftX        = 0U;
    srcFbConfig.topLeftY        = 0U;
    srcFbConfig.width           = DEMO_IMG_WIDTH;
    srcFbConfig.height          = DEMO_IMG_HEIGHT;
    srcFbConfig.rotateFlipMode  = kLCDIF_Rotate0;

    LCDIF_GetPorterDuffConfig(porterDuffModeArray[loops % ARRAY_SIZE(porterDuffModeArray)], &(srcFbConfig.alpha));

    LCDIF_SetOverlayLayerConfig(DEMO_LCDIF, 0, &srcFbConfig, 0U);

    PRINTF("Currently show %s mode\r\n\r\n", porterDuffNameArray[loops % ARRAY_SIZE(porterDuffNameArray)]);

    LCDIF_Start(DEMO_LCDIF);

    while (1)
    {
        /* Wait for previous frame complete. */
        while (!s_frameDone)
        {
        }
        loops++;

        s_frameDone = false;

        PRINTF("Currently show %s mode\r\n\r\n", porterDuffNameArray[loops % ARRAY_SIZE(porterDuffNameArray)]);

        LCDIF_GetPorterDuffConfig(porterDuffModeArray[loops % ARRAY_SIZE(porterDuffModeArray)], &(srcFbConfig.alpha));

        LCDIF_SetOverlayLayerConfig(DEMO_LCDIF, 0, &srcFbConfig, 0U);

        LCDIF_SetUpdateReady(DEMO_LCDIF);

        /* Let the picture be displayed for 2s then change. */
        SDK_DelayAtLeastUs(1000000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitAHBSC();
    BOARD_InitBootPins();
    BOARD_InitPsRamPins_Xspi2();

#if ((USE_DBI) && (USE_DBI_PANEL == PANEL_TFT_PROTO_5))
    CLOCK_EnableClock(kCLOCK_Gpio2);
    RESET_PeripheralReset(kGPIO2_RST_SHIFT_RSTn);
    
    BOARD_InitLcdDBIPanelPins();
#else
#if (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
    BOARD_InitI2cPins();
#endif
    BOARD_InitMipiPanelPinsEvk();

    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Gpio3);
    RESET_PeripheralReset(kGPIO1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kGPIO3_RST_SHIFT_RSTn);
#endif

    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitLcdifClock();
    BOARD_Init16bitsPsRam(XSPI2);
    /* Change the PFD2 divide to 17. Then the DBI source frequency shall be 528 * 18 / 17 / 2 = 279.53MHz. */
    CLOCK_InitMainPfd(kCLOCK_Pfd2, 17);
    CLOCK_SetClkDiv(kCLOCK_DivMediaMainClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD2_to_MEDIA_MAIN);

    PRINTF("LCDIF porter duff example start...\r\n");

    DEMO_LCDIF_Init();

    DEMO_LCDIF_Porterduff();

    while (1)
    {
    }
}
