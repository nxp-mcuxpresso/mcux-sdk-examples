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


project(tflm_kws)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_release flexspi_nor_sdram_debug)

set(MCUX_SDK_PROJECT_NAME tflm_kws.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/middleware/eiq/common/demo_info.h"
"${SdkRootDirPath}/middleware/eiq/common/model/get_top_n.cpp"
"${SdkRootDirPath}/middleware/eiq/common/model/get_top_n.h"
"${SdkRootDirPath}/middleware/eiq/common/model/model.h"
"${SdkRootDirPath}/middleware/eiq/common/model/output_postproc.h"
"${SdkRootDirPath}/middleware/eiq/common/audio/audio.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.hpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.hpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/model.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/output_postproc.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/audio_data.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/demo_config.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/labels.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/main.cpp"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/model_data.h"
"${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws/model_ds_cnn_ops_micro.cpp"
"${SdkRootDirPath}/middleware/eiq/common/audio/audio_capture.cpp"
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
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/kws
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/examples/common/audio
    ${SdkRootDirPath}/middleware/eiq/common/audio
    ${SdkRootDirPath}/middleware/eiq/common/model
    ${SdkRootDirPath}/middleware/eiq/common
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/codec/wm8960
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/drivers/sai
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/middleware/eiq
    ${SdkRootDirPath}/core/CMSIS/DSP
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1064
    ${SdkRootDirPath}/core/devices/MIMXRT1064/drivers
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/devices/MIMXRT1064/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1064/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/edma
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/middleware/eiq/tensorflow-lite/third_party/cmsis
)

# include modules
include(driver_wm8960)

include(driver_codec)

include(driver_common)

include(component_wm8960_adapter)

include(component_codec_i2c_MIMXRT1064)

include(component_lpi2c_adapter)

include(driver_sai)

include(driver_sai_edma)

include(driver_dmamux)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(middleware_eiq_worker_audio)

include(CMSIS_DSP_Source)

include(middleware_eiq_tensorflow_lite_micro)

include(component_lists)

include(component_lpuart_adapter)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup)

include(driver_clock)

include(driver_igpio)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1064)

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(CMSIS_Include_core_cm)

include(driver_edma_MIMXRT1064)

include(middleware_eiq_worker)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(utilities_misc_utilities_MIMXRT1064)

include(device_system)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


