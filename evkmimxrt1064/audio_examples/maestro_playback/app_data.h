/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_DATA_H_
#define _APP_DATA_H_

#include <stdint.h>
#include <streamer_element_properties.h>

#ifdef CPU_LPC55S69JBD100_cm33_core0
#define EAP_MAX_PRESET 6
#else
#define EAP_MAX_PRESET 10
#endif

typedef struct _app_data
{
    int lastXOOperatingMode; // buffer for Crossover enable/disable request handling
    int lastPreset;          // buffer for last active preset selection
    int logEnabled;          // enable log to increase debug verbosity
    ext_proc_args eap_args;
} app_data_t;

app_data_t *get_app_data();

#endif
