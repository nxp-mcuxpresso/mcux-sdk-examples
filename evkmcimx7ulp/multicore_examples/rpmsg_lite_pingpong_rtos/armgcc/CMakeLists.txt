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


project(rpmsg_lite_pingpong_rtos_imxcm4)

set(MCUX_SDK_PROJECT_NAME rpmsg_lite_pingpong_rtos_imxcm4.elf)

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
"${ProjDirPath}/../../../../../core/boards/evkmcimx7ulp/board.c"
"${ProjDirPath}/../../../../../core/boards/evkmcimx7ulp/board.h"
"${ProjDirPath}/../../../../../core/boards/evkmcimx7ulp/clock_config.c"
"${ProjDirPath}/../../../../../core/boards/evkmcimx7ulp/clock_config.h"
"${ProjDirPath}/../fsl_iomuxc.h"
"${ProjDirPath}/../app_srtm.c"
"${ProjDirPath}/../app_srtm.h"
"${ProjDirPath}/../srtm/channels/srtm_rpmsg_endpoint.h"
"${ProjDirPath}/../srtm/channels/srtm_rpmsg_endpoint.c"
"${ProjDirPath}/../srtm/services/srtm_lfcl_service.h"
"${ProjDirPath}/../srtm/services/srtm_lfcl_service.c"
"${ProjDirPath}/../srtm/services/srtm_pmic_service.h"
"${ProjDirPath}/../srtm/services/srtm_pmic_service.c"
"${ProjDirPath}/../srtm/services/srtm_pf1550_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_pf1550_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_audio_service.h"
"${ProjDirPath}/../srtm/services/srtm_audio_service.c"
"${ProjDirPath}/../srtm/services/srtm_sai_edma_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_sai_edma_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_i2c_codec_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_i2c_codec_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_io_service.h"
"${ProjDirPath}/../srtm/services/srtm_io_service.c"
"${ProjDirPath}/../srtm/services/srtm_keypad_service.h"
"${ProjDirPath}/../srtm/services/srtm_keypad_service.c"
"${ProjDirPath}/../srtm/services/srtm_rtc_service.h"
"${ProjDirPath}/../srtm/services/srtm_rtc_service.c"
"${ProjDirPath}/../srtm/services/srtm_snvs_lp_rtc_adapter.h"
"${ProjDirPath}/../srtm/services/srtm_snvs_lp_rtc_adapter.c"
"${ProjDirPath}/../srtm/services/srtm_sensor_service.h"
"${ProjDirPath}/../srtm/services/srtm_sensor_service.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/.. debug
    ${ProjDirPath}/.. debug
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../examples/evkmcimx7ulp/multicore_examples/rpmsg_lite_pingpong_rtos
    ${ProjDirPath}/../../../../../core/boards/evkmcimx7ulp
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
include(component_codec_i2c_MCIMX7U5)

include(driver_common)

include(component_lpi2c_adapter)

include(driver_codec)

include(component_wm8960_adapter)

include(driver_gpio)

include(driver_port)

include(driver_clock)

include(driver_mu)

include(middleware_freertos-kernel_MCIMX7U5)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_msmc)

include(utility_assert)

include(driver_dmamux)

include(CMSIS_Include_core_cm4)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(driver_edma)

include(driver_lpi2c)

include(driver_lpi2c_freertos)

include(middleware_freertos-kernel_extension)

include(driver_pmc0)

include(driver_llwu)

include(driver_snvs_lp)

include(driver_pf1550)

include(driver_sai)

include(driver_sai_edma)

include(driver_wm8960)

include(driver_srtm_freertos)

include(driver_srtm)

include(middleware_multicore_rpmsg_lite_imx7ulp_m4_freertos)

include(middleware_multicore_rpmsg_lite)

include(middleware_freertos-kernel_heap_4)

include(middleware_issdk_sensor_allregdefs)

include(CMSIS_Driver_Include_I2C)

include(CMSIS_Driver_Include_Common)

include(CMSIS_Driver_Include_SPI)

include(middleware_issdk_algorithms_pedometer_lib_cm4)

include(middleware_issdk_algorithms_pedometer_common)

include(utilities_misc_utilities)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/rpmsg_lite_pingpong_rtos.bin)

