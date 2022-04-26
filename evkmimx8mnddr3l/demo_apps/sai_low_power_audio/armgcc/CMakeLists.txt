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


project(sai_low_power_audio)

set(MCUX_SDK_PROJECT_NAME sai_low_power_audio.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../sai_low_power_audio.c"
"${ProjDirPath}/../sai_low_power_audio.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../lpm.c"
"${ProjDirPath}/../lpm.h"
"${ProjDirPath}/../rsc_table.c"
"${ProjDirPath}/../rsc_table.h"
"${ProjDirPath}/../remoteproc.h"
"${ProjDirPath}/../fsl_iomuxc.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../app_srtm.c"
"${ProjDirPath}/../app_srtm.h"
"${SdkRootDirPath}/core/components/srtm/channels/srtm_rpmsg_endpoint.h"
"${SdkRootDirPath}/core/components/srtm/channels/srtm_rpmsg_endpoint.c"
"${SdkRootDirPath}/core/components/srtm/services/srtm_audio_service.h"
"${SdkRootDirPath}/core/components/srtm/services/srtm_audio_service.c"
"${SdkRootDirPath}/core/components/srtm/services/srtm_sai_sdma_adapter.h"
"${SdkRootDirPath}/core/components/srtm/services/srtm_sai_sdma_adapter.c"
"${SdkRootDirPath}/core/components/srtm/services/srtm_wm8524_adapter.h"
"${SdkRootDirPath}/core/components/srtm/services/srtm_wm8524_adapter.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../..
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkmimx8mnddr3l/demo_apps/sai_low_power_audio
    ${SdkRootDirPath}/core/boards/evkmimx8mnddr3l
    ${SdkRootDirPath}/core/components/srtm/channels
    ${SdkRootDirPath}/core/components/srtm/services
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_igpio)

include(driver_iuart)

include(driver_gpc_2)

include(driver_gpt)

include(driver_codec)

include(driver_common)

include(component_codec_adapters)

include(component_wm8524_adapter)

include(middleware_multicore_rpmsg_lite_MIMX8MN6)

include(middleware_multicore_rpmsg_lite_freertos)

include(driver_clock)

include(driver_sai)

include(driver_sai_sdma)

include(driver_rdc)

include(device_CMSIS)

include(utility_debug_console)

include(component_iuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(device_startup)

include(utility_assert)

include(CMSIS_Include_core_cm)

include(driver_sdma)

include(driver_memory)

include(driver_mu)

include(driver_wm8524)

include(driver_srtm_freertos)

include(driver_srtm_MIMX8MN6)

include(middleware_multicore_rpmsg_lite_imx8mn_m7_freertos)

include(middleware_freertos-kernel_MIMX8MN6)

include(middleware_freertos-kernel_extension)

include(middleware_freertos-kernel_heap_4)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sai_low_power_audio.bin)

