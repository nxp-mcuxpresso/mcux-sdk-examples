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


project(mpp_camera_view_cm7)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_debug flexspi_nor_sdram_release)

set(MCUX_SDK_PROJECT_NAME mpp_camera_view_cm7.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_view/camera_view.c"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_view/mpp_config.h"
"${SdkRootDirPath}/middleware/eiq/mpp/boards/evkmimxrt1170/inc/board_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${SdkRootDirPath}/middleware/eiq/mpp/README.md"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../display_support.c"
"${ProjDirPath}/../display_support.h"
"${ProjDirPath}/../camera_support.c"
"${ProjDirPath}/../camera_support.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_view
    ${SdkRootDirPath}/middleware/eiq/mpp/boards/evkmimxrt1170/inc
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/video/camera/device
    ${SdkRootDirPath}/core/components/video/camera/receiver
    ${SdkRootDirPath}/core/components/video/camera/device/sccb
    ${SdkRootDirPath}/core/components/video/camera/device/ov5640
    ${SdkRootDirPath}/core/components/video/display/rm68191
    ${SdkRootDirPath}/core/components/video/display/rm68200
    ${SdkRootDirPath}/core/components/video/display/hx8394
    ${SdkRootDirPath}/core/components/video/camera
    ${SdkRootDirPath}/core/components/video/camera/receiver/csi
    ${SdkRootDirPath}/core/drivers/csi
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/i2c
    ${SdkRootDirPath}/core/drivers/elcdif
    ${SdkRootDirPath}/core/drivers/pxp
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/components/video/display/dc/elcdif
    ${SdkRootDirPath}/core/components/video/display/dc/lcdifv2
    ${SdkRootDirPath}/core/components/video/display
    ${SdkRootDirPath}/core/components/video/display/mipi_dsi_cmd
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/mipi_dsi_split
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/drivers/mipi_csi2rx
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/middleware/eiq
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm7
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xmcd
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/lcdifv2
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_camera-device-common)

include(driver_camera-receiver-common)

include(driver_camera-device-sccb)

include(driver_camera-device-ov5640)

include(driver_display-rm68191)

include(driver_display-rm68200)

include(driver_display-hx8394)

include(driver_camera-common)

include(driver_camera-receiver-csi)

include(driver_csi)

include(driver_video-common)

include(driver_video-i2c)

include(driver_elcdif)

include(driver_pxp)

include(driver_dc-fb-common)

include(driver_dc-fb-elcdif)

include(driver_dc-fb-lcdifv2)

include(driver_display-common)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_mipi_dsi_split)

include(driver_igpio)

include(driver_mipi_csi2rx)

include(driver_soc_mipi_csi2rx)

include(driver_soc_src)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(middleware_eiq_mpp_MIMXRT1176_cm7)

include(component_lists)

include(component_lpuart_adapter)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1176_cm7)

include(driver_dcdc_soc)

include(driver_pmu_1)

include(driver_clock)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_cache_armv7_m7)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(driver_xmcd_evkmimxrt1170)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(driver_lcdifv2)

include(middleware_freertos-kernel_MIMXRT1176_cm7)

include(middleware_freertos-kernel_extension)

include(middleware_freertos-kernel_heap_4)

include(middleware_eiq_tensorflow_lite_micro)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(middleware_eiq_deepviewrt)

include(utilities_misc_utilities_MIMXRT1176_cm7)

include(device_system_MIMXRT1176_cm7)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/mpp/lib/libmpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/deepviewrt/lib/libdeepview-rt-cortex-m7f.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/mpp/lib/libmpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/deepviewrt/lib/libdeepview-rt-cortex-m7f.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


