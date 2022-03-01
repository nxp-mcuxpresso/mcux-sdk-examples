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


project(dev_mtp_bm)

set(MCUX_SDK_PROJECT_NAME dev_mtp_bm.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../mtp.c"
"${ProjDirPath}/../mtp.h"
"${ProjDirPath}/../mtp_operation.c"
"${ProjDirPath}/../mtp_operation.h"
"${ProjDirPath}/../mtp_file_system_adapter.c"
"${ProjDirPath}/../mtp_file_system_adapter.h"
"${ProjDirPath}/../mtp_object_handle.c"
"${ProjDirPath}/../mtp_object_handle.h"
"${ProjDirPath}/../usb_device_descriptor.c"
"${ProjDirPath}/../usb_device_descriptor.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
"${ProjDirPath}/../usb_device_mtp.c"
"${ProjDirPath}/../usb_device_mtp.h"
"${ProjDirPath}/../usb_device_mtp_operation.c"
"${ProjDirPath}/../usb_device_mtp_operation.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../../core/boards/frdmk66f
    ${ProjDirPath}/../../../../../../examples/frdmk66f/usb_examples/usb_device_mtp/bm
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_device_khci)

include(driver_sdhc)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_sdhc)

include(middleware_sdmmc_host_sdhc_interrupt)

include(middleware_fatfs)

include(middleware_fatfs_sd)

include(middleware_baremetal)

include(driver_smc)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(device_startup)

include(driver_lpuart)

include(driver_uart)

include(driver_gpio)

include(driver_sysmpu)

include(utility_assert)

include(utility_debug_console)

include(component_uart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_port)

include(middleware_usb_device_controller_driver_MK66F18)

include(component_osa)

include(middleware_usb_device_ehci)

include(middleware_usb_device_common_header)

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(middleware_sdmmc_osa_bm)

include(component_osa_bm)

include(utilities_misc_utilities)

include(device_system)

include(middleware_usb_phy)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


