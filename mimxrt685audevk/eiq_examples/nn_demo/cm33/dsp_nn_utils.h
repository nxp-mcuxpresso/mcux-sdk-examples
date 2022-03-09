/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __hifi_FIRMWARE_H__
#define __hifi_FIRMWARE_H__

/*!
 * @brief Send an echo command to hifi.
 * This is useful to measure the overhead.
 */
unsigned int xa_nn_echo();

/*!
 * @brief Allocate memory within the firmware.
 * @param size          the size to be allocated
 *
 * @retval              a pointer to the newly allocated chunk or NULL if the
 *                      chunk couldn't be allocated
 */
void *xa_nn_malloc(unsigned int size);

/*!
 * @brief Free memory within the firmware.
 * @param p             pointer of the memory chunk to be freed
 */
void xa_nn_free(void *p);

#endif // __hifi_FIRMWARE_H__
