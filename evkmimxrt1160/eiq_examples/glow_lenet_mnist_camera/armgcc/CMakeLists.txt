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


project(glow_lenet_mnist_camera_cm7)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_release flexspi_nor_sdram_debug)

set(MCUX_SDK_PROJECT_NAME glow_lenet_mnist_camera_cm7.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../board/board_init.h"
"${ProjDirPath}/../board/board_init.c"
"${ProjDirPath}/../source/image/demo_config.h"
"${ProjDirPath}/../source/image/image_data.h"
"${ProjDirPath}/../source/image/image.h"
"${ProjDirPath}/../source/image/image_capture.c"
"${ProjDirPath}/../source/image/image_conv_rotate.c"
"${ProjDirPath}/../source/image/image_decode_raw.c"
"${ProjDirPath}/../source/image/image_resize_nearest.c"
"${ProjDirPath}/../source/image/image_draw.c"
"${ProjDirPath}/../source/image/image_utils.h"
"${ProjDirPath}/../source/lenet_mnist.h"
"${ProjDirPath}/../source/lenet_mnist.o"
"${ProjDirPath}/../source/lenet_mnist.weights.bin"
"${ProjDirPath}/../source/lenet_mnist.weights.txt"
"${ProjDirPath}/../source/glow_lenet_mnist_camera.c"
"${ProjDirPath}/../source/timer.c"
"${ProjDirPath}/../source/timer.h"
"${ProjDirPath}/../eiq/cmsis_nn/Include/arm_nn_tables.h"
"${ProjDirPath}/../eiq/cmsis_nn/Include/arm_nnfunctions.h"
"${ProjDirPath}/../eiq/cmsis_nn/Include/arm_nnsupportfunctions.h"
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
    ${ProjDirPath}/../source/image
    ${ProjDirPath}/../source/video
    ${ProjDirPath}/../eiq/cmsis_nn/Include
    ${ProjDirPath}/../source
    ${ProjDirPath}/../board
    ${SdkRootDirPath}/middleware/eiq/glow/examples/lenet_mnist_camera/evkmimxrt1160
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/core/devices/MIMXRT1166
    ${SdkRootDirPath}/core/components/video/display/mipi_dsi_cmd
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/mipi_dsi_split
    ${SdkRootDirPath}/core/components/video/display/rm68191
    ${SdkRootDirPath}/core/components/video/display/rm68200
    ${SdkRootDirPath}/core/components/video/display/hx8394
    ${SdkRootDirPath}/core/components/video/display
    ${SdkRootDirPath}/core/components/video/camera/device
    ${SdkRootDirPath}/core/components/video/camera
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/camera/receiver
    ${SdkRootDirPath}/core/components/video/camera/device/sccb
    ${SdkRootDirPath}/core/components/video/camera/device/ov5640
    ${SdkRootDirPath}/core/drivers/mipi_csi2rx
    ${SdkRootDirPath}/core/components/video/i2c
    ${SdkRootDirPath}/core/components/video/camera/receiver/csi
    ${SdkRootDirPath}/core/drivers/csi
    ${SdkRootDirPath}/core/drivers/elcdif
    ${SdkRootDirPath}/core/drivers/pxp
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/components/video/display/dc/elcdif
    ${SdkRootDirPath}/core/components/video/display/dc/lcdifv2
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/devices/MIMXRT1166/drivers
    ${SdkRootDirPath}/core/CMSIS/DSP
    ${SdkRootDirPath}/middleware/eiq
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1166/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1160/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/lcdifv2
)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(utilities_misc_utilities_MIMXRT1166_cm7)

include(device_system_MIMXRT1166_cm7)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_mipi_dsi_split)

include(driver_display-rm68191)

include(driver_display-rm68200)

include(driver_display-hx8394)

include(driver_display-common)

include(driver_camera-device-common)

include(driver_camera-common)

include(driver_video-common)

include(driver_camera-receiver-common)

include(driver_camera-device-sccb)

include(driver_camera-device-ov5640)

include(driver_mipi_csi2rx)

include(driver_video-i2c)

include(driver_camera-receiver-csi)

include(driver_csi)

include(driver_elcdif)

include(driver_pxp)

include(driver_dc-fb-common)

include(driver_dc-fb-elcdif)

include(driver_dc-fb-lcdifv2)

include(driver_lpi2c)

include(driver_soc_src)

include(driver_soc_mipi_csi2rx)

include(CMSIS_DSP_Source)

include(middleware_eiq_worker)

include(middleware_eiq_worker_video_MIMXRT1166_cm7)

include(middleware_eiq_gui_printf)

include(middleware_eiq_glow)

include(component_lists)

include(component_lpuart_adapter)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1166_cm7)

include(driver_clock)

include(driver_igpio)

include(driver_iomuxc)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1160)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(driver_lcdifv2)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


