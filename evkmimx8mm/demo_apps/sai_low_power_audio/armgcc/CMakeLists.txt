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


project(sai_low_power_audio)

set(MCUX_SDK_PROJECT_NAME sai_low_power_audio.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../lpm.c"
"${ProjDirPath}/../lpm.h"
"${ProjDirPath}/../sai_low_power_audio.c"
"${ProjDirPath}/../sai_low_power_audio.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../rsc_table.c"
"${ProjDirPath}/../rsc_table.h"
"${ProjDirPath}/../remoteproc.h"
"${ProjDirPath}/../../../../../core/boards/evkmimx8mm/board.c"
"${ProjDirPath}/../../../../../core/boards/evkmimx8mm/board.h"
"${ProjDirPath}/../../../../../core/boards/evkmimx8mm/clock_config.c"
"${ProjDirPath}/../../../../../core/boards/evkmimx8mm/clock_config.h"
"${ProjDirPath}/../fsl_iomuxc.h"
"${ProjDirPath}/../app_srtm.c"
"${ProjDirPath}/../app_srtm.h"
"${ProjDirPath}/../../../../../core/components/srtm/channels/srtm_rpmsg_endpoint.h"
"${ProjDirPath}/../../../../../core/components/srtm/channels/srtm_rpmsg_endpoint.c"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_audio_service.h"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_audio_service.c"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_sai_sdma_adapter.h"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_sai_sdma_adapter.c"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_i2c_codec_adapter.h"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_i2c_codec_adapter.c"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_wm8524_adapter.h"
"${ProjDirPath}/../../../../../core/components/srtm/services/srtm_wm8524_adapter.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../examples/evkmimx8mm/demo_apps/sai_low_power_audio debug
    ${ProjDirPath}/../../../../../core/boards/evkmimx8mm debug
    ${ProjDirPath}/../../../../../core/boards/evkmimx8mm
    ${ProjDirPath}/../../../../../examples/evkmimx8mm/demo_apps/sai_low_power_audio
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../core/components/srtm/channels
    ${ProjDirPath}/../../../../../core/components/srtm/services
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
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

include(component_codec_ak4497_adapter)

include(component_codec_i2c_MIMX8MM6)

include(component_ii2c_adapter)

include(component_codec_wm8524_adapter)

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

include(CMSIS_Include_core_cm4)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(driver_sdma)

include(driver_memory)

include(driver_mu)

include(driver_ak4497)

include(driver_srtm_freertos)

include(driver_srtm)

include(driver_ii2c_freertos)

include(driver_ii2c)

include(middleware_freertos-kernel_MIMX8MM6)

include(middleware_freertos-kernel_extension)

include(middleware_multicore_rpmsg_lite_imx8mm_m4_freertos)

include(middleware_multicore_rpmsg_lite)

include(middleware_freertos-kernel_heap_4)

include(driver_wm8524)

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

