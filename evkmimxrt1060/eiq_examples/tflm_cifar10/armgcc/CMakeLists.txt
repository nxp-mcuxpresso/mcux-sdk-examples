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


project(tflm_cifar10)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_release flexspi_nor_sdram_debug)

set(MCUX_SDK_PROJECT_NAME tflm_cifar10.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/middleware/eiq/common/image/image_conv.c"
"${SdkRootDirPath}/middleware/eiq/common/demo_info.h"
"${SdkRootDirPath}/middleware/eiq/common/image/image.h"
"${SdkRootDirPath}/middleware/eiq/common/image/image_decode_raw.c"
"${SdkRootDirPath}/middleware/eiq/common/image/image_utils.h"
"${SdkRootDirPath}/middleware/eiq/common/model/get_top_n.cpp"
"${SdkRootDirPath}/middleware/eiq/common/model/get_top_n.h"
"${SdkRootDirPath}/middleware/eiq/common/model/model.h"
"${SdkRootDirPath}/middleware/eiq/common/model/output_postproc.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/model/output_postproc.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10/demo_config.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10/image_data.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10/labels.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/model.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/image/main.cpp"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10/model_cifarnet_ops_micro.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10/pcq/model_data.h"
"${SdkRootDirPath}/middleware/eiq/common/image/image_capture.c"
"${SdkRootDirPath}/middleware/eiq/common/image/image_draw.c"
"${SdkRootDirPath}/middleware/eiq/common/image/image_resize_nearest.c"
"${SdkRootDirPath}/middleware/eiq/common/timer.h"
"${SdkRootDirPath}/middleware/eiq/common/timer.c"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10
    ${SdkRootDirPath}/middleware/eiq/common
    ${SdkRootDirPath}/middleware/eiq/common/image
    ${SdkRootDirPath}/middleware/eiq/common/model
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/cifar10/pcq
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/video/camera/device
    ${SdkRootDirPath}/core/components/video/camera/receiver
    ${SdkRootDirPath}/core/components/video/camera/device/sccb
    ${SdkRootDirPath}/core/components/video/camera/device/ov7725
    ${SdkRootDirPath}/core/components/video/camera/device/mt9m114
    ${SdkRootDirPath}/core/components/video/camera
    ${SdkRootDirPath}/core/components/video/camera/receiver/csi
    ${SdkRootDirPath}/core/drivers/csi
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/i2c
    ${SdkRootDirPath}/core/drivers/elcdif
    ${SdkRootDirPath}/core/drivers/pxp
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/components/video/display/dc/elcdif
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/middleware/eiq
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1062
    ${SdkRootDirPath}/core/devices/MIMXRT1062/drivers
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/devices/MIMXRT1062/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1060/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/third_party/cmsis
)

# include modules
include(driver_camera-device-common)

include(driver_camera-receiver-common)

include(driver_camera-device-sccb)

include(driver_camera-device-ov7725)

include(driver_camera-device-mt9m114)

include(driver_camera-common)

include(driver_camera-receiver-csi)

include(driver_csi)

include(driver_video-common)

include(driver_video-i2c)

include(driver_elcdif)

include(driver_pxp)

include(driver_dc-fb-common)

include(driver_dc-fb-elcdif)

include(driver_igpio)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(middleware_eiq_worker_video_MIMXRT1062)

include(middleware_eiq_gui_printf)

include(middleware_eiq_tensorflow_lite_micro)

include(component_lists)

include(component_lpuart_adapter)

include(driver_common)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup)

include(driver_clock)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1060)

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(CMSIS_Include_core_cm)

include(middleware_eiq_worker)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(utilities_misc_utilities)

include(device_system)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


