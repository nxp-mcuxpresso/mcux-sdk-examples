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


project(glow_lenet_mnist_camera)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_debug flexspi_nor_sdram_release)

set(MCUX_SDK_PROJECT_NAME glow_lenet_mnist_camera.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../source/image.c"
"${ProjDirPath}/../source/image.h"
"${ProjDirPath}/../gui_print/chgui.c"
"${ProjDirPath}/../gui_print/chgui.h"
"${ProjDirPath}/../gui_print/font.h"
"${ProjDirPath}/../display_support.c"
"${ProjDirPath}/../display_support.h"
"${ProjDirPath}/../camera_support.c"
"${ProjDirPath}/../camera_support.h"
"${ProjDirPath}/../source/lenet_mnist.h"
"${ProjDirPath}/../source/lenet_mnist.o"
"${ProjDirPath}/../source/lenet_mnist.weights.bin"
"${ProjDirPath}/../source/lenet_mnist.weights.txt"
"${ProjDirPath}/../source/glow_lenet_mnist_camera.c"
"${ProjDirPath}/../source/timer.c"
"${ProjDirPath}/../source/timer.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../peripherals.h"
"${ProjDirPath}/../peripherals.c"
"${ProjDirPath}/../scripts/glow_process_image.py"
"${ProjDirPath}/../scripts/glow_process_sound.py"
"${ProjDirPath}/../scripts/glow_show_detections.py"
"${ProjDirPath}/../scripts/glow_show_segmentation.py"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/../gui_print
    ${ProjDirPath}/../source
    ${ProjDirPath}/../board
    ${SdkRootDirPath}/middleware/eiq/glow/examples/lenet_mnist_camera/evkmimxrt1064
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/devices/MIMXRT1064
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/core/components/video/camera/device
    ${SdkRootDirPath}/core/components/video/camera
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/camera/receiver
    ${SdkRootDirPath}/core/components/video/camera/device/sccb
    ${SdkRootDirPath}/core/components/video/camera/device/ov7725
    ${SdkRootDirPath}/core/components/video/camera/device/mt9m114
    ${SdkRootDirPath}/core/components/video/i2c
    ${SdkRootDirPath}/core/components/video/camera/receiver/csi
    ${SdkRootDirPath}/core/drivers/csi
    ${SdkRootDirPath}/core/drivers/elcdif
    ${SdkRootDirPath}/core/drivers/pxp
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/components/video/display/dc/elcdif
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/middleware/eiq
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1064/drivers
    ${SdkRootDirPath}/core/devices/MIMXRT1064/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1064/xip
    ${SdkRootDirPath}/core/CMSIS/DSP
    ${SdkRootDirPath}/core/CMSIS/Core/Include
)

# include modules
include(utility_debug_console)

include(utility_assert)

include(device_CMSIS)

include(utilities_misc_utilities_MIMXRT1064)

include(device_system)

include(driver_camera-device-common)

include(driver_camera-common)

include(driver_video-common)

include(driver_camera-receiver-common)

include(driver_camera-device-sccb)

include(driver_camera-device-ov7725)

include(driver_camera-device-mt9m114)

include(driver_video-i2c)

include(driver_camera-receiver-csi)

include(driver_csi)

include(driver_elcdif)

include(driver_pxp)

include(driver_dc-fb-common)

include(driver_dc-fb-elcdif)

include(driver_igpio)

include(driver_lpi2c)

include(middleware_eiq_glow)

include(component_lists)

include(component_lpuart_adapter)

include(component_serial_manager)

include(driver_common)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_clock)

include(driver_iomuxc)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1064)

include(CMSIS_DSP_Source)

include(CMSIS_Include_core_cm)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


