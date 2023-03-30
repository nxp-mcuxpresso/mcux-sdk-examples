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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(power_mode_switch_bm_core1)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME power_mode_switch_bm_core1.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../power_mode_switch_core1.c"
"${SdkRootDirPath}/examples/evkbmimxrt1170/demo_apps/power_mode_switch/bm/common/lpm.c"
"${SdkRootDirPath}/examples/evkbmimxrt1170/demo_apps/power_mode_switch/bm/common/lpm.h"
"${SdkRootDirPath}/examples/evkbmimxrt1170/demo_apps/power_mode_switch/bm/common/chip_init_def.h"
"${SdkRootDirPath}/examples/evkbmimxrt1170/demo_apps/power_mode_switch/bm/common/setpoint_table_def.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/board.c"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/board.h"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/clock_config.c"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/clock_config.h"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/dcd.c"
"${SdkRootDirPath}/core/boards/evkbmimxrt1170/dcd.h"
"${ProjDirPath}/../evkbmimxrt1170_connect_cm4_cm4side.jlinkscript"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${SdkRootDirPath}/examples/evkbmimxrt1170/demo_apps/power_mode_switch/bm/common
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkbmimxrt1170
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/drivers/mu
    ${SdkRootDirPath}/core/drivers/ssarc
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/boards/evkbmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm4
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(driver_igpio)

include(driver_dcdc_soc)

include(driver_mu)

include(driver_gpc_3)

include(driver_pgmc)

include(driver_soc_src)

include(driver_pmu_1)

include(driver_ssarc)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart_MIMXRT1176_cm4)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm4)

include(driver_iomuxc)

include(utility_assert)

include(driver_xip_device)

include(driver_xip_board_evkbmimxrt1170)

include(driver_cache_lmem)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm4)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/core1_image.bin)

