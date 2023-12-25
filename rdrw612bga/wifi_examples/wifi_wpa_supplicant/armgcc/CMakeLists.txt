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


project(wifi_wpa_supplicant)

set(MCUX_BUILD_TYPES flash_release flash_debug)

set(MCUX_SDK_PROJECT_NAME wifi_wpa_supplicant.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/examples/rdrw612bga/wifi_examples/common/lpm/lpm.c"
"${SdkRootDirPath}/examples/rdrw612bga/wifi_examples/common/lpm/lpm.h"
"${SdkRootDirPath}/examples/rdrw612bga/wifi_examples/common/lpm/host_sleep.c"
"${SdkRootDirPath}/examples/rdrw612bga/wifi_examples/common/lpm/host_sleep.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../lwippools.h"
"${ProjDirPath}/../lwiphooks.h"
"${ProjDirPath}/../usb/usb_support.h"
"${ProjDirPath}/../usb/usb_support.c"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../usb/usb_api.h"
"${ProjDirPath}/../usb/host_msd_fatfs.h"
"${ProjDirPath}/../usb/host_msd_fatfs.c"
"${ProjDirPath}/../usb/ffconf.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../wifi_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../app_config.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/rdrw612bga
    ${SdkRootDirPath}/examples/rdrw612bga/wifi_examples/common/lpm
    ${ProjDirPath}/../usb
    ${SdkRootDirPath}/examples/rdrw612bga/wifi_examples/common
)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="wpa_supp_els_pkc_mbedtls_config.h" LWIP_HOOK_FILENAME="lwip_default_hooks.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="wpa_supp_els_pkc_mbedtls_config.h" LWIP_HOOK_FILENAME="lwip_default_hooks.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_freertos-kernel_cm33_nonsecure_port)

include(middleware_wifi_wifidriver_RW612)

include(middleware_freertos-kernel_RW612)

include(utility_debug_console)

include(middleware_wifi_imu)

include(middleware_wifi_common_files)

include(component_wireless_imu_adapter)

include(driver_common)

include(driver_gdma)

include(driver_imu)

include(component_osa_free_rtos_RW612)

include(driver_flexcomm_usart_freertos)

include(driver_conn_fwloader)

include(driver_ocotp)

include(driver_lpc_rtc)

include(component_power_manager_rdrw610)

include(middleware_wireless_wpa_supplicant_rtos)

include(middleware_wireless_wpa_supplicant_dpp)

include(driver_trng)

include(middleware_mbedtls_RW612)

include(component_els_pkc_platform_rw61x)

include(middleware_mbedtls_port_els_pkc)

include(middleware_usb_host_stack_RW612)

include(component_osa)

include(middleware_usb_host_ehci_RW612)

include(middleware_usb_phy)

include(middleware_freertos-kernel_heap_4)

include(middleware_lwip_RW612)

include(middleware_wifi_RW612)

include(middleware_lwip_apps_lwiperf_RW612)

include(middleware_wifi_cli_RW612)

include(middleware_fatfs_RW612)

include(middleware_fatfs_usb)

include(driver_clock)

include(device_CMSIS)

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

include(middleware_freertos-kernel_extension)

include(CMSIS_Include_core_cm)

include(component_osa_interface)

include(component_wifi_bt_module_tx_pwr_limits)

include(component_power_manager_core)

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

include(driver_memory)

include(middleware_usb_host_common_header)

include(middleware_usb_common_header)

include(middleware_usb_host_msd)

include(utilities_misc_utilities_RW612)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

