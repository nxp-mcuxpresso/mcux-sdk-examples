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


project(tensorflow_lite_micro_kws_cm33)

set(MCUX_SDK_PROJECT_NAME tensorflow_lite_micro_kws_cm33.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../../../../middleware/eiq/common/demo_info.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/get_top_n.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/get_top_n.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/model.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/output_postproc.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/audio/audio.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.hpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.hpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/model.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/output_postproc.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/audio_data.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/demo_config.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/labels.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/main.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/model_data.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws/model_ds_cnn_ops_micro.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/common/timer.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/timer.c"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/kws
    ${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/audio
    ${ProjDirPath}/../../../../../middleware/eiq/common/audio
    ${ProjDirPath}/../../../../../middleware/eiq/common/model
    ${ProjDirPath}/../../../../../middleware/eiq/common
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(component_codec_i2c_MIMXRT685S_cm33)

include(driver_common)

include(component_i3c_adapter)

include(driver_codec)

include(component_wm8904_adapter)

include(driver_dmic)

include(driver_dmic_dma)

include(driver_flexcomm_i2c)

include(driver_flexcomm_i2s)

include(driver_flexcomm_i2s_dma)

include(driver_flexcomm_usart)

include(driver_i3c)

include(driver_inputmux)

include(driver_inputmux_connections)

include(driver_lpc_dma)

include(driver_wm8904)

include(middleware_eiq_audio)

include(CMSIS_DSP_Library)

include(middleware_eiq_tensorflow_lite_micro)

include(component_lists)

include(component_usart_adapter)

include(device_CMSIS)

include(device_startup)

include(driver_cache_cache64)

include(driver_clock)

include(driver_flash_config_evkmimxrt685)

include(driver_flexcomm)

include(driver_flexspi)

include(driver_lpc_gpio)

include(driver_lpc_iopctl)

include(driver_power)

include(driver_reset)

include(middleware_eiq_tensorflow_lite_micro_cmsis_nn)

include(CMSIS_Include_core_cm)

include(middleware_eiq_tensorflow_lite_micro_third_party_flatbuffers)

include(middleware_eiq_tensorflow_lite_micro_third_party_gemmlowp)

include(middleware_eiq_tensorflow_lite_micro_third_party_ruy)

include(utilities_misc_utilities)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../core/CMSIS/DSP/Lib/GCC/libarm_ARMv8MMLldfsp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../core/CMSIS/DSP/Lib/GCC/libarm_ARMv8MMLldfsp_math.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


