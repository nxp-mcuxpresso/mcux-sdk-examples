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


project(rom_dev_msc_ram_bm)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME rom_dev_msc_ram_bm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../inc/usbd_5460x.h"
"${ProjDirPath}/../inc/romapi_5460x.h"
"${ProjDirPath}/../msc_desc.c"
"${ProjDirPath}/../msc_disk.h"
"${ProjDirPath}/../msc_main.c"
"${ProjDirPath}/../app_usbd_cfg.h"
"${ProjDirPath}/../msc_ram.c"
"${ProjDirPath}/../ramdisk.c"
"${ProjDirPath}/../inc/error.h"
"${ProjDirPath}/../inc/usbd.h"
"${ProjDirPath}/../inc/usbd_cdcuser.h"
"${ProjDirPath}/../inc/usbd_dfuuser.h"
"${ProjDirPath}/../inc/usbd_msc.h"
"${ProjDirPath}/../inc/iap.h"
"${ProjDirPath}/../inc/usbd_core.h"
"${ProjDirPath}/../inc/usbd_hid.h"
"${ProjDirPath}/../inc/usbd_mscuser.h"
"${ProjDirPath}/../inc/lpc_types.h"
"${ProjDirPath}/../inc/usbd_adc.h"
"${ProjDirPath}/../inc/usbd_desc.h"
"${ProjDirPath}/../inc/usbd_hiduser.h"
"${ProjDirPath}/../inc/usbd_rom_api.h"
"${ProjDirPath}/../inc/usbd_cdc.h"
"${ProjDirPath}/../inc/usbd_dfu.h"
"${ProjDirPath}/../inc/usbd_hw.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../inc
)


include(${SdkRootDirPath}/core/devices/LPC54S018/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

