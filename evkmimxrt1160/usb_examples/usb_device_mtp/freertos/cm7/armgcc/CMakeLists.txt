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


project(dev_mtp_freertos_cm7)

set(MCUX_SDK_PROJECT_NAME dev_mtp_freertos_cm7.elf)

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
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
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
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimxrt1160
    ${SdkRootDirPath}/examples/evkmimxrt1160/usb_examples/usb_device_mtp/freertos/cm7
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_device_ehci)

include(middleware_usb_phy)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_freertos_MIMXRT1166_cm7)

include(middleware_fatfs)

include(middleware_fatfs_sd)

include(component_igpio_adapter)

include(middleware_freertos-kernel_heap_4)

include(middleware_freertos-kernel_MIMXRT1166_cm7)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup_MIMXRT1166_cm7)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1160)

include(middleware_usb_device_controller_driver_MIMXRT1166_cm7)

include(component_osa)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(middleware_usb_common_header)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(middleware_usb_device_common_header)

include(middleware_sdmmc_osa_freertos)

include(component_osa_free_rtos)

include(middleware_freertos-kernel_extension)

include(driver_usdhc)

include(driver_memory)

include(utilities_misc_utilities_MIMXRT1166_cm7)

include(device_system_MIMXRT1166_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


