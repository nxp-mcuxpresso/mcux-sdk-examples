/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STATIC_QUEUE_H_
#define STATIC_QUEUE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>

/*!
 * @brief Base class which implements static queue as ring buffer that operates on data type void*.
 */

typedef struct static_queue
{
    /*! @brief Capacity of queue. */
    uint32_t capacity;
    /*! @brief Pointer to data of queue. */
    void *data;
    /*! @brief Index to free slot. */
    uint32_t head;
    /*! @brief Index to slot with data. */
    uint32_t tail;
    /*! @brief Size of one element. */
    uint32_t elementSize;
} static_queue_t;

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
bool static_queue_init(static_queue_t *sq, uint8_t *buffer, uint32_t maxCapacity, uint32_t elementSize);

/*!
 * @brief Destructor of BaseStaticQueue class.
 *
 * This function frees allocated buffer for data.
 *
 * @param[in] sq Pointer to structure of static queue.
 */
void static_queue_deinit(static_queue_t *sq);

/*!
 * @brief This function adds element to queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 * @param[in] element Pointer to element for adding.
 *
 * @return true Element was added.
 * @return false Element was not added, queue is full.
 */
bool static_queue_add(static_queue_t *sq, void *element);

/*!
 * @brief This function returns pointer to element from queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 *
 * @return void* Pointer to element.
 */
void *static_queue_get(static_queue_t *sq);

/*!
 * @brief This function returns number of elements in queue.
 *
 * @param[in] sq Pointer to structure of static queue.
 *
 * @return uint32_t Number of elements in queue.
 */
uint32_t static_queue_size(static_queue_t *sq);

#if defined(__cplusplus)
}
#endif

#endif /* STATIC_QUEUE_H_ */
