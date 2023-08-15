/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_dcic.h"
#include "display_support.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_DCIC DCIC1

#ifndef USE_RGB565
#define USE_RGB565 0
#endif

#if USE_RGB565

/* Use 16-bit RGB565 format. */
typedef union
{
    struct
    {
        uint16_t B : 5;
        uint16_t G : 6;
        uint16_t R : 5;
    } c;
    uint16_t u;
} pixel_t;

#define APP_RED       0xF800U
#define APP_GREEN     0x07E0U
#define APP_BLUE      0x001FU
#define APP_WHITE     0xFFFFU
#define APP_DC_FORMAT kVIDEO_PixelFormatRGB565

#else

/* Use 32-bit XRGB888 format. */
typedef union
{
    struct
    {
        uint32_t B : 8;
        uint32_t G : 8;
        uint32_t R : 8;
        uint32_t X : 8;
    } c;
    uint32_t u;
} pixel_t;

#define APP_RED       0x00FF0000U
#define APP_GREEN     0x0000FF00U
#define APP_BLUE      0x000000FFU
#define APP_WHITE     0x00FFFFFFU
#define APP_DC_FORMAT kVIDEO_PixelFormatXRGB8888

#endif

#define APP_DCIC_ROI_ULX 0
#define APP_DCIC_ROI_ULY 0
#define APP_DCIC_ROI_LRX ((DEMO_BUFFER_WIDTH / 2) - 1)
#define APP_DCIC_ROI_LRY ((DEMO_BUFFER_HEIGHT / 2) - 1)

#define APP_DCIC_REGION 0 /* Use ROI 0. */

#define APP_CRC32_TABLE_SIZE 256

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_FillRightBuffer(pixel_t *buffer);
static void APP_FillWrongBuffer(pixel_t *buffer);
static uint32_t APP_GetRefCRC(
    pixel_t *buffer, uint16_t upperLeftX, uint16_t upperLeftY, uint16_t lowerRightX, uint16_t lowerRightY);
static void APP_InitDisplay(void *frameBuffer);
static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer);
static void APP_Dcic(void);
static void APP_MakeCRC32Table(uint32_t table[], uint32_t polynomial);
static uint32_t APP_UpdateCRC32(uint32_t crcTable[], uint32_t crc, const uint8_t *data, uint32_t dataLen);
static uint32_t APP_PixelTo24Bit(pixel_t p);

/*******************************************************************************
 * Variables
 ******************************************************************************/

AT_NONCACHEABLE_SECTION_ALIGN(static pixel_t s_rightLcdBuffer[DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH],
                              FRAME_BUFFER_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static pixel_t s_wrongLcdBuffer[DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH],
                              FRAME_BUFFER_ALIGN);

/*
 * When new frame buffer sent to display, it might not be shown immediately.
 * Application could use callback to get new frame shown notification, at the
 * same time, when this flag is set, application could write to the older
 * frame buffer.
 */
static volatile bool s_newFrameShown = false;
static dc_fb_info_t fbInfo;
static uint32_t s_crc32Table[APP_CRC32_TABLE_SIZE];
static uint32_t refCrc;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_ResetDisplayMix(void)
{
    /*
     * Reset the displaymix, otherwise during debugging, the
     * debugger may not reset the display, then the behavior
     * is not right.
     */
    SRC_AssertSliceSoftwareReset(SRC, kSRC_DisplaySlice);
    while (kSRC_SliceResetInProcess == SRC_GetSliceResetState(SRC, kSRC_DisplaySlice))
    {
    }
}

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootClocks();
    BOARD_ResetDisplayMix();
    BOARD_InitBootPins();
    BOARD_InitDebugConsole();

    PRINTF("\r\nDCIC example start...\r\n");

    APP_MakeCRC32Table(s_crc32Table, DCIC_CRC32_POLYNOMIAL);
    APP_FillRightBuffer((pixel_t *)s_rightLcdBuffer);
    APP_FillWrongBuffer((pixel_t *)s_wrongLcdBuffer);

    refCrc = APP_GetRefCRC((pixel_t *)s_rightLcdBuffer, APP_DCIC_ROI_ULX, APP_DCIC_ROI_ULY, APP_DCIC_ROI_LRX,
                           APP_DCIC_ROI_LRY);

    /* Init the display and show right buffer at first. */
    APP_InitDisplay(s_rightLcdBuffer);

    APP_Dcic();

    while (1)
    {
    }
}

static void APP_InitDisplay(void *frameBuffer)
{
    status_t status;

    BOARD_PrepareDisplayController();

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(0);
    }

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);
    fbInfo.pixelFormat = APP_DC_FORMAT;
    fbInfo.width       = DEMO_BUFFER_WIDTH;
    fbInfo.height      = DEMO_BUFFER_HEIGHT;
    fbInfo.startX      = DEMO_BUFFER_START_X;
    fbInfo.startY      = DEMO_BUFFER_START_Y;
    fbInfo.strideBytes = DEMO_BUFFER_WIDTH * sizeof(pixel_t);
    g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo);

    g_dc.ops->setCallback(&g_dc, 0, APP_BufferSwitchOffCallback, NULL);

    s_newFrameShown = false;
    g_dc.ops->setFrameBuffer(&g_dc, 0, frameBuffer);

    /* For the DBI interface display, application must wait for the first
     * frame buffer sent to the panel.
     */
    if ((g_dc.ops->getProperty(&g_dc) & kDC_FB_ReserveFrameBuffer) == 0)
    {
        while (s_newFrameShown == false)
        {
        }
    }

    s_newFrameShown = true;

    g_dc.ops->enableLayer(&g_dc, 0);
}

static void APP_FillRightBuffer(pixel_t *buffer)
{
    uint32_t x, y;

    memset(buffer, 0, DEMO_BUFFER_HEIGHT * DEMO_BUFFER_WIDTH * sizeof(pixel_t));

    /* The ROI is set to blue. */
    for (y = APP_DCIC_ROI_ULY; y <= APP_DCIC_ROI_LRY; y++)
    {
        for (x = APP_DCIC_ROI_ULX; x <= APP_DCIC_ROI_LRX; x++)
        {
            buffer[y * DEMO_BUFFER_WIDTH + x].u = APP_BLUE;
        }
    }
}

static void APP_FillWrongBuffer(pixel_t *buffer)
{
    uint32_t x, y;

    APP_FillRightBuffer(buffer);

    /* Change part of the ROI. */
    y = (APP_DCIC_ROI_ULY + APP_DCIC_ROI_LRY) / 2;

    for (x = APP_DCIC_ROI_ULX; x <= APP_DCIC_ROI_LRX; x++)
    {
        buffer[y * DEMO_BUFFER_WIDTH + x].u = APP_RED;
    }

    x = (APP_DCIC_ROI_ULX + APP_DCIC_ROI_LRX) / 2;

    for (y = APP_DCIC_ROI_ULY; y <= APP_DCIC_ROI_LRY; y++)
    {
        buffer[y * DEMO_BUFFER_WIDTH + x].u = APP_RED;
    }
}

static uint32_t APP_GetRefCRC(
    pixel_t *buffer, uint16_t upperLeftX, uint16_t upperLeftY, uint16_t lowerRightX, uint16_t lowerRightY)
{
    uint16_t x, y;
    uint32_t crc = DCIC_CRC32_INIT_VALUE;

    uint32_t pixel24Bit;

    for (y = upperLeftY; y <= lowerRightY; y++)
    {
        for (x = upperLeftX; x <= lowerRightX; x++)
        {
            pixel24Bit = APP_PixelTo24Bit(buffer[y * DEMO_BUFFER_WIDTH + x]);

            crc = APP_UpdateCRC32(s_crc32Table, crc, (uint8_t *)&pixel24Bit, 3);
        }
    }

    return crc;
}

static void APP_Dcic(void)
{
    bool showRightFrame = true;

    dcic_config_t dcicConfig;
    dcic_region_config_t dcicRegionConfig = {0};

    /*
     *  config->polarityFlags = kDCIC_VsyncActiveLow | kDCIC_HsyncActiveLow |
     *                          kDCIC_DataEnableActiveLow | kDCIC_DriveDataOnFallingClkEdge;
     *  config->enableExternalSignal = false;
     *  config->enableInterrupts = 0;
     */
    DCIC_GetDefaultConfig(&dcicConfig);

    dcicConfig.enableExternalSignal = true;
    dcicConfig.polarityFlags =
        kDCIC_VsyncActiveLow | kDCIC_HsyncActiveLow | kDCIC_DataEnableActiveHigh | kDCIC_DriveDataOnFallingClkEdge;

    DCIC_Init(APP_DCIC, &dcicConfig);

    /* Configure the region. */
    dcicRegionConfig.lock        = false;
    dcicRegionConfig.upperLeftX  = APP_DCIC_ROI_ULX;
    dcicRegionConfig.upperLeftY  = APP_DCIC_ROI_ULY;
    dcicRegionConfig.lowerRightX = APP_DCIC_ROI_LRX;
    dcicRegionConfig.lowerRightY = APP_DCIC_ROI_LRY;
    dcicRegionConfig.refCrc      = refCrc;

    DCIC_EnableRegion(APP_DCIC, APP_DCIC_REGION, &dcicRegionConfig);

    DCIC_Enable(APP_DCIC, true);

    showRightFrame = false;

    while (1)
    {
        PRINTF("Press any key to show new image\r\n\r\n");
        GETCHAR();

        s_newFrameShown = false;

        if (showRightFrame)
        {
            PRINTF("Show right image\r\n");
            showRightFrame = false;
            g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_rightLcdBuffer);

            while (!s_newFrameShown)
            {
            }

            /* Wait for new frame CRC calculation finished. */
            DCIC_ClearStatusFlags(APP_DCIC, kDCIC_FunctionalInterruptStatus);
            while (0U == (kDCIC_FunctionalInterruptStatus & DCIC_GetStatusFlags(APP_DCIC)))
            {
            }

            /* Clear the pervious error flags. */
            DCIC_ClearStatusFlags(APP_DCIC, kDCIC_ErrorInterruptStatus | kDCIC_Region0MismatchStatus);
        }
        else
        {
            PRINTF("Show wrong image\r\n");
            showRightFrame = true;
            g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_wrongLcdBuffer);

            while (!s_newFrameShown)
            {
            }
        }
    }
}

uint32_t APP_PixelTo24Bit(pixel_t p)
{
#if USE_RGB565
    uint8_t r, g, b;
    uint32_t pixel24Bit = 0;

    r = (p.c.R << 3) + (p.c.R >> 2);
    g = (p.c.G << 2) + (p.c.G >> 4);
    b = (p.c.B << 3) + (p.c.B >> 2);

    pixel24Bit = (r << 16) | (g << 8) | b;

    return pixel24Bit;
#else
    return p.u;
#endif
}

static void APP_MakeCRC32Table(uint32_t table[], uint32_t polynomial)
{
    uint32_t r;
    uint32_t i;
    uint8_t bit;

    for (i = 0; i < APP_CRC32_TABLE_SIZE; i++)
    {
        r = i << 24;

        for (bit = 0; bit < 8; bit++)
        {
            if (r & 0x80000000UL)
            {
                r = (r << 1) ^ polynomial;
            }
            else
            {
                r = r << 1;
            }
        }

        table[i] = r;
    }
}

static uint32_t APP_UpdateCRC32(uint32_t crcTable[], uint32_t crc, const uint8_t *data, uint32_t dataLen)
{
    uint32_t offset = dataLen;
    uint8_t tableIndex;

    while (offset--)
    {
        tableIndex = (crc >> 24) ^ data[offset];
        crc        = crcTable[tableIndex] ^ (crc << 8);
    }

    return crc;
}

static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
}
