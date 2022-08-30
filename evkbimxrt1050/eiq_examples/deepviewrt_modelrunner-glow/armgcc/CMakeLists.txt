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


project(deepviewrt_modelrunner-glow)

set(MCUX_SDK_PROJECT_NAME deepviewrt_modelrunner-glow.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../source/modelrunner.c"
"${ProjDirPath}/../source/modelrunner.h"
"${ProjDirPath}/../source/modelhttp.c"
"${ProjDirPath}/../source/picohttp.c"
"${ProjDirPath}/../source/picohttp.h"
"${ProjDirPath}/../source/picohttpparser.c"
"${ProjDirPath}/../source/picohttpparser.h"
"${ProjDirPath}/../source/lwip_dhcp_freertos.c"
"${ProjDirPath}/../source/server_utils.c"
"${ProjDirPath}/../source/lwipopts.h"
"${ProjDirPath}/../source/FreeRTOSConfig.h"
"${ProjDirPath}/../source/model.h"
"${ProjDirPath}/../source/model.o"
"${ProjDirPath}/../source/model.weights.txt"
"${ProjDirPath}/../source/bald_eagle.inc"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../cr_section_macros.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../source
    ${ProjDirPath}/..
    ${SdkRootDirPath}/rtos/freertos/libraries/abstractions/posix/include
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_lwip_apps_httpsrv)

include(middleware_lwip)

include(middleware_freertos-kernel_MIMXRT1052)

include(middleware_lwip_enet_ethernetif_MIMXRT1052)

include(driver_phy-common)

include(driver_enet)

include(middleware_lwip_empty_ethernetif)

include(middleware_freertos-kernel_heap_3)

include(driver_cache_armv7_m7)

include(driver_phy-device-ksz8081)

include(driver_pxp)

include(driver_igpio)

include(driver_mdio-enet)

include(driver_gpt)

include(middleware_eiq_glow)

include(middleware_eiq_deepviewrt_deps_json)

include(middleware_eiq_deepviewrt_deps_stb)

include(middleware_eiq_deepviewrt_deps_flatcc)

include(component_silicon_id)

include(driver_common)

include(middleware_eiq_deepviewrt)

include(component_lists)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_serial_manager_uart)

include(driver_lpuart)

include(utility_debug_console)

include(device_CMSIS)

include(device_startup)

include(driver_clock)

include(driver_iomuxc)

include(driver_xip_device)

include(driver_xip_board_evkbimxrt1050)

include(driver_lpi2c)

include(utility_assert)

include(middleware_freertos-kernel_extension)

include(driver_mdio-common)

include(CMSIS_Include_core_cm)

include(device_system)

include(utilities_misc_utilities_MIMXRT1052)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/deepviewrt/lib/libdeepview-rt-cortex-m7f.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/deepviewrt/lib/libdeepview-rt-cortex-m7f.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


