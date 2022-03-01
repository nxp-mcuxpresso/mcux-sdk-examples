/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* This is a small demo of the high-performance GUIX graphics framework. */

#include <stdio.h>
#include "gx_api.h"

#include "sample_guix_washing_machine_resources.h"
#include "sample_guix_washing_machine_specifications.h"

#define ID_RADIAL_SLIDER_WASHER_ON 0xff1
#define POWER_ON                   1
#define POWER_OFF                  2

#define RADIAL_SLIDER_WIDTH                 147
#define RADIAL_SLIDER_HEIGHT                146
#define RADIAL_SLIDER_TRACK_WIDTH           24
#define RADIAL_SLIDER_RADIUS                60
#define TEMPERATURE_WINDOW_LONG_LINE_WIDTH  35
#define TEMPERATURE_WINDOW_SHORT_LINE_WIDTH 8

VOID washer_on_page_init();
VOID washer_on_page_power_off();
VOID washer_mode_radial_slider_create();
VOID garments_page_init();
void garments_page_power_off();
VOID garments_mode_radial_slider_create();
VOID water_level_page_init();
VOID water_level_page_power_off();
VOID temperature_page_init();
VOID temperature_page_power_off();
VOID temperature_radial_slider_create();
VOID widget_enable_disable(GX_WIDGET *widget, INT status);
void memory_free(VOID *mem);
UINT string_length_get(GX_CONST GX_CHAR *input_string, UINT max_string_length);
