/*
 * Copyright 2018-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_pca9420.h"
#include "fsl_dsp.h"
#include "board.h"
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
    dsp_copy_image_t literal_image;
    dsp_copy_image_t text_image;
    dsp_copy_image_t data_image;
#ifdef DSP_NCACHE
    dsp_copy_image_t ncache_image;
#endif

    literal_image.destAddr = DSP_LITERAL_ADDRESS;
    text_image.destAddr    = DSP_BOOT_ADDRESS;
    data_image.destAddr    = DSP_SRAM_ADDRESS;
#ifdef DSP_NCACHE
    ncache_image.destAddr = DSP_NCACHE_ADDRESS;
#endif

#if defined(__CC_ARM)
    size = (uint32_t)&Image$$DSP_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__dsp_literal_section"
    literal_image.srcAddr = DSP_IMAGE_LITERAL_START;
    literal_image.size    = DSP_IMAGE_LITERAL_SIZE;
#pragma section = "__dsp_text_section"
    text_image.srcAddr    = DSP_IMAGE_TEXT_START;
    text_image.size       = DSP_IMAGE_TEXT_SIZE;
#pragma section = "__dsp_data_section"
    data_image.srcAddr    = DSP_IMAGE_DATA_START;
    data_image.size       = DSP_IMAGE_DATA_SIZE;
#ifdef DSP_NCACHE
#pragma section = "__dsp_ncache_section"
    ncache_image.srcAddr  = DSP_IMAGE_NCACHE_START;
    ncache_image.size     = DSP_IMAGE_NCACHE_SIZE;
#endif
#elif defined(__GNUC__)
    literal_image.srcAddr = DSP_IMAGE_LITERAL_START;
    literal_image.size    = DSP_IMAGE_LITERAL_SIZE;
    text_image.srcAddr    = DSP_IMAGE_TEXT_START;
    text_image.size       = DSP_IMAGE_TEXT_SIZE;
    data_image.srcAddr    = DSP_IMAGE_DATA_START;
    data_image.size       = DSP_IMAGE_DATA_SIZE;
#ifdef DSP_NCACHE
    ncache_image.srcAddr  = DSP_IMAGE_NCACHE_START;
    ncache_image.size     = DSP_IMAGE_NCACHE_SIZE;
#endif
#endif
#endif
    /* Initialize PMIC PCA9420 */
    BOARD_InitPmic();
    /* Configure PMIC Vddcore value according to main/dsp clock. */
    BOARD_SetPmicVoltageForFreq(kPartTemp_0C_P85C, kVoltOpFullRange, CLOCK_GetFreq(kCLOCK_CoreSysClk), 594000000U);

    /* Enable DSP PLL clock 594MHz. */
    CLOCK_InitSysPfd(kCLOCK_Pfd1, 16);
    /*Let DSP run on DSP PLL clock with divider 1 (594Mhz). */
    CLOCK_AttachClk(kDSP_PLL_to_DSP_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivDspCpuClk, 1);
    CLOCK_SetClkDiv(kCLOCK_DivDspRamClk, 2);

    /* Initializing DSP core */
    DSP_Init();

#if DSP_IMAGE_COPY_TO_RAM
    /* Copy literals to DSP DTCM */
    DSP_CopyImage(&literal_image);
    /* Copy vectors to DSP ITCM */
    DSP_CopyImage(&text_image);

    /* Copy application from RAM to DSP_RAM */
    DSP_CopyImage(&data_image);
#ifdef DSP_NCACHE
    /* Copy ncache section to DSP_RAM */
    DSP_CopyImage(&ncache_image);
#endif
#endif

    /* Run DSP core */
    DSP_Start();
}
