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


project(power_mode_switch)

set(MCUX_BUILD_TYPES release flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME power_mode_switch.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../lpm.c"
"${ProjDirPath}/../lpm.h"
"${ProjDirPath}/lpm_asm.S"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../clock_scg.c"
"${ProjDirPath}/../power_mode_switch.c"
"${ProjDirPath}/../power_mode_switch.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../rsc_table.c"
"${ProjDirPath}/../rsc_table.h"
"${ProjDirPath}/../remoteproc.h"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/board.c"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/board.h"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmcimx7ulp/clock_config.h"
"${ProjDirPath}/../srtm_config.h"
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

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmcimx7ulp
    ${ProjDirPath}/../srtm/channels
    ${ProjDirPath}/../srtm/services
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/core/drivers/port
    ${SdkRootDirPath}/core/drivers/llwu
    ${SdkRootDirPath}/core/drivers/lptmr
    ${SdkRootDirPath}/core/drivers/gpio
    ${SdkRootDirPath}/core/devices/MCIMX7U5/drivers
    ${SdkRootDirPath}/core/devices/MCIMX7U5
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/drivers/msmc
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/edma
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/drivers/pmc0
    ${SdkRootDirPath}/core/drivers/snvs_lp
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core/components/pf1550
    ${SdkRootDirPath}/core/drivers/sai
    ${SdkRootDirPath}/core/components/codec/wm8960
    ${SdkRootDirPath}/core/components/srtm/port
    ${SdkRootDirPath}/core/components/srtm
    ${SdkRootDirPath}/core/middleware/issdk
    ${SdkRootDirPath}/core/CMSIS/Driver/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(component_codec_i2c_MCIMX7U5)

include(driver_common)

include(component_lpi2c_adapter)

include(driver_codec)

include(component_wm8960_adapter)

include(middleware_multicore_rpmsg_lite_MCIMX7U5)

include(middleware_multicore_rpmsg_lite_freertos)

include(driver_port)

include(driver_llwu)

include(driver_lptmr)

include(driver_gpio)

include(driver_clock)

include(driver_iomuxc)

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

include(CMSIS_Include_core_cm)

include(driver_edma_MCIMX7U5)

include(driver_lpi2c)

include(driver_lpi2c_freertos)

include(middleware_freertos-kernel_MCIMX7U5)

include(middleware_freertos-kernel_extension)

include(driver_pmc0)

include(driver_snvs_lp)

include(driver_mu)

include(driver_pf1550)

include(driver_sai)

include(driver_sai_edma)

include(driver_wm8960)

include(driver_srtm_freertos)

include(driver_srtm_MCIMX7U5)

include(middleware_multicore_rpmsg_lite_imx7ulp_m4_freertos)

include(middleware_freertos-kernel_heap_4)

include(middleware_issdk_sensor_allregdefs)

include(CMSIS_Driver_Include_I2C)

include(CMSIS_Driver_Include_Common)

include(CMSIS_Driver_Include_SPI)

include(middleware_issdk_algorithms_pedometer_lib_cm4)

include(middleware_issdk_algorithms_pedometer_common)

include(utilities_misc_utilities_MCIMX7U5)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/middleware/issdk/algorithms/pedometer/lib/libpedometerm4.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/imx7ulp_m4_demo.bin)

