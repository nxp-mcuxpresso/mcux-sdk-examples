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


project(tensorflow_lite_micro_multicore_cm4)

set(MCUX_SDK_PROJECT_NAME tensorflow_lite_micro_multicore_cm4.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../../../../../middleware/eiq/common/demo_info.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/get_top_n.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/get_top_n.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/model.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/output_postproc.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/audio/audio.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.hpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.hpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/model.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/multicore/m4/output_postproc.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/audio_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/demo_config.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/labels.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/multicore/m4/main.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/model_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/model_ds_cnn_ops_micro.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/common/audio/audio_capture.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/common/timer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/timer.c"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/multicore/m4
    ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws
    ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio
    ${ProjDirPath}/../../../../../../middleware/eiq/common/audio
    ${ProjDirPath}/../../../../../../middleware/eiq/common/model
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_wm8960)

include(driver_codec)

include(driver_common)

include(component_wm8960_adapter)

include(component_codec_i2c_MIMXRT1176_cm4)

include(component_lpi2c_adapter)

include(driver_sai)

include(driver_sai_edma)

include(driver_dmamux)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_mu)

include(driver_gpc_3)

include(driver_pgmc)

include(driver_soc_src)

include(middleware_eiq_worker_audio)

include(CMSIS_DSP_Library)

include(middleware_eiq_tensorflow_lite_micro)

include(component_lists)

include(component_lpuart_adapter)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1176_cm4)

include(driver_dcdc_soc)

include(driver_pmu_1)

include(driver_clock)

include(driver_igpio)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_edma_MIMXRT1176_cm4)

include(driver_memory)

include(middleware_eiq_worker)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm4)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM4lf_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../core/CMSIS/DSP/Lib/GCC/libarm_cortexM4lf_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/core1_image.bin)

