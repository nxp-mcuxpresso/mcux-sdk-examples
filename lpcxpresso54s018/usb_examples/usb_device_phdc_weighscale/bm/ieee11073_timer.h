/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IEEE11073_TIMER_H_
#define _IEEE11073_TIMER_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief max timer object support */
#ifndef IEEE_MAX_TIMER_OBJECTS
#define IEEE_MAX_TIMER_OBJECTS 0U
#endif
#if IEEE_MAX_TIMER_OBJECTS

/*! @brief timer callback function prototype */
typedef void (*ieee11073_timer_callback)(void *timerArgument);

/*! @brief timer object structure */
typedef struct _ieee11073_timer_object
{
    int32_t timerCount;                     /*!< Time out value in seconds */
    ieee11073_timer_callback timerCallback; /*!< Callback function */
    void *timerArgument;                    /*!< Callback function argument */
} ieee11073_timer_object_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*! @brief global function prototypes */
#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief timer initialization.
 *
 * This function initializes the timer object queue and system clock counter.
 *
 * @param controller_ID     the identify of timer controller.
 *
 * @retval success of error.
 */
extern void IEEE_TimerInit(void);

/*!
 * @brief add timer queue.
 *
 * This function is called to add timer object to timer queue.
 *
 * @param pTimerObject      timer object.
 *
 * @retval success of error.
 */
extern uint8_t IEEE_AddTimerQueue(ieee11073_timer_object_t *timerObject);

/*!
 * @brief remove timer queue.
 *
 * This function is called to remove timer object from timer queue.
 *
 * @param pTimerObject      index of timer object in queue.
 *
 * @retval success of error.
 */
extern void IEEE_RemoveTimerQueue(uint8_t timerIndex);
#if defined(__cplusplus)
}
#endif
#endif
#endif
/* _IEEE11073_TIMER_H_ */
