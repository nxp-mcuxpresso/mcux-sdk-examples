/*
 * Copyright 2023 NXP
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
 * SCG_LPFLL: 72MHz.
 *
 * VLPR:
 *  - Clock source: SIRC
 * RUN:
 *  - Clock source: LPFLL
 */

/*
 * SIRC clock setting.
 * SIRC clock           : 8MHz
 * SIRCDIV2_CLK output  : 4MHz
 */
const scg_sirc_config_t s_scgSircConfig = {.enableMode = kSCG_SircEnable | kSCG_SircEnableInLowPower,
                                           .div2       = kSCG_AsyncClkDivBy2,
                                           .range      = kSCG_SircRangeHigh};

/*
 * FIRC clock setting.
 * FIRC clock           : 48MHz
 * FIRCDIV2_CLK output  : 48MHz
 */
const scg_firc_config_t s_scgFircConfig = {
    .enableMode = kSCG_FircEnable, .div2 = kSCG_AsyncClkDivBy1, .range = kSCG_FircRange48M, .trimConfig = NULL};

/*
 * LPFLL clock setting in RUN mode.
 * LPFLL clock       : 72MHz
 * LPFLLDIV2 output  : 36MHz
 */
const scg_lpfll_config_t s_scgSysLpFllConfigRun = {
    .enableMode = kSCG_LpFllEnable, .div2 = kSCG_AsyncClkDivBy2, .range = kSCG_LpFllRange72M, .trimConfig = NULL};

/*
 * LPFLL clock setting in HSRUN mode.
 * LPFLL clock       : 96MHz
 * LPFLLDIV2 output  : 48MHz
 */
const scg_lpfll_config_t s_scgSysLpFllConfigHsRun = {
    .enableMode = kSCG_LpFllEnable, .div2 = kSCG_AsyncClkDivBy2, .range = kSCG_LpFllRange96M, .trimConfig = NULL};

/*
 * System clock configuration while using SIRC in RUN mode.
 * Core clock : 8MHz
 * Slow clock : 4MHz
 */
const scg_sys_clk_config_t s_sysClkConfigSircInRun = {
    .divSlow = kSCG_SysClkDivBy2, /* Slow clock divider. */
    .divCore = kSCG_SysClkDivBy1, /* Core clock divider. */
    .src     = kSCG_SysClkSrcSirc /* System clock source. */
};
/*
 * System clock configuration while using SIRC in VLPR mode.
 * Core clock : 4MHz
 * Slow clock : 1MHz
 */
const scg_sys_clk_config_t s_sysClkConfigSircInVlpr = {.divSlow = kSCG_SysClkDivBy8, /* Slow clock divider. */
                                                       .divCore = kSCG_SysClkDivBy2, /* Core clock divider. */
                                                       .src     = kSCG_SysClkSrcSirc};

/*
 * System clock configuration while using LpFLL in RUN mode.
 * Core clock : 72MHz
 * Slow clock : 24MHz
 */
const scg_sys_clk_config_t s_sysClkConfigLpFllInRun = {
    .divSlow = kSCG_SysClkDivBy3,  /* Slow clock divider. */
    .divCore = kSCG_SysClkDivBy1,  /* Core clock divider. */
    .src     = kSCG_SysClkSrcLpFll /* System clock source. */
};

/*
 * System clock configuration while using LpFLL in HSRUN mode.
 * Core clock : 96MHz
 * Slow clock : 24MHz
 */
const scg_sys_clk_config_t s_sysClkConfigLpFllInHsRun = {
    .divSlow = kSCG_SysClkDivBy4,  /* Slow clock divider. */
    .divCore = kSCG_SysClkDivBy1,  /* Core clock divider. */
    .src     = kSCG_SysClkSrcLpFll /* System clock source. */
};

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

    CLOCK_InitSirc(&s_scgSircConfig);
    CLOCK_SetRunModeSysClkConfig(&s_sysClkConfigSircInRun);

    /* Wait for clock source change finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curSysClkConfig);
    } while (curSysClkConfig.src != s_sysClkConfigSircInRun.src);

    CLOCK_InitFirc(&s_scgFircConfig);
    CLOCK_InitLpFll(&s_scgSysLpFllConfigRun);

    CLOCK_SetRunModeSysClkConfig(&s_sysClkConfigLpFllInRun);

    /* Wait for clock source change finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curSysClkConfig);
    } while (curSysClkConfig.src != s_sysClkConfigLpFllInRun.src);

    CLOCK_SetVlprModeSysClkConfig(&s_sysClkConfigSircInVlpr);
}

/*
 * Set the clock configuration for RUN mode from VLPR mode.
 */
void APP_SetClockRunFromVlpr(void)
{
    while (!CLOCK_IsLpFllValid())
    {
    }
}

/*
 * Set the clock configuration for VLPR mode.
 */
void APP_SetClockVlpr(void)
{
}

void APP_SetClockHsrun(void)
{
    scg_sys_clk_config_t curSysClkConfig;

    CLOCK_SetHsrunModeSysClkConfig(&s_sysClkConfigSircInRun);
    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curSysClkConfig);
    } while (curSysClkConfig.src != s_sysClkConfigSircInRun.src);
    CLOCK_InitLpFll(&s_scgSysLpFllConfigHsRun);
    CLOCK_SetHsrunModeSysClkConfig(&s_sysClkConfigLpFllInHsRun);
    while (!CLOCK_IsLpFllValid())
    {
    }
}

void APP_SetClockRunFromHsrun(void)
{
    scg_sys_clk_config_t curSysClkConfig;

    CLOCK_SetHsrunModeSysClkConfig(&s_sysClkConfigSircInRun);
    /* Wait for clock source switch finished. */
    do
    {
        CLOCK_GetCurSysClkConfig(&curSysClkConfig);
    } while (curSysClkConfig.src != s_sysClkConfigSircInRun.src);
    CLOCK_InitLpFll(&s_scgSysLpFllConfigRun);
    while (!CLOCK_IsLpFllValid())
    {
    }
}
