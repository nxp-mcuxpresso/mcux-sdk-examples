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


project(glow_cifar10_cm33)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME glow_cifar10_cm33.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_cm33.c"
"${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/glow_cifar10/test/input.bin"
"${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/glow_cifar10/test/output.bin"
"${ProjDirPath}/../counter.c"
"${ProjDirPath}/../counter.h"
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../srtm_config.h"
"${ProjDirPath}/../srtm_config_audio.h"
"${ProjDirPath}/../srtm_config_nn.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../incbin.S"
"${ProjDirPath}/../test_incbin.S"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../dsp_ipc.c"
"${ProjDirPath}/../dsp_ipc.h"
"${ProjDirPath}/../dsp_nn_utils.c"
"${ProjDirPath}/../dsp_nn_utils.h"
"${ProjDirPath}/../dsp_nn.c"
"${ProjDirPath}/../dsp_nn.h"
"${ProjDirPath}/../config_nn.h"
"${SdkRootDirPath}/middleware/eiq/glow/bundle_utils/glow_bundle_utils.h"
"${SdkRootDirPath}/middleware/eiq/glow/bundle_utils/glow_bundle_utils.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${ProjDirPath}/../glow_bundle/model.h"
"${ProjDirPath}/../glow_bundle/model.o"
"${ProjDirPath}/../glow_bundle/model.weights.bin"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt685/clock_config.h"
"${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/glow_cifar10/dsp/binary/dsp_text_release.bin"
"${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/glow_cifar10/dsp/binary/dsp_data_release.bin"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/glow_cifar10/test
    ${ProjDirPath}/..
    ${ProjDirPath}/../glow_bundle
    ${SdkRootDirPath}/examples/evkmimxrt685/eiq_examples/glow_cifar10/dsp/binary
    ${SdkRootDirPath}/examples/evkmimxrt685/dsp_examples/nn_demo/cm33
    ${SdkRootDirPath}/middleware/eiq/glow/bundle_utils
    ${SdkRootDirPath}/core/boards/evkmimxrt685
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/devices/MIMXRT685S/drivers
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/drivers/i3c
    ${SdkRootDirPath}/core/drivers/lpc_dma
    ${SdkRootDirPath}/middleware/multicore
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/inputmux
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core/components/pca9420
    ${SdkRootDirPath}/core/devices/MIMXRT685S
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/boards/evkmimxrt685/flash_config
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_clock)

include(driver_flexcomm_i2c)

include(driver_i3c)

include(driver_lpc_dma)

include(middleware_multicore_rpmsg_lite_MIMXRT685S_cm33)

include(middleware_multicore_rpmsg_lite_freertos)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_common)

include(driver_power)

include(driver_reset)

include(driver_dsp)

include(driver_inputmux)

include(middleware_multicore_rpmsg_lite_evkmimxrt685_freertos)

include(driver_mu)

include(driver_pca9420)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_flash_config_evkmimxrt685)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_lpc_gpio)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_MIMXRT685S_cm33)

include(middleware_freertos-kernel_extension)

include(driver_inputmux_connections)

include(middleware_freertos-kernel_heap_4)

include(utilities_misc_utilities_MIMXRT685S_cm33)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

