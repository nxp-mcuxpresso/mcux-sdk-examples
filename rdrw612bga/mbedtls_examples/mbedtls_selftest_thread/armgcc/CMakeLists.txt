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


project(mbedtls_selftest_thread)

set(MCUX_BUILD_TYPES debug release flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME mbedtls_selftest_thread.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../selftest_thread.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/rdrw612bga
)

if(CMAKE_BUILD_TYPE STREQUAL debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_thread_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_thread_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_thread_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_thread_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_trng)

include(component_els_pkc_platform_rw61x)

include(middleware_mbedtls_port_els_pkc)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(middleware_mbedtls_RW612)

include(driver_clock)

include(middleware_freertos-kernel_heap_4)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_power)

include(driver_flash_config_rdrw612bga)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_reset)

include(driver_cns_io_mux)

include(component_silicon_id_RW612)

include(component_silicon_id_rw610)

include(component_els_pkc_platform_rw61x_standalone_clib_gdet_sensor)

include(driver_ocotp)

include(CMSIS_Include_core_cm)

include(component_els_pkc)

include(component_els_pkc_els)

include(component_els_pkc_hash)

include(component_els_pkc_session)

include(component_els_pkc_flow_protection)

include(component_els_pkc_core)

include(component_els_pkc_secure_counter)

include(component_els_pkc_pre_processor)

include(component_els_pkc_memory)

include(component_els_pkc_param_integrity)

include(component_els_pkc_data_integrity)

include(component_els_pkc_toolchain)

include(component_els_pkc_random)

include(component_els_pkc_random_modes)

include(component_els_pkc_aes)

include(component_els_pkc_key)

include(component_els_pkc_ecc)

include(component_els_pkc_pkc)

include(component_els_pkc_math)

include(component_els_pkc_rsa)

include(component_els_pkc_trng)

include(component_els_pkc_prng)

include(component_els_pkc_hashmodes)

include(component_els_pkc_hmac)

include(component_els_pkc_mac_modes)

include(component_els_pkc_mac)

include(component_els_pkc_padding)

include(component_els_pkc_aead_modes)

include(component_els_pkc_aead)

include(component_els_pkc_cipher_modes)

include(component_els_pkc_cipher)

include(component_els_pkc_els_header_only)

include(component_els_pkc_els_common)

include(component_els_pkc_standalone_keyManagement)

include(component_els_pkc_standalone_gdet)

include(component_els_pkc_platform_rw61x_interface_files)

include(component_els_pkc_trng_type_rng4)

include(middleware_mbedtls_port_els)

include(middleware_mbedtls_els_pkc_config)

include(middleware_freertos-kernel_RW612)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities_RW612)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

