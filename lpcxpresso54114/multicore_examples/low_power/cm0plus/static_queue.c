/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "static_queue.h"

/*!
 * @brief This function initializes queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 * @param[in] buffer Buffer used for storing elements.
 * @param[in] maxCapacity Capacity of queue.
 * @param[in] elementSize Size of one element in bytes.
 *
 * @return true The initialization was successful.
 * @return false The initialization was not successful.
 */
bool static_queue_init(static_queue_t *sq, uint8_t *buffer, uint32_t maxCapacity, uint32_t elementSize)
{
    if ((sq == NULL) || (buffer == NULL))
    {
        return false;
    }

    sq->data        = NULL;
    sq->elementSize = elementSize;
    sq->capacity    = maxCapacity;
    sq->head        = 0;
    sq->tail        = 0;

    sq->data = (void *)buffer;
    return true;
}

/*!
 * @brief Destructor of BaseStaticQueue class.
 *
 * This function frees allocated buffer for data.
 *
 * @param[in] sq Pointer to structure of static queue.
 */
void static_queue_deinit(static_queue_t *sq)
{
    free(sq->data);
}

/*!
 * @brief This function adds element to queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 * @param[in] element Pointer to element for adding.
 *
 * @return true Element was added.
 * @return false Element was not added, queue is full.
 */
bool static_queue_add(static_queue_t *sq, void *element)
{
    if ((sq->head + 1U) % sq->capacity != sq->tail)
    {
        (void)memcpy((void *)((uint32_t)sq->data + sq->head * sq->elementSize), element, sq->elementSize);
        sq->head = (sq->head + 1U) % sq->capacity;
        return true;
    }
    return false;
}

/*!
 * @brief This function returns pointer to element from queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 *
 * @return void* Pointer to element.
 */
void *static_queue_get(static_queue_t *sq)
{
    void *element = NULL;
    if (sq->tail != sq->head)
    {
        element  = (void *)((uint32_t)sq->data + sq->tail * sq->elementSize);
        sq->tail = (sq->tail + 1U) % sq->capacity;
    }
    return element;
}

/*!
 * @brief This function returns number of elements in queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 *
 * @return uint32_t Number of elements in queue.
 */
uint32_t static_queue_size(static_queue_t *sq)
{
    if (sq->head >= sq->tail)
    {
        return (sq->head - sq->tail);
    }
    return (sq->capacity - sq->tail + sq->head);
}
