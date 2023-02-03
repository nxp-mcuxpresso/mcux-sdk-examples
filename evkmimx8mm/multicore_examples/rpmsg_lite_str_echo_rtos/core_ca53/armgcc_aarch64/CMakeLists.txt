# SOC DEVICE SETTING
SET(MCUX_DEVICE "MIMX8MM6_ca53")

# config to select component, the format is CONFIG_USE_${component}
set(CONFIG_USE_middleware_multicore_rpmsg_lite_freertos true)
set(CONFIG_USE_middleware_freertos-kernel_MIMX8MM6 true)

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

ADD_DEFINITIONS(-DAARCH64=1)

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

SET(AppPath "${ProjDirPath}/..")
SET(SdkDirPath "${SdkRootDirPath}/core")
SET(MulticorePath "${SdkRootDirPath}/middleware/multicore")
SET(RtosPath "${SdkRootDirPath}/rtos/freertos/freertos_kernel")
SET(BoardPath "${SdkRootDirPath}/core/boards/evkmimx8mm_ca53")
SET(CommonPath "${SdkRootDirPath}/core/rtos/freertos")

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

project(rpmsg_lite_str_echo_rtos_imxca53)

set(MCUX_SDK_PROJECT_NAME rpmsg_lite_str_echo_rtos_imxca53.elf)

include(${BoardPath}/armgcc_aarch64/flags.cmake)
add_executable(${MCUX_SDK_PROJECT_NAME}
    "${AppPath}/app_mmu.h"
    "${AppPath}/fsl_iomuxc.h"
    "${AppPath}/main_remote.c"
    "${AppPath}/pin_mux.c"
    "${AppPath}/pin_mux.h"
    "${AppPath}/remoteproc.h"
    "${AppPath}/rpmsg_config.h"
    "${AppPath}/rsc_table.c"
    "${AppPath}/rsc_table.h"
    "${BoardPath}/board.c"
    "${BoardPath}/clock_config.c"
    "${BoardPath}/mmu.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${AppPath}
    ${CommonPath}
    ${BoardPath}
    ${SdkDirPath}/devices/MIMX8MM6
)

set(CMAKE_MODULE_PATH
    ${CommonPath}
    ${CommonPath}/armv8a_startup
    ${MulticorePath}
    ${RtosPath}
    ${SdkDirPath}
)

# include entry for cmake components
include(all_devices)

# include modules
include(middleware_multicore_rpmsg_lite_aarch64_freertos)

include(middleware_multicore_rpmsg_lite_MIMX8MM6)

include(middleware_multicore_rpmsg_lite_freertos)

include(device_system_ca53)

include(common_freertos)

include(freertos_armv8a_startup)

include(component_iuart_adapter)

include(component_lists)

include(component_serial_manager)

include(component_serial_manager_uart)

include(driver_clock)

include(driver_common)

include(driver_iuart)

include(middleware_freertos-kernel_extension)

include(middleware_freertos-kernel_heap_4)

include(middleware_freertos-kernel_ca53)

include(utility_assert)

include(utility_debug_console)

include(utilities_misc_utilities)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/rpmsg_lite_str_echo_rtos.bin)
