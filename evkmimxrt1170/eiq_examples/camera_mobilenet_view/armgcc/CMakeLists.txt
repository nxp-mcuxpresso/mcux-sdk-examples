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


project(mpp_camera_mobilenet_view_cm7)

set(MCUX_SDK_PROJECT_NAME mpp_camera_mobilenet_view_cm7.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_mobilenet_view.c"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/stopwatch168_208_vuyx.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/get_top_n.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/mobilenetv1_labels.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/mobilenetv1_model_data.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/output_postproc.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/get_top_n.cpp"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/mobilenetv1_ops_micro.cpp"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/tflite/mobilenetv1_output_postproc.cpp"
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

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
)

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

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(middleware_eiq_mpp)

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

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(driver_xmcd_evkmimxrt1170)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(driver_lcdifv2)

include(middleware_eiq_tensorflow_lite_micro)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)

include(middleware_freertos-kernel_MIMXRT1176_cm7)

include(middleware_freertos-kernel_extension)

include(middleware_freertos-kernel_heap_4)

include(utilities_misc_utilities_MIMXRT1176_cm7)

include(device_system_MIMXRT1176_cm7)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/mpp/lib/libmpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/eiq/mpp/lib/libmpp.a)
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_sdram_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


