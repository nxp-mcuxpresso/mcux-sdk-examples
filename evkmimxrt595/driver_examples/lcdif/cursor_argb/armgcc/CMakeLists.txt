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


project(lcdif_cursor_argb)

set(MCUX_BUILD_TYPES debug release flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME lcdif_cursor_argb.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../lcdif_cursor_argb.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/examples/evkmimxrt595/driver_examples/lcdif/lcdif_support.h"
"${SdkRootDirPath}/examples/evkmimxrt595/driver_examples/lcdif/lcdif_support.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkmimxrt595/driver_examples/lcdif
    ${SdkRootDirPath}/core/boards/evkmimxrt595
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/devices/MIMXRT595S/drivers
    ${SdkRootDirPath}/core/drivers/inputmux
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/display
    ${SdkRootDirPath}/core/components/video/display/rm68191
    ${SdkRootDirPath}/core/drivers/mipi_dsi
    ${SdkRootDirPath}/core/components/video/display/mipi_dsi_cmd
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/components/video/display/rm68200
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/video/display/hx8394
    ${SdkRootDirPath}/core/drivers/lcdif
    ${SdkRootDirPath}/core/devices/MIMXRT595S
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/boards/evkmimxrt595/flash_config
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_clock)

include(driver_power)

include(driver_inputmux)

include(driver_video-common)

include(driver_display-common)

include(driver_display-rm68191)

include(driver_mipi_dsi)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_lpc_gpio)

include(driver_display-rm68200)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_display-hx8394)

include(driver_lcdif)

include(device_CMSIS)

include(component_usart_adapter)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_lists)

include(driver_flexcomm_usart)

include(driver_flash_config_evkmimxrt595)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(driver_reset)

include(CMSIS_Include_core_cm)

include(driver_inputmux_connections)

include(driver_soc_mipi_dsi)

include(utilities_misc_utilities_MIMXRT595S_cm33)

include(driver_iap)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

