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

#include "pnp_deviceinfo_component.h"

#include "nx_azure_iot_pnp_helpers.h"
#include "fsl_debug_console.h"

#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)

/* Reported property keys and values */
static const az_span sample_pnp_device_info_software_version_property_name = AZ_SPAN_LITERAL_FROM_STR("swVersion");
static const az_span sample_pnp_device_info_software_version_property_value = AZ_SPAN_LITERAL_FROM_STR("1.0.0.0");
static const az_span sample_pnp_device_info_manufacturer_property_name = AZ_SPAN_LITERAL_FROM_STR("manufacturer");
static const az_span sample_pnp_device_info_manufacturer_property_value = AZ_SPAN_LITERAL_FROM_STR("Sample-Manufacturer");
static const az_span sample_pnp_device_info_model_property_name = AZ_SPAN_LITERAL_FROM_STR("model");
static const az_span sample_pnp_device_info_model_property_value = AZ_SPAN_LITERAL_FROM_STR("pnp-sample-Model-123");
static const az_span sample_pnp_device_info_os_name_property_name = AZ_SPAN_LITERAL_FROM_STR("osName");
static const az_span sample_pnp_device_info_os_name_property_value = AZ_SPAN_LITERAL_FROM_STR("AzureRTOS");
static const az_span sample_pnp_device_info_processor_architecture_property_name = AZ_SPAN_LITERAL_FROM_STR("processorArchitecture");
static const az_span sample_pnp_device_info_processor_architecture_property_value = AZ_SPAN_LITERAL_FROM_STR("Contoso-Arch-64bit");
static const az_span sample_pnp_device_info_processor_manufacturer_property_name = AZ_SPAN_LITERAL_FROM_STR("processorManufacturer");
static const az_span sample_pnp_device_info_processor_manufacturer_property_value = AZ_SPAN_LITERAL_FROM_STR("Processor Manufacturer(TM)");
static const az_span sample_pnp_device_info_total_storage_property_name = AZ_SPAN_LITERAL_FROM_STR("totalStorage");
static const double sample_pnp_device_info_total_storage_property_value = 1024.0;
static const az_span sample_pnp_device_info_total_memory_property_name = AZ_SPAN_LITERAL_FROM_STR("totalMemory");
static const double sample_pnp_device_info_total_memory_property_value = 128;

static UCHAR scratch_buffer[512];

static UINT append_properties(az_json_writer *json_writer, VOID *context)
{
    UINT status;

    NX_PARAMETER_NOT_USED(context);

    if (az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_manufacturer_property_name)) &&
        az_result_succeeded(az_json_writer_append_string(json_writer, sample_pnp_device_info_manufacturer_property_value)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_model_property_name)) &&
        az_result_succeeded(az_json_writer_append_string(json_writer, sample_pnp_device_info_model_property_value)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_software_version_property_name)) &&
        az_result_succeeded(az_json_writer_append_string(json_writer, sample_pnp_device_info_software_version_property_value)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_os_name_property_name)) &&
        az_result_succeeded(az_json_writer_append_string(json_writer, sample_pnp_device_info_os_name_property_value)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_processor_architecture_property_name)) &&
        az_result_succeeded(az_json_writer_append_string(json_writer, sample_pnp_device_info_processor_architecture_property_value)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_processor_manufacturer_property_name)) &&
        az_result_succeeded(az_json_writer_append_string(json_writer, sample_pnp_device_info_processor_manufacturer_property_value)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_total_storage_property_name)) &&
        az_result_succeeded(az_json_writer_append_double(json_writer, sample_pnp_device_info_total_storage_property_value, DOUBLE_DECIMAL_PLACE_DIGITS)) &&
        az_result_succeeded(az_json_writer_append_property_name(json_writer, sample_pnp_device_info_total_memory_property_name)) &&
        az_result_succeeded(az_json_writer_append_double(json_writer, sample_pnp_device_info_total_memory_property_value, DOUBLE_DECIMAL_PLACE_DIGITS)))
    {
        status = NX_AZURE_IOT_SUCCESS;
    }
    else
    {
        status = NX_NOT_SUCCESSFUL;
    }

    return(status);
}

UINT sample_pnp_deviceinfo_report_all_properties(UCHAR *component_name_ptr, UINT component_name_len,
                                                 NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
    UINT reported_properties_length;
    UINT status;
    UINT response_status;
    UINT request_id;
    ULONG reported_property_version;

    if ((status = nx_azure_iot_pnp_helper_build_reported_property(component_name_ptr, component_name_len,
                                                                  append_properties, NX_NULL,
                                                                  (UCHAR *)scratch_buffer, sizeof(scratch_buffer),
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
