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


project(freertos_mpu_ns)

set(MCUX_SDK_PROJECT_NAME freertos_mpu_ns.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_ns.c"
"${ProjDirPath}/../../freertos_mpu_s/nsc_functions.h"
"${SdkRootDirPath}/rtos/freertos/freertos_kernel/portable/GCC/ARM_CM33/secure/secure_context.h"
"${SdkRootDirPath}/rtos/freertos/freertos_kernel/portable/GCC/ARM_CM33/secure/secure_init.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../freertos_mpu_s
    ${SdkRootDirPath}/rtos/freertos/freertos_kernel/portable/GCC/ARM_CM33/secure
    ${SdkRootDirPath}/examples/evkmimxrt595/rtos_examples/freertos_mpu/freertos_mpu_ns
    ${SdkRootDirPath}/core/boards/evkmimxrt595
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_freertos-kernel_cm33_secure_port)

include(driver_clock)

include(middleware_freertos-kernel_heap_4)

include(driver_common)

include(driver_power)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

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

include(driver_reset)

include(middleware_freertos-kernel_MIMXRT595S_cm33)

include(middleware_freertos-kernel_extension)

include(middleware_freertos-kernel_mpu_wrappers)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities_MIMXRT595S_cm33)

include(driver_iap)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../freertos_mpu_s/armgcc/${CMAKE_BUILD_TYPE}/freertos_mpu_s_CMSE_lib.o)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../freertos_mpu_s/armgcc/${CMAKE_BUILD_TYPE}/freertos_mpu_s_CMSE_lib.o)
endif(CMAKE_BUILD_TYPE STREQUAL release)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../freertos_mpu_s/armgcc/${CMAKE_BUILD_TYPE}/freertos_mpu_s_CMSE_lib.o)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../freertos_mpu_s/armgcc/${CMAKE_BUILD_TYPE}/freertos_mpu_s_CMSE_lib.o)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

