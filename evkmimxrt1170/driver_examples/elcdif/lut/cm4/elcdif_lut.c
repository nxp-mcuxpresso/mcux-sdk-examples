/*
 * Copyright 2017-2019, 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_elcdif.h"
#include "fsl_debug_console.h"

#include "fsl_soc_src.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "elcdif_support.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* How many frames to show before swithing to the other picture. */
#define APP_FRAME_PER_PIC 60

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile uint32_t s_curFrame = 0; /* Frame counting. */

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_frameBuffer[APP_IMG_HEIGHT][APP_IMG_WIDTH], FRAME_BUFFER_ALIGN);

uint32_t lutData[2][ELCDIF_LUT_ENTRY_NUM];

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


void APP_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = ELCDIF_GetInterruptStatus(APP_ELCDIF);

    ELCDIF_ClearInterruptStatus(APP_ELCDIF, intStatus);

    if (intStatus & kELCDIF_CurFrameDone)
    {
        s_curFrame++;
    }
    SDK_ISR_EXIT_BARRIER;
}

void APP_ELCDIF_Init(void)
{
    const elcdif_rgb_mode_config_t config = {
        .panelWidth    = APP_IMG_WIDTH,
        .panelHeight   = APP_IMG_HEIGHT,
        .hsw           = APP_HSW,
        .hfp           = APP_HFP,
        .hbp           = APP_HBP,
        .vsw           = APP_VSW,
        .vfp           = APP_VFP,
        .vbp           = APP_VBP,
        .polarityFlags = APP_POL_FLAGS,
        .bufferAddr    = (uint32_t)s_frameBuffer,
        .pixelFormat   = kELCDIF_PixelFormatRAW8,
        .dataBus       = kELCDIF_DataBus8Bit,
    };

#if (defined(APP_ELCDIF_HAS_DISPLAY_INTERFACE) && APP_ELCDIF_HAS_DISPLAY_INTERFACE)
    BOARD_InitDisplayInterface();
#endif

    ELCDIF_RgbModeInit(APP_ELCDIF, &config);

    /* Load the LUT data. */
    ELCDIF_UpdateLut(APP_ELCDIF, kELCDIF_Lut0, 0, lutData[0], ELCDIF_LUT_ENTRY_NUM);
    ELCDIF_UpdateLut(APP_ELCDIF, kELCDIF_Lut1, 0, lutData[1], ELCDIF_LUT_ENTRY_NUM);

    ELCDIF_EnableLut(APP_ELCDIF, true);
}

uint32_t APP_MakeLutData(uint8_t r, uint8_t g, uint8_t b)
{
#if (APP_DATA_BUS == 16)
    /* 16 bit data bus. */
    return (((r >> 3)) << 11) | (((g >> 2)) << 5) | (((b >> 3)) << 0);
#else
    /* 24-bit data bus. */
    return (r << 16) | (g << 8) | (b << 0);
#endif
}

/*
 * Fill the LUT data.
 *
 * The first LUT memory is red, the second LUT memory is blue.
 */
void APP_FillLutData(void)
{
    uint32_t i;

    for (i = 0; i < ELCDIF_LUT_ENTRY_NUM; i++)
    {
        lutData[0][i] = APP_MakeLutData(i, 0, 0);
    }

    for (i = 0; i < ELCDIF_LUT_ENTRY_NUM; i++)
    {
        lutData[1][i] = APP_MakeLutData(0, 0, i);
    }
}

/*
 * Fill the frame buffer.
 *
 * The color degree changes in horizontal direction.
 */
void APP_FillFrameBuffer(void)
{
    uint32_t i, j;

    for (i = 0; i < APP_IMG_HEIGHT; i++)
    {
        for (j = 0; j < APP_IMG_WIDTH; j++)
        {
            s_frameBuffer[i][j] = j;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t lutIndex    = 0;
    uint32_t frameToWait = 0;

    BOARD_ConfigMPU();
    BOARD_BootClockRUN();
    BOARD_ResetDisplayMix();
    BOARD_InitLpuartPins();
    BOARD_InitMipiPanelPins();
    BOARD_InitDebugConsole();
    BOARD_InitLcdifClock();

    PRINTF("LCDIF LUT example start...\r\n");

    /* Clear the frame buffer. */
    memset(s_frameBuffer, 0, sizeof(s_frameBuffer));

    APP_FillLutData();
    APP_FillFrameBuffer();

    APP_ELCDIF_Init();

    BOARD_EnableLcdInterrupt();

    ELCDIF_EnableInterrupts(APP_ELCDIF, kELCDIF_CurFrameDoneInterruptEnable);
    ELCDIF_RgbModeStart(APP_ELCDIF);

    while (1)
    {
        frameToWait += APP_FRAME_PER_PIC;

        while (frameToWait != s_curFrame)
        {
        }

        lutIndex ^= 1U;

        /* The LSB of frame buffer determines which LUT memory to use. */
        ELCDIF_SetNextBufferAddr(APP_ELCDIF, (uint32_t)s_frameBuffer | lutIndex);
    }
}
