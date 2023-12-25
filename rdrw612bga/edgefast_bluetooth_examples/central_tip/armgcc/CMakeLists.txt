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


project(central_tip)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME central_tip.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../app_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../central_tip.c"
"${ProjDirPath}/../central_tip.h"
"${ProjDirPath}/../edgefast_bluetooth_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs/ffconf.h"
"${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs/usb_host_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs
)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(component_osa)

include(driver_clock)

include(middleware_freertos-kernel_heap_4)

include(middleware_freertos-kernel_RW612)

include(middleware_edgefast_bluetooth_common_ethermind)

include(middleware_edgefast_bluetooth_config_ethermind)

include(middleware_edgefast_bluetooth_extension_common_ethermind)

include(middleware_edgefast_bluetooth_pal)

include(middleware_mbedtls_RW612)

include(middleware_edgefast_bluetooth_porting)

include(middleware_edgefast_bluetooth_pal_platform_ethermind)

include(middleware_edgefast_bluetooth_pal_host_msd_fatfs_ethermind)

include(middleware_edgefast_bluetooth_pal_db_gen_ethermind)

include(middleware_edgefast_bluetooth_ble_ethermind_cm33nodsp)

include(middleware_edgefast_bluetooth_porting_work_queue)

include(middleware_edgefast_bluetooth_porting_toolchain)

include(middleware_edgefast_bluetooth_porting_net)

include(middleware_edgefast_bluetooth_porting_list)

include(middleware_edgefast_bluetooth_porting_atomic)

include(middleware_edgefast_bluetooth_profile_tip)

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

include(component_mflash_file_RW612)

include(component_mflash_rdrw610)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_flexcomm_i2c)

include(driver_trng)

include(driver_cns_io_mux)

include(middleware_littlefs)

include(component_osa_free_rtos_RW612)

include(component_wireless_imu_adapter)

include(driver_gdma)

include(driver_imu)

include(middleware_edgefast_bluetooth_common_ethermind_hci)

include(middleware_edgefast_bluetooth_common_ethermind_hci_platform)

include(driver_conn_fwloader)

include(driver_ocotp)

include(middleware_usb_host_stack_RW612)

include(middleware_usb_host_ehci_RW612)

include(middleware_fatfs_RW612)

include(middleware_fatfs_usb)

include(middleware_edgefast_bluetooth_rw610_controller)

include(middleware_wireless_framework_platform_ble)

include(middleware_wireless_framework_platform_common)

include(middleware_mbedtls_port_els_pkc)

include(component_mem_manager)

include(component_messaging)

include(component_software_rng_adapter)

include(component_ostimer_time_stamp_adapter)

include(component_els_pkc_platform_rw61x)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_extension)

include(component_log)

include(utility_str)

include(component_log_backend_debugconsole_RW612)

include(middleware_edgefast_bluetooth_ble_ethermind_lib_cm33nodsp)

include(utilities_misc_utilities_RW612)

include(component_mflash_common)

include(component_osa_interface)

include(driver_memory)

include(middleware_usb_host_common_header)

include(middleware_usb_common_header)

include(middleware_usb_host_msd)

include(middleware_wireless_framework_platform)

include(middleware_wireless_framework_fwk_debug)

include(middleware_wireless_framework_platform_coex)

include(middleware_mbedtls_port_els)

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

include(middleware_mbedtls_els_pkc_config)

include(component_mem_manager_light)

include(driver_ostimer)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_ga.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_core.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_gatt.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_protocol.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_util.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm33nodsp/gcc/libethermind_ble_ga.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

