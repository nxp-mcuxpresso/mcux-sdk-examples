# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

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

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(rpmsg_lite_str_echo_rtos_imxcm4)

set(MCUX_SDK_PROJECT_NAME rpmsg_lite_str_echo_rtos_imxcm4.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_remote.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../rsc_table.c"
"${ProjDirPath}/../rsc_table.h"
"${ProjDirPath}/../remoteproc.h"
"${ProjDirPath}/../../../../../core/boards/mekmimx8qx/board.c"
"${ProjDirPath}/../../../../../core/boards/mekmimx8qx/board.h"
"${ProjDirPath}/../../../../../core/boards/mekmimx8qx/clock_config.c"
"${ProjDirPath}/../../../../../core/boards/mekmimx8qx/clock_config.h"
"${ProjDirPath}/../app_srtm.c"
"${ProjDirPath}/../app_srtm.h"
"${ProjDirPath}/../srtm/channels/srtm_rpmsg_endpoint.h"
"${ProjDirPath}/../srtm/channels/srtm_rpmsg_endpoint.c"
"${ProjDirPath}/../srtm/channels/srtm_mu_endpoint.h"
"${ProjDirPath}/../srtm/channels/srtm_mu_endpoint.c"
"${ProjDirPath}/../srtm/services/srtm_i2c_service.h"
"${ProjDirPath}/../srtm/services/srtm_i2c_service.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/.. debug
    ${ProjDirPath}/.. debug
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../examples/mekmimx8qx/multicore_examples/rpmsg_lite_str_echo_rtos
    ${ProjDirPath}/../../../../../core/boards/mekmimx8qx
    ${ProjDirPath}/../srtm/channels
    ${ProjDirPath}/../srtm/services
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_cs42888)

include(driver_irqsteer)

include(driver_lpi2c)

include(middleware_freertos-kernel_heap_4)

include(driver_codec)

include(driver_common)

include(component_cs42888_adapter)

include(component_lpi2c_adapter)

include(middleware_multicore_rpmsg_lite_imx8qx_m4_freertos)

include(component_codec_i2c_MIMX8QX6)

include(driver_clock)

include(driver_mu)

include(middleware_freertos-kernel_MIMX8QX6)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_scfw_api)

include(driver_igpio)

include(utility_assert)

include(driver_dma3)

include(driver_memory)

include(CMSIS_Include_core_cm4)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(driver_lpi2c_freertos)

include(middleware_freertos-kernel_extension)

include(driver_sai_edma)

include(driver_sai)

include(driver_srtm_freertos)

include(driver_srtm)

include(middleware_multicore_rpmsg_lite)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/rpmsg_lite_str_echo_rtos.bin)

