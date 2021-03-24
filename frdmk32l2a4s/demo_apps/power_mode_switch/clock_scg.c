/*
 * Copyright 2019 NXP
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
 * In SCG based example, SCG clock sources are configured as:
 *
 * SCG_SOSC: Disabled.
 * SCG_SIRC: 8MHz.
 * SCG_FIRC: 48MHz.
 * SCG_SPLL: 96MHz.
 *
 * VLPR:
 *  - Clock source: SIRC
 * RUN:
 *  - Clock source: FIRC
 * HSRUN:
 *  - Clock source: SPLL
 */

/*
 * SIRC clock setting.
 * SIRC clock           : 8MHz
 * SIRCDIV1_CLK output  : Disable
 * SIRCDIV2_CLK output  : 4MHz
 * SIRCDIV3_CLK output  : 4MHz
 */
const scg_sirc_config_t s_scgSircConfig = {.enableMode = kSCG_SircEnable | kSCG_SircEnableInLowPower,
                                           .div1       = kSCG_AsyncClkDisable,
                                           .div2       = kSCG_AsyncClkDivBy2,
                                           .div3       = kSCG_AsyncClkDivBy2,
                                           .range      = kSCG_SircRangeHigh};

/*
 * FIRC clock setting.
 * FIRC clock           : 48MHz
 * FIRCDIV1_CLK output  : 48MHz
 * FIRCDIV2_CLK output  : Disable
 * FIRCDIV3_CLK output  : 48MHz
 */
const scg_firc_config_t s_scgFircConfig = {.enableMode = kSCG_FircEnable,
                                           .div3       = kSCG_AsyncClkDivBy1,
                                           .div2       = kSCG_AsyncClkDisable,
                                           .div1       = kSCG_AsyncClkDivBy1,
                                           .range      = kSCG_FircRange48M,
                                           .trimConfig = NULL};

/*
 * SYSPLL clock setting.
 * SYSPLL clock       : 96MHz (with the internal divider = 2)
 * SYSPLLDIV1 output  : 96MHz
 * SYSPLLDIV2 output  : Disable
 * SYSPLLDIV3 output  : 48MHz
 */
const scg_spll_config_t s_scgSysPllConfig = {.enableMode  = kSCG_SysPllEnable,
                                             .monitorMode = kSCG_SysPllMonitorDisable,
                                             .div1        = kSCG_AsyncClkDivBy1,
                                             .div2        = kSCG_AsyncClkDisable,
                                             .div3        = kSCG_AsyncClkDivBy2,
                                             .src         = kSCG_SysPllSrcFirc, /* 48MHz. */
                                             .prediv      = 0x5,                /* Div = 6. */
                                             .mult        = 0x8};                      /* Mult = 24 */

/*
 * System clock configuration while using SIRC in RUN mode.
 * Core clock : 8MHz
 * Slow clock : 4MHz
 */
const scg_sys_clk_config_t s_sysClkConfigSircInRun = {.divSlow = kSCG_SysClkDivBy2,
#if (defined(FSL_FEATURE_SCG_HAS_DIVBUS) && FSL_FEATURE_SCG_HAS_DIVBUS)
                                                      .divBus = kSCG_SysClkDivBy1,
#endif
#if (defined(FSL_FEATURE_SCG_HAS_DIVPLAT) && FSL_FEATURE_SCG_HAS_DIVPLAT)
                                                      .divPlat = kSCG_SysClkDivBy1,
#endif
                                                      .divCore = kSCG_SysClkDivBy1,
                                                      .src     = kSCG_SysClkSrcSirc};

/*
 * System clock configuration while using SIRC in VLPR mode.
 * Core clock : 4MHz
 * Slow clock : 1MHz
 */
const scg_sys_clk_config_t s_sysClkConfigSircInVlpr = {.divSlow = kSCG_SysClkDivBy4,
#if (defined(FSL_FEATURE_SCG_HAS_DIVBUS) && FSL_FEATURE_SCG_HAS_DIVBUS)
                                                       .divBus = kSCG_SysClkDivBy1,
#endif
#if (defined(FSL_FEATURE_SCG_HAS_DIVPLAT) && FSL_FEATURE_SCG_HAS_DIVPLAT)
                                                       .divPlat = kSCG_SysClkDivBy1,
#endif
                                                       .divCore = kSCG_SysClkDivBy2,
                                                       .src     = kSCG_SysClkSrcSirc};

/*
 * System clock configuration while using FIRC in RUN mode.
 * Core clock : 48MHz
 * Slow clock : 24MHz
 */
const scg_sys_clk_config_t s_sysClkConfigFircInRun = {.divSlow = kSCG_SysClkDivBy2,
#if (defined(FSL_FEATURE_SCG_HAS_DIVBUS) && FSL_FEATURE_SCG_HAS_DIVBUS)
                                                      .divBus = kSCG_SysClkDivBy2,
#endif
#if (defined(FSL_FEATURE_SCG_HAS_DIVPLAT) && FSL_FEATURE_SCG_HAS_DIVPLAT)
                                                      .divPlat = kSCG_SysClkDivBy1,
#endif
                                                      .divCore = kSCG_SysClkDivBy1,
                                                      .src     = kSCG_SysClkSrcFirc};

#if (defined(FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE) && FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE)
/*
 * System clock configuration while using SYSPLL in HSRUN mode.
 * Core clock : 96MHz
 * Slow clock : 24MHz
 */
const scg_sys_clk_config_t s_sysClkConfigSysPllInHsrun = {.divSlow = kSCG_SysClkDivBy4,
#if (defined(FSL_FEATURE_SCG_HAS_DIVBUS) && FSL_FEATURE_SCG_HAS_DIVBUS)
                                                          .divBus = kSCG_SysClkDivBy2,
#endif
#if (defined(FSL_FEATURE_SCG_HAS_DIVPLAT) && FSL_FEATURE_SCG_HAS_DIVPLAT)
                                                          .divPlat = kSCG_SysClkDivBy1,
#endif
                                                          .divCore = kSCG_SysClkDivBy1,
                                                          .src     = kSCG_SysClkSrcSysPll};
#endif /* FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */

/*
 * Initialize SCG setting after system boot up.
 */
void APP_InitClock(void)
{
    scg_sys_clk_config_t sysClkConfig;
    scg_sys_clk_config_t curSysClkConfig;

    /*
     * Setup SIRC and FIRC:
     * On some platforms, SIRC is used by default after reset, while on some
     * other platforms, FIRC is used after reset. So at the begining, the
     * workflow is different.
     */
    CLOCK_GetCurSysClkConfig(&sysClkConfig);

    if (kSCG_SysClkSrcFirc == sysClkConfig.src) /* FIRC used by default. */
    {
        CLOCK_InitSirc(&s_scgSircConfig);
        CLOCK_SetRunModeSysClkConfig(&s_sysClkConfigSircInRun);
        /* Wait for clock source change finished. */
        do
        {
            CLOCK_GetCurSysClkConfig(&curSysClkConfig);
        } while (curSysClkConfig.src != s_sysClkConfigSircInRun.src);

        CLOCK_InitFirc(&s_scgFircConfig);
        CLOCK_SetRunModeSysClkConfig(&s_sysClkConfigFircInRun);
        /* Wait for clock source change finished. */
        do
        {
            CLOCK_GetCurSysClkConfig(&curSysClkConfig);
        } while (curSysClkConfig.src != s_sysClkConfigFircInRun.src);
    }
    else /* SIRC used by default. */
    {
        CLOCK_InitFirc(&s_scgFircConfig);
        CLOCK_SetRunModeSysClkConfig(&s_sysClkConfigFircInRun);
        /* Wait for clock source change finished. */
        do
        {
            CLOCK_GetCurSysClkConfig(&curSysClkConfig);
        } while (curSysClkConfig.src != s_sysClkConfigFircInRun.src);

        CLOCK_InitSirc(&s_scgSircConfig);
    }

    CLOCK_InitSysPll(&s_scgSysPllConfig);

    CLOCK_SetVlprModeSysClkConfig(&s_sysClkConfigSircInVlpr);
#if (defined(FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE) && FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE)
    CLOCK_SetHsrunModeSysClkConfig(&s_sysClkConfigSysPllInHsrun);
#endif
}

#if (defined(FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE) && FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE)
/*
 * Set the clock configuration for HSRUN mode.
 */
void APP_SetClockHsrun(void)
{
    while (!CLOCK_IsSysPllValid())
    {
    }
}

/*
 * Set the clock configuration for RUN mode from HSRUN mode.
 */
void APP_SetClockRunFromHsrun(void)
{
}
#endif /* FSL_FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */

/*
 * Set the clock configuration for RUN mode from VLPR mode.
 */
void APP_SetClockRunFromVlpr(void)
{
    while (!CLOCK_IsFircValid())
    {
    }
}

/*
 * Set the clock configuration for VLPR mode.
 */
void APP_SetClockVlpr(void)
{
}
