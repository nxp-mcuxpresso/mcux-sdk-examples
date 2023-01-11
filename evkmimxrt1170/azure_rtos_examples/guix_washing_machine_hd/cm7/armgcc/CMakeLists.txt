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


project(guix_washing_machine_hd)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_debug flexspi_nor_sdram_release)

set(MCUX_SDK_PROJECT_NAME guix_washing_machine_hd.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

SET(GX_USER_FILE ${ProjDirPath}/../gx_user.h)

SET(TX_USER_FILE ${ProjDirPath}/../tx_user.h)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../display_support.h"
"${ProjDirPath}/../display_support.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../touch_support.c"
"${ProjDirPath}/../touch_support.h"
"${ProjDirPath}/../gx_display_driver_imxrt11xx_565rgb.c"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../gx_user.h"
"${ProjDirPath}/../demo_guix_washing_machine.c"
"${ProjDirPath}/../demo_guix_washing_machine.h"
"${ProjDirPath}/../garments.c"
"${ProjDirPath}/../demo_guix_washing_machine_resources.c"
"${ProjDirPath}/../demo_guix_washing_machine_resources.h"
"${ProjDirPath}/../demo_guix_washing_machine_specifications.c"
"${ProjDirPath}/../demo_guix_washing_machine_specifications.h"
"${ProjDirPath}/../temperature.c"
"${ProjDirPath}/../washer_on.c"
"${ProjDirPath}/../water_level.c"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/rtos/azure-rtos
    ${SdkRootDirPath}/core/components/video
    ${SdkRootDirPath}/core/components/video/display
    ${SdkRootDirPath}/core/components/video/display/rm68200
    ${SdkRootDirPath}/core/components/video/display/hx8394
    ${SdkRootDirPath}/core/components/video/display/rm68191
    ${SdkRootDirPath}/core/drivers/mipi_dsi_split
    ${SdkRootDirPath}/core/components/video/display/mipi_dsi_cmd
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/video/display/dc/lcdifv2
    ${SdkRootDirPath}/core/drivers/elcdif
    ${SdkRootDirPath}/core/components/video/display/dc/elcdif
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/components/gt911
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/drivers/pxp
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm7
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xmcd
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/components/video/display/dc
    ${SdkRootDirPath}/core/drivers/lcdifv2
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_azure_rtos_gx)

include(driver_video-common)

include(driver_display-common)

include(driver_display-rm68200)

include(driver_display-hx8394)

include(driver_display-rm68191)

include(driver_mipi_dsi_split)

include(driver_display-mipi-dsi-cmd)

include(driver_common)

include(driver_dc-fb-lcdifv2)

include(driver_elcdif)

include(driver_dc-fb-elcdif)

include(driver_lpi2c)

include(driver_gt911)

include(driver_soc_src)

include(driver_pxp)

include(component_lists)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1176_cm7)

include(driver_cache_armv7_m7)

include(driver_clock)

include(driver_dcdc_soc)

include(driver_igpio)

include(driver_iomuxc)

include(driver_pmu_1)

include(driver_xip_board_evkmimxrt1170)

include(driver_xip_device)

include(utility_assert)

include(utility_debug_console)

include(middleware_azure_rtos_tx)

include(middleware_azure_rtos_tx_template_MIMXRT1176_cm7)

include(driver_xmcd_evkmimxrt1170)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_soc_mipi_csi2rx)

include(driver_dc-fb-common)

include(driver_lcdifv2)

include(driver_memory)

include(utilities_misc_utilities_MIMXRT1176_cm7)

include(device_system_MIMXRT1176_cm7)


add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/guix ./CMakeFiles/libguix)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::guix")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


