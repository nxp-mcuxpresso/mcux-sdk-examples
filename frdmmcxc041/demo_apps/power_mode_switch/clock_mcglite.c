/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include <assert.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * Set the clock configuration for RUN mode from VLPR mode.
 */
void APP_SetClockRunFromVlpr(void)
{
    const mcglite_config_t mcgliteConfig = {.outSrc                  = kMCGLITE_ClkSrcHirc,
                                            .irclkEnableMode         = 0U,
                                            .ircs                    = kMCGLITE_Lirc8M,
                                            .fcrdiv                  = kMCGLITE_LircDivBy1,
                                            .lircDiv2                = kMCGLITE_LircDivBy1,
                                            .hircEnableInNotHircMode = true};

    const sim_clock_config_t simConfig = {
        .clkdiv1 = 0x00010000U, /* SIM_CLKDIV1. */
#if (defined(FSL_FEATURE_SIM_OPT_HAS_OSC32K_SELECTION) && FSL_FEATURE_SIM_OPT_HAS_OSC32K_SELECTION)
        .er32kSrc = 0U,         /* SIM_SOPT1[OSC32KSEL]. */
#endif
    };

    CLOCK_SetSimSafeDivs();
    CLOCK_SetMcgliteConfig(&mcgliteConfig);
    CLOCK_SetSimConfig(&simConfig);
}

/*
 * Set the clock configuration for VLPR mode.
 */
void APP_SetClockVlpr(void)
{
    const mcglite_config_t mcgliteConfig = {
        .outSrc                  = kMCGLITE_ClkSrcLirc,
        .irclkEnableMode         = kMCGLITE_IrclkEnable,
        .ircs                    = kMCGLITE_Lirc2M,
        .fcrdiv                  = kMCGLITE_LircDivBy1,
        .lircDiv2                = kMCGLITE_LircDivBy1,
        .hircEnableInNotHircMode = false,
    };

    const sim_clock_config_t simConfig = {
        .clkdiv1 = 0x00010000U, /* SIM_CLKDIV1. */
#if (defined(FSL_FEATURE_SIM_OPT_HAS_OSC32K_SELECTION) && FSL_FEATURE_SIM_OPT_HAS_OSC32K_SELECTION)
        .er32kSrc = 0U,         /* SIM_SOPT1[OSC32KSEL]. */
#endif
    };

    CLOCK_SetSimSafeDivs();
    CLOCK_SetMcgliteConfig(&mcgliteConfig);
    CLOCK_SetSimConfig(&simConfig);
}
