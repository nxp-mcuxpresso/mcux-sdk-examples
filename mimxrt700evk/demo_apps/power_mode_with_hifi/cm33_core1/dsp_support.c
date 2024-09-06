/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
// #include "fsl_pca9422.h"
#include "fsl_dsp.h"
#include "board.h"
#include "dsp_support.h"
// #include "pmic_support.h"
#include "fsl_power.h"
#if defined(MIMXRT798S_cm33_core0_SERIES)
#include "fsl_cache.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_DSP_Init(uint32_t src, uint32_t div, bool copyImage)
{
#if DSP_IMAGE_COPY_TO_RAM
#if defined(MIMXRT798S_cm33_core0_SERIES)
    dsp_copy_image_t literal_image;
#endif
#if defined(MIMXRT798S_cm33_core1_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core1_SERIES)
    dsp_copy_image_t vector_image;
#endif
    dsp_copy_image_t text_image;
    dsp_copy_image_t data_image;
#ifdef DSP_NCACHE
    dsp_copy_image_t ncache_image;
#endif

#if defined(MIMXRT798S_cm33_core0_SERIES)
    literal_image.destAddr = DSP_LITERAL_ADDRESS;
    text_image.destAddr    = DSP_BOOT_ADDRESS;
    data_image.destAddr    = DSP_SRAM_ADDRESS;
#endif
#if defined(MIMXRT798S_cm33_core1_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core1_SERIES)
    vector_image.destAddr = DSP_BOOT_ADDRESS;
    text_image.destAddr   = DSP_TEXT_ADDRESS;
    data_image.destAddr   = DSP_DATA_ADDRESS;
#endif
#ifdef DSP_NCACHE
    ncache_image.destAddr = DSP_NCACHE_ADDRESS;
#endif

#if defined(__CC_ARM)
    size = (uint32_t)&Image$$DSP_REGION$$Length;
#elif defined(__ICCARM__)
#if defined(MIMXRT798S_cm33_core0_SERIES)
#pragma section = "__dsp_literal_section"
    literal_image.srcAddr = DSP_IMAGE_LITERAL_START;
    literal_image.size    = DSP_IMAGE_LITERAL_SIZE;
#endif
#if defined(MIMXRT798S_cm33_core1_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core1_SERIES)
#pragma section = "__dsp_vector_section"
    vector_image.srcAddr  = DSP_IMAGE_VECTOR_START;
    vector_image.size     = DSP_IMAGE_VECTOR_SIZE;
#endif
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
#if defined(MIMXRT798S_cm33_core0_SERIES)
    literal_image.srcAddr = DSP_IMAGE_LITERAL_START;
    literal_image.size    = DSP_IMAGE_LITERAL_SIZE;
#endif
#if defined(MIMXRT798S_cm33_core1_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core1_SERIES)
    vector_image.srcAddr  = DSP_IMAGE_VECTOR_START;
    vector_image.size     = DSP_IMAGE_VECTOR_SIZE;
#endif
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
#if defined(MIMXRT798S_cm33_core0_SERIES)
    POWER_DisablePD(kPDRUNCFG_PD_VDD2_DSP);
    POWER_ApplyPD();

    /* Set clock source. */
    CLKCTL0->DSPCPUCLKSEL = CLKCTL0_DSPCPUCLKSEL_SEL(src) | CLKCTL0_DSPCPUCLKSEL_SEL_EN_MASK;
    CLOCK_SetClkDiv(kCLOCK_DivDspClk, div);
#else
    CLKCTL1->SENSEDSPCPUCLKSEL = CLKCTL1_SENSEBASECLKSEL_SEL(src) | CLKCTL1_SENSEDSPCPUCLKSEL_SEL_EN_MASK;
    CLOCK_SetClkDiv(kCLOCK_DivSenseDspClk, div);
    /* Set DSP to use static vector base(0x0060_0000). */
    /* DSP_SetVecRemap(kDSP_StatVecSel1, 0U); */
#endif

    /* Initializing DSP core */
    DSP_Init();

#if DSP_IMAGE_COPY_TO_RAM
    if (copyImage)
    {
#if defined(MIMXRT798S_cm33_core0_SERIES) /* Only for HiFi4 */
        /* Copy literals to DSP DTCM */
        DSP_CopyImage(&literal_image);
#endif
        /* Only for HiFi1 */
#if defined(MIMXRT798S_cm33_core1_SERIES) || defined(MIMXRT758S_cm33_core1_SERIES) || \
    defined(MIMXRT735S_cm33_core1_SERIES)
        /* Copy literals to DSP RAM */
        DSP_CopyImage(&vector_image);
#endif
        /* Copy vectors to DSP ITCM */
        DSP_CopyImage(&text_image);
        /* Copy application from RAM to DSP_RAM */
        DSP_CopyImage(&data_image);
#if defined(MIMXRT798S_cm33_core0_SERIES) /* Only for HiFi4 */
        XCACHE_CleanInvalidateCacheByRange((uint32_t)data_image.destAddr, data_image.size);
#endif
#ifdef DSP_NCACHE
        /* Copy ncache section to DSP_RAM */
        DSP_CopyImage(&ncache_image);
#endif
    }
#endif

    /* Run DSP core */
    DSP_Start();
}
