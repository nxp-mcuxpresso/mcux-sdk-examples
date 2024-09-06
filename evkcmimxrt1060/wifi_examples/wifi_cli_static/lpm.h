/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include <stdint.h>
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
extern uint32_t g_savedPrimask;

#if defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1)
#define LPM_EnterCritical()                        \
                                                   \
    do                                             \
    {                                              \
        g_savedPrimask = DisableGlobalIRQ();       \
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; \
                                                   \
    } while (0)

#define LPM_ExitCritical()                        \
                                                  \
    do                                            \
    {                                             \
        EnableGlobalIRQ(g_savedPrimask);          \
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; \
                                                  \
    } while (0)

#else
#define LPM_EnterCritical()
#define LPM_ExitCritical()
#endif

/*
 * Power mode definition of low power management.
 */
typedef enum _lpm_power_mode
{
    LPM_PowerModeOverRun = 0, /* Over RUN mode, CPU won't stop running */

    LPM_PowerModeFullRun,     /* Full RUN mode, CPU won't stop running */

    LPM_PowerModeLowSpeedRun,

    LPM_PowerModeLowPowerRun,

    LPM_PowerModeRunEnd = LPM_PowerModeLowPowerRun,
    /* In system wait mode, cpu clock is gated.
     * All peripheral can remain active, clock gating decided by CCGR setting.
     * DRAM enters auto-refresh mode when there is no access.
     */
    LPM_PowerModeSysIdle, /* System WAIT mode, also system low speed idle */

    /* In low power idle mode, all PLL/PFD is off, cpu power is off.
     * Analog modules running in low power mode.
     * All high-speed peripherals are power gated
     * Low speed peripherals can remain running at low frequency
     * DRAM in self-refresh.
     */
    LPM_PowerModeLPIdle, /* Low Power Idle mode */

    /* In deep sleep mode, all PLL/PFD is off, XTAL is off, cpu power is off.
     * All clocks are shut off except 32K RTC clock
     * All high-speed peripherals are power gated
     * Low speed peripherals are clock gated
     * DRAM in self-refresh.
     * If RTOS is used, systick will be disabled in DSM
     */
    LPM_PowerModeSuspend, /* Deep Sleep mode, suspend. */

    LPM_PowerModeSNVS,    /* Power off mode, or shutdown mode */

    LPM_PowerModeEnd = LPM_PowerModeSNVS
} lpm_power_mode_t;

typedef bool (*lpm_power_mode_callback_t)(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data);

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

AT_QUICKACCESS_SECTION_CODE(void CLOCK_SET_MUX(clock_mux_t mux, uint32_t value));
AT_QUICKACCESS_SECTION_CODE(void CLOCK_SET_DIV(clock_div_t divider, uint32_t value));
AT_QUICKACCESS_SECTION_CODE(void LPM_EnterSleepMode(clock_mode_t mode));

/* Initialize the Low Power Management */
bool LPM_Init(lpm_power_mode_t run_mode);

/* Deinitialize the Low Power Management */
void LPM_Deinit(void);

/* Enable wakeup source in low power mode */
void LPM_EnableWakeupSource(uint32_t irq);

/* Disable wakeup source in low power mode */
void LPM_DisableWakeupSource(uint32_t irq);

/* Set power mode, all registered listeners will be notified.
 * Return true if all the registered listeners return true.
 */
bool LPM_SetPowerMode(lpm_power_mode_t mode);

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
 */
void LPM_WaitForInterrupt(uint32_t timeoutMilliSec);

void LPM_EnableWakeupSource(uint32_t irq);
void LPM_DisableWakeupSource(uint32_t irq);
void LPM_OverDriveRun(lpm_power_mode_t curRunMode);
void LPM_FullSpeedRun(lpm_power_mode_t curRunMode);
void LPM_LowSpeedRun(lpm_power_mode_t curRunMode);
void LPM_LowPowerRun(lpm_power_mode_t curRunMode);
void LPM_EnterSystemIdle(lpm_power_mode_t curRunMode);
void LPM_ExitSystemIdle(lpm_power_mode_t curRunMode);
void LPM_EnterLowPowerIdle(lpm_power_mode_t curRunMode);
void LPM_ExitLowPowerIdle(lpm_power_mode_t curRunMode);
void LPM_EnterSuspend(void);
void LPM_EnterSNVS(void);

void vPortPRE_SLEEP_PROCESSING(unsigned long timeoutMilliSec);
void vPortPOST_SLEEP_PROCESSING(unsigned long timeoutMilliSec);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _LPM_H_ */
