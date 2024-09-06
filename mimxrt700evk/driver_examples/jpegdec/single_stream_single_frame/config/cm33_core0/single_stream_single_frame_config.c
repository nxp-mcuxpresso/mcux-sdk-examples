/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_cache.h"
#include "fsl_jpegdec.h"
#include "jpeg.h"
#include "display_support.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
JPEG_DECODER_Type g_jpegdec = {
    .core    = JPEGDEC,
    .wrapper = JPGDECWRP,
};

#define APP_JPEGDEC            (&g_jpegdec)
#define APP_JPEGDEC_IRQHandler JPEGDEC_IRQHandler
#define APP_JPEGDEC_IRQn       JPEGDEC_IRQn

#define DEMO_BUFFER2_ADDR 0x60400000U
#define DEMO_BUFFER3_ADDR 0x60600000U

#define DEMO_FB_ADDR  0x60400000U
#define DEMO_FB0_ADDR 0x60800000U
#define DEMO_FB1_ADDR 0x60A00000U
#define APP_FB_BPP    2
#define APP_FB_FORMAT kVIDEO_PixelFormatRGB565

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_BufferSwitchOffCallback(void *param, void *switchOffBuffer);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dc_fb_info_t fbInfo;
static volatile bool s_newFrameShown = false;
static uint32_t s_decodedAddr[2]     = {DEMO_BUFFER0_ADDR, DEMO_BUFFER1_ADDR};
static uint32_t s_frameBufferAddr    = DEMO_FB_ADDR;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if ((DEMO_PANEL_RM67162 == DEMO_PANEL) && (!RM67162_USE_LCDIF))
void BOARD_MIPI_TE_GPIO_IRQ_Handler(void)
{
    uint32_t intStat;

    intStat = GPIO_PinGetInterruptFlag(BOARD_MIPI_TE_GPIO, BOARD_MIPI_TE_PIN);

    GPIO_GpioClearInterruptFlags(BOARD_MIPI_TE_GPIO, 1U << BOARD_MIPI_TE_PIN);

    if (intStat & (1U << BOARD_MIPI_TE_PIN))
    {
        BOARD_DisplayTEPinHandler();
    }
}
#endif

static uint8_t BYTECLIP(int val)
{
    if (val < 0)
    {
        return 0U;
    }
    else if (val > 255)
    {
        return 255U;
    }
    else
    {
        return (uint8_t)val;
    }
}

void Convert_yuv420_to_rgb565(uint16_t width, uint16_t height, uint32_t yAddr, uint32_t uvAddr, uint32_t rgbAddr)
{
    uint16_t *rgb = (void *)rgbAddr;
    int16_t r, g, b;
    uint8_t R, G, B, y, u, v;

    uint16_t i, j;

    for (i = 0U; i < height; i++)
    {
        for (j = 0U; j < width; j++)
        {
            y = *(((uint8_t *)yAddr) + width * i + j);
            u = *(((uint8_t *)uvAddr) + width * (i / 2) + j - (j % 2));
            v = *(((uint8_t *)uvAddr) + width * (i / 2) + j + 1 - (j % 2));
            r = y + 1402 * (v - 128) / 1000;
            g = y - (344 * (u - 128) + 714 * (v - 128)) / 1000;
            b = y + 1772 * (u - 128) / 1000;
            R = BYTECLIP(r);
            G = BYTECLIP(g);
            B = BYTECLIP(b);
            *rgb++ =
                (uint16_t)((((uint16_t)R & 0xF8) << 8U) | (((uint16_t)G & 0xFC) << 3U) | (((uint16_t)B & 0xF8) >> 3U));
        }
    }
}

void DEMO_Decode_JPEG(void)
{
    jpegdec_config_t config;
    jpegdec_decoder_config_t decConfig;
    uint32_t status;

    /* Step 1: Init JPEG decoder module. */
    JPEGDEC_GetDefaultConfig(&config);

    config.slots = kJPEGDEC_Slot0; /* Enable only one slot. */

    JPEGDEC_Init(APP_JPEGDEC, &config);

    /* Step 2: Set source buffer, buffer size. */
    JPEGDEC_SetJpegBuffer(&decConfig, (uint8_t *)jpegImg, jpegImgLen);

    /* Step 3: Set buffer of generated image for JPEG decoder. */
    JPEGDEC_SetOutputBuffer(&decConfig, (uint8_t *)s_decodedAddr[0], (uint8_t *)s_decodedAddr[1]);

    /* Step 4: Parse header. */
    JPEGDEC_ParseHeader(&decConfig);

    /* Step 5: Set decoder option. */
    JPEGDEC_SetDecodeOption(&decConfig, decConfig.width, false, false);

    /* Step 6: Set decoder configuration. */
    JPEGDEC_ConfigDecoder(APP_JPEGDEC, &decConfig);

    /* Step 7: Start the decoder. */
    JPEGDEC_StartDecode(APP_JPEGDEC);

    /* Step 8: Wait for decoding complete. */
    status = JPEGDEC_GetStatusFlags(APP_JPEGDEC, 0);

    while ((status & (kJPEGDEC_DecodeCompleteFlag | kJPEGDEC_ErrorFlags)) == 0U)
    {
        status = JPEGDEC_GetStatusFlags(APP_JPEGDEC, 0);
    }

    if ((status & kJPEGDEC_DecodeCompleteFlag) == 0U)
    {
        JPEGDEC_ClearStatusFlags(APP_JPEGDEC, 0, status);
        PRINTF("Error occured during JPEG decoding\r\n");
        assert(false);
    }

    /* Step 9: Convert the YUV420 format pixel to RGB565 to display. */
    Convert_yuv420_to_rgb565(decConfig.width, decConfig.height, s_decodedAddr[0], s_decodedAddr[1], s_frameBufferAddr);

    /* Step 10: Configure dispaly layer configuration. */
    fbInfo.pixelFormat = APP_FB_FORMAT;
    fbInfo.width       = decConfig.width;
    fbInfo.height      = decConfig.height;
    fbInfo.startX      = (DEMO_PANEL_WIDTH - decConfig.width) / 2U;
    fbInfo.startY      = (DEMO_PANEL_HEIGHT - decConfig.height) / 2U;
    fbInfo.strideBytes = decConfig.width * APP_FB_BPP;
    if (kStatus_Success != g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo))
    {
        PRINTF("Error: Could not configure the display controller\r\n");
        assert(false);
    }

    g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_frameBufferAddr);
    
    if ((g_dc.ops->getProperty(&g_dc) & kDC_FB_ReserveFrameBuffer) == 0)
    {
        while (s_newFrameShown == false)
        {
        }
    }

    s_newFrameShown = true;
    
    /* Step 11: Enable layer and display the decoded image. */
    g_dc.ops->enableLayer(&g_dc, 0);
}

void DEMO_InitDisplay(void)
{
    status_t status;

    BOARD_PrepareDisplayController();

    memset((void *)s_frameBufferAddr, 0, DEMO_PANEL_HEIGHT * DEMO_PANEL_WIDTH * APP_FB_BPP);

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(false);
    }

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);
    g_dc.ops->setCallback(&g_dc, 0, DEMO_BufferSwitchOffCallback, NULL);
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

#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
    BOARD_InitFlexIOPanelPins();
#else /* SSD1963_DRIVEN_BY_LCDIF */
    BOARD_InitLcdDBIPanelPins();
#endif

    CLOCK_EnableClock(kCLOCK_Gpio2);
    RESET_PeripheralReset(kGPIO2_RST_SHIFT_RSTn);
#else
#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
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
    BOARD_Init16bitsPsRam(XSPI2);

    /* Disable JPEG decoder power down. */
    POWER_DisablePD(kPDRUNCFG_PPD_JPEGDEC);
    POWER_DisablePD(kPDRUNCFG_APD_JPEGDEC);
    POWER_ApplyPD();

    PRINTF("JPEG decoder demo start:\r\n");
    PRINTF("One frame of JPEG picture will be decoded by slot 0\r\n");
    PRINTF("First we configure the decoder then start the decode process\r\n");

    DEMO_InitDisplay();

    PRINTF("Decoding the image...\r\n");
    DEMO_Decode_JPEG();
    PRINTF("done!\r\n");

    while (1)
    {
    }
}

static void DEMO_BufferSwitchOffCallback(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
}
