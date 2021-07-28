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

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(power_mode_switch_bm_core0)

set(MCUX_SDK_PROJECT_NAME power_mode_switch_bm_core0.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../power_mode_switch_core0.c"
"${ProjDirPath}/../../common/lpm.c"
"${ProjDirPath}/../../common/lpm.h"
"${ProjDirPath}/../../common/chip_init_def.h"
"${ProjDirPath}/../../common/setpoint_table_def.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160/board.c"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160/board.h"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160/clock_config.c"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160/clock_config.h"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160/dcd.c"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160/dcd.h"
"${ProjDirPath}/../evkmimxrt1160_connect_cm4_cm7side.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../common
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../../examples/evkmimxrt1160/demo_apps/power_mode_switch/bm/core0
    ${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1160
)

if(CMAKE_BUILD_TYPE STREQUAL debug OR CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../core1/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL debug OR CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL release OR CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../core1/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL release OR CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_igpio)

include(driver_dcdc_soc)

include(driver_mu)

include(driver_gpc_3)

include(driver_pgmc)

include(driver_soc_src)

include(driver_pmu_1)

include(driver_ssarc)

include(utility_incbin)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup_MIMXRT1166_cm7)

include(driver_iomuxc)

include(utility_assert)

include(driver_xip_device)

include(driver_xip_board)

include(driver_cache_armv7_m7)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities)

include(device_system_MIMXRT1166_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


