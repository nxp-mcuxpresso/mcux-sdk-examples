/*
 * @brief Module to initialize/deinitialize the PLL
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "audio_codec.h"
#include "pin_mux.h"
#include "board.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_power.h"

const pll_setup_t pllSetup = {
    .syspllctrl   = SYSCON_SYSPLLCTRL_BANDSEL_MASK | SYSCON_SYSPLLCTRL_SELP(0x1FU) | SYSCON_SYSPLLCTRL_SELI(0x8U),
    .syspllndec   = SYSCON_SYSPLLNDEC_NDEC(0x2DU),
    .syspllpdec   = SYSCON_SYSPLLPDEC_PDEC(0x42U),
    .syspllssctrl = {SYSCON_SYSPLLSSCTRL0_MDEC(0x34D3U) | SYSCON_SYSPLLSSCTRL0_SEL_EXT_MASK, 0x00000000U},
    .pllRate      = 24576000U, /* 16 bits * 2 channels * 48 kHz * 16 */
    .flags        = PLL_SETUPFLAG_WAITLOCK};

void audio_pll_setup(void)
{
    /* Stop auto trimming of the FRO from USB */
    SYSCON->FROCTRL = (SYSCON->FROCTRL & ~((0x01U << 15U) | (0xFU << 26U) | SYSCON_FROCTRL_USBCLKADJ_MASK));

    /* Initialize PLL clock */
    CLOCK_AttachClk(kFRO12M_to_SYS_PLL);
    CLOCK_SetPLLFreq(&pllSetup);

    /* MCLK output for I2S */
    CLOCK_EnableClock(kCLOCK_Iocon);
    IOCON_PinMuxSet(IOCON, 1, 17, IOCON_FUNC4 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    CLOCK_DisableClock(kCLOCK_Iocon);

    /* Attach PLL clock to MCLK for I2S, no divider */
    SYSCON->MCLKIO = 1U;
    CLOCK_AttachClk(kSYS_PLL_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
}

void audio_trim_up(void)
{
    uint32_t val    = SYSCON->FROCTRL;
    val             = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) + 1) << 16) | (1UL << 31);
    SYSCON->FROCTRL = val;
#if !defined(LOWPOWEROPERATION)
    GPIO_PortToggle(GPIO, BOARD_LED_RED_GPIO_PORT, (0x01 << BOARD_LED_RED_GPIO_PIN));
#endif
}

/* Reduce the audio frequency */
void audio_trim_down(void)
{
    uint32_t val    = SYSCON->FROCTRL;
    SYSCON->FROCTRL = (val & ~(0xff << 16)) | ((((val >> 16) & 0xFF) - 1) << 16) | (1UL << 31);
#if !defined(LOWPOWEROPERATION)
    GPIO_PortToggle(GPIO, BOARD_LED_GREEN_GPIO_PORT, (0x01 << BOARD_LED_GREEN_GPIO_PIN));
#endif
}

/* Stop the audio pll */
void audio_pll_stop(void)
{
    /* Stop the MCLK OUT to save power */
    CLOCK_AttachClk(kNONE_to_MCLK);
    SYSCON->MCLKIO = 0;

    /* Turn off PLL */
    CLOCK_AttachClk(kNONE_to_SYS_PLL);
    /* power down pll */
    POWER_DisablePD(kPDRUNCFG_PD_SYS_PLL0);
}

/* Restart the audio pll */
void audio_pll_start(void)
{
    CLOCK_AttachClk(kFRO12M_to_SYS_PLL);
    CLOCK_SetPLLFreq(&pllSetup);

    SYSCON->MCLKIO = 1U;
    CLOCK_AttachClk(kSYS_PLL_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
}
