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


project(dev_audio_speaker_lite_bm)

set(MCUX_BUILD_TYPES release debug)

set(MCUX_SDK_PROJECT_NAME dev_audio_speaker_lite_bm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../usb_audio_config.h"
"${ProjDirPath}/../audio_speaker.c"
"${ProjDirPath}/../audio_speaker.h"
"${ProjDirPath}/../usb_device_descriptor.c"
"${ProjDirPath}/../usb_device_descriptor.h"
"${ProjDirPath}/../usb_device_audio.c"
"${ProjDirPath}/../usb_device_audio.h"
"${ProjDirPath}/../usb_device_ch9.c"
"${ProjDirPath}/../usb_device_ch9.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/middleware/usb
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/drivers/ctimer
    ${SdkRootDirPath}/core/drivers/lpc_dma
    ${SdkRootDirPath}/core/components/codec/wm8904
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/audio
    ${SdkRootDirPath}/core
    ${SdkRootDirPath}/core/devices/LPC55S16/drivers
    ${SdkRootDirPath}/core/devices/LPC55S16
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/drivers/lpc_iocon
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_usb_device_ip3511fs)

include(driver_flexcomm_i2c)

include(driver_flexcomm_i2s)

include(driver_flexcomm_i2s_dma)

include(driver_ctimer)

include(driver_lpc_dma)

include(driver_wm8904)

include(driver_codec)

include(driver_common)

include(component_wm8904_adapter)

include(component_codec_i2c_LPC55S16)

include(component_flexcomm_i2c_adapter)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(component_audio_flexcomm_i2s_dma_adapter)

include(middleware_baremetal)

include(driver_clock)

include(driver_power)

include(device_CMSIS)

include(device_startup)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_lpc_iocon)

include(driver_reset)

include(middleware_usb_phy)

include(component_usart_adapter)

include(component_lists)

include(middleware_usb_device_controller_driver_LPC55S16)

include(component_osa)

include(middleware_usb_device_common_header)

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(utilities_misc_utilities_LPC55S16)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


