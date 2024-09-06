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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(hid_host_freertos)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME hid_host_freertos.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../app_services_init.c"
"${ProjDirPath}/../app_ble_init.c"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../hardware_init.c"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../app_conn.c"
"${ProjDirPath}/../app_conn.h"
"${ProjDirPath}/../ble_init.c"
"${ProjDirPath}/../ble_init.h"
"${ProjDirPath}/../app_nvm.c"
"${ProjDirPath}/../app_low_power.c"
"${ProjDirPath}/../app_scanner.c"
"${ProjDirPath}/../app_scanner.h"
"${ProjDirPath}/../app_connection.c"
"${SdkRootDirPath}/examples/frdmmcxw71/wireless_examples/bluetooth/hid_host/app_config.c"
"${SdkRootDirPath}/examples/frdmmcxw71/wireless_examples/bluetooth/hid_host/gatt_db.h"
"${SdkRootDirPath}/examples/frdmmcxw71/wireless_examples/bluetooth/hid_host/gatt_uuid128.h"
"${SdkRootDirPath}/examples/frdmmcxw71/wireless_examples/bluetooth/hid_host/hid_host.c"
"${SdkRootDirPath}/examples/frdmmcxw71/wireless_examples/bluetooth/hid_host/hid_host.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../app_preinclude.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board_platform.h"
"${ProjDirPath}/../board_comp.c"
"${ProjDirPath}/../board_comp.h"
"${ProjDirPath}/../board_dcdc.c"
"${ProjDirPath}/../board_dcdc.h"
"${ProjDirPath}/../board_lp.c"
"${ProjDirPath}/../board_lp.h"
"${ProjDirPath}/../app_preinclude_common.h"
"${SdkRootDirPath}/middleware/wireless/ble_controller/bin/k32w1_nbu_ble_hosted.bin"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/frdmmcxw71/wireless_examples/bluetooth/hid_host
)

set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")

if(CMAKE_BUILD_TYPE STREQUAL debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE SSS_CONFIG_FILE="fsl_sss_config_elemu.h" SSCP_CONFIG_FILE="fsl_sscp_config_elemu.h")
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE SSS_CONFIG_FILE="fsl_sss_config_elemu.h" SSCP_CONFIG_FILE="fsl_sscp_config_elemu.h")
endif(CMAKE_BUILD_TYPE STREQUAL release)

include(${SdkRootDirPath}/core/devices/MCXW716C/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lgcc -lnosys -lc")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Osrec ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/hid_host_frdmmcxw71.srec)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/hid_host_frdmmcxw71.srec")

