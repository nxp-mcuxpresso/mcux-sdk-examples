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


project(lcdifv2_porter_duff_cm7)

set(MCUX_SDK_PROJECT_NAME lcdifv2_porter_duff_cm7.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../lcdifv2_porter_duff.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../../../lcdifv2_support.c"
"${ProjDirPath}/../../../lcdifv2_support.h"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170/board.c"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170/board.h"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170/clock_config.c"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170/clock_config.h"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170/dcd.c"
"${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170/dcd.h"
"${ProjDirPath}/../evkmimxrt1170_connect_cm4_cm7side.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../../examples/evkmimxrt1170/driver_examples/lcdifv2
    ${ProjDirPath}/../../../../../../../examples/evkmimxrt1170/driver_examples/lcdifv2/porter_duff/cm7
    ${ProjDirPath}/../../../../../../../core/boards/evkmimxrt1170
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_video-common)

include(driver_display-common)

include(driver_display-rm68191)

include(driver_mipi_dsi_split)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_display-rm68200)

include(driver_soc_src)

include(driver_lcdifv2)

include(driver_clock)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm7)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_soc_mipi_csi2rx)

include(driver_memory)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


