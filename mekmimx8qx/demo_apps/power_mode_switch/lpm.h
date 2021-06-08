/*
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LPM_H_
#define _LPM_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef FSL_RTOS_FREE_RTOS
#define SYSTICK_BASE    LSIO__GPT4
#define SYSTICK_IRQn    LSIO_GPT4_INT_IRQn
#define SYSTICK_HANDLER LSIO_GPT4_INT_IRQHandler
#define SYSTICK_CLOCK   kCLOCK_LSIO_Gpt4
#define SYSTICK_RSRC    SC_R_GPT_4
#endif /* FSL_RTOS_FREE_RTOS */

#define IPC_MU            CM4__MU1_A
#define IPC_MU_IRQHandler M4_MU1_A_IRQHandler
#define IPC_MU_IRQn       M4_MU1_A_IRQn
#define IPC_MU_RSRC       SC_R_M4_0_MU_1A
#define CPU_RSRC          SC_R_M4_0_PID0
#define IRQSTEER_RSRC     SC_R_IRQSTR_M4_0

#define RTN_ERR(X)                        \
    if ((X) != SC_ERR_NONE)               \
    {                                     \
        assert("Error in SCFW API call"); \
    }
/*!
 * @brief Power mode definition of low power management.
 * Waken up duration Off > Dsm > Idle > Wait > Run.
 */
typedef enum _lpm_power_mode
{
    LPM_PowerModeRun = 0, /*!< Normal RUN mode */
    LPM_PowerModeWait,    /*!< WAIT mode. */
    LPM_PowerModeStop,    /*!< STOP mode. */
    LPM_PowerModeVlpr,    /*!< VLPR mode. */
    LPM_PowerModeVlpw,    /*!< VLPW mode. */
    LPM_PowerModeVlps,    /*!< VLPS mode. */
    LPM_PowerModeLls,     /*!< LLS mode */
    LPM_PowerModeVlls,    /*!< VLLS mode */
} lpm_power_mode_t;

#ifdef FSL_RTOS_FREE_RTOS
typedef bool (*lpm_power_mode_callback_t)(lpm_power_mode_t curMode, lpm_power_mode_t newMode, void *data);
#endif
typedef void (*sc_irq_handler_t)(void *param);
/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief Initialize the Low Power Management.
 */
bool LPM_Init(void);

/*!
 * @brief Deinitialize the Low Power Management.
 */
void LPM_Deinit(void);

/*!
 * @brief Check if target power mode is valid to transit.
 *
 * @param targetPowerMode the target power mode.
 * @param pErrorMsg the pointer of error message.
 * @return is target mode valid.
 */
bool LPM_IsTargetModeValid(lpm_power_mode_t targetPowerMode, const char **pErrorMsg);

/*!
 * @brief Set power mode, all registered listeners will be notified.
 * @param mode the mode need to change to.
 * @return Return true if all the registered listeners return true.
 */
bool LPM_SetPowerMode(lpm_power_mode_t mode);

/*!
 * @brief Get low power mode previously set.
 * @return return current low power mode.
 */
lpm_power_mode_t LPM_GetPowerMode(void);

/*!
 * @brief LPM_SetPowerMode() won't switch system power status immediately,
 * instead, such operation is done by LPM_WaitForInterrupt().
 * It can be callled in idle task of FreeRTOS, or main loop in bare
 * metal application. The sleep depth of this API depends
 * on current power mode set by LPM_SetPowerMode().
 * @param timeoutMilliSec The timeoutMilliSec means if no interrupt occurs before timeout, the
 * system will be waken up by systick. If timeout exceeds hardware timer
 * limitation, timeout will be reduced to maximum time of hardware.
 * timeoutMilliSec only works in FreeRTOS, in bare metal application,
 * it's just ignored.
 * @return Return true if power mode switch succeeds.
 */
bool LPM_WaitForInterrupt(uint32_t timeoutMilliSec);

/*!
 * @brief This function register the SC(system controller) broadcast IRQ handler. The SCU provides interrupt trigger
 * capability for M4 cores through scfw_api/svc/irq API, once the set event occurred the SCU trigger general interrupt
 * through the IPC MU. Then M4 can use scfw_api to poll the event status in this MU interrupt. For RTOS environment, due
 * to the SCFW API can't be called in interrupt context, the registered handler actully run in software timer task.
 * @param handler the SCU boradcast IRQ handler
 * @param param the parameter passed to the handler
 * @return return true if succeed.
 */
bool LPM_RegisterSCIRQHandler(sc_irq_handler_t handler, void *param);

/*!
 * @brief Unregister SC(system controller) boardcast IRQ handler.
 */
void LPM_UnregisterSCIRQHandler(void);

#ifdef FSL_RTOS_FREE_RTOS
/*!
 * @brief Register power mode switch listener. When LPM_SetPowerMode()
 * is called, all the registered listeners will be invoked.
 *
 * @param callback callback function
 * @param data data passed to callback function
 * @return The listener returns true if it allows the power mode switch, otherwise returns FALSE.
 */
void LPM_RegisterPowerListener(lpm_power_mode_callback_t callback, void *data);

/*!
 *@brief Unregister power mode switch listener
 */
void LPM_UnregisterPowerListener(lpm_power_mode_callback_t callback, void *data);
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus*/
#endif /* _LPM_H_ */
