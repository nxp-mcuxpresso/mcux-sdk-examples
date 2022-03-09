/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "board.h"
#include "fsl_dsp.h"
#include "dsp_support.h"
#include "pmic_support.h"

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
    /* Initialize PMIC PCA9420 */
    BOARD_InitPmic();

    CLOCK_InitSysPfd(kCLOCK_Pfd1, 24); /* Enable dsp PLL clock 396MHz. */

    /*Let DSP run on SYS PLL PFD1 with divider 2 (198Mhz). */
    CLOCK_AttachClk(kDSP_PLL_to_DSP_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivDspCpuClk, 2);

    /* Configure PMIC Vddcore value according to main/dsp clock. */
    BOARD_SetPmicVoltageForFreq(CLOCK_GetFreq(kCLOCK_CoreSysClk), CLOCK_GetFreq(kCLOCK_DspCpuClk));

    /* Set DSP to use primary static vector base(0x0000_0000) and remap to 0x180000. */
    DSP_SetVecRemap(kDSP_StatVecSelPrimary, 0x600U);

    /* Initializing DSP core */
    DSP_Init();

#if DSP_IMAGE_COPY_TO_RAM
    /* Copy application from RAM to DSP_TCM */
    DSP_CopyImage(&reset_image);
    DSP_CopyImage(&text_image);
    DSP_CopyImage(&data_image);
#endif

    /* Run DSP core */
    DSP_Start();
}
