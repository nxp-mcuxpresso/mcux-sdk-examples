/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DFU_TIMER_H_
#define _DFU_TIMER_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief max timer object support */
#ifndef DFU_MAX_TIMER_OBJECTS
#define DFU_MAX_TIMER_OBJECTS 1U
#endif
#if DFU_MAX_TIMER_OBJECTS

/*! @brief timer callback function prototype */
typedef void (*dfu_timer_callback)(void);

/*! @brief timer object structure */
typedef struct _dfu_timer_object
{
    int32_t timerCount;               /*!< Time out value in milliseconds */
    dfu_timer_callback timerCallback; /*!< Callback function */
} dfu_timer_object_t;

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
 *
 * @retval success of error.
 */
extern void DFU_TimerInit(void);
/*!
 * @brief timer initialization.
 *
 * This function initializes the timer' hardware.
 *
 *
 * @retval success of error.
 */
extern void DFU_TimerHWInit(void);
/*!
 * @brief hardware timer control.
 *
 * This function enable or disable the timer' irq.
 *
 *
 * @retval success of error.
 */
extern void HW_TimerControl(uint8_t enable);
/*!
 * @brief add timer queue.
 *
 * This function is called to add timer object to timer queue.
 *
 * @param pTimerObject      timer object.
 *
 * @retval success of error.
 */
extern uint8_t DFU_AddTimerQueue(dfu_timer_object_t *timerObject);

/*!
 * @brief remove timer queue.
 *
 * This function is called to remove timer object from timer queue.
 *
 * @param pTimerObject      index of timer object in queue.
 *
 * @retval success of error.
 */
extern void DFU_RemoveTimerQueue(uint8_t timerId);
/*!
 * @brief timer interrupt service function.
 *
 * This function services programmable interrupt timer when a timer object
 * expired, then removes the timer object from timer queue and calls to the
 * callback function (if registered).
 */
extern void DFU_TimerISR(void);
#if defined(__cplusplus)
}
#endif
#endif
#endif
/* _DFU_TIMER_H_ */
