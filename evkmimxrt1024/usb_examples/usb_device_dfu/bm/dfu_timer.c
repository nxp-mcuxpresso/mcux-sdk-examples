/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <string.h>
#include "usb_device_config.h"
#include "usb.h"
#include "fsl_device_registers.h"
#include "dfu_timer.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief array of timer objects */
dfu_timer_object_t s_dfuTimerObjectArray[DFU_MAX_TIMER_OBJECTS];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief timer initialization.
 *
 * This function initializes the timer object queue and system clock counter.
 *
 * @param controller_ID     the identify of timer controller.
 *
 * @retval success of error.
 */
void DFU_TimerInit(void)
{
    /* Clear timer object array */
    (void)memset(s_dfuTimerObjectArray, 0U, sizeof(s_dfuTimerObjectArray));
    DFU_TimerHWInit();
}

/*!
 * @brief add timer queue.
 *
 * This function is called to add timer object to timer queue.
 *
 * @param timerObject       Timer object.
 *
 * @retval timerIndex       The timer queue is not full.
 * @retval -1               The input timer object is NULL.
                            The timer queue is full.
 */
uint8_t DFU_AddTimerQueue(dfu_timer_object_t *timerObject)
{
    uint8_t index;
    index = DFU_MAX_TIMER_OBJECTS;
    if (NULL != timerObject)
    {
        /* Timer Index return value */
        uint8_t timerId = 0U;
        /* Queue full checking */
        uint8_t isQueueFull = 1U;
        /* Disable the timer */
        HW_TimerControl(0U);
        /* Add timerObject to queue */
        for (timerId = 0U; timerId < DFU_MAX_TIMER_OBJECTS; timerId++)
        {
            if (s_dfuTimerObjectArray[timerId].timerCallback == NULL)
            {
                isQueueFull = 0U;
                (void)memcpy(&s_dfuTimerObjectArray[timerId], timerObject, sizeof(dfu_timer_object_t));
                break;
            }
        }
        if (isQueueFull)
        {
            /* Timer queue is full */
            index = DFU_MAX_TIMER_OBJECTS;
        }
        else
        {
            /* only enable the timer if queue is not full*/
            HW_TimerControl(1U);
            index = timerId;
        }
    }
    /* Invalid parameter */
    return index;
}

/*!
 * @brief remove timer queue.
 *
 * This function is called to remove timer object from timer queue.
 *
 * @param timerId      index of timer object in queue.
 */
void DFU_RemoveTimerQueue(uint8_t timerId)
{
    uint8_t i;

    if (timerId < DFU_MAX_TIMER_OBJECTS)
    {
        /* Disable the  timer */
        HW_TimerControl(0U);
        if (NULL != s_dfuTimerObjectArray[timerId].timerCallback)
        {
            /* Clear the time object in queue corresponding with timerId */
            (void)memset(&s_dfuTimerObjectArray[timerId], 0U, sizeof(dfu_timer_object_t));
            s_dfuTimerObjectArray[timerId].timerCallback = NULL;
        }
        /* Queue empty checking */
        for (i = 0U; i < DFU_MAX_TIMER_OBJECTS; i++)
        {
            if (NULL != s_dfuTimerObjectArray[i].timerCallback)
            {
                /* Queue is not empty, enable the timer again */
                HW_TimerControl(1U);
                break;
            }
        }
    }
}

/*!
 * @brief timer interrupt service function.
 *
 * This function services programmable interrupt timer when a timer object
 * expired, then removes the timer object from timer queue and calls to the
 * callback function (if registered).
 */
void DFU_TimerISR(void)
{
    uint8_t index;
    for (index = 0U; index < DFU_MAX_TIMER_OBJECTS; index++)
    {
        if (NULL != s_dfuTimerObjectArray[index].timerCallback)
        {
            dfu_timer_object_t *timerObject = &s_dfuTimerObjectArray[index];
            timerObject->timerCount--;
            if (timerObject->timerCount <= 0)
            {
                /* Call Pending Timer CallBacks */
                timerObject->timerCallback();
                /* remove timer object from timer queue */
                DFU_RemoveTimerQueue(index);
            }
        }
    }
}
