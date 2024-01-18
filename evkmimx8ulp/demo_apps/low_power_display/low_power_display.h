/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _POWER_MODE_SWITCH_H_
#define _POWER_MODE_SWITCH_H_
#include "app_srtm.h"
#include "lpm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef GEN_CASE_ENUM_NAME
#define GEN_CASE_ENUM_NAME(e) \
    case (e):                 \
        return (char *)#e
#endif

/* Power mode definition used in application. */
typedef enum _app_power_mode
{
    kAPP_PowerModeActive = 'A',  /* Active mode */
    kAPP_PowerModeWait,          /* WAIT mode. */
    kAPP_PowerModeStop,          /* STOP mode. */
    kAPP_PowerModeSleep,         /* Sleep mode. */
    kAPP_PowerModeDeepSleep,     /* Deep Sleep mode. */
    kAPP_PowerModePowerDown,     /* Power Down mode. */
    kAPP_PowerModeDeepPowerDown, /* Deep Power Down mode */
} app_power_mode_e;

typedef enum
{
    MODE_COMBI_NO  = 0,
    MODE_COMBI_YES = 1,
} allow_combi_e;

typedef struct
{
    lpm_rtd_power_mode_e rtd_mode;
    lpm_ad_power_mode_e ad_mode;
    allow_combi_e allow_combi; /* Allow Combination */
} mode_combi_t;

typedef enum _allos_give_sig
{
    RTD_GIVE_SIG_NO  = 0,
    RTD_GIVE_SIG_YES = 1,
} allow_give_sig_e;

typedef struct rtd_mode_and_irq_allow
{
    lpm_rtd_power_mode_e current_rtd_mode; /* current mode of rtd */
    lpm_rtd_power_mode_e last_rtd_mode;    /* last mode of rtd */
    IRQn_Type irq_num;                     /* irq number */
    allow_give_sig_e give_semaphore_flag;  /* whether give semaphore for power mode switch in freertos task, true: give
                                  semaphore,  false: not give semaphore */
} rtd_mode_and_irq_allow_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _POWER_MODE_SWITCH_H_ */

/*******************************************************************************
 * EOF
 ******************************************************************************/
