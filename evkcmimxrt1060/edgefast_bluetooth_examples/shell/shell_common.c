/** @file
 * @brief Bluetooth shell module
 *
 * Provides some common functions that can be used by other shell files.
 */

/*
 * Copyright 2024 NXP
 * All rights reserved.
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/byteorder.h>
#include <porting.h>

/* Microseconds from 01-Jan-0000 till epoch(in ISO 8601: 2021 - 04 - 30T00 : 00 : 00Z). */
#define SHELL_uS_TIMESTAMP_PSEUDO      0x00E29EE5C36CE000ULL
/**
 *  \fn SHELL_get_us_timestamp
 *
 *  \brief To get the microsecond timestamp
 *
 *  \par Description:
 *  This function implements the OS Wrapper to get system time in microseconds
 *
 *  \return System time in microseconds
 */
uint64_t SHELL_get_us_timestamp(void)
{
    uint64_t timestamp;
    timestamp = OSA_TimeGetMsec() * (uint64_t)1000UL;

    timestamp = timestamp + SHELL_uS_TIMESTAMP_PSEUDO;

    return timestamp;
}