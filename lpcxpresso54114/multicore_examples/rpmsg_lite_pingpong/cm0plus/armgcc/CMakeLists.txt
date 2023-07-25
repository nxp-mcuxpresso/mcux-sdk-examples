# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

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


project(rpmsg_lite_pingpong_cm0plus)

set(MCUX_SDK_PROJECT_NAME rpmsg_lite_pingpong_cm0plus.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../main_remote.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../rpmsg_config.h"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/board.c"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/board.h"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/clock_config.c"
"${SdkRootDirPath}/core/boards/lpcxpresso54114/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/lpcxpresso54114/multicore_examples/rpmsg_lite_pingpong/cm0plus
    ${SdkRootDirPath}/core/boards/lpcxpresso54114
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_multicore_mcmgr_lpc54114)

include(middleware_multicore_rpmsg_lite_lpc5411x_bm)

include(middleware_multicore_rpmsg_lite_LPC54114_cm0plus)

include(middleware_multicore_rpmsg_lite_bm)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_mailbox)

include(driver_lpc_iocon)

include(driver_clock)

include(middleware_baremetal)

include(middleware_multicore_mcmgr)

include(driver_common)

include(driver_power)

include(driver_reset)

include(device_CMSIS)

include(CMSIS_Include_core_cm)

include(device_startup_LPC54114_cm0plus)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(component_usart_adapter)

include(component_lists)

include(middleware_multicore_rpmsg_lite)

include(utilities_misc_utilities)

include(CMSIS_Include_core_cm)

include(CMSIS_DSP_Include)

include(device_system_LPC54114_cm0plus)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/devices/LPC54114/gcc/libpower_cm0.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/devices/LPC54114/gcc/libpower_cm0.a)
endif()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/core1_image.bin)

