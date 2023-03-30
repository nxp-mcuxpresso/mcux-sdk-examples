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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(enet_txrx_multiring_transfer_cm4)

set(MCUX_BUILD_TYPES debug release sdram_debug sdram_release flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME enet_txrx_multiring_transfer_cm4.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../enet_txrx_multiring_transfer.c"
"${SdkRootDirPath}/examples/evkbmimxrt1170/driver_examples/enet/txrx_multiring_transfer/pin_mux.c"
"${SdkRootDirPath}/examples/evkbmimxrt1170/driver_examples/enet/txrx_multiring_transfer/pin_mux.h"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/board.c"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/board.h"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/clock_config.c"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/clock_config.h"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/dcd.c"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/dcd.h"
"${ProjDirPath}/../evkbmimxrt1170_connect_cm4_cm4side.jlinkscript"
"${ProjDirPath}/../evkbmimxrt1170_connect_cm4_cm4side_sdram.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/evkbmimxrt1170/driver_examples/enet/txrx_multiring_transfer
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkbmimxrt1170
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/phy/device/phyrtl8211f
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/silicon_id
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/silicon_id/socs/imxrt1170
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/drivers/enet
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/boards/evkbmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm4
    ${SdkRootDirPath}/core/components/phy
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_phy-device-rtl8211f)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(component_silicon_id)

include(driver_common)

include(component_silicon_id_rt1170)

include(driver_clock)

include(driver_enet)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm4)

include(driver_iomuxc)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkbmimxrt1170)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_lmem)

include(driver_phy-common)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm4)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


