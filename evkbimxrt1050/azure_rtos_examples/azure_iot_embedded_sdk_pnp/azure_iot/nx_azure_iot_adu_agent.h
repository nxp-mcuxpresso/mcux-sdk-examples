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

/* Version: 6.1 */

/**
 * @file nx_azure_iot_adu_agent.h
 *
 * @brief Definition for the Azure IoT ADU agent interface.
 *
 */

#ifndef NX_AZURE_IOT_ADU_AGENT_H
#define NX_AZURE_IOT_ADU_AGENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_azure_iot_hub_client_properties.h"
#include "nx_web_http_client.h"

#ifndef NX_ENABLE_EXTENDED_NOTIFY_SUPPORT
#error "NX_ENABLE_EXTENDED_NOTIFY_SUPPORT must be defined"
#endif /* NX_ENABLE_EXTENDED_NOTIFY_SUPPORT */

/* Define the proxy update count. The value is 0 by default, it means only supporting host-level update.
   To enable proxy update for leaf-level update, redefine NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT
#define NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT                       0
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

/* Define the ADU agent component name.  */
#define NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME                           "deviceUpdate"

/* Define the ADU agent interface ID.  */
#define NX_AZURE_IOT_ADU_AGENT_INTERFACE_ID                             "dtmi:azure:iot:deviceUpdate;1"

/* Define the compatibility.  */
#define NX_AZURE_IOT_ADU_AGENT_COMPATIBILITY                            "manufacturer,model"

/* Define the ADU agent property name "agent" and sub property names.  */
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_AGENT                      "agent"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICEPROPERTIES           "deviceProperties"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MANUFACTURER               "manufacturer"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MODEL                      "model"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INTERFACE_ID               "interfaceId"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ADU_VERSION                "aduVer"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DO_VERSION                 "doVer"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_COMPAT_PROPERTY_NAMES      "compatPropertyNames"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTALLED_CONTENT_ID       "installedUpdateId"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_PROVIDER                   "provider"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_NAME                       "name"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_VERSION                    "version"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_LAST_INSTALL_RESULT        "lastInstallResult"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_CODE                "resultCode"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE       "extendedResultCode"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_DETAILS             "resultDetails"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STEP_RESULTS               "stepResults"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STATE                      "state"

/* Define the ADU agent property name "service" and sub property names.  */
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SERVICE                    "service"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW                   "workflow"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION                     "action"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID                         "id"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP            "retryTimestamp"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST            "updateManifest"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST_SIGNATURE  "updateManifestSignature"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILEURLS                   "fileUrls"

#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MANIFEST_VERSION           "manifestVersion"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_ID                  "updateId"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_COMPATIBILITY              "compatibility"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICE_MANUFACTURER        "deviceManufacturer"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICE_MODEL               "deviceModel"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_GROUP                      "group"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTRUCTIONS               "instructions"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STEPS                      "steps"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_TYPE                       "type"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HANDLE                     "handler"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILES                      "files"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DETACHED_MANIFEST_FILED    "detachedManifestFileId"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTALLED_CRITERIA         "installedCriteria"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILE_NAME                  "fileName"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SIZE_IN_BYTES              "sizeInBytes"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HASHES                     "hashes"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SHA256                     "sha256"
#define NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_CREATED_DATE_TIME          "createdDateTime"

/* Define ADU events. These events are processed by the cloud thread.  */
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_EVENT                             ((ULONG)0x00000001)
#define NX_AZURE_IOT_ADU_AGENT_DNS_RESPONSE_RECEIVE_EVENT               ((ULONG)0x00000002)
#define NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_DONE_EVENT                  ((ULONG)0x00000004)
#define NX_AZURE_IOT_ADU_AGENT_HTTP_RECEIVE_EVENT                       ((ULONG)0x00000008)
#define NX_AZURE_IOT_ADU_AGENT_APPLY_EVENT                              ((ULONG)0x00000010)

/* Define the agent state values. Interaction between agent and server.  */
#define NX_AZURE_IOT_ADU_AGENT_STATE_IDLE                               0
#define NX_AZURE_IOT_ADU_AGENT_STATE_DEPLOYMENT_IN_PROGRESS             6
#define NX_AZURE_IOT_ADU_AGENT_STATE_FAILED                             255

/* Define the agent action values. Interaction between agent and server.  */
#define NX_AZURE_IOT_ADU_AGENT_ACTION_APPLY_DEPLOYMENT                  3
#define NX_AZURE_IOT_ADU_AGENT_ACTION_CANCEL                            255

/* Define the result code value. Interaction between agent and server.  */
#define NX_AZURE_IOT_ADU_AGENT_RESULT_CODE_FAILURE                      0
#define NX_AZURE_IOT_ADU_AGENT_RESULT_CODE_IDLE_SUCCESS                 200
#define NX_AZURE_IOT_ADU_AGENT_RESULT_CODE_APPLY_SUCCESS                700
#define NX_AZURE_IOT_ADU_AGENT_RESULT_CODE_APPLY_INPROGRESS             701

/* Define the agent update state for adu_agent_update_notify().  */
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_RECEIVED                          0
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_DOWNLOADED                        1
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_INSTALLED                         2

/* FIXME: status codes should be defined in iothub client.  */
/* Status codes, closely mapping to HTTP status. */
#define NX_AZURE_IOT_ADU_AGENT_STATUS_SUCCESS                           200
#define NX_AZURE_IOT_ADU_AGENT_STATUS_BAD_FORMAT                        400
#define NX_AZURE_IOT_ADU_AGENT_STATUS_NOT_FOUND                         404
#define NX_AZURE_IOT_ADU_AGENT_STATUS_INTERNAL_ERROR                    500

/* Define the crypto size.  */
#define NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE                         32
#define NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_BASE64_SIZE                  44
#define NX_AZURE_IOT_ADU_AGENT_RSA3072_SIZE                             384

/* Define the sha256 metadata buffer size used for verifying file hash. 
   The default value is software sha256 crypto metadata (sizeof(NX_CRYPTO_SHA256)).  */
#ifndef NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE
#define NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE                     360
#endif /* NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE */

/* Define the max size of workflow id.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_WORKFLOW_ID_SIZE
#define NX_AZURE_IOT_ADU_AGENT_WORKFLOW_ID_SIZE                         48
#endif /* NX_AZURE_IOT_ADU_AGENT_WORKFLOW_ID_SIZE */

/* Define the max size of workflow retry timestamp.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_WORKFLOW_RETRY_TIMESTAMP_SIZE
#define NX_AZURE_IOT_ADU_AGENT_WORKFLOW_RETRY_TIMESTAMP_SIZE            80
#endif /* NX_AZURE_IOT_ADU_AGENT_WORKFLOW_RETRY_TIMESTAMP_SIZE */

/* Define the max size for one file string including id, file name, file size and hashes.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_FILE_STRING_SIZE
#define NX_AZURE_IOT_ADU_AGENT_FILE_STRING_SIZE                         256
#endif /* NX_AZURE_IOT_ADU_AGENT_FILE_STRING_SIZE */

/* Define the max update manifest size, the buffer is used to store the original string data.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE                     (1024 + (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT * NX_AZURE_IOT_ADU_AGENT_FILE_STRING_SIZE))
#endif /* NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE */

/* Define the max update manifest signature size (base64).  */
#ifndef NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE           3072
#endif /* NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE */

/* Define the max update manifest sjwk size (base64).  */
#ifndef NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE                2048
#endif /* NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE */

/* Define the buffer for parsing file url, update manifest content in service property. FIXME: consider to use packet.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE
#define NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE                              2048
#endif /* NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE */

/* Define the max steps of update manifest instructions. (1 step for host update and steps for proxy update)  */
#ifndef NX_AZURE_IOT_ADU_AGENT_STEPS_MAX
#define NX_AZURE_IOT_ADU_AGENT_STEPS_MAX                                (1 + NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT)
#endif /* NX_AZURE_IOT_ADU_AGENT_STEPS_MAX */

/* Define the max number of file updates (1 host update + (1 leaf manifest + 1 leaf update) * count).  */
#ifndef NX_AZURE_IOT_ADU_AGENT_FILES_MAX
#define NX_AZURE_IOT_ADU_AGENT_FILES_MAX                                (1 +  (2 * NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT))
#endif /* NX_AZURE_IOT_ADU_AGENT_FILES_MAX */

/* Define the buffer size of storing file URLs.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_FILE_URLS_SIZE
#define NX_AZURE_IOT_ADU_AGENT_FILE_URLS_SIZE                           (320 * NX_AZURE_IOT_ADU_AGENT_FILES_MAX)
#endif /* NX_AZURE_IOT_ADU_AGENT_FILE_URLS_SIZE */

/* Define the update count.  */
#define NX_AZURE_IOT_ADU_AGENT_UPDATE_COUNT                             (1 + NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT)

#define NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(a, b, c, d)                   { \
                                                                            (a) = (c); \
                                                                            (c) += (b); \
                                                                            (d) -= (b); \
                                                                        }

/* Define the initial timeout for DNS query, the default wait time is 1s.
   For the next retransmission, the timeout will be doubled.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_DNS_INITIAL_TIMEOUT 
#define NX_AZURE_IOT_ADU_AGENT_DNS_INITIAL_TIMEOUT                      (1)
#endif /* NX_AZURE_IOT_ADU_AGENT_DNS_INITIAL_TIMEOUT */

/* Define the maximum number of retries to a DNS server. The default count is 3.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_DNS_RETRANSMIT_COUNT 
#define NX_AZURE_IOT_ADU_AGENT_DNS_RETRANSMIT_COUNT                     (3)
#endif /* NX_AZURE_IOT_ADU_AGENT_DNS_RETRANSMIT_COUNT */

/* Define the window size of HTTP for downloading firmware.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_HTTP_WINDOW_SIZE
#define NX_AZURE_IOT_ADU_AGENT_HTTP_WINDOW_SIZE                         (16 * 1024)
#endif /* NX_AZURE_IOT_ADU_AGENT_HTTP_WINDOW_SIZE  */

/* Define the timeout of HTTP for connecting. The default time is 30s.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_TIMEOUT
#define NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_TIMEOUT                     (30)
#endif /* NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_TIMEOUT */

/* Define the total timeout of HTTP for downloading the whole firmware. The default time is 300s.  */
#ifndef NX_AZURE_IOT_ADU_AGENT_HTTP_DOWNLOAD_TIMEOUT
#define NX_AZURE_IOT_ADU_AGENT_HTTP_DOWNLOAD_TIMEOUT                    (300)
#endif /* NX_AZURE_IOT_ADU_AGENT_HTTP_DOWNLOAD_TIMEOUT */

/* Define the http protocol string.  */
#define NX_AZURE_IOT_ADU_AGENT_HTTP_PROTOCOL                            "http://"

/* Define the firmware update state.  */
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_IDLE                          0
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_MANIFEST_DOWNLOAD_STARTED     1
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_MANIFEST_DOWNLOAD_SUCCEEDED   2
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_STARTED     3
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_SUCCEEDED   4
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_INSTALL_STARTED      5
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_INSTALL_SUCCEEDED    6
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_APPLY_STARTED        7
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_APPLY_SUCCEEDED      8
#define NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED                        255

/* Define the download type.  */
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_MANIFEST                 0
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE                 1

/* Define the downloader state.  */
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_IDLE                          0
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_URL_PARSED                    1
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_QUERY                 2
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_DONE                  3
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONNECT                  4
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONTENT_GET              5
#define NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_DONE                          6

/* Define ADU driver constants.  */
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE                        0
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_UPDATE_CHECK                      1
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_PREPROCESS                        2
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_WRITE                             3
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_INSTALL                           4
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_APPLY                             5
#define NX_AZURE_IOT_ADU_AGENT_DRIVER_CANCEL                            6

/* Define string macro.  */
#define NX_AZURE_IOT_ADU_AGENT_STRING(p)                                p, sizeof(p) - 1

/**
 * @brief ADU update id struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_UPDATE_ID_STRUCT
{

    /* Manufacturer.  */
    const UCHAR                            *provider;
    UINT                                    provider_length;

    /* Name/model.  */
    const UCHAR                            *name;
    UINT                                    name_length;

    /* Version. */
    const UCHAR                            *version;
    UINT                                    version_length;

} NX_AZURE_IOT_ADU_AGENT_UPDATE_ID;

/**
 * @brief ADU driver struct
 *
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_DRIVER_STRUCT
{

    /* Define the driver command.  */
    UINT                                    nx_azure_iot_adu_agent_driver_command;

    /* Define the driver return status.  */
    UINT                                    nx_azure_iot_adu_agent_driver_status;

    /* Define the firmware size for the driver to preprocess.  */
    UINT                                    nx_azure_iot_adu_agent_driver_firmware_size;
    
    /* Define the firmware data for the driver to write.   */
    UINT                                    nx_azure_iot_adu_agent_driver_firmware_data_offset;
    UCHAR                                  *nx_azure_iot_adu_agent_driver_firmware_data_ptr; 
    UINT                                    nx_azure_iot_adu_agent_driver_firmware_data_size;

    /* Define the return pointer for raw driver command requests.  */
    ULONG                                  *nx_azure_iot_adu_agent_driver_return_ptr;

    /* Update ID.  */
    NX_AZURE_IOT_ADU_AGENT_UPDATE_ID       *nx_azure_iot_adu_agent_driver_update_id;

} NX_AZURE_IOT_ADU_AGENT_DRIVER;

/**
 * @brief ADU crypto struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_CRYPTO_STRUCT
{

    /* RS256.  */

    /* RSA. Reuse the metadata from TLS cipher metadata.  */
    NX_CRYPTO_METHOD                        *method_rsa;
    UCHAR                                   *method_rsa_metadata;
    ULONG                                    method_rsa_metadata_size;
    VOID                                    *handler;

    /* SHA256.  */
    NX_CRYPTO_METHOD                        *method_sha256;
    UCHAR                                    method_sha256_metadata[NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE];

} NX_AZURE_IOT_ADU_AGENT_CRYPTO;

/**
 * @brief ADU workflow struct.
 * Format:
 *    {
        "action": 3,
        "id": "someguid",
        "retryTimestamp": "2020-04-22T12:12:12.0000000+00:00"
    }
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_WORKFLOW_STRUCT
{

    /* Action.  */
    UINT                                    action;

    /* ID.  */
    UCHAR                                   id[NX_AZURE_IOT_ADU_AGENT_WORKFLOW_ID_SIZE];
    UINT                                    id_length;

    /* Retry Timestamp.  */
    UCHAR                                   retry_timestamp[NX_AZURE_IOT_ADU_AGENT_WORKFLOW_RETRY_TIMESTAMP_SIZE];
    UINT                                    retry_timestamp_length;

} NX_AZURE_IOT_ADU_AGENT_WORKFLOW;

/**
 * @brief ADU result struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_RESULT_STRUCT
{

    /* Result.  */
    UINT                                    result_code;

    /* Extended result code.  */
    UINT                                    extended_result_code;

} NX_AZURE_IOT_ADU_AGENT_RESULT;

/**
 * @brief ADU device properties struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_DEVICE_PROPERTIES_STRUCT
{

    /* Manufacturer.  */
    const UCHAR                            *manufacturer;
    UINT                                    manufacturer_length;

    /* Name/model.  */
    const UCHAR                            *model;
    UINT                                    model_length;

} NX_AZURE_IOT_ADU_AGENT_DEVICE_PROPERTIES;

/**
 * @brief ADU update manifest steps struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_STEP_STRUCT
{

    /* Type.  */
    const UCHAR                            *type;
    UINT                                    type_length;

    /* Handler.  */
    const UCHAR                            *handler;
    UINT                                    handler_length;

    /* File id. */
    const UCHAR                            *file_id;
    UINT                                    file_id_length;

    /* Step state.  */
    UINT                                    state;

    /* Result.  */
    NX_AZURE_IOT_ADU_AGENT_RESULT           result;

    /* Driver entry point for firmware.  */
    VOID                                  (*driver_entry)(NX_AZURE_IOT_ADU_AGENT_DRIVER *);

} NX_AZURE_IOT_ADU_AGENT_STEP;

/**
 * @brief ADU update manifest files struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_FILE_STRUCT
{

    /* File number.  */
    const UCHAR                            *file_id;
    UINT                                    file_id_length;

    /* File name.  */
    UCHAR                                  *file_name;
    UINT                                    file_name_length;

    /* File size in bytes.  */
    UINT                                    file_size_in_bytes;

    /* File sha256.  */
    UCHAR                                  *file_sha256;
    UINT                                    file_sha256_length;

    /* File url.  */
    UCHAR                                  *file_url;
    UINT                                    file_url_length;

} NX_AZURE_IOT_ADU_AGENT_FILE;

/**
 * @brief ADU update manifest content struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT_STRUCT
{

    /* Manifest version.  */
    UCHAR                                  *manifest_version;
    UINT                                    manifest_version_length;

    /* Update Id.  */
    NX_AZURE_IOT_ADU_AGENT_UPDATE_ID        update_id;

    /* Compatibility: deviceManufacturer.  */
    UCHAR                                  *device_manufacturer;
    UINT                                    device_manufacturer_length;

    /* Compatibility: deviceModel.  */
    UCHAR                                  *device_model;
    UINT                                    device_model_length;

    /* Compatibility: group.  */
    const UCHAR                            *group;
    UINT                                    group_length;

    /* Instructions: steps.  */
    NX_AZURE_IOT_ADU_AGENT_STEP             steps[NX_AZURE_IOT_ADU_AGENT_STEPS_MAX];
    UINT                                    steps_count;

    /* Files.  */
    NX_AZURE_IOT_ADU_AGENT_FILE             files[NX_AZURE_IOT_ADU_AGENT_FILES_MAX];
    UINT                                    files_count;

} NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT;

/**
 * @brief ADU file urls file struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_FILE_URLS_FILE_STRUCT
{

    /* File id string.  */
    UCHAR                                  *file_id;
    UINT                                    file_id_length;

    /* File URL.  */
    UCHAR                                  *file_url;
    UINT                                    file_url_length;

} NX_AZURE_IOT_ADU_AGENT_FILE_URLS_FILE;

/**
 * @brief ADU file urls struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_FILE_URLS_STRUCT
{

    /* File struct.  */
    NX_AZURE_IOT_ADU_AGENT_FILE_URLS_FILE   file_urls[NX_AZURE_IOT_ADU_AGENT_FILES_MAX];

    /* Count of file urls.  */
    UINT                                    file_urls_count;

    /* Buffer.   */
    UCHAR                                   file_urls_buffer[NX_AZURE_IOT_ADU_AGENT_FILE_URLS_SIZE];

} NX_AZURE_IOT_ADU_AGENT_FILE_URLS;

/**
 * @brief ADU downloader struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_STRUCT
{

    /* Download type (manifest or firmware).  */
    UINT                                    type;

    /* Download file.  */
    NX_AZURE_IOT_ADU_AGENT_FILE            *file;

    /* HTTP Client for downloading firmware.  */
    NX_WEB_HTTP_CLIENT                      http_client;

    /* Host string.  */
    UCHAR                                  *host;
    UINT                                    host_length;

    /* Resource string.  */
    UCHAR                                  *resource;
    UINT                                    resource_length;

    /* HTTP server address.  */
    NXD_ADDRESS                             address;

    /* HTTP server port.  */
    UINT                                    port;

    /* Received firmware size.  */
    UINT                                    received_firmware_size;

    /* Downloading state.  */
    UINT                                    state;

    /* DNS.  */
    NX_DNS                                 *dns_ptr;

    /* DNS query count.  */
    UINT                                    dns_query_count;

    /* Timeout.  */
    ULONG                                   timeout;

    /* Buffer for manifest.  */
    UCHAR                                  *manifest_buffer_ptr;
    UINT                                    manifest_buffer_size;

    /* Driver entry point for firmware.  */
    VOID                                  (*driver_entry)(NX_AZURE_IOT_ADU_AGENT_DRIVER *);

} NX_AZURE_IOT_ADU_AGENT_DOWNLOADER;

/**
 * @brief ADU RSA root key struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY_STRUCT
{
    const UCHAR                            *kid;
    const UINT                              kid_size;

    const UCHAR                            *n;
    const UINT                              n_size;

    const UCHAR                            *e;
    const UINT                              e_size;
} NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY;

/**
 * @brief ADU update struct.
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_UPDATE_STRUCT
{

    /* State.  */
    UINT                                    valid;

    /* Update Id.  */
    NX_AZURE_IOT_ADU_AGENT_UPDATE_ID        update_id;

    /* Driver entry point for firmware.  */
    VOID                                  (*update_driver_entry)(NX_AZURE_IOT_ADU_AGENT_DRIVER *);

} NX_AZURE_IOT_ADU_AGENT_UPDATE;

/**
 * @brief ADU agent struct
 *
 */
typedef struct NX_AZURE_IOT_ADU_AGENT_STRUCT
{

    /* IoTHub client pointer.  */
    NX_AZURE_IOT_HUB_CLIENT                *nx_azure_iot_hub_client_ptr;

    /* Mutex.  */
    TX_MUTEX                               *nx_azure_iot_adu_agent_mutex_ptr;

    /* Cloud module.  */
    NX_CLOUD_MODULE                         nx_azure_iot_adu_agent_cloud_module;

    /* Downloader.  */
    NX_AZURE_IOT_ADU_AGENT_DOWNLOADER       nx_azure_iot_adu_agent_downloader;

    /* ADU crypto.  */
    NX_AZURE_IOT_ADU_AGENT_CRYPTO           nx_azure_iot_adu_agent_crypto;

    /* ADU device properties.  */
    NX_AZURE_IOT_ADU_AGENT_DEVICE_PROPERTIES nx_azure_iot_adu_agent_device_properties;

    /* Firmware update state.  */
    UINT                                    nx_azure_iot_adu_agent_state;

    /* Workflow.  */
    NX_AZURE_IOT_ADU_AGENT_WORKFLOW         nx_azure_iot_adu_agent_workflow;

    /* Deployable update manifest string.  */
    UCHAR                                   nx_azure_iot_adu_agent_update_manifest[NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE];
    UINT                                    nx_azure_iot_adu_agent_update_manifest_size;

    /* Update manifest signature. After verifying the deployable update manifest, the buffer will be used for leaf update manifest.  */
    UCHAR                                   nx_azure_iot_adu_agent_update_manifest_signature[NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE];
    UINT                                    nx_azure_iot_adu_agent_update_manifest_signature_size;

    /* SJWK. After verifying the deployable update manifest, the buffer will be used for leaf update manifest content.  */
    UCHAR                                   nx_azure_iot_adu_agent_update_manifest_sjwk[NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE];
    UINT                                    nx_azure_iot_adu_agent_update_manifest_sjwk_size;

    /* File URL.  */
    NX_AZURE_IOT_ADU_AGENT_FILE_URLS        nx_azure_iot_adu_agent_file_urls;

    /* Deployable update manifest content.  */
    NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT nx_azure_iot_adu_agent_update_manifest_content;
    NX_AZURE_IOT_ADU_AGENT_STEP            *nx_azure_iot_adu_agent_current_step;

    /* Buffer for storing file number string, file urls and update manifest content.  */
    UCHAR                                   nx_azure_iot_adu_agent_buffer[NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE];
    UINT                                    nx_azure_iot_adu_agent_buffer_size;

    /* Update info.  */
    NX_AZURE_IOT_ADU_AGENT_UPDATE           nx_azure_iot_adu_agent_update[NX_AZURE_IOT_ADU_AGENT_UPDATE_COUNT];

#if NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT
    /* Leaf-level manifest content.  */
    NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT nx_azure_iot_adu_agent_proxy_update_manifest_content;
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

    /* Define the callback function for new update notification. If specified
       by the application, this function is called when new update occurs.  */
    VOID                                  (*nx_azure_iot_adu_agent_update_notify)(struct NX_AZURE_IOT_ADU_AGENT_STRUCT *adu_agent_ptr,
                                                                                  UINT update_state,
                                                                                  UCHAR *provider, UINT provider_length,
                                                                                  UCHAR *name, UINT name_length,
                                                                                  UCHAR *version, UINT version_length);

} NX_AZURE_IOT_ADU_AGENT;

/**
 * @brief Start Azure IoT ADU agent
 *
 * @param[in] adu_agent_ptr A pointer to a #NX_AZURE_IOT_ADU_AGENT.
 * @param[in] iothub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] manufacturer A `UCHAR` pointer to the manufacturer
 * @param[in] manufacturer_length Length of the `manufacturer`. Does not include the `NULL` terminator.
 * @param[in] model A `UCHAR` pointer to the model
 * @param[in] model_length Length of the `model`. Does not include the `NULL` terminator.
 * @param[in] provider A `UCHAR` pointer to the provider.
 * @param[in] provider_length Length of the `provider`. Does not include the `NULL` terminator.
 * @param[in] name A `UCHAR` pointer to the name
 * @param[in] name_length Length of the `name`. Does not include the `NULL` terminator.
 * @param[in] version A `UCHAR` pointer to the version
 * @param[in] version_length Length of the `version`. Does not include the `NULL` terminator.
 * @param[in] adu_agent_update_notify User supplied update receive change callback.
 * @param[in] adu_agent_update_driver User supplied driver for flash operation.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully start ADU agent.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to start the Azure IoT ADU agent due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NO_AVAILABLE_CIPHER Fail to start the Azure IoT ADU agent due to no available cipher.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to start the Azure IoT ADU agent due to insufficient buffer space.
 */
UINT nx_azure_iot_adu_agent_start(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                  const UCHAR *manufacturer, UINT manufacturer_length,
                                  const UCHAR *model, UINT model_length,
                                  const UCHAR *provider, UINT provider_length,
                                  const UCHAR *name, UINT name_length,
                                  const UCHAR *version, UINT version_length,
                                  VOID (*adu_agent_update_notify)(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                                  UINT update_state,
                                                                  UCHAR *provider, UINT provider_length,
                                                                  UCHAR *name, UINT name_length,
                                                                  UCHAR *version, UINT version_length),
                                  VOID (*adu_agent_update_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *));

#if NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT
/**
 * @brief Add proxy update on device update agent
 *
 * @param[in] adu_agent_ptr A pointer to a #NX_AZURE_IOT_ADU_AGENT.
 * @param[in] provider A `UCHAR` pointer to the provider.
 * @param[in] provider_length Length of the `provider`. Does not include the `NULL` terminator.
 * @param[in] name A `UCHAR` pointer to the name
 * @param[in] name_length Length of the `name`. Does not include the `NULL` terminator.
 * @param[in] version A `UCHAR` pointer to the version
 * @param[in] version_length Length of the `version`. Does not include the `NULL` terminator.
 * @param[in] adu_agent_proxy_driver User supplied driver for proxy update operation.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully add ADU agent proxy update on leaf device.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to start the Azure IoT ADU agent due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NO_MORE_ENTRIES Fail to start the Azure IoT ADU agent due to no more entries.
 */
UINT nx_azure_iot_adu_agent_proxy_update_add(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                             const UCHAR *provider, UINT provider_length,
                                             const UCHAR *name, UINT name_length,
                                             const UCHAR *version, UINT version_length,
                                             VOID (*adu_agent_proxy_update_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *));
#endif /* NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT */

/**
 * @brief Stop Azure IoT ADU agent
 *
 * @param[in] adu_agent_ptr A pointer to a #NX_AZURE_IOT_ADU_AGENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully stop ADU agent.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to stop the Azure IoT ADU agent due to invalid parameter.
 */
UINT nx_azure_iot_adu_agent_stop(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);

/**
 * @brief Start to download and install firmware immediately.
 * @note Agent will download and install new firmware.
 *
 * @param[in] adu_agent_ptr A pointer to a #NX_AZURE_IOT_ADU_AGENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully start to update new firmware.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to start to update new firmware due to invalid parameter.
 */
UINT nx_azure_iot_adu_agent_update_download_install(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);

/**
 * @brief Start to apply firmware immediately.
 * @note Agent will apply new firmware, may reboot the device.
 *
 * @param[in] adu_agent_ptr A pointer to a #NX_AZURE_IOT_ADU_AGENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully start to apply new firmware.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to start to apply new firmware due to invalid parameter.
 */
UINT nx_azure_iot_adu_agent_update_apply(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_ADU_AGENT_H */
