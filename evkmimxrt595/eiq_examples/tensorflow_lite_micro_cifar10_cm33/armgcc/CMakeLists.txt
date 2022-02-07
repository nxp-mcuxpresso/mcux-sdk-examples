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


project(tensorflow_lite_micro_cifar10_cm33)

set(MCUX_SDK_PROJECT_NAME tensorflow_lite_micro_cifar10_cm33.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../../../../middleware/eiq/common/demo_info.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/image/image.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/image/image_decode_raw.c"
"${ProjDirPath}/../../../../../middleware/eiq/common/image/image_utils.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/get_top_n.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/get_top_n.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/model.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/model/output_postproc.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/model/output_postproc.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10/demo_config.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10/image_data.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10/labels.h"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10/model_cifarnet_ops_micro.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/model.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/image/main.cpp"
"${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10/pcq/model_data.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/timer.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/timer.c"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../../../../../middleware/eiq/common/image/image_load.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10
    ${ProjDirPath}/../../../../../middleware/eiq/common
    ${ProjDirPath}/../../../../../middleware/eiq/common/image
    ${ProjDirPath}/../../../../../middleware/eiq/common/model
    ${ProjDirPath}/../../../../../middleware/eiq/tensorflow-lite/examples/cifar10/pcq
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

include(middleware_eiq_tensorflow_lite_micro)

include(component_lists)

include(driver_common)

include(component_usart_adapter)

include(driver_flexcomm_usart)

include(device_CMSIS)

include(device_startup)

include(driver_cache_cache64)

include(driver_clock)

include(driver_flash_config)

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

include(driver_iap)

include(middleware_eiq_tensorflow_lite_micro_third_party_cmsis_nn)

include(CMSIS_DSP_Library)


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


