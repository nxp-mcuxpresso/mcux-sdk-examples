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
    SET(SdkRootDirPath "${ProjDirPath}/../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(mcuboot_opensource)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME mcuboot_opensource.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../boot.c"
"${ProjDirPath}/../boot.h"
"${ProjDirPath}/../bootutil_hooks.c"
"${ProjDirPath}/../keys.c"
"${ProjDirPath}/../os.c"
"${ProjDirPath}/../sbl_boot.c"
"${ProjDirPath}/../include/mbedtls_config.h"
"${ProjDirPath}/../include/mcuboot_config/mcuboot_config.h"
"${ProjDirPath}/../include/mcuboot_config/mcuboot_logging.h"
"${ProjDirPath}/../include/sbl.h"
"${ProjDirPath}/../include/sbldef.h"
"${ProjDirPath}/../include/flash_map.h"
"${ProjDirPath}/../include/flash_map_backend/flash_map_backend.h"
"${ProjDirPath}/../include/os/os_malloc.h"
"${ProjDirPath}/../include/sysflash/sysflash.h"
"${ProjDirPath}/../flashiap/flash_api.c"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../sblconfig.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../flash_partitioning.h"
"${ProjDirPath}/../flash_partitioning.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1060/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1060/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1060/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1060/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1060/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1060/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${ProjDirPath}/../include
    ${ProjDirPath}/../include/mcuboot_config
    ${ProjDirPath}/../include/sysflash
    ${ProjDirPath}/../flashiap
    ${ProjDirPath}/../secure
    ${SdkRootDirPath}/core/boards/evkmimxrt1060
)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/middleware/mbedtls
    ${SdkRootDirPath}/core/components/flash/mflash/mimxrt1062
    ${SdkRootDirPath}/middleware/mcuboot_opensource/boot/bootutil
    ${SdkRootDirPath}/core/devices/MIMXRT1062/drivers
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT1062
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1062/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1060/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/dcp
    ${SdkRootDirPath}/core/drivers/trng
    ${SdkRootDirPath}/core/components/flash/mflash
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_cache_armv7_m7)

include(middleware_mbedtls_port_ksdk)

include(middleware_mbedtls_MIMXRT1062)

include(middleware_mbedtls_rt)

include(component_mflash_rt1060)

include(middleware_mcuboot_bootutil)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart_MIMXRT1062)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1060)

include(CMSIS_Include_core_cm)

include(driver_dcp)

include(driver_trng)

include(component_mflash_common)

include(driver_flexspi)

include(utilities_misc_utilities)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


