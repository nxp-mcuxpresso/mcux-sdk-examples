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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(wifi_wpa_supplicant)

set(MCUX_BUILD_TYPES flash_release flash_debug)

set(MCUX_SDK_PROJECT_NAME wifi_wpa_supplicant.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/examples/frdmrw612/wifi_examples/common/lpm/lpm.c"
"${SdkRootDirPath}/examples/frdmrw612/wifi_examples/common/lpm/lpm.h"
"${SdkRootDirPath}/examples/frdmrw612/wifi_examples/common/lpm/host_sleep.c"
"${SdkRootDirPath}/examples/frdmrw612/wifi_examples/common/lpm/host_sleep.h"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../usb/usb_support.h"
"${ProjDirPath}/../usb/usb_support.c"
"${ProjDirPath}/../usb/usb_api.h"
"${ProjDirPath}/../usb/host_msd_fatfs.h"
"${ProjDirPath}/../usb/host_msd_fatfs.c"
"${ProjDirPath}/../usb/ffconf.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../wifi_config.h"
"${ProjDirPath}/../lwippools.h"
"${ProjDirPath}/../lwiphooks.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../app_config.h"
"${SdkRootDirPath}/core/boards/frdmrw612/board.c"
"${SdkRootDirPath}/core/boards/frdmrw612/board.h"
"${SdkRootDirPath}/core/boards/frdmrw612/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmrw612/clock_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/frdmrw612
    ${SdkRootDirPath}/examples/frdmrw612/wifi_examples/common/lpm
    ${ProjDirPath}/../usb
    ${SdkRootDirPath}/examples/frdmrw612/wifi_examples/common
)

set_source_files_properties("${ProjDirPath}/../usb_host_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_usb_host_ehci_config_header")
set_source_files_properties("${ProjDirPath}/../usb/ffconf.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_fatfs_template_usb")
set_source_files_properties("${ProjDirPath}/../wifi_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")
set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")
set_source_files_properties("${ProjDirPath}/../lwipopts.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_lwip_template")
set_source_files_properties("${ProjDirPath}/../app_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_wifi_template")

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_USER_CONFIG_FILE="wpa_supp_els_pkc_mbedtls_config.h" MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_config.h" LWIP_HOOK_FILENAME="lwip_default_hooks.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_USER_CONFIG_FILE="wpa_supp_els_pkc_mbedtls_config.h" MBEDTLS_CONFIG_FILE="els_pkc_mbedtls_config.h" LWIP_HOOK_FILENAME="lwip_default_hooks.h")
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

include(${SdkRootDirPath}/core/devices/RW612/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin")

