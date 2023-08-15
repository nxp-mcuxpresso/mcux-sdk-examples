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

#include "fsl_debug_console.h"
#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nxd_mqtt_client.h"
#include "nxd_dns.h"
#include "nx_azure_iot_cert.h"

/* @TEST_ANCHOR */

#ifndef HOST_NAME
#define HOST_NAME ""
#endif /* HOST_NAME */

#ifndef DEVICE_ID
#define DEVICE_ID ""
#endif /* DEVICE_ID */

#ifndef DEVICE_SAS
#define DEVICE_SAS ""
#endif /* DEVICE_SAS */

#define USERNAME        "%s/%s"

#define PUBLISH_TOPIC   "devices/%s/messages/events/"
#define SUBSCRIBE_TOPIC "devices/%s/messages/devicebound/#"

#define NX_AZURE_IOT_HUB_CLIENT_WEB_SOCKET_PATH         "/$iothub/websocket"

/* Define the timeout for establishing TLS connection, default 40s. NX_IP_PERIODIC_RATE indicate one second.  */
#ifndef THREADX_TLS_WAIT_OPTION
#define THREADX_TLS_WAIT_OPTION (40 * NX_IP_PERIODIC_RATE)
#endif /* THREADX_TLS_WAIT_OPTION  */

/* Define the timeout for sending TLS packet, default 1s. NX_IP_PERIODIC_RATE indicate one second.  */
#ifndef THREADX_TLS_SEND_WAIT_OPTION
#define THREADX_TLS_SEND_WAIT_OPTION (NX_IP_PERIODIC_RATE)
#endif /* THREADX_TLS_SEND_WAIT_OPTION  */

/* Define the metadata size for THREADX TLS.  */
#ifndef THREADX_TLS_METADATA_BUFFER
#define THREADX_TLS_METADATA_BUFFER (16 * 1024)
#endif /* THREADX_TLS_METADATA_BUFFER  */

/* Define the remote certificate count for THREADX TLS.  */
#ifndef THREADX_TLS_REMOTE_CERTIFICATE_COUNT
#define THREADX_TLS_REMOTE_CERTIFICATE_COUNT        4
#endif /* THREADX_TLS_REMOTE_CERTIFICATE_COUNT  */

/* Define the remote certificate buffer for THREADX TLS.  */
#ifndef THREADX_TLS_REMOTE_CERTIFICATE_BUFFER
#define THREADX_TLS_REMOTE_CERTIFICATE_BUFFER       (1024 * 4)
#endif /* THREADX_TLS_REMOTE_CERTIFICATE_BUFFER  */

/* Define the packet buffer for THREADX TLS.  */
#ifndef THREADX_TLS_PACKET_BUFFER
#define THREADX_TLS_PACKET_BUFFER 4096
#endif /* THREADX_TLS_PACKET_BUFFER  */

#define APP_CERTIFICATE_COUNT      (3)

/* stucture containing the data and size of the certificate */
struct app_cert_info {
    const unsigned char *data;
    unsigned int size;
};

static struct app_cert_info root_certs[APP_CERTIFICATE_COUNT];
static NX_SECURE_X509_CERT root_ca_cert[APP_CERTIFICATE_COUNT];

static UCHAR threadx_tls_metadata_buffer[THREADX_TLS_METADATA_BUFFER];
static NX_SECURE_X509_CERT threadx_tls_remote_certificate[THREADX_TLS_REMOTE_CERTIFICATE_COUNT];
static UCHAR threadx_tls_remote_cert_buffer[THREADX_TLS_REMOTE_CERTIFICATE_COUNT]
                                           [THREADX_TLS_REMOTE_CERTIFICATE_BUFFER];

static UCHAR threadx_tls_packet_buffer[THREADX_TLS_PACKET_BUFFER];

/* Use the default ciphersuite defined in nx_secure/nx_crypto_generic_ciphersuites.c  */
extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers;

/* MQTT.  */
static NXD_MQTT_CLIENT mqtt_client_secure;
static UCHAR mqtt_client_stack[4096];
static UCHAR mqtt_topic_buffer[200];
static UINT mqtt_topic_length;
static UCHAR mqtt_message_buffer[200];
static UINT mqtt_message_length;

/* Fan info.  */
static UINT fan_on      = NX_FALSE;
static UINT temperature = 20;

/* Process command.  */
static VOID my_notify_func(NXD_MQTT_CLIENT *client_ptr, UINT number_of_messages)
{
    UINT status;

    /* Get the mqtt client message.  */
    status = nxd_mqtt_client_message_get(&mqtt_client_secure, mqtt_topic_buffer, sizeof(mqtt_topic_buffer),
                                         &mqtt_topic_length, mqtt_message_buffer, sizeof(mqtt_message_buffer),
                                         &mqtt_message_length);
    if (status == NXD_MQTT_SUCCESS)
    {
        mqtt_topic_buffer[mqtt_topic_length]     = 0;
        mqtt_message_buffer[mqtt_message_length] = 0;
        PRINTF("[Received]  topic = %s, message = %s\r\n", mqtt_topic_buffer, mqtt_message_buffer);

        if (strstr((CHAR *)mqtt_message_buffer, "fan"))
        {
            if (strstr((CHAR *)mqtt_message_buffer, "off"))
            {
                fan_on = NX_FALSE;
            }
            else if (strstr((CHAR *)mqtt_message_buffer, "on"))
            {
                fan_on = NX_TRUE;
            }
        }
    }

    return;
}

static void app_mqtt_tls_cert_init(NX_SECURE_TLS_SESSION *tls_session)
{
    UINT status;

    for (int i = 0; i < THREADX_TLS_REMOTE_CERTIFICATE_COUNT; i++)
    {
        /* Need to allocate space for the certificate coming in from the remote host. */
        nx_secure_tls_remote_certificate_allocate(tls_session, &threadx_tls_remote_certificate[i],
                                                  threadx_tls_remote_cert_buffer[i],
                                                  sizeof(threadx_tls_remote_cert_buffer[i]));
    }

    for (int i = 0; i < APP_CERTIFICATE_COUNT; i++)
    {
        /* Initialize CA certificates.  */
        status = nx_secure_x509_certificate_initialize(&root_ca_cert[i],
                                                       (UCHAR *)root_certs[i].data, (USHORT)root_certs[i].size,
                                                       NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE);
        if (status == NX_SECURE_X509_SUCCESS)
        {
            status = nx_secure_tls_trusted_certificate_add(tls_session, &root_ca_cert[i]);
        }

        if (status != NX_SECURE_X509_SUCCESS)
        {
            PRINTF("Failed to initialize ROOT CA certificate #%d!: error code = 0x%08x\r\n", i, status);
        }
    }
}

static UINT threadx_mqtt_tls_setup(NXD_MQTT_CLIENT *client_ptr,
                                   NX_SECURE_TLS_SESSION *tls_session,
                                   NX_SECURE_X509_CERT *certificate,
                                   NX_SECURE_X509_CERT *trusted_certificate)
{
    app_mqtt_tls_cert_init(tls_session);

    nx_secure_tls_session_packet_buffer_set(tls_session, threadx_tls_packet_buffer, sizeof(threadx_tls_packet_buffer));

    return (NX_SUCCESS);
}

VOID sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr)
{
    UINT status;
    UINT time_counter = 0;
    NXD_ADDRESS server_address;
    char mqtt_user_name[100];
    char mqtt_publish_topic[100];
    char mqtt_subscribe_topic[100];
    UINT server_port;

    root_certs[0].data = _nx_azure_iot_root_cert;
    root_certs[0].size = _nx_azure_iot_root_cert_size;
    root_certs[1].data = _nx_azure_iot_root_cert_2;
    root_certs[1].size = _nx_azure_iot_root_cert_size_2;
    root_certs[2].data = _nx_azure_iot_root_cert_3;
    root_certs[2].size = _nx_azure_iot_root_cert_size_3;

    /* Build string for NetX MQTT.  */
    sprintf(mqtt_user_name, USERNAME, HOST_NAME, DEVICE_ID);
    sprintf(mqtt_publish_topic, PUBLISH_TOPIC, DEVICE_ID);
    sprintf(mqtt_subscribe_topic, SUBSCRIBE_TOPIC, DEVICE_ID);

    /* Create MQTT Client.  */
    status = nxd_mqtt_client_create(&mqtt_client_secure, "MQTT_CLIENT", DEVICE_ID, strlen(DEVICE_ID), ip_ptr, pool_ptr,
                                    (VOID *)mqtt_client_stack, sizeof(mqtt_client_stack), 2, NX_NULL, 0);

    /* Check status.  */
    if (status)
    {
        PRINTF("Error in creating MQTT client: 0x%02x\r\n", status);
        return;
    }

#ifdef NXD_MQTT_OVER_WEBSOCKET
    status = nxd_mqtt_client_websocket_set(&mqtt_client_secure, (UCHAR *)HOST_NAME, strlen(HOST_NAME),
                                           (UCHAR *)NX_AZURE_IOT_HUB_CLIENT_WEB_SOCKET_PATH,
                                           strlen((const char *)NX_AZURE_IOT_HUB_CLIENT_WEB_SOCKET_PATH));
    if (status)
    {
        PRINTF("Error in setting MQTT over websocket: 0x%02x\r\n", status);
        return;
    }
#endif

    /* Create TLS session.  */
    status = nx_secure_tls_session_create(&(mqtt_client_secure.nxd_mqtt_tls_session), &nx_crypto_tls_ciphers,
                                          threadx_tls_metadata_buffer, THREADX_TLS_METADATA_BUFFER);

    /* Check status.  */
    if (status)
    {
        PRINTF("Error in creating TLS Session: 0x%02x\r\n", status);
        return;
    }

    /* Set username and password.  */
    status = nxd_mqtt_client_login_set(&mqtt_client_secure, mqtt_user_name, strlen(mqtt_user_name), DEVICE_SAS,
                                       strlen(DEVICE_SAS));

    /* Check status.  */
    if (status)
    {
        PRINTF("Error in setting username and password: 0x%02x\r\n", status);
        return;
    }

    /* Resolve the server name to get the address.  */
    status =
        nxd_dns_host_by_name_get(dns_ptr, (UCHAR *)HOST_NAME, &server_address, NX_IP_PERIODIC_RATE, NX_IP_VERSION_V4);
    if (status)
    {
        PRINTF("Error in getting host address: 0x%02x\r\n", status);
        return;
    }

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (mqtt_client_secure.nxd_mqtt_client_use_websocket == NX_TRUE)
    {
        server_port = NXD_MQTT_OVER_WEBSOCKET_TLS_PORT;
    }
    else
    {
        server_port = NXD_MQTT_TLS_PORT;
    }
#else
    server_port = NXD_MQTT_TLS_PORT;
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Start MQTT connection.  */
    status = nxd_mqtt_client_secure_connect(&mqtt_client_secure, &server_address, server_port, threadx_mqtt_tls_setup,
                                            6 * NX_IP_PERIODIC_RATE, NX_TRUE, NX_WAIT_FOREVER);
    if (status)
    {
        PRINTF("Error in connecting to server: 0x%02x\r\n", status);
        return;
    }

    PRINTF("Connected to server\r\n");

    /* Subscribe.  */
    status = nxd_mqtt_client_subscribe(&mqtt_client_secure, mqtt_subscribe_topic, strlen(mqtt_subscribe_topic), 0);
    if (status)
    {
        PRINTF("Error in subscribing to server: 0x%02x\r\n", status);
        return;
    }

    PRINTF("Subscribed to server\r\n");

    /* Set notify function.  */
    status = nxd_mqtt_client_receive_notify_set(&mqtt_client_secure, my_notify_func);
    if (status)
    {
        PRINTF("Error in setting receive notify: 0x%02x\r\n", status);
        return;
    }

    /* Loop to send publish message and wait for command.  */
    while (1)
    {
        /* Send publish message every five seconds.  */
        if ((time_counter % 5) == 0)
        {
            /* Check if turn fan on.  */
            if (fan_on == NX_FALSE)
            {
                if (temperature < 40)
                    temperature++;
            }
            else
            {
                if (temperature > 0)
                    temperature--;
            }

            /* Publish message.  */
            sprintf((CHAR *)mqtt_message_buffer, "{\"temperature\": %d}", temperature);
            status = nxd_mqtt_client_publish(&mqtt_client_secure, mqtt_publish_topic, strlen(mqtt_publish_topic),
                                             (CHAR *)mqtt_message_buffer, strlen((CHAR const *)mqtt_message_buffer), 0,
                                             1, NX_WAIT_FOREVER);
            if (status)
            {
                PRINTF("Publish failed: 0x%02x\r\n", status);
            }
            else
            {
                PRINTF("[Published] topic = %s, message: %s\r\n", mqtt_publish_topic, (CHAR *)mqtt_message_buffer);
            }
        }

        /* Sleep */
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);

        /* Update the counter.  */
        time_counter++;
    }
}
