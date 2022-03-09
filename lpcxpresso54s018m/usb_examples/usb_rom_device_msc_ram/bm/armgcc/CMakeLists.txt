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


project(rom_dev_msc_ram_bm)

set(MCUX_SDK_PROJECT_NAME rom_dev_msc_ram_bm.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
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
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../../examples/lpcxpresso54s018m/usb_examples/usb_rom_device_msc_ram/bm/inc
    ${ProjDirPath}/..
    ${ProjDirPath}/../inc
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_lpc_iocon)

include(driver_power)

include(driver_reset)

include(driver_emc)

include(component_usart_adapter)

include(component_lists)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities)

include(device_system)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/devices/LPC54S018M/gcc/libpower_hardabi.a)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/devices/LPC54S018M/gcc/libpower_hardabi.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


