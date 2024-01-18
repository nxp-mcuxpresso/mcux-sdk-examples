/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "fsl_fusion.h"
#include "dsp_support.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_DSP_Init(void)
{
    /*
     * Select FRO as system clock source, before configuring other clock sources.
     * Clock source   : FRO
     * Core clock     : 192MHz
     * Bus clock      : 96MHz
     * Slow clock     : 24MHz
     */
    const cgc_rtd_sys_clk_config_t g_sysClkConfigFroSource = {
        .divCore = 0,                    /* Core clock divider. */
        .divBus  = 1,                    /* Bus clock divider. */
        .divSlow = 3,                    /* Slow clock divider. */
        .src     = kCGC_RtdSysClkSrcFro, /* System clock source. */
        .locked  = 0,                    /* Register not locked. */
    };

#if DSP_IMAGE_COPY_TO_RAM
    dsp_copy_image_t reset_image;
    dsp_copy_image_t text_image;
    dsp_copy_image_t data_image;

    reset_image.destAddr = DSP_RESET_ADDRESS;
    text_image.destAddr  = DSP_TEXT_ADDRESS;
    data_image.destAddr  = DSP_SRAM_ADDRESS;

#if defined(__ICCARM__)
#pragma section = "__dsp_reset_section"
    reset_image.srcAddr = DSP_IMAGE_RESET_START;
    reset_image.size    = DSP_IMAGE_RESET_SIZE;
#pragma section = "__dsp_text_section"
    text_image.srcAddr = DSP_IMAGE_TEXT_START;
    text_image.size    = DSP_IMAGE_TEXT_SIZE;
#pragma section = "__dsp_data_section"
    data_image.srcAddr = DSP_IMAGE_DATA_START;
    data_image.size    = DSP_IMAGE_DATA_SIZE;
#elif defined(__GNUC__)
    reset_image.srcAddr = DSP_IMAGE_RESET_START;
    reset_image.size    = DSP_IMAGE_RESET_SIZE;
    text_image.srcAddr  = DSP_IMAGE_TEXT_START;
    text_image.size     = DSP_IMAGE_TEXT_SIZE;
    data_image.srcAddr  = DSP_IMAGE_DATA_START;
    data_image.size     = DSP_IMAGE_DATA_SIZE;
#endif
#endif

    CLOCK_SetFusionSysClkConfig(&g_sysClkConfigFroSource);

    Fusion_SetVecRemap(0x20); /* Remap vector to 0x8000. */

    /* Initializing DSP core */
    Fusion_Init();
    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

#if DSP_IMAGE_COPY_TO_RAM
    /* Copy DSP application to SSRAM. */
    DSP_CopyImage(&reset_image);
    DSP_CopyImage(&text_image);
    DSP_CopyImage(&data_image);
#endif

    /* Run DSP core */
    Fusion_Start();
}
