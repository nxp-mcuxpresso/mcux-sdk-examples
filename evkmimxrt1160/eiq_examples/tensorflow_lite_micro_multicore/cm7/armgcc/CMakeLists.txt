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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(tensorflow_lite_micro_multicore_cm7)

set(MCUX_SDK_PROJECT_NAME tensorflow_lite_micro_multicore_cm7.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/middleware/eiq/common/image/image_conv_rotate.c"
"${SdkRootDirPath}/middleware/eiq/common/demo_info.h"
"${SdkRootDirPath}/middleware/eiq/common/image/image.h"
"${SdkRootDirPath}/middleware/eiq/common/image/image_decode_raw.c"
"${SdkRootDirPath}/middleware/eiq/common/image/image_utils.h"
"${SdkRootDirPath}/middleware/eiq/common/model/get_top_n.cpp"
"${SdkRootDirPath}/middleware/eiq/common/model/get_top_n.h"
"${SdkRootDirPath}/middleware/eiq/common/model/model.h"
"${SdkRootDirPath}/middleware/eiq/common/model/output_postproc.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/model/output_postproc.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image/demo_config.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image/image_data.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image/labels.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image/model_mobilenet_ops_micro.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/model.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/multicore/m7/main.c"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/multicore/m7/lpm.c"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/multicore/m7/lpm.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image/pcq/model_data.h"
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
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image
    ${SdkRootDirPath}/middleware/eiq/common
    ${SdkRootDirPath}/middleware/eiq/common/image
    ${SdkRootDirPath}/middleware/eiq/common/model
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/label_image/pcq
)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm4/armgcc/debug)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../cm4/armgcc/release)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

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

include(driver_gpc_3)

include(driver_pgmc)

include(driver_clock)

include(driver_mu)

include(utility_incbin)

include(middleware_eiq_worker_video_MIMXRT1166_cm7)

include(middleware_eiq_gui_printf)

include(middleware_eiq_tensorflow_lite_micro)

include(component_lists)

include(component_lpuart_adapter)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1166_cm7)

include(driver_dcdc_soc)

include(driver_pmu_1)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1160)

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(driver_lcdifv2)

include(middleware_eiq_worker)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(utilities_misc_utilities_MIMXRT1166_cm7)

include(device_system_MIMXRT1166_cm7)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


