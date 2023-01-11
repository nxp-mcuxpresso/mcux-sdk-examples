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


project(rom_dev_audio_bm)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME rom_dev_audio_bm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../inc/usbd_5460x.h"
"${ProjDirPath}/../inc/romapi_5460x.h"
"${ProjDirPath}/../Power_Tasks.c"
"${ProjDirPath}/../audio_codec.c"
"${ProjDirPath}/../audio_main.c"
"${ProjDirPath}/../delay.c"
"${ProjDirPath}/../Power_Tasks.h"
"${ProjDirPath}/../audio_codec.h"
"${ProjDirPath}/../audio_usbd.c"
"${ProjDirPath}/../delay.h"
"${ProjDirPath}/../i2s_if.c"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../sine_file.c"
"${ProjDirPath}/../app_usbd_cfg.h"
"${ProjDirPath}/../audio_desc.c"
"${ProjDirPath}/../audio_usbd.h"
"${ProjDirPath}/../i2s_if.h"
"${ProjDirPath}/../pll_setup.c"
"${ProjDirPath}/../sine_file.h"
"${ProjDirPath}/../inc/error.h"
"${ProjDirPath}/../inc/usbd.h"
"${ProjDirPath}/../inc/usbd_cdcuser.h"
"${ProjDirPath}/../inc/usbd_dfuuser.h"
"${ProjDirPath}/../inc/usbd_msc.h"
"${ProjDirPath}/../inc/iap.h"
"${ProjDirPath}/../inc/usbd_core.h"
"${ProjDirPath}/../inc/usbd_hid.h"
"${ProjDirPath}/../inc/usbd_mscuser.h"
"${ProjDirPath}/../inc/lpc_types.h"
"${ProjDirPath}/../inc/usbd_adc.h"
"${ProjDirPath}/../inc/usbd_desc.h"
"${ProjDirPath}/../inc/usbd_hiduser.h"
"${ProjDirPath}/../inc/usbd_rom_api.h"
"${ProjDirPath}/../inc/usbd_cdc.h"
"${ProjDirPath}/../inc/usbd_dfu.h"
"${ProjDirPath}/../inc/usbd_hw.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/../inc
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/components/codec/wm8904
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/devices/LPC54S018M/drivers
    ${SdkRootDirPath}/core/devices/LPC54S018M
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/drivers/lpc_iocon
    ${SdkRootDirPath}/core/drivers/emc
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_flexcomm_i2c)

include(driver_flexcomm_i2s)

include(driver_wm8904)

include(driver_codec)

include(driver_common)

include(component_wm8904_adapter)

include(component_codec_i2c_LPC54S018M)

include(component_flexcomm_i2c_adapter)

include(driver_clock)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_lpc_iocon)

include(driver_power)

include(driver_reset)

include(driver_emc)

include(component_usart_adapter)

include(component_lists)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities_LPC54S018M)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/devices/LPC54S018M/gcc/libpower_hardabi.a)
endif(CMAKE_BUILD_TYPE STREQUAL debug)

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/core/devices/LPC54S018M/gcc/libpower_hardabi.a)
endif(CMAKE_BUILD_TYPE STREQUAL release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


