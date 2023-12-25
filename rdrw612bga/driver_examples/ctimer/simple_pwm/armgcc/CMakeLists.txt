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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(ctimer_pwm_example)

set(MCUX_BUILD_TYPES debug release flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME ctimer_pwm_example.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../simple_pwm.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/rdrw612bga
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_flexcomm_usart)

include(driver_ctimer)

include(driver_common)

include(driver_clock)

include(driver_power)

include(driver_lpc_gpio)

include(driver_inputmux)

include(device_CMSIS)

include(component_usart_adapter)

include(driver_flash_config_rdrw612bga)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_lists)

include(device_startup)

include(driver_flexcomm)

include(driver_reset)

include(driver_cns_io_mux)

include(component_silicon_id_RW612)

include(component_silicon_id_rw610)

include(component_els_pkc_platform_rw61x_standalone_clib_gdet_sensor)

include(driver_ocotp)

include(CMSIS_Include_core_cm)

include(driver_inputmux_connections)

include(utilities_misc_utilities_RW612)

include(component_els_pkc_els_header_only)

include(component_els_pkc_els_common)

include(component_els_pkc_core)

include(component_els_pkc_standalone_keyManagement)

include(component_els_pkc_memory)

include(component_els_pkc_param_integrity)

include(component_els_pkc_flow_protection)

include(component_els_pkc_secure_counter)

include(component_els_pkc_pre_processor)

include(component_els_pkc_data_integrity)

include(component_els_pkc_toolchain)

include(component_els_pkc_standalone_gdet)

include(component_els_pkc_platform_rw61x_interface_files)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

