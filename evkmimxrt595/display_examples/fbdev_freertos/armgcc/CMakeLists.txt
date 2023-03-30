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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(fbdev_freertos)

set(MCUX_BUILD_TYPES debug release flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME fbdev_freertos.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../fbdev_freertos.c"
"${ProjDirPath}/../display_support.h"
"${ProjDirPath}/../display_support.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimxrt595
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/drivers/flexio
    ${SdkRootDirPath}/core/drivers/lpc_dma
    ${SdkRootDirPath}/core/components/ssd1963
    ${SdkRootDirPath}/core/devices/MIMXRT595S/drivers
    ${SdkRootDirPath}/core/drivers/lpc_gpio
    ${SdkRootDirPath}/core/components/video/display/dbi
    ${SdkRootDirPath}/core/components/video/display/dbi/flexio
    ${SdkRootDirPath}/core/components/video/display/dc/ssd1963
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/drivers/inputmux
    ${SdkRootDirPath}/core/components/video/display/dc/lcdif
    ${SdkRootDirPath}/core/components/video/display
    ${SdkRootDirPath}/core/components/video/display/rm68191
    ${SdkRootDirPath}/core/drivers/mipi_dsi
    ${SdkRootDirPath}/core/drivers/lcdif
    ${SdkRootDirPath}/core/components/video/display/mipi_dsi_cmd
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/video/display/rm68200
    ${SdkRootDirPath}/core/components/video/display/rm67162
    ${SdkRootDirPath}/core/components/video/display/dc/dsi_cmd
    ${SdkRootDirPath}/core/components/video/display/hx8394
    ${SdkRootDirPath}/core/components/video/display/fbdev
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/devices/MIMXRT595S
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/drivers/flexspi
    ${SdkRootDirPath}/core/drivers/cache/cache64
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/flexcomm
    ${SdkRootDirPath}/core/boards/evkmimxrt595/flash_config
    ${SdkRootDirPath}/core/drivers/lpc_iopctl
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/smartdma
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_flexio)

include(driver_flexio_mculcd)

include(driver_lpc_dma)

include(driver_ssd1963)

include(driver_clock)

include(driver_lpc_gpio)

include(driver_dbi)

include(driver_dbi_flexio_smartdma)

include(driver_dc-fb-ssd1963)

include(driver_power)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_inputmux)

include(driver_flexio_mculcd_smartdma)

include(middleware_freertos-kernel_heap_4)

include(driver_dc-fb-lcdif)

include(driver_display-common)

include(driver_display-rm68191)

include(driver_mipi_dsi)

include(driver_lcdif)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_display-rm68200)

include(driver_display-rm67162)

include(driver_dc-fb-dsi-cmd)

include(driver_mipi_dsi_smartdma)

include(driver_display-hx8394)

include(driver_fbdev)

include(driver_video-common)

include(driver_reset)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart_MIMXRT595S_cm33)

include(driver_flexcomm_usart)

include(driver_flash_config_evkmimxrt595)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_iopctl)

include(CMSIS_Include_core_cm)

include(driver_lpc_smartdma)

include(driver_dc-fb-common)

include(middleware_freertos-kernel_MIMXRT595S_cm33)

include(middleware_freertos-kernel_extension)

include(driver_inputmux_connections)

include(driver_soc_mipi_dsi)

include(utilities_misc_utilities)

include(driver_iap)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

