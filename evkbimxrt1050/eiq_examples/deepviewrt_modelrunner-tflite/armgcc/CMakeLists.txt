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


project(deepviewrt_modelrunner-tflite)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_release flexspi_nor_sdram_debug)

set(MCUX_SDK_PROJECT_NAME deepviewrt_modelrunner-tflite.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../source/main.c"
"${ProjDirPath}/../source/lwipopts.h"
"${ProjDirPath}/../source/FreeRTOSConfig.h"
"${ProjDirPath}/../source/modelrunner.h"
"${ProjDirPath}/../source/modelrunner_server.h"
"${ProjDirPath}/../source/modelrunner.c"
"${ProjDirPath}/../source/http_handler.c"
"${ProjDirPath}/../source/tf_benchmark.cpp"
"${ProjDirPath}/../source/tf_benchmark.h"
"${ProjDirPath}/../source/picohttp.c"
"${ProjDirPath}/../source/picohttp.h"
"${ProjDirPath}/../source/model.h"
"${ProjDirPath}/../source/picohttpparser.c"
"${ProjDirPath}/../source/picohttpparser.h"
"${ProjDirPath}/../source/get_top_n.cpp"
"${ProjDirPath}/../source/get_top_n.h"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/../source
    ${ProjDirPath}/..
    ${SdkRootDirPath}/rtos/freertos/libraries/abstractions/posix/include
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/middleware/lwip
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/components/phy
    ${SdkRootDirPath}/core/drivers/enet
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/core/components/phy/device/phyksz8081
    ${SdkRootDirPath}/core/drivers/pxp
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/drivers/gpt
    ${SdkRootDirPath}/core/components/silicon_id
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/silicon_id/socs/rt10xx
    ${SdkRootDirPath}/middleware/eiq
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1052
    ${SdkRootDirPath}/core/devices/MIMXRT1052/drivers
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/devices/MIMXRT1052/xip
    ${SdkRootDirPath}/core/boards/evkbimxrt1050/xip
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/third_party/cmsis
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_lwip_apps_httpsrv)

include(middleware_lwip)

include(middleware_freertos-kernel_MIMXRT1052)

include(middleware_lwip_enet_ethernetif_MIMXRT1052)

include(driver_phy-common_MIMXRT1052)

include(driver_enet)

include(middleware_lwip_empty_ethernetif)

include(middleware_freertos-kernel_heap_3)

include(driver_cache_armv7_m7)

include(driver_phy-device-ksz8081)

include(driver_pxp)

include(driver_igpio)

include(driver_gpt)

include(component_silicon_id_MIMXRT1052)

include(driver_common)

include(component_silicon_id_rt10xx)

include(middleware_eiq_deepviewrt_deps_json)

include(middleware_eiq_deepviewrt_deps_stb)

include(middleware_eiq_deepviewrt_deps_flatcc)

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(middleware_eiq_deepviewrt)

include(component_lists)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup)

include(driver_clock)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_xip_device)

include(driver_xip_board_evkbimxrt1050)

include(utility_assert)

include(utility_debug_console)

include(middleware_freertos-kernel_extension)

include(CMSIS_Include_core_cm)

include(middleware_eiq_tensorflow_lite_micro)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)

include(utilities_misc_utilities_MIMXRT1052)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/deepviewrt/lib/libdeepview-rt-cortex-m7f.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/deepviewrt/lib/libdeepview-rt-cortex-m7f.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


