/*
 * Copyright 2021, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef NOR_CMD_LUT_SEQ_IDX_CONFIG
#define NOR_CMD_LUT_SEQ_IDX_CONFIG 6
#endif

#define SYSTICK_BASE       LPTMR0
#define SYSTICK_IRQn       LPTMR0_IRQn
#define SYSTICK_HANDLER    LPTMR0_IRQHandler
#define SYSTICK_CLOCK_NAME kCLOCK_Lptmr0

#define IN_REG_CFG(v, m)          \
    {                             \
        .volt = (v), .mode = (m), \
    }

#define PMIC_CFG(v, m)            \
    {                             \
        .volt = (v), .mode = (m), \
    }
#define PAD_CFG(c, r, t)                                       \
    {                                                          \
        .pad_close = (c), .pad_reset = (r), .pad_tqsleep = (t) \
    }
#define MON_CFG(hvd_en, lvd_en, lvdlvl)                                        \
    {                                                                          \
        .mon_hvd_en = (hvd_en), .mon_lvd_en = (lvd_en), .mon_lvdlvl = (lvdlvl) \
    }
#define BIAS_CFG(m, n, p, mbias) \
    {                            \
        .dombias_cfg =           \
            {                    \
                .mode = (m),     \
                .rbbn = (n),     \
                .rbbp = (p),     \
            },                   \
        .membias_cfg = {mbias},  \
    }
#define PWRSYS_LPM_CFG(m) \
    {                     \
        .lpm_mode = (m),  \
    }
#define SWT_BOARD(swt_on, msk)         \
    {                                  \
        .on = (swt_on), .mask = (msk), \
    }
#define SWT_MEM(a, p, m)                         \
    {                                            \
        .array = (a), .perif = (p), .mask = (m), \
    }

// clang-format off
/*
 * Power mode definition of low power management for RTD.
 * Recovery time: Sleep < Deep Sleep < Power Down < Deep Power Down
 * Active:
 * Wait: sleep mode of cortex-m
 *                                              - Sleep
 *                                              |
 *                                              |-Deep Sleep
 *                                              |
 * Stop: deep sleep mode of cortex-m--(extend)--
 *                                              |
 *                                              |-Power Down
 *                                              |
 *                                              - Deep Power Down
 *
 */
// clang-format on
typedef enum _lpm_rtd_power_mode
{
    LPM_PowerModeActive = 0,    /* Active mode */
    LPM_PowerModeWait,          /* WAIT mode.(sleep mode of cortex-m) */
    LPM_PowerModeStop,          /* STOP mode.(deep sleep mode of cortex-m) */
    LPM_PowerModeSleep,         /* Sleep mode. */
    LPM_PowerModeDeepSleep,     /* Deep Sleep mode. */
    LPM_PowerModePowerDown,     /* Power Down mode. */
    LPM_PowerModeDeepPowerDown, /* Deep Power Down mode */
} lpm_rtd_power_mode_e;

typedef bool (*lpm_power_mode_callback_t)(lpm_rtd_power_mode_e curMode, lpm_rtd_power_mode_e newMode, void *data);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/* Initialize the Low Power Management */
bool LPM_Init(void);

/* Deinitialize the Low Power Management */
void LPM_Deinit(void);

/* Save target mode to internal variable, not program hardware yet */
bool LPM_SetPowerMode(lpm_rtd_power_mode_e mode);

/* Get low power mode previously set.
 */
lpm_rtd_power_mode_e LPM_GetPowerMode(void);

/* LPM_SetPowerMode() won't switch system power status immediately,
 * instead, such operation is done by LPM_WaitForInterrupt().
 * It can be callled in idle task of FreeRTOS, or main loop in bare
 * metal application. The sleep depth of this API depends
 * on current power mode set by LPM_SetPowerMode().
 * The timeoutMilliSec means if no interrupt occurs before timeout, the
 * system will be waken up by systick. If timeout exceeds hardware timer
 * limitation, timeout will be reduced to maximum time of hardware.
 * timeoutMilliSec only works in FreeRTOS, in bare metal application,
 * it's just ignored.
 * Return true if power mode switch succeeds.
 */
bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec);

/* Register power mode switch listener. When LPM_SetPowerMode()
 * is called, all the registered listeners will be invoked. The
 * listener returns true if it allows the power mode switch,
 * otherwise returns FALSE.
 */
void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data);

/* Unregister power mode switch listener */
void LPM_UnregisterPowerListener(lpm_power_mode_callback_t callback, void *data);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _LPM_H_ */
