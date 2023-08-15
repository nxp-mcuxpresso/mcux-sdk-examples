# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION 2)
SET (MCUXPRESSO_CMAKE_FORMAT_MINOR_VERSION 0)

include(ide_overrides.cmake OPTIONAL)

if(CMAKE_SCRIPT_MODE_FILE)
  message("${MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION}")
  return()
endif()


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


project(ota_mcuboot_server_enet)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME ota_mcuboot_server_enet.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../ota_mcuboot_server.c"
"${ProjDirPath}/../network_cfg.h"
"${ProjDirPath}/../init_enet.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../httpsrv_multipart.c"
"${ProjDirPath}/../httpsrv_multipart.h"
"${ProjDirPath}/../httpsrv_fs_data.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../mbedtls_config.h"
"${ProjDirPath}/../mcuboot_app_support.c"
"${ProjDirPath}/../mcuboot_app_support.h"
"${ProjDirPath}/../bootutil_port/board_specific/flash_partitioning.h"
"${ProjDirPath}/../bootutil_port/board_specific/flash_partitioning.c"
"${ProjDirPath}/../bootutil_port/board_specific/sblconfig.h"
"${ProjDirPath}/../bootutil_port/mcuboot_config/mcuboot_config.h"
"${ProjDirPath}/../bootutil_port/mcuboot_config/mcuboot_logging.h"
"${ProjDirPath}/../bootutil_port/flash_map.h"
"${ProjDirPath}/../bootutil_port/sysflash/sysflash.h"
"${ProjDirPath}/../bootutil_port/sbldef.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../bootutil_port
    ${ProjDirPath}/../bootutil_port/mcuboot_config
    ${ProjDirPath}/../bootutil_port/sysflash
    ${ProjDirPath}/../bootutil_port/board_specific
    ${SdkRootDirPath}/core/boards/evkmimxrt1040
)

set_source_files_properties("${ProjDirPath}/../lwipopts.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_lwip_template")
set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

include(${SdkRootDirPath}/core/devices/MIMXRT1042/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/ota_mcuboot_server.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/ota_mcuboot_server.bin")

