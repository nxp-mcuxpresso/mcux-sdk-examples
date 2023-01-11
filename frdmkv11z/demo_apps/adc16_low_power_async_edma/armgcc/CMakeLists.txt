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


project(adc16_low_power_async_edma)

set(MCUX_BUILD_TYPES debug release)

set(MCUX_SDK_PROJECT_NAME adc16_low_power_async_edma.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../adc16_low_power_async_edma.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${SdkRootDirPath}/core/boards/frdmkv11z/board.c"
"${SdkRootDirPath}/core/boards/frdmkv11z/board.h"
"${SdkRootDirPath}/core/boards/frdmkv11z/clock_config.c"
"${SdkRootDirPath}/core/boards/frdmkv11z/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/frdmkv11z
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/port
    ${SdkRootDirPath}/core/drivers/lptmr
    ${SdkRootDirPath}/core/drivers/gpio
    ${SdkRootDirPath}/core/devices/MKV11Z7/drivers
    ${SdkRootDirPath}/core/drivers/adc16
    ${SdkRootDirPath}/core/drivers/edma
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/core/drivers/sim
    ${SdkRootDirPath}/core/drivers/pmc
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/devices/MKV11Z7
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/uart
    ${SdkRootDirPath}/core/drivers/smc
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_port)

include(driver_lptmr)

include(driver_gpio)

include(driver_clock)

include(driver_adc16)

include(driver_edma_MKV11Z7)

include(driver_dmamux)

include(driver_sim)

include(driver_pmc)

include(driver_common)

include(device_CMSIS)

include(component_uart_adapter)

include(component_lists)

include(driver_uart)

include(device_startup)

include(driver_smc)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities_MKV11Z7)

include(device_system)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


