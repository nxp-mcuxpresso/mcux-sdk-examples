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


project(dev_dfu_bm)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME dev_dfu_bm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../usb_flash.h"
"${ProjDirPath}/../usb_dfu_ram.c"
"${ProjDirPath}/../usb_device_descriptor.c"
"${ProjDirPath}/../usb_device_descriptor.h"
"${ProjDirPath}/../dfu.c"
"${ProjDirPath}/../dfu.h"
"${ProjDirPath}/../dfu_app.c"
"${ProjDirPath}/../dfu_app.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../dfu_timer.c"
"${ProjDirPath}/../dfu_timer.h"
"${ProjDirPath}/../usb_device_dfu.c"
"${ProjDirPath}/../usb_device_dfu.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
"${SdkRootDirPath}/examples/frdmmcxa153/board.c"
"${SdkRootDirPath}/examples/frdmmcxa153/board.h"
"${SdkRootDirPath}/examples/frdmmcxa153/clock_config.c"
"${SdkRootDirPath}/examples/frdmmcxa153/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/frdmmcxa153
)

set_source_files_properties("${ProjDirPath}/../usb_device_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_usb_device_khci_config_header")

include(${SdkRootDirPath}/core/devices/MCXA153/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

