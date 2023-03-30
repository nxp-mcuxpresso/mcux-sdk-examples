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


project(dcic_cm4)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_debug flexspi_nor_sdram_release sdram_debug sdram_release)

set(MCUX_SDK_PROJECT_NAME dcic_cm4.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../dcic.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../display_support.h"
"${ProjDirPath}/../display_support.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1170/dcd.h"
"${ProjDirPath}/../evkmimxrt1170_connect_cm4_cm4side_sdram.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimxrt1170
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/display
    ${SdkRootDirPath}/core/components/video/display/rm68191
    ${SdkRootDirPath}/core/drivers/mipi_dsi_split
    ${SdkRootDirPath}/core/components/video/display/mipi_dsi_cmd
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/video/display/rm68200
    ${SdkRootDirPath}/core/components/video/display/dc/lcdifv2
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/drivers/elcdif
    ${SdkRootDirPath}/core/components/video/display/dc/elcdif
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/video/display/hx8394
    ${SdkRootDirPath}/core/drivers/dcic
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm4
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xmcd
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/drivers/lcdifv2
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_video-common)

include(driver_display-common)

include(driver_display-rm68191)

include(driver_mipi_dsi_split)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_display-rm68200)

include(driver_dc-fb-lcdifv2)

include(driver_soc_src)

include(driver_elcdif)

include(driver_dc-fb-elcdif)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_display-hx8394)

include(driver_dcic)

include(driver_clock)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm4)

include(driver_iomuxc)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_lmem)

include(driver_xmcd_evkmimxrt1170)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_soc_mipi_csi2rx)

include(driver_dc-fb-common)

include(driver_lcdifv2)

include(driver_memory)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm4)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


