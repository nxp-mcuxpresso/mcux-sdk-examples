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


project(nbu_bb_bm)

set(MCUX_BUILD_TYPES flash_debug)

set(MCUX_SDK_PROJECT_NAME nbu_bb_bm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/examples/rdrw612bga/board.c"
"${SdkRootDirPath}/examples/rdrw612bga/board.h"
"${SdkRootDirPath}/examples/rdrw612bga/clock_config.c"
"${SdkRootDirPath}/examples/rdrw612bga/clock_config.h"
"${ProjDirPath}/../hardware_init.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../nbu_bb_bm.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/rdrw612bga
    ${SdkRootDirPath}/middleware/wireless/framework/Common
    ${SdkRootDirPath}/middleware/wireless/framework/FunctionLib
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(component_lists)

include(component_osa_bm)

include(component_common_task)

include(component_mem_manager_light)

include(component_wireless_imu_adapter)

include(driver_common)

include(driver_gdma)

include(driver_imu)

include(component_usart_adapter)

include(component_serial_manager_uart)

include(component_serial_manager)

include(driver_flexcomm_usart)

include(component_power_manager_rdrw610)

include(driver_power)

include(driver_flexspi)

include(driver_cache_cache64)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_cns_io_mux)

include(driver_lpc_rtc)

include(driver_flash_config_rdrw612bga)

include(driver_clock)

include(driver_conn_fwloader)

include(driver_ocotp)

include(utility_debug_console)

include(utility_assert)

include(component_els_pkc_platform_rw61x)

include(middleware_wireless_framework_Common)

include(middleware_wireless_framework_function_lib)

include(device_CMSIS)

include(device_startup)

include(driver_reset)

include(CMSIS_Include_core_cm)

include(component_osa_interface)

include(component_osa)

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

include(utilities_misc_utilities_RW612)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


