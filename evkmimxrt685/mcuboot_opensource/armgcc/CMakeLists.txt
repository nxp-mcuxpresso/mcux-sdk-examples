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

set(MCUX_BUILD_TYPES flash_debug flash_release)

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
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${ProjDirPath}/../include
    ${ProjDirPath}/../include/mcuboot_config
    ${ProjDirPath}/../include/sysflash
    ${ProjDirPath}/../flashiap
    ${ProjDirPath}/../secure
    ${SdkRootDirPath}/core/boards/evkmimxrt685
)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/middleware/mcuboot_opensource/boot/bootutil
    ${SdkRootDirPath}/middleware/mbedtls
    ${SdkRootDirPath}/core/components/flash/mflash/mimxrt685
    ${SdkRootDirPath}/core/devices/MIMXRT685S/drivers
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT685S
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/boards/evkmimxrt685/flash_config
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/drivers/casper
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/hashcrypt
    ${SdkRootDirPath}/core/drivers/trng
    ${SdkRootDirPath}/core/components/flash/mflash
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_mcuboot_bootutil)

include(middleware_mbedtls_MIMXRT685S_cm33)

include(middleware_mbedtls_port_ksdk)

include(middleware_mbedtls_rt1)

include(component_mflash_rt685)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_power)

include(driver_flash_config_evkmimxrt685)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_lpc_gpio)

include(driver_reset)

include(driver_casper)

include(CMSIS_Include_core_cm)

include(driver_hashcrypt)

include(driver_trng)

include(component_mflash_common)

include(utilities_misc_utilities_MIMXRT685S_cm33)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

