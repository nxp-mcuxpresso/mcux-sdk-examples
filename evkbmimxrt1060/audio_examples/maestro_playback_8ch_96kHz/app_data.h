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

#define EAP_MAX_PRESET 10

typedef struct _app_data
{
    int lastXOOperatingMode; // buffer for Crossover enable/disable request handling
    int lastPreset;          // buffer for last active preset selection
    int logEnabled;          // enable log to increase debug verbosity
    uint8_t num_channels;    // number of channels set with cli
    ext_proc_args eap_args;
} app_data_t;

app_data_t *get_app_data();

#endif
