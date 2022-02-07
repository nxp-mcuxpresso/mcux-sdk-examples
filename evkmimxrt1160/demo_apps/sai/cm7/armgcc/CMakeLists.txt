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


project(sai_cm7)

set(MCUX_SDK_PROJECT_NAME sai_cm7.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../recordtoSdcard.c"
"${ProjDirPath}/../sai.h"
"${ProjDirPath}/../sai.c"
"${ProjDirPath}/../recordPlayback.c"
"${ProjDirPath}/../playbackSineWave.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/board.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/board.h"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/clock_config.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/clock_config.h"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/dcd.c"
"${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160/dcd.h"
"${ProjDirPath}/../evkmimxrt1160_connect_cm4_cm7side.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../examples/evkmimxrt1160/demo_apps/sai/cm7
    ${ProjDirPath}/../../../../../../core/boards/evkmimxrt1160
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_lpi2c)

include(driver_wm8960)

include(CMSIS_DSP_Library)

include(driver_usdhc)

include(middleware_fatfs)

include(middleware_fatfs_sd)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_interrupt_MIMXRT1166_cm7)

include(driver_codec)

include(driver_common)

include(component_wm8960_adapter)

include(component_codec_i2c_MIMXRT1166_cm7)

include(component_lpi2c_adapter)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_clock)

include(driver_sai)

include(driver_sai_edma)

include(driver_dmamux)

include(driver_edma)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(driver_lpuart)

include(device_startup_MIMXRT1166_cm7)

include(driver_iomuxc)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(middleware_sdmmc_osa_bm)

include(component_osa_bm)

include(utilities_misc_utilities)

include(device_system_MIMXRT1166_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL sdram_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL sdram_debug)

if(CMAKE_BUILD_TYPE STREQUAL sdram_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL sdram_release)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM7lfdp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


