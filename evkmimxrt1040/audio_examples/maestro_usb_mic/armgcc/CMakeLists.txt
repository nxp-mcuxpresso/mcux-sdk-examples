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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(maestro_usb_mic)

set(MCUX_BUILD_TYPES flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME maestro_usb_mic.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../streamer_pcm.c"
"${ProjDirPath}/../streamer_pcm_app.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../startup_armgcc.c"
"${ProjDirPath}/../app_streamer.c"
"${ProjDirPath}/../app_streamer.h"
"${ProjDirPath}/../cmd.c"
"${ProjDirPath}/../cmd.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../main.h"
"${ProjDirPath}/../usb_device_descriptor.h"
"${ProjDirPath}/../usb_device_descriptor.c"
"${ProjDirPath}/../audio_microphone.h"
"${ProjDirPath}/../audio_microphone.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../app_definitions.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/clock_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/dcd.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1040/dcd.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_class.c"
"${ProjDirPath}/../usb_device_class.h"
"${ProjDirPath}/../usb_device_audio.c"
"${ProjDirPath}/../usb_device_audio.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimxrt1040
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/drivers/sai
    ${SdkRootDirPath}/core/components/codec/wm8960
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/core/drivers/edma
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/middleware/usb
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/middleware/maestro
    ${SdkRootDirPath}/core/devices/MIMXRT1042/utilities
    ${SdkRootDirPath}/middleware/fatfs
    ${SdkRootDirPath}/core/devices/MIMXRT1042/drivers
    ${SdkRootDirPath}/core/devices/MIMXRT1042
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1042/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1040/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/middleware/maestro/streamer
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_lpi2c)

include(driver_sai)

include(driver_wm8960)

include(driver_codec)

include(driver_common)

include(component_wm8960_adapter)

include(component_codec_i2c_MIMXRT1042)

include(component_lpi2c_adapter)

include(driver_sai_edma)

include(driver_dmamux)

include(driver_edma_MIMXRT1042)

include(driver_cache_armv7_m7)

include(middleware_usb_device_ehci_MIMXRT1042)

include(middleware_usb_phy)

include(middleware_usb_device_controller_driver_MIMXRT1042)

include(component_osa)

include(middleware_freertos-kernel_heap_4)

include(middleware_maestro_framework)

include(utility_shell)

include(middleware_fatfs)

include(middleware_fatfs_ram)

include(driver_clock)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart_MIMXRT1042)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1040)

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(middleware_usb_device_common_header)

include(middleware_freertos-kernel_MIMXRT1042)

include(middleware_freertos-kernel_extension)

include(middleware_maestro_framework_doc)

include(middleware_maestro_framework_streamer)

include(utilities_misc_utilities)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libmp3.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libwav.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libaac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libflac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libmp3.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libwav.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libaac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm7f/armgcc/release/libflac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


