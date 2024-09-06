/*
 * Copyright (c) 2024 NXP
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
#define DEMO_BYTE_PER_PIXEL     2U
#define DEMO_IMG_HEIGHT         DEMO_PANEL_HEIGHT
#define DEMO_IMG_WIDTH          DEMO_PANEL_WIDTH
#define DEMO_IMG_BYTES_PER_LINE LCDIF_ALIGN_ADDR((DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL), LCDIF_FB_ALIGN)

#define DEMO_MAKE_COLOR(red, green, blue) \
    ((((uint16_t)(red)&0xF8U) << 8U) | (((uint16_t)(green)&0xFCU) << 3U) | (((uint16_t)(blue)&0xF8U) >> 3U))

#define DEMO_COLOR_BLUE  DEMO_MAKE_COLOR(0, 0, 255)
#define DEMO_COLOR_RED   DEMO_MAKE_COLOR(255, 0, 0)
#define DEMO_COLOR_GREEN DEMO_MAKE_COLOR(0, 255, 0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_frameBufferAddr                    = DEMO_FB0_ADDR;
static volatile bool s_frameDone                     = false;
static uint32_t loops                                = 0U;
static const lcdif_layer_rotate_flip_t rotateMode[4] = {kLCDIF_Rotate0, kLCDIF_Rotate180, kLCDIF_FlipHorizontal,
                                                        kLCDIF_FlipVertical};

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
    uint16_t(*fb)[DEMO_IMG_WIDTH] = (void *)s_frameBufferAddr;

    /*
     * The video/graphic layer input buffer is:
     *
     *  -----------------------------
     *  |             |             |
     *  |             |             |
     *  |   BLUE      |     RED     |
     *  |             |             |
     *  |             |             |
     *  |-------------+-------------|
     *  |                           |
     *  |                           |
     *  |           GREEN           |
     *  |                           |
     *  |                           |
     *  -----------------------------
     */
    for (i = 0; i < (DEMO_IMG_HEIGHT / 2); i++)
    {
        for (j = 0; j < (DEMO_IMG_WIDTH / 2); j++)
        {
            ((uint16_t *)(((uint8_t *)fb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = DEMO_COLOR_BLUE;
        }

        for (; j < DEMO_IMG_WIDTH; j++)
        {
            ((uint16_t *)(((uint8_t *)fb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = DEMO_COLOR_RED;
        }
    }

    for (; i < DEMO_IMG_HEIGHT; i++)
    {
        for (j = 0; j < DEMO_IMG_WIDTH; j++)
        {
            ((uint16_t *)(((uint8_t *)fb) + DEMO_IMG_BYTES_PER_LINE * i))[j] = DEMO_COLOR_GREEN;
        }
    }
}

void DEMO_LCDIF_Init(void)
{
    const lcdif_dpi_config_t dpiConfig = {
        .panelWidth    = DEMO_IMG_WIDTH,
        .panelHeight   = DEMO_IMG_HEIGHT,
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

void DEMO_LCDIF_RotateFlip(void)
{
    static lcdif_fb_config_t fbConfig;

    (void)memset(&fbConfig, 0, sizeof(fbConfig));

    LCDIF_SetFrameBufferBackground(DEMO_LCDIF, 0U, DEMO_COLOR_RED);

    DEMO_FillFrameBuffer();

    /* Set the video layer config, it performs as the destination layer. */
    LCDIF_SetFrameBufferStride(DEMO_LCDIF, 0, DEMO_IMG_BYTES_PER_LINE);
    LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_frameBufferAddr);

    fbConfig.enable          = true;
    fbConfig.inOrder         = kLCDIF_PixelInputOrderARGB;
    fbConfig.format          = kLCDIF_PixelFormatRGB565;
    fbConfig.alpha.enable    = false;
    fbConfig.colorkey.enable = false;
    fbConfig.topLeftX        = 0U;
    fbConfig.topLeftY        = 0U;
    fbConfig.width           = DEMO_IMG_WIDTH;
    fbConfig.height          = DEMO_IMG_HEIGHT;

    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);

    LCDIF_Start(DEMO_LCDIF);

    while (1)
    {
        s_frameDone             = false;
        fbConfig.rotateFlipMode = rotateMode[loops % ARRAY_SIZE(rotateMode)];

        LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);

        LCDIF_SetUpdateReady(DEMO_LCDIF);

        /* Wait for previous frame complete. */
        while (!s_frameDone)
        {
        }
        loops++;

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

    PRINTF("LCDIF rotate flip example start...\r\n");

    DEMO_LCDIF_Init();

    DEMO_LCDIF_RotateFlip();

    while (1)
    {
    }
}
