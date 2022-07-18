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

#include <stdio.h>

#include "fsl_debug_console.h"

#include "nx_api.h"
#include "nx_azure_iot_adu_agent.h"
#include "nx_azure_iot_provisioning_client.h"

/* These are sample files, user can build their own certificate and ciphersuites.  */
#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"
#include "sample_config.h"


/* Device properties.  */
#ifndef SAMPLE_DEVICE_MANUFACTURER
#define SAMPLE_DEVICE_MANUFACTURER                                      "NXP"
#endif /* SAMPLE_DEVICE_MANUFACTURER*/

#ifndef SAMPLE_DEVICE_MODEL
#define SAMPLE_DEVICE_MODEL                                             "MIMXRT1060"
#endif /* SAMPLE_DEVICE_MODEL */

/* Current update id.  */
#ifndef SAMPLE_UPDATE_ID_PROVIDER
#define SAMPLE_UPDATE_ID_PROVIDER                                       SAMPLE_DEVICE_MANUFACTURER
#endif /* SAMPLE_UPDATE_ID_PROVIDER */

#ifndef SAMPLE_UPDATE_ID_NAME
#define SAMPLE_UPDATE_ID_NAME                                           SAMPLE_DEVICE_MODEL
#endif /* SAMPLE_UPDATE_ID_NAME */

#ifndef SAMPLE_UPDATE_ID_VERSION
#define SAMPLE_UPDATE_ID_VERSION                                        "1.0.0"
#endif /* SAMPLE_UPDATE_ID_VERSION */

#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
/* Current update id for leaf device.  */
#ifndef SAMPLE_LEAF_UPDATE_ID_PROVIDER
#define SAMPLE_LEAF_UPDATE_ID_PROVIDER                                  SAMPLE_DEVICE_MANUFACTURER
#endif /* SAMPLE_LEAF_UPDATE_ID_PROVIDER*/

#ifndef SAMPLE_LEAF_UPDATE_ID_NAME
#define SAMPLE_LEAF_UPDATE_ID_NAME                                      "IoTDevice-Leaf"
#endif /* SAMPLE_LEAF_UPDATE_ID_NAME */
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

/* Define sample wait option.  */
#ifndef SAMPLE_WAIT_OPTION
#define SAMPLE_WAIT_OPTION                                              (NX_NO_WAIT)
#endif /* SAMPLE_WAIT_OPTION */

/* Define sample events.  */
#define SAMPLE_ALL_EVENTS                                               ((ULONG)0xFFFFFFFF)
#define SAMPLE_CONNECTED_EVENT                                          ((ULONG)0x00000001)
#define SAMPLE_DISCONNECT_EVENT                                         ((ULONG)0x00000002)
#define SAMPLE_PERIODIC_EVENT                                           ((ULONG)0x00000004)
#define SAMPLE_TELEMETRY_SEND_EVENT                                     ((ULONG)0x00000008)
#define SAMPLE_COMMAND_RECEIVE_EVENT                                    ((ULONG)0x00000010)
#define SAMPLE_PROPERTIES_RECEIVE_EVENT                                 ((ULONG)0x00000020)
#define SAMPLE_WRITABLE_PROPERTIES_RECEIVE_EVENT                        ((ULONG)0x00000040)
#define SAMPLE_REPORTED_PROPERTIES_SEND_EVENT                           ((ULONG)0x00000080)
#define SAMPLE_CLEANUP_EVENT                                            ((ULONG)0x00000100)


#define SAMPLE_DEAFULT_START_TEMP_CELSIUS                               (22)
#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)

#define SAMPLE_PNP_MODEL_ID                                             "dtmi:azure:iot:deviceUpdateModel;1"
#define SAMPLE_PNP_DPS_PAYLOAD                                          "{\"modelId\":\"" SAMPLE_PNP_MODEL_ID "\"}"


/* Generally, IoTHub Client and DPS Client do not run at the same time, user can use union as below to
   share the memory between IoTHub Client and DPS Client.

   NOTE: If user can not make sure sharing memory is safe, IoTHub Client and DPS Client must be defined seperately.  */
typedef union SAMPLE_CLIENT_UNION
{
    NX_AZURE_IOT_HUB_CLIENT                         iothub_client;

#ifdef ENABLE_DPS_SAMPLE
    NX_AZURE_IOT_PROVISIONING_CLIENT                prov_client;
#endif /* ENABLE_DPS_SAMPLE */

} SAMPLE_CLIENT;

static SAMPLE_CLIENT                                client;

#define iothub_client client.iothub_client
#ifdef ENABLE_DPS_SAMPLE
#define prov_client client.prov_client
#endif /* ENABLE_DPS_SAMPLE */

void sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

#ifdef ENABLE_DPS_SAMPLE
static UINT sample_dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                             UCHAR **iothub_hostname, UINT *iothub_hostname_length,
                             UCHAR **iothub_device_id, UINT *iothub_device_id_length);
#endif /* ENABLE_DPS_SAMPLE */

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static NX_SECURE_X509_CERT root_ca_cert_2;
static NX_SECURE_X509_CERT root_ca_cert_3;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

/* Using X509 certificate authenticate to connect to IoT Hub,
   set the device certificate as your device.  */
#if (USE_DEVICE_CERTIFICATE == 1)
extern const UCHAR sample_device_cert_ptr[];
extern const UINT sample_device_cert_len;
extern const UCHAR sample_device_private_key_ptr[];
extern const UINT sample_device_private_key_len;
NX_SECURE_X509_CERT device_certificate;
#endif /* USE_DEVICE_CERTIFICATE */

/* Define buffer for IoTHub info.  */
#ifdef ENABLE_DPS_SAMPLE
static UCHAR sample_iothub_hostname[SAMPLE_MAX_BUFFER];
static UCHAR sample_iothub_device_id[SAMPLE_MAX_BUFFER];
#endif /* ENABLE_DPS_SAMPLE */

/* Define the prototypes for AZ IoT.  */
static NX_AZURE_IOT nx_azure_iot;

static TX_EVENT_FLAGS_GROUP sample_events;
static TX_TIMER sample_timer;
static volatile UINT sample_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
static volatile ULONG sample_periodic_counter = 0;

/* Telemetry.  */
static const CHAR telemetry_name[] = "Message ID: ";
static UCHAR scratch_buffer[256];
static UINT sample_telemetry_id = 0;

static NX_AZURE_IOT_ADU_AGENT adu_agent;
static UINT adu_agent_started = NX_FALSE;

extern void nx_azure_iot_adu_agent_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);
#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
extern void nx_azure_iot_adu_agent_proxy_driver(NX_AZURE_IOT_ADU_AGENT_DRIVER *driver_req_ptr);
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

/* Include the connection monitor function from sample_azure_iot_embedded_sdk_connect.c.  */
extern VOID sample_connection_monitor(NX_IP *ip_ptr, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, UINT connection_status,
                                      UINT (*iothub_init)(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr));

static void adu_agent_update_notify(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                    UINT update_state,
                                    UCHAR *provider, UINT provider_length,
                                    UCHAR *name, UINT name_length,
                                    UCHAR *version, UINT version_length)
{

    if (update_state == NX_AZURE_IOT_ADU_AGENT_UPDATE_RECEIVED)
    {

        /* Received new update.  */
        PRINTF("Received new update: Provider: %.*s; Name: %.*s, Version: %.*s\r\n",
               provider_length, provider, name_length, name, version_length, version);

        /* Stop timer. */
        tx_timer_deactivate(&sample_timer);

        /* Start to download and install update immediately for testing.  */
        nx_azure_iot_adu_agent_update_download_install(adu_agent_ptr);
    }
    else if(update_state == NX_AZURE_IOT_ADU_AGENT_UPDATE_INSTALLED)
    {

        /* Prepare for reboot. */
        tx_event_flags_set(&sample_events, SAMPLE_CLEANUP_EVENT, TX_OR);
        tx_thread_sleep(NX_IP_PERIODIC_RATE);

        /* Start to apply update immediately for testing.  */
        nx_azure_iot_adu_agent_update_apply(adu_agent_ptr);
    }
}

static void sample_writable_properties_receive_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    UINT status = 0;
    NX_PACKET *packet_ptr;

    /* Receive writable properties.  */
    if ((status = nx_azure_iot_hub_client_writable_properties_receive(hub_client_ptr,
                                                                      &packet_ptr,
                                                                      NX_NO_WAIT)))
    {
        PRINTF("Receive writable property receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Call nx_azure_iot_hub_client_properties_component_property_next_get to process properties.  */

    nx_packet_release(packet_ptr);
}

static void sample_properties_receive_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    UINT status = 0;
    NX_PACKET *packet_ptr;

    /* Receive full properties.  */
    if ((status = nx_azure_iot_hub_client_properties_receive(hub_client_ptr,
                                                             &packet_ptr,
                                                             NX_NO_WAIT)))
    {
        PRINTF("Get all properties receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Call nx_azure_iot_hub_client_properties_component_property_next_get to process properties.  */

    nx_packet_release(packet_ptr);
}

/* Send telemetry message.  */
static void sample_telemetry_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    UINT status = 0;
    NX_PACKET *packet_ptr;
    NX_AZURE_IOT_JSON_WRITER json_writer;
    UINT buffer_length;

    /* Create a telemetry message packet.  */
    if ((status = nx_azure_iot_hub_client_telemetry_message_create(hub_client_ptr,
                                                                   &packet_ptr,
                                                                   NX_WAIT_FOREVER)))
    {
        PRINTF("Telemetry message create failed!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Build telemetry JSON payload.  */
    if(nx_azure_iot_json_writer_with_buffer_init(&json_writer, scratch_buffer, sizeof(scratch_buffer)))
    {
        PRINTF("Telemetry message failed to build message\r\n");
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return;
    }

    if (nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
        nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                   (UCHAR *)telemetry_name,
                                                                   sizeof(telemetry_name) - 1,
                                                                   (int32_t)sample_telemetry_id++) ||
         nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        PRINTF("Telemetry message failed to build message\r\n");
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return;
    }

    buffer_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    if ((status = nx_azure_iot_hub_client_telemetry_send(hub_client_ptr, packet_ptr,
                                                         (UCHAR *)scratch_buffer, buffer_length,
                                                         SAMPLE_WAIT_OPTION)))
    {
        PRINTF("Telemetry message send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return;
    }

    PRINTF("Telemetry message send: %.*s.\r\n", buffer_length, scratch_buffer);
}

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT status)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    if (status)
    {
        PRINTF("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
        tx_event_flags_set(&sample_events, SAMPLE_DISCONNECT_EVENT, TX_OR);
    }
    else
    {
        PRINTF("Connected to IoTHub.\r\n");
        tx_event_flags_set(&sample_events, SAMPLE_CONNECTED_EVENT, TX_OR);
    }

    sample_connection_status = status;
}

static VOID message_receive_callback_writable_properties(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(sample_events), SAMPLE_WRITABLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(sample_events), SAMPLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static UINT sample_initialize_iothub(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
    UINT status;
    UCHAR *iothub_hostname = NX_NULL;
    UCHAR *iothub_device_id = NX_NULL;
    UINT iothub_hostname_length = 0;
    UINT iothub_device_id_length = 0;

#ifdef ENABLE_DPS_SAMPLE

    /* Run DPS.  */
    if ((status = sample_dps_entry(&prov_client, &iothub_hostname, &iothub_hostname_length,
                                   &iothub_device_id, &iothub_device_id_length)))
    {
        PRINTF("Failed on sample_dps_entry!: error code = 0x%08x\r\n", status);
        return(status);
    }
#endif /* ENABLE_DPS_SAMPLE */

    if (iothub_hostname == NX_NULL || iothub_device_id == NX_NULL)
    {
        if (strlen(HOST_NAME) == 0 || strlen(DEVICE_ID) == 0)
        {
            PRINTF("Invalid macros, HOST_NAME, DEVICE_ID\r\n");
            return NX_INVALID_PARAMETERS;
        }

        iothub_hostname = (UCHAR *)HOST_NAME;
        iothub_device_id = (UCHAR *)DEVICE_ID;
        iothub_hostname_length = strlen(HOST_NAME);
        iothub_device_id_length = strlen(DEVICE_ID);
    }

    PRINTF("IoTHub Host Name: %.*s; Device ID: %.*s.\r\n",
           iothub_hostname_length, iothub_hostname, iothub_device_id_length, iothub_device_id);

    /* Initialize IoTHub client.  */
    if ((status = nx_azure_iot_hub_client_initialize(iothub_client_ptr, &nx_azure_iot,
                                                     iothub_hostname, iothub_hostname_length,
                                                     iothub_device_id, iothub_device_id_length,
                                                     (const UCHAR *)MODULE_ID, sizeof(MODULE_ID) - 1,
                                                     _nx_azure_iot_tls_supported_crypto,
                                                     _nx_azure_iot_tls_supported_crypto_size,
                                                     _nx_azure_iot_tls_ciphersuite_map,
                                                     _nx_azure_iot_tls_ciphersuite_map_size,
                                                     nx_azure_iot_tls_metadata_buffer,
                                                     sizeof(nx_azure_iot_tls_metadata_buffer),
                                                     &root_ca_cert)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Set the model id.  */
    if ((status = nx_azure_iot_hub_client_model_id_set(iothub_client_ptr,
                                                       (const UCHAR *)SAMPLE_PNP_MODEL_ID,
                                                       sizeof(SAMPLE_PNP_MODEL_ID) - 1)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_model_id_set!: error code = 0x%08x\r\n", status);
    }
    
    /* Add more CA certificates.  */
    else if ((status = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &root_ca_cert_2)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &root_ca_cert_3)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Initialize the device certificate.  */
    else if ((status = nx_secure_x509_certificate_initialize(&device_certificate,
                                                             (UCHAR *)sample_device_cert_ptr, (USHORT)sample_device_cert_len,
                                                             NX_NULL, 0,
                                                             (UCHAR *)sample_device_private_key_ptr, (USHORT)sample_device_private_key_len,
                                                             DEVICE_KEY_TYPE)))
    {
        PRINTF("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_hub_client_device_cert_set(iothub_client_ptr, &device_certificate)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    else if ((status = nx_azure_iot_hub_client_symmetric_key_set(iothub_client_ptr,
                                                                 (UCHAR *)DEVICE_SYMMETRIC_KEY,
                                                                 strlen(DEVICE_SYMMETRIC_KEY))))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
    }
#endif /* USE_DEVICE_CERTIFICATE */

    /* Enable command and properties features.  */
    else if ((status = nx_azure_iot_hub_client_command_enable(iothub_client_ptr)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_command_enable!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_properties_enable(iothub_client_ptr)))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", status);
    }

    /* Set connection status callback.  */
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(iothub_client_ptr,
                                                                              connection_status_callback)))
    {
        PRINTF("Failed on connection_status_callback!\r\n");
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_PROPERTIES,
                                                                    message_receive_callback_properties,
                                                                    NX_NULL)))
    {
        PRINTF("device properties callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                                                                    message_receive_callback_writable_properties,
                                                                    NX_NULL)))
    {
        PRINTF("device writable properties callback set!: error code = 0x%08x\r\n", status);
    }

    if (status)
    {
        nx_azure_iot_hub_client_deinitialize(iothub_client_ptr);
    }

    return(status);
}

#ifdef ENABLE_DPS_SAMPLE
static UINT sample_dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                             UCHAR **iothub_hostname, UINT *iothub_hostname_length,
                             UCHAR **iothub_device_id, UINT *iothub_device_id_length)
{
    UINT status;

    PRINTF("Start Provisioning Client...\r\n");

    /* Initialize IoT provisioning client.  */
    if ((status = nx_azure_iot_provisioning_client_initialize(prov_client_ptr, &nx_azure_iot,
                                                              (UCHAR *)ENDPOINT, sizeof(ENDPOINT) - 1,
                                                              (UCHAR *)ID_SCOPE, sizeof(ID_SCOPE) - 1,
                                                              (UCHAR *)REGISTRATION_ID, sizeof(REGISTRATION_ID) - 1,
                                                              _nx_azure_iot_tls_supported_crypto,
                                                              _nx_azure_iot_tls_supported_crypto_size,
                                                              _nx_azure_iot_tls_ciphersuite_map,
                                                              _nx_azure_iot_tls_ciphersuite_map_size,
                                                              nx_azure_iot_tls_metadata_buffer,
                                                              sizeof(nx_azure_iot_tls_metadata_buffer),
                                                              &root_ca_cert)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Initialize length of hostname and device ID.  */
    *iothub_hostname_length = sizeof(sample_iothub_hostname);
    *iothub_device_id_length = sizeof(sample_iothub_device_id);

    /* Add more CA certificates.  */
    if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(prov_client_ptr, &root_ca_cert_2)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(prov_client_ptr, &root_ca_cert_3)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Initialize the device certificate.  */
    else if ((status = nx_secure_x509_certificate_initialize(&device_certificate, (UCHAR *)sample_device_cert_ptr, (USHORT)sample_device_cert_len, NX_NULL, 0,
                                                             (UCHAR *)sample_device_private_key_ptr, (USHORT)sample_device_private_key_len, DEVICE_KEY_TYPE)))
    {
        PRINTF("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_provisioning_client_device_cert_set(prov_client_ptr, &device_certificate)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    else if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(prov_client_ptr, (UCHAR *)DEVICE_SYMMETRIC_KEY,
                                                                          strlen(DEVICE_SYMMETRIC_KEY))))
    {
        PRINTF("Failed on nx_azure_iot_hub_client_symmetric_key_set!: error code = 0x%08x\r\n", status);
    }
#endif /* USE_DEVICE_CERTIFICATE */
    else if ((status = nx_azure_iot_provisioning_client_registration_payload_set(prov_client_ptr, (UCHAR *)SAMPLE_PNP_DPS_PAYLOAD,
                                                                                 sizeof(SAMPLE_PNP_DPS_PAYLOAD) - 1)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_registration_payload_set!: error code = 0x%08x\r\n", status);
    }
    /* Register device.  */
    else if ((status = nx_azure_iot_provisioning_client_register(prov_client_ptr, NX_WAIT_FOREVER)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_register!: error code = 0x%08x\r\n", status);
    }

    /* Get Device info.  */
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(prov_client_ptr,
                                                                               sample_iothub_hostname, iothub_hostname_length,
                                                                               sample_iothub_device_id, iothub_device_id_length)))
    {
        PRINTF("Failed on nx_azure_iot_provisioning_client_iothub_device_info_get!: error code = 0x%08x\r\n", status);
    }
    else
    {
        *iothub_hostname = sample_iothub_hostname;
        *iothub_device_id = sample_iothub_device_id;
        PRINTF("Registered Device Successfully.\r\n");
    }

    /* Destroy Provisioning Client.  */
    nx_azure_iot_provisioning_client_deinitialize(prov_client_ptr);

    return(status);
}
#endif /* ENABLE_DPS_SAMPLE */

static VOID sample_connected_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    UINT status;

    if (adu_agent_started == NX_FALSE)
    {

        /* Start ADU agent.  */
        if (nx_azure_iot_adu_agent_start(&adu_agent, hub_client_ptr,
                                         (const UCHAR *)SAMPLE_DEVICE_MANUFACTURER, sizeof(SAMPLE_DEVICE_MANUFACTURER) - 1,
                                         (const UCHAR *)SAMPLE_DEVICE_MODEL, sizeof(SAMPLE_DEVICE_MODEL) - 1,
                                         (const UCHAR *)SAMPLE_UPDATE_ID_PROVIDER, sizeof(SAMPLE_UPDATE_ID_PROVIDER) - 1,
                                         (const UCHAR *)SAMPLE_UPDATE_ID_NAME, sizeof(SAMPLE_UPDATE_ID_NAME) - 1,
                                         (const UCHAR *)SAMPLE_UPDATE_ID_VERSION, sizeof(SAMPLE_UPDATE_ID_VERSION) - 1,
                                         adu_agent_update_notify,
                                         nx_azure_iot_adu_agent_driver))
        {
            PRINTF("Failed on nx_azure_iot_adu_agent_start!\r\n");
            return;
        }

#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
        /* Enable proxy update for leaf device.  */
        if (nx_azure_iot_adu_agent_proxy_update_add(&adu_agent,
                                                    (const UCHAR *)SAMPLE_LEAF_UPDATE_ID_PROVIDER, sizeof(SAMPLE_LEAF_UPDATE_ID_PROVIDER) - 1,
                                                    (const UCHAR *)SAMPLE_LEAF_UPDATE_ID_NAME, sizeof(SAMPLE_LEAF_UPDATE_ID_NAME) - 1,
                                                    NX_NULL, 0,
                                                    nx_azure_iot_adu_agent_proxy_driver))
        {
            PRINTF("Failed on nx_azure_iot_adu_agent_proxy_update_add!\r\n");
            return;
        }
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

        PRINTF("Device Properties: manufacturer: %s, model: %s\r\n", SAMPLE_DEVICE_MANUFACTURER, SAMPLE_DEVICE_MODEL);
        PRINTF("Installed Update ID: provider: %s, name: %s, version: %s\r\n", SAMPLE_UPDATE_ID_PROVIDER, SAMPLE_UPDATE_ID_NAME, SAMPLE_UPDATE_ID_VERSION);

        adu_agent_started = NX_TRUE;
    }

    /* Request all properties.  */
    if ((status = nx_azure_iot_hub_client_properties_request(hub_client_ptr, NX_WAIT_FOREVER)))
    {
        PRINTF("Properties request failed!: error code = 0x%08x\r\n", status);
    }
    else
    {
        PRINTF("Sent properties request.\r\n");
    }
}

static VOID sample_periodic_timer_entry(ULONG context)
{

    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(sample_events), SAMPLE_PERIODIC_EVENT, TX_OR);
}

static VOID sample_periodic_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    
    if ((sample_periodic_counter % 5) == 0)
    {

        /* Set telemetry send event and reported properties send event.  */
        tx_event_flags_set(&(sample_events), (SAMPLE_TELEMETRY_SEND_EVENT | SAMPLE_REPORTED_PROPERTIES_SEND_EVENT), TX_OR);
    }

    sample_periodic_counter++;
}

static void log_callback(az_log_classification classification, UCHAR *msg, UINT msg_len)
{
    if (classification == AZ_LOG_IOT_AZURERTOS)
    {
        PRINTF("%.*s", msg_len, (CHAR *)msg);
    }
}

void sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time))
{
    UINT status = 0;
    UINT loop = NX_TRUE;
    ULONG app_events;

    nx_azure_iot_log_init(log_callback);

    /* Create Azure IoT handler.  */
    if ((status = nx_azure_iot_create(&nx_azure_iot, (UCHAR *)"Azure IoT", ip_ptr, pool_ptr, dns_ptr,
                                      nx_azure_iot_thread_stack, sizeof(nx_azure_iot_thread_stack),
                                      NX_AZURE_IOT_THREAD_PRIORITY, unix_time_callback)))
    {
        PRINTF("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Initialize CA certificates.  */
    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert, (UCHAR *)_nx_azure_iot_root_cert,
                                                        (USHORT)_nx_azure_iot_root_cert_size,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        PRINTF("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert_2, (UCHAR *)_nx_azure_iot_root_cert_2,
                                                        (USHORT)_nx_azure_iot_root_cert_size_2,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        PRINTF("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert_3, (UCHAR *)_nx_azure_iot_root_cert_3,
                                                        (USHORT)_nx_azure_iot_root_cert_size_3,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        PRINTF("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    tx_timer_create(&(sample_timer), (CHAR*)"sample_app_timer", sample_periodic_timer_entry, 0,
                    NX_IP_PERIODIC_RATE, NX_IP_PERIODIC_RATE, TX_AUTO_ACTIVATE);
    tx_event_flags_create(&sample_events, (CHAR*)"sample_app_event");

    while (loop)
    {

        /* Pickup sample event flags.  */
        tx_event_flags_get(&(sample_events), SAMPLE_ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_WAIT_FOREVER);

        if (app_events & SAMPLE_CONNECTED_EVENT)
        {
            sample_connected_action(&iothub_client);
        }

        if (app_events & SAMPLE_PERIODIC_EVENT)
        {
            sample_periodic_action(&iothub_client);
        }

        if (app_events & SAMPLE_TELEMETRY_SEND_EVENT)
        {
            sample_telemetry_action(&iothub_client);
        }

        if (app_events & SAMPLE_PROPERTIES_RECEIVE_EVENT)
        {
            sample_properties_receive_action(&iothub_client);
        }

        if (app_events & SAMPLE_WRITABLE_PROPERTIES_RECEIVE_EVENT)
        {
            sample_writable_properties_receive_action(&iothub_client);
        }

        if (app_events & SAMPLE_CLEANUP_EVENT)
        {
            nx_azure_iot_hub_client_disconnect(&iothub_client);
            break;
        }

        /* Connection monitor.  */
        sample_connection_monitor(ip_ptr, &iothub_client, sample_connection_status, sample_initialize_iothub);
    }

    /* Cleanup.  */
    tx_event_flags_delete(&sample_events);
    tx_timer_delete(&sample_timer);
    nx_azure_iot_hub_client_deinitialize(&iothub_client);
    nx_azure_iot_delete(&nx_azure_iot);
}
