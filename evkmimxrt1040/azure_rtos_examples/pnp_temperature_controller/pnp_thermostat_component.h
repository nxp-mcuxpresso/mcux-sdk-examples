/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation 
 * 
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 * 
 * SPDX-License-Identifier: MIT
 **************************************************************************/

#ifndef SAMPLE_PNP_THERMOSTAT_COMPONENT_H
#define SAMPLE_PNP_THERMOSTAT_COMPONENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "azure/core/az_json.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_api.h"

typedef struct SAMPLE_PNP_THERMOSTAT_COMPONENT_TAG
{
    /* Name of this component */
    UCHAR *component_name_ptr;

    UINT component_name_length;

    /* Current temperature of this thermostat component */
    double currentTemperature;

    /* Minimum temperature this thermostat has been at during current execution run of this thermostat component */
    double minTemperature;

    /* Maximum temperature thermostat has been at during current execution run of this thermostat component */
    double maxTemperature;

    /* Number of times temperature has been updated, counting the initial setting as 1.  Used to determine average temperature of this thermostat component */
    UINT numTemperatureUpdates;

    /* Total of all temperature updates during current exceution run.  Used to determine average temperature of this thermostat component */
    double allTemperatures;

    double avgTemperature;
} SAMPLE_PNP_THERMOSTAT_COMPONENT;

UINT sample_pnp_thermostat_init(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                UCHAR *component_name_ptr, UINT component_name_length,
                                double default_temp);

UINT sample_pnp_thermostat_process_command(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                           UCHAR *component_name_ptr, UINT component_name_length,
                                           UCHAR *pnp_command_name_ptr, UINT pnp_command_name_length,
                                           NX_PACKET *packet_ptr, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                           VOID *context_ptr, USHORT context_length);

UINT sample_pnp_thermostat_telemetry_send(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);

UINT sample_pnp_thermostat_report_max_temp_since_last_reboot_property(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                                                      NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr);


UINT sample_pnp_thermostat_process_property_update(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                                   NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                   UCHAR *component_name_ptr, UINT component_name_length,
                                                   UCHAR *property_name_ptr, UINT property_name_length,
                                                   az_json_reader *property_value_reader_ptr, UINT version);

#ifdef __cplusplus
}
#endif
#endif /* SAMPLE_PNP_THERMOSTAT_COMPONENT_H */
