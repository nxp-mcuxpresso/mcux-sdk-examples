# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

if (DEFINED ENV{SdkRootDirPath})
    SET(SdkRootDirPath "$ENV{SdkRootDirPath}")
else()
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(azure_iot_embedded_sdk_adu)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME azure_iot_embedded_sdk_adu.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

SET(NX_USER_FILE ${ProjDirPath}/../nx_user.h)

SET(FX_USER_FILE ${ProjDirPath}/../fx_user.h)

SET(TX_USER_FILE ${ProjDirPath}/../tx_user.h)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/az_core.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/az_iot.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/_az_cfg.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/_az_cfg_prefix.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/_az_cfg_suffix.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_base64.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_config.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_context.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_credentials.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_http.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_http_transport.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_json.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_log.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_platform.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_precondition.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_result.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_span.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/az_version.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_config_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_credentials_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_http_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_log_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_precondition_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_result_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_retry_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/core/internal/az_span_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/iot/az_iot_common.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/iot/az_iot_hub_client.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/iot/az_iot_hub_client_properties.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/iot/az_iot_provisioning_client.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc/azure/iot/internal/az_iot_common_internal.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_base64.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_context.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_hex_private.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_header_validation_private.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_pipeline.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_policy.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_policy_logging.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_policy_logging_private.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_policy_retry.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_private.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_request.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_http_response.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_json_private.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_json_reader.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_json_token.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_json_writer.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_log.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_precondition.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_span.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/core/az_span_private.h"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_common.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_c2d.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_commands.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_methods.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_properties.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_sas.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_telemetry.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_hub_client_twin.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_provisioning_client.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/iot/az_iot_provisioning_client_sas.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/platform/az_nohttp.c"
"${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/src/azure/platform/az_noplatform.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/inc/configs/RTOS_BASE/asc_config.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/inc/iot_security_module/model/objects/object_network_activity_ext.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/inc/iot_security_module/mti.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_accessors.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_alloc.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_assert.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_emitter.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_endian.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_epilogue.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_flatbuffers.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_identifier.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_iov.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_prologue.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_refmap.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_rtconfig.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/flatcc_types.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/paligned_alloc.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pdiagnostic.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pdiagnostic_pop.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pdiagnostic_push.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pendian.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pendian_detect.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pstatic_assert.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pstdalign.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pstdint.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include/flatcc/portable/pwarnings.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime/builder.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime/emitter.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime/refmap.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/asc_result.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/collector.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/collector_collection.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/collectors_info.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/component_id.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/component_info.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/components_factory.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/components_factory_declarations.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/components_factory_enum.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/components_factory_type.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/components_manager.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/core.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/logger.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/mti.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/objects/network_activity.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/objects/objects.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/objects/system_information.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/objects/transport_protocol.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/baseline_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/baseline_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/baseline_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/baseline_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/event_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/event_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/event_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/event_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/flatbuffers_common_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/flatbuffers_common_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/heartbeat_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/heartbeat_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/heartbeat_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/heartbeat_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/listening_ports_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/listening_ports_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/listening_ports_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/listening_ports_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/log_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/log_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/log_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/log_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/message_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/message_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/message_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/message_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/network_activity_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/network_activity_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/network_activity_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/network_activity_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/payload_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/payload_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/payload_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/payload_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_type_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_type_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_type_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_event_type_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/process_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/protocol_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/protocol_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/protocol_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/protocol_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/system_information_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/system_information_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/system_information_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/system_information_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/uuid_builder.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/uuid_json_parser.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/uuid_json_printer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/schema/uuid_reader.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/model/security_message.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/object_pool.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/object_pool_def.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/object_pool_dynamic.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/object_pool_static.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/serializer.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/serializer/custom_builder_allocator.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/serializer/page_allocator.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/bit_vector.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/collection.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/hashset.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/hashtable.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/linked_list.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/list.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/collection/stack.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/containerof.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/event_loop_be.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/iconv.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/ievent_loop.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/ifile.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/irand.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/istream.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/itime.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/macros.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/notifier.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/notifier_topics.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/num2str.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/os_utils.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/string_utils.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/utils/uuid.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc/asc_security_core/version.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/collector_collection.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/collectors/collector_heartbeat.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/collectors_info.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/components_factory.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/components_manager.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/core.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/logger.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/model/collector.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/model/security_message.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/object_pool_static.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/extensions/custom_builder_allocator.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/extensions/page_allocator.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/heartbeat.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/network_activity.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/protocol_serialize_enum.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/serializer.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/serializer_private.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/serializer_private.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/system_information.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/collection/bit_vector.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/collection/hashtable.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/collection/list.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/collection/stack.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/event_loop_be.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/iconv.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/notifier.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/string_utils.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/src/utils/uuid.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/nx_azure_iot_security_module.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/nx_azure_iot_security_module.h"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/collectors/collector_network_activity.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/collectors/collector_system_information.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/model/objects/object_network_activity_ext.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/utils/ievent_loop.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/utils/irand.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/utils/itime.c"
"${ProjDirPath}/../azure_iot/azure_iot_security_module/src/utils/os_utils.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot.h"
"${ProjDirPath}/../azure_iot/nx_azure_iot_adu_agent.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_adu_agent.h"
"${ProjDirPath}/../azure_iot/nx_azure_iot_adu_root_key.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_hub_client.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_hub_client.h"
"${ProjDirPath}/../azure_iot/nx_azure_iot_hub_client_properties.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_hub_client_properties.h"
"${ProjDirPath}/../azure_iot/nx_azure_iot_json_reader.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_json_reader.h"
"${ProjDirPath}/../azure_iot/nx_azure_iot_json_writer.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_json_writer.h"
"${ProjDirPath}/../azure_iot/nx_azure_iot_provisioning_client.c"
"${ProjDirPath}/../azure_iot/nx_azure_iot_provisioning_client.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../azure_iot_embedded_sdk_pnp.c"
"${ProjDirPath}/../azure_iot_embedded_sdk_adu.c"
"${ProjDirPath}/../sample_azure_iot_embedded_sdk_connect.c"
"${ProjDirPath}/../secure_storage.c"
"${ProjDirPath}/../get_seed.c"
"${ProjDirPath}/../sample_config.h"
"${ProjDirPath}/../nx_azure_iot_cert.c"
"${ProjDirPath}/../nx_azure_iot_cert.h"
"${ProjDirPath}/../nx_azure_iot_ciphersuites.c"
"${ProjDirPath}/../nx_azure_iot_ciphersuites.h"
"${ProjDirPath}/../sample_device_identity.c"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../nx_driver_imxrt_irq.c"
"${ProjDirPath}/../nx_driver_imxrt.c"
"${ProjDirPath}/../nx_driver_imxrt.h"
"${ProjDirPath}/../nx_azure_iot_adu_agent_mflash_driver.c"
"${ProjDirPath}/../nx_user.h"
"${ProjDirPath}/../fx_user.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../scripts/sign_image.py"
"${ProjDirPath}/../scripts/gen_manifest.py"
"${ProjDirPath}/../keys/sign-rsa2048-pub.pem"
"${ProjDirPath}/../keys/sign-rsa2048-priv.pem"
"${ProjDirPath}/../mcuboot/mcuboot_app_support.c"
"${ProjDirPath}/../mcuboot/mcuboot_app_support.h"
"${ProjDirPath}/../mcuboot/flash_map.h"
"${ProjDirPath}/../mcuboot/sbldef.h"
"${ProjDirPath}/../mcuboot/sblconfig.h"
"${ProjDirPath}/../mcuboot/mcuboot_config.h"
"${ProjDirPath}/../mcuboot/flash_partitioning.c"
"${ProjDirPath}/../mcuboot/flash_partitioning.h"
"${ProjDirPath}/../mcuboot/sysflash/sysflash.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/../azure_iot/azure-sdk-for-c/sdk/inc
    ${ProjDirPath}/../azure_iot
    ${ProjDirPath}/../azure_iot/azure_iot_security_module
    ${ProjDirPath}/../azure_iot/azure_iot_security_module/inc
    ${ProjDirPath}/../azure_iot/azure_iot_security_module/inc/configs/RTOS_BASE
    ${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/inc
    ${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime
    ${ProjDirPath}/../azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include
    ${ProjDirPath}/..
    ${ProjDirPath}/../mcuboot
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/rtos/azure-rtos
    ${SdkRootDirPath}/core/drivers/enet
    ${SdkRootDirPath}/core/components/phy/device/phyksz8081
    ${SdkRootDirPath}/core/drivers/trng
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/dcp
    ${SdkRootDirPath}/core/components/flash/mflash
    ${SdkRootDirPath}/core/components/flash/mflash/mimxrt1052
    ${SdkRootDirPath}/core/devices/MIMXRT1052/drivers
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT1052
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/components/phy
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_azure_rtos_azure_iot)

include(middleware_azure_rtos_nxd)

include(driver_enet)

include(driver_phy-device-ksz8081)

include(driver_trng)

include(driver_flexspi)

include(driver_dcp)

include(component_mflash_file_MIMXRT1052)

include(component_mflash_rt1050)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(middleware_azure_rtos_nxd_template_MIMXRT1052)

include(middleware_azure_rtos_fx)

include(middleware_azure_rtos_fx_template_MIMXRT1052)

include(middleware_azure_rtos_tx)

include(middleware_azure_rtos_tx_template_MIMXRT1052)

include(CMSIS_Include_core_cm)

include(driver_phy-common_MIMXRT1052)

include(driver_cache_armv7_m7)

include(component_mflash_common)

include(utilities_misc_utilities_MIMXRT1052)

include(device_system)


add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/netxduo ./CMakeFiles/libnetxduo)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/filex ./CMakeFiles/libfilex)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::netxduo")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::filex")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/azure_iot_embedded_sdk_adu.bin)

