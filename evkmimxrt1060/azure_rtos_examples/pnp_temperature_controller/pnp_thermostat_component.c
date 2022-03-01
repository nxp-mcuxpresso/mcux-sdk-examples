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

#include "pnp_thermostat_component.h"

#include "nx_azure_iot_pnp_helpers.h"
#include "fsl_debug_console.h"

#define SAMPLE_DEAFULT_START_TEMP_CELSIUS                               (22)
#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)
#define SAMPLE_COMMAND_SUCCESS_STATUS                                   (200)
#define SAMPLE_COMMAND_ERROR_STATUS                                     (500)

/* Telemetry key */
static const az_span telemetry_name = AZ_SPAN_LITERAL_FROM_STR("temperature");

/* Pnp command supported */
static const CHAR get_max_min_report[] = "getMaxMinReport";

/* Names of properties for desired/reporting */
static const az_span reported_max_temp_since_last_reboot = AZ_SPAN_LITERAL_FROM_STR("maxTempSinceLastReboot");
static const az_span report_max_temp_name_span = AZ_SPAN_LITERAL_FROM_STR("maxTemp");
static const az_span report_min_temp_name_span = AZ_SPAN_LITERAL_FROM_STR("minTemp");
static const az_span report_avg_temp_name_span = AZ_SPAN_LITERAL_FROM_STR("avgTemp");
static const az_span report_start_time_name_span = AZ_SPAN_LITERAL_FROM_STR("startTime");
static const az_span report_end_time_name_span = AZ_SPAN_LITERAL_FROM_STR("endTime");
static const CHAR target_temp_property_name[] = "targetTemperature";
static const CHAR temp_response_description_success[] = "success";
static const CHAR temp_response_description_failed[] = "failed";

/* Fake device data */
static const az_span fake_start_report_time = AZ_SPAN_LITERAL_FROM_STR("2020-01-10T10:00:00Z");
static const az_span fake_end_report_time = AZ_SPAN_LITERAL_FROM_STR("2023-01-10T10:00:00Z");

static UCHAR scratch_buffer[256];

/* sample direct method implementation */
static UINT sample_get_maxmin_report(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                     NX_PACKET *packet_ptr, UCHAR *buffer,
                                     UINT buffer_size, UINT *bytes_copied)
{
    UINT status;
    az_json_writer json_writer;
    az_json_reader jp;
    az_span response = az_span_create(buffer, (INT)buffer_size);
    az_span start_time_span = fake_start_report_time;
    az_span payload_span = az_span_create(packet_ptr -> nx_packet_prepend_ptr,
                                      (INT)(packet_ptr -> nx_packet_append_ptr -
                                            packet_ptr -> nx_packet_prepend_ptr));
    INT time_len;
    UCHAR time_buf[32];

    if (az_span_size(payload_span) != 0)
    {
        if (!(az_result_succeeded(az_json_reader_init(&jp, payload_span, NX_NULL)) &&
              az_result_succeeded(az_json_reader_next_token(&jp)) &&
              az_result_succeeded(az_json_token_get_string(&jp.token, (CHAR *)time_buf, sizeof(time_buf), (int32_t *)&time_len))))
        {
             return(NX_NOT_SUCCESSFUL);
        }

        start_time_span = az_span_create(time_buf, time_len);
    }

    /* Build the method response payload */
    if (az_result_succeeded(az_json_writer_init(&json_writer, response, NULL)) &&
        az_result_succeeded(az_json_writer_append_begin_object(&json_writer)) &&
        az_result_succeeded(az_json_writer_append_property_name(&json_writer, report_max_temp_name_span)) &&
        az_result_succeeded(az_json_writer_append_double(&json_writer, handle -> maxTemperature, DOUBLE_DECIMAL_PLACE_DIGITS)) &&
        az_result_succeeded(az_json_writer_append_property_name(&json_writer, report_min_temp_name_span)) &&
        az_result_succeeded(az_json_writer_append_double(&json_writer, handle -> minTemperature, DOUBLE_DECIMAL_PLACE_DIGITS)) &&
        az_result_succeeded(az_json_writer_append_property_name(&json_writer, report_avg_temp_name_span)) &&
        az_result_succeeded(az_json_writer_append_double(&json_writer, handle -> avgTemperature, DOUBLE_DECIMAL_PLACE_DIGITS)) &&
        az_result_succeeded(az_json_writer_append_property_name(&json_writer, report_start_time_name_span)) &&
        az_result_succeeded(az_json_writer_append_string(&json_writer, start_time_span)) &&
        az_result_succeeded(az_json_writer_append_property_name(&json_writer, report_end_time_name_span)) &&
        az_result_succeeded(az_json_writer_append_string(&json_writer, fake_end_report_time)) &&
        az_result_succeeded(az_json_writer_append_end_object(&json_writer)))
    {
        status = NX_AZURE_IOT_SUCCESS;
        *bytes_copied = (UINT)az_span_size(az_json_writer_get_bytes_used_in_destination(&json_writer));
    }
    else
    {
        status = NX_NOT_SUCCESSFUL;
    }

    return(status);
}

static UINT append_temp(az_json_writer *json_writer, VOID *context)
{
    double temp = *(double *)context;
    UINT status;

    if (az_result_succeeded(az_json_writer_append_double(json_writer, temp, DOUBLE_DECIMAL_PLACE_DIGITS)))
    {
        status = NX_AZURE_IOT_SUCCESS;
    }
    else
    {
        status = NX_NOT_SUCCESSFUL;
    }

    return(status);
}

static UINT append_max_temp(az_json_writer *json_writer, VOID *context)
{
    SAMPLE_PNP_THERMOSTAT_COMPONENT *handle = (SAMPLE_PNP_THERMOSTAT_COMPONENT *)context;
    UINT status;

    if (az_result_succeeded(az_json_writer_append_property_name(json_writer, reported_max_temp_since_last_reboot)) &&
        az_result_succeeded(az_json_writer_append_double(json_writer, (INT)handle -> maxTemperature, DOUBLE_DECIMAL_PLACE_DIGITS)))
    {
        status = NX_AZURE_IOT_SUCCESS;
    }
    else
    {
        status = NX_NOT_SUCCESSFUL;
    }

    return(status);
}

static VOID sample_send_target_temperature_report(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, double temp,
                                                  INT status_code, UINT version, const CHAR *description)
{
    UINT bytes_copied;
    UINT response_status;
    UINT request_id;
    ULONG reported_property_version;

    if (nx_azure_iot_pnp_helper_build_reported_property_with_status(handle -> component_name_ptr, handle -> component_name_length,
                                                                    (UCHAR *)target_temp_property_name,
                                                                    sizeof(target_temp_property_name) - 1,
                                                                    append_temp, (VOID *)&temp, status_code,
                                                                    (UCHAR *)description,
                                                                    strlen(description), version, scratch_buffer,
                                                                    sizeof(scratch_buffer),
                                                                    &bytes_copied))
    {
        PRINTF("Failed to create reported response\r\n");
    }
    else
    {
        if (nx_azure_iot_hub_client_device_twin_reported_properties_send(iothub_client_ptr,
                                                                         scratch_buffer, bytes_copied,
                                                                         &request_id, &response_status,
				                                                                 &reported_property_version,
                                                                         (5 * NX_IP_PERIODIC_RATE)))
        {
            PRINTF("Failed to send reported response\r\n");
        }
    }
}

UINT sample_pnp_thermostat_init(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                UCHAR *component_name_ptr, UINT component_name_length,
                                double default_temp)
{
    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    handle -> component_name_ptr = component_name_ptr;
    handle -> component_name_length = component_name_length;
    handle -> currentTemperature = default_temp;
    handle -> minTemperature = default_temp;
    handle -> maxTemperature = default_temp;
    handle -> allTemperatures = default_temp;
    handle -> numTemperatureUpdates = 1;
    handle -> avgTemperature = default_temp;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_thermostat_process_command(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                           UCHAR *component_name_ptr, UINT component_name_length,
                                           UCHAR *pnp_command_name_ptr, UINT pnp_command_name_length,
                                           NX_PACKET *packet_ptr, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                           VOID *context_ptr, USHORT context_length)
{
    UINT status;
    UINT response_payload_len = 0;
    UINT dm_status;

    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (handle -> component_name_length != component_name_length ||
        strncmp((CHAR *)handle -> component_name_ptr, (CHAR *)component_name_ptr, component_name_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (pnp_command_name_length != (sizeof(get_max_min_report) - 1) ||
        strncmp((CHAR *)pnp_command_name_ptr, (CHAR *)get_max_min_report, pnp_command_name_length) != 0)
    {
        PRINTF("PnP command=%.*s is not supported on thermostat component\r\n", pnp_command_name_length, pnp_command_name_ptr);
        dm_status = 404;
    }
    else
    {
        dm_status = (sample_get_maxmin_report(handle, packet_ptr, scratch_buffer, sizeof(scratch_buffer),
                                              &response_payload_len) != NX_AZURE_IOT_SUCCESS) ? SAMPLE_COMMAND_ERROR_STATUS :
                                                                                                SAMPLE_COMMAND_SUCCESS_STATUS;
    }

    if ((status = nx_azure_iot_hub_client_direct_method_message_response(iothub_client_ptr, dm_status,
                                                                         context_ptr, context_length, scratch_buffer,
                                                                         response_payload_len, NX_WAIT_FOREVER)))
    {
        PRINTF("Direct method response failed!: error code = 0x%08x\r\n", status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT sample_pnp_thermostat_telemetry_send(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
    UINT status;
    NX_PACKET *packet_ptr;
    az_json_writer json_writer;
    UINT buffer_length;

    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Create a telemetry message packet. */
    if ((status = nx_azure_iot_pnp_helper_telemetry_message_create(iothub_client_ptr, handle -> component_name_ptr,
                                                                   handle -> component_name_length,
                                                                   &packet_ptr, NX_WAIT_FOREVER)))
    {
        PRINTF("Telemetry message create failed!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Build telemetry JSON payload */
    if(!(az_result_succeeded(az_json_writer_init(&json_writer, AZ_SPAN_FROM_BUFFER(scratch_buffer), NULL)) &&
         az_result_succeeded(az_json_writer_append_begin_object(&json_writer)) &&
         az_result_succeeded(az_json_writer_append_property_name(&json_writer, telemetry_name)) &&
         az_result_succeeded(az_json_writer_append_double(&json_writer, handle -> currentTemperature, DOUBLE_DECIMAL_PLACE_DIGITS)) &&
         az_result_succeeded(az_json_writer_append_end_object(&json_writer))))
    {
        PRINTF("Telemetry message failed to build message\r\n");
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    buffer_length = (UINT)az_span_size(az_json_writer_get_bytes_used_in_destination(&json_writer));
    if ((status = nx_azure_iot_hub_client_telemetry_send(iothub_client_ptr, packet_ptr,
                                                         (UCHAR *)scratch_buffer, buffer_length, NX_WAIT_FOREVER)))
    {
        PRINTF("Telemetry message send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return(status);
    }

    PRINTF("Thermostat %.*s Telemetry message send: %.*s.\r\n", handle -> component_name_length,
           handle -> component_name_ptr, buffer_length, scratch_buffer);

    return(status);
}

UINT sample_pnp_thermostat_report_max_temp_since_last_reboot_property(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                                                      NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
    UINT reported_properties_length;
    UINT status;
    UINT response_status;
    UINT request_id;
    ULONG reported_property_version;
	

    if ((status = nx_azure_iot_pnp_helper_build_reported_property(handle -> component_name_ptr,
                                                                  handle -> component_name_length,
                                                                  append_max_temp, (VOID *)handle,
                                                                  (UCHAR *)scratch_buffer,
                                                                  sizeof(scratch_buffer),
                                                                  &reported_properties_length)))
    {
        PRINTF("Failed to build reported property!: error code = 0x%08x\r\n", status);
        return(status);
    }

    if ((status = nx_azure_iot_hub_client_device_twin_reported_properties_send(iothub_client_ptr,
                                                                               scratch_buffer,
                                                                               reported_properties_length,
                                                                               &request_id, &response_status,
		                                                                           &reported_property_version,
                                                                               (5 * NX_IP_PERIODIC_RATE))))
    {
        PRINTF("Device twin reported properties failed!: error code = 0x%08x\r\n", status);
        return(status);
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        PRINTF("device twin report properties failed with code : %d\r\n", response_status);
        return(NX_NOT_SUCCESSFUL);
    }

    return(status);
}

UINT sample_pnp_thermostat_process_property_update(SAMPLE_PNP_THERMOSTAT_COMPONENT *handle,
                                                   NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                   UCHAR *component_name_ptr, UINT component_name_length,
                                                   UCHAR *property_name_ptr, UINT property_name_length,
                                                   az_json_reader *property_value_reader_ptr, UINT version)
{
    double parsed_value = 0;
    INT status_code;
    const CHAR *description;

    if (handle == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (handle -> component_name_length != component_name_length ||
        strncmp((CHAR *)handle -> component_name_ptr, (CHAR *)component_name_ptr, component_name_length) != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    if (property_name_length != (sizeof(target_temp_property_name) - 1) ||
        strncmp((CHAR *)property_name_ptr, (CHAR *)target_temp_property_name, property_name_length) != 0)
    {
        PRINTF("PnP property=%.*s is not supported on thermostat component\r\n", property_name_length, property_name_ptr);
        status_code = 404;
        description = temp_response_description_failed;
    }
    else if (az_result_failed(az_json_token_get_double(&(property_value_reader_ptr -> token), &parsed_value)))
    {
        status_code = 401;
        description = temp_response_description_failed;
    }
    else
    {
        status_code = 200;
        description = temp_response_description_success;

        handle -> currentTemperature = parsed_value;
        if (handle -> currentTemperature > handle -> maxTemperature)
        {
            handle -> maxTemperature = handle -> currentTemperature;
        }

        if (handle -> currentTemperature < handle -> minTemperature)
        {
            handle -> minTemperature = handle -> currentTemperature;
        }

        /* Increment the avg count, add the new temp to the total, and calculate the new avg */
        handle -> numTemperatureUpdates++;
        handle -> allTemperatures += handle -> currentTemperature;
        handle -> avgTemperature = handle -> allTemperatures / handle -> numTemperatureUpdates;
    }

    sample_send_target_temperature_report(handle, iothub_client_ptr, parsed_value,
                                          status_code, version, description);

    return(NX_AZURE_IOT_SUCCESS);
}
