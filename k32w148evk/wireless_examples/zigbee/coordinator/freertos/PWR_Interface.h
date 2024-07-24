/**********************************************************************************
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

#ifndef _PWR_INTERFACE_H_
#define _PWR_INTERFACE_H_

/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *****************************************************************************/

#include <stdbool.h>

#include "EmbeddedTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @addtogroup LowPower
 * The LowPower module
 *
 * The low-power module (LPM) offers access to interface functions and macros that allow
 * the developer to perform the following actions:
 * - Enable or disable the device to enter low-power modes.
 * - Check whether the device is enabled to enter low-power modes.
 * - Put the device into one of the low-power modes.
 * - Configure which low-power modes the device must use.
 * - Set the low-power state duration in milliseconds or symbols.
 * - Configure the wake-up sources from the low-power modes.
 * - Power off the device.
 * - Reset the device.
 * -Get the system reset status.
 *
 * The LPM API provides access to three types of low-power modes, Sleep, Deep Sleep, and
 * Power Down. The Sleep/Deep Sleep modes are statically configured at compile-time through
 * properties specified in the file, PWR_Configuration.h.
 *
 * The configuration for the Sleep and Deep Sleep functions corresponds to a combination of
 * low-power modes of the processor-radio.
 *
 * The API functions determine whether the low-power configuration is valid and activate the
 * selected low-power mode. Wake-up sources are also configured at the application level.
 * The duration of the low-power state can be configured for some of the low-power modes
 * (in milliseconds or symbols) statically at the compile-time or at the run time. After
 * waking up, the LPM functions return the wake-up reason (if applicable).
 *
 * The application must call the PWR_Init() function to initialize the low-power library.
 * The code for entering the low-power state must be placed in the Idle task. It can also
 * be placed in another low-priority application task that runs only after all the other
 * (higher-priority) tasks finish all their work.
 *
 * @{
 */

/***********************************************************************************
 *                                      MACROS                                      *
 ************************************************************************************/

/*****************************************************************************
 *                        PUBLIC TYPE DEFINITIONS                            *
 *****************************************************************************/

/*!
 * \brief Enumerated data type definition for status returned for PWR operations
 */
typedef enum PWR_ReturnStatus_tag
{
    PWR_Success,           /*!<  operation successful    */
    PWR_ErrorNotSupported, /*!<  operation not supported */
    PWR_Error              /*!<  generic failure         */
} PWR_ReturnStatus_t;

/*!
 * \brief Enumerated data type definition for supported power modes
 */
typedef enum PWR_PowerModes_tag
{
    PWR_WFI           = 0, /*!<  Wait For Interrupt    */
    PWR_Sleep         = 1, /*!<  sleep mode            */
    PWR_DeepSleep     = 2, /*!<  deep sleep mode       */
    PWR_PowerDown     = 3, /*!<  power down mode       */
    PWR_DeepPowerDown = 4, /*!<  deep power down mode  */
} PWR_LowpowerMode_t;

/*****************************************************************************
 *                            PUBLIC FUNCTIONS                               *
 *****************************************************************************/

/*!
 * \brief Initializes Connectivity Low Power module
 *
 */
void PWR_Init(void);

/*!
 * \brief Starts enter low power procedure based on system constraints
 * \warning Function shall be called with interrupts masked with PRIMASK
 *
 * \param[in] timeoutUs maximum period in low power in us (0 means no timeout)
 * \return uint64_t actual low power duration
 */
uint64_t PWR_EnterLowPower(uint64_t timeoutUs);

/*!
 * \brief   Set new low power mode constraint, SDK Power manager will then select the best
 *            suitable low power mode that meets all lowpower requests
 * \details See PWR_PowerMode_t definition for available modes
 *
 * \param[in] mode low power mode
 * \return PWR_ReturnStatus_t
 */
PWR_ReturnStatus_t PWR_SetLowPowerModeConstraint(PWR_LowpowerMode_t mode);

/*!
 * \brief   Release previously set low power mode constraint,
 * \details See PWR_PowerMode_t definition for available modes
 *
 * \param[in] mode low power mode
 * \return PWR_ReturnStatus_t
 */
PWR_ReturnStatus_t PWR_ReleaseLowPowerModeConstraint(PWR_LowpowerMode_t mode);

/*!
 * \brief Prevent low power entry until the application re-allows it.
 *        This function can be called multiple times, therefore the same amount of call to PWR_AllowDeviceToSleep
 *        must be done in order to re-allow low power entry.
 */
void PWR_DisallowDeviceToSleep(void);

/*!
 * \brief Re-allow low power entry.
 *        It is not allowed to call it if no call to PWR_DisallowDeviceToSleep was done previously.
 *
 */
void PWR_AllowDeviceToSleep(void);

/*!
 * \brief Return number of outstanding actions that need to complete before
 *        sleep is possible
 * \return uint8_t status
 */
uint8_t PWR_IsDeviceAllowedToSleep(void);

/*!
 * \brief Enter Critical lowpower section to forbid some lowpower modes because of Hardware activities,
 *       Such as byte transmission on peripheral, DMA transfer, cryptographic processing...\n
 *       This function will set a constraint to prevent device lowpower\n
 *       Need to call PWR_LowPowperExitCritical to release the lowpower section.
 * \param[in] power_mode power_mode low power mode
 * \return    int32_t return status
 *
 */
int32_t PWR_LowPowerEnterCritical(int32_t power_mode);

/*!
 * \brief Exit Critical lowpower section that forbid some lowpower modes because of Hardware activities,
 *       Such as byte transmission on peripheral, DMA transfer, cryptographic processing...\n
 *       This function will release a constraint to prevent device lowpower\n
 *       Need to call PWR_LowPowperEnterCritical to set the lowpower section first
 * \param[in] power_mode power_mode low power mode
 * \return    int32_t return status
 *
 */
int32_t PWR_LowPowerExitCritical(int32_t power_mode);

/*!
 * \brief Disable systicks, and prepare for going to lowpower,
 *      Disable systicks
 *      Check if no task to be scheduled
 *      Compensate syticks timebase with cumulated error
 *      provide indication to abort low power entry if needed
 * \param[in] xExpectedIdleTime Idle time in systick periods
 * \param[out] expectedIdleTimeUs provide expected Idle time in Us before next OS event
 * \return bool return
 *      true if low power entry shall be aborted, in this case systicks are not disabled
 *      otherwise, need to call PWR_SysticksPostProcess on wakeup from low power
 */
bool PWR_SysticksPreProcess(uint32_t xExpectedIdleTime, uint64_t *expectedIdleTimeUs);

/*!
 * \brief Re-enable systicks, and compensate systick timebase,
 *      Shall be called if PWR_SysticksPreProcess() return false
 * \param[in] expectedIdleTimeUs provide expected Idle time in Us before next OS event
 * \param[in] actualIdleTimeUs provide actual Idle time in Us reported by PWR_EnterLowPower()
 *
 */
void PWR_SysticksPostProcess(uint64_t expectedIdleTimeUs, uint64_t actualIdleTimeUs);

/*!
 * @brief Enable LP related structures needed by the OSA abstraction in order
 *        to properly account for the time spent sleeping.
 *
 * This function calls the proper functions for enabling/disabling the SysTick
 * interrupt and calculating the proper time based on the timestamp functionality
 * keeping to be corrected with the sleep duration (taken from a low power
 * timer. This is available only in BM context and only if the systick is used
 * as a time source for the OSA.
 *
 */
void PWR_SysTicksLowPowerInit(void);

/*!
 * @} end of LowPower addtogroup
 */
#ifdef __cplusplus
}
#endif

#endif /* _PWR_INTERFACE_H_ */
