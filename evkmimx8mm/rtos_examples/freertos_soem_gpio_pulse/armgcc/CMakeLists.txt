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


project(soem_gpio_pulse_freertos)

set(MCUX_BUILD_TYPES ddr_debug ddr_release debug release flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME soem_gpio_pulse_freertos.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../soem_gpio_pulse.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${SdkRootDirPath}/core/boards/evkmimx8mm/board.c"
"${SdkRootDirPath}/core/boards/evkmimx8mm/board.h"
"${SdkRootDirPath}/core/boards/evkmimx8mm/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimx8mm/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/evkmimx8mm
    ${SdkRootDirPath}/core/components/SOEM/soem/
    ${SdkRootDirPath}/core/components/SOEM/oshw/mcux-sdk
	${SdkRootDirPath}/core/components/SOEM/osal/
	${SdkRootDirPath}/core/components/SOEM/osal/freertos/
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/components/phy/device/phyar8031
    ${SdkRootDirPath}/core/drivers/cache/lmem
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/components/silicon_id
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/silicon_id/socs/imx8
    ${SdkRootDirPath}/core/devices/MIMX8MM6/drivers
    ${SdkRootDirPath}/core/drivers/enet
    ${SdkRootDirPath}/core/drivers/rdc
    ${SdkRootDirPath}/core/devices/MIMX8MM6
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/iuart
    ${SdkRootDirPath}/core/components/phy
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/gpt
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel
    ${SdkRootDirPath}/core/components/SOEM
    ${SdkRootDirPath}/core/components/phy
)

# include modules
include(driver_phy-device-ar8031)

include(driver_cache_lmem)

include(driver_igpio)

include(utility_debug_console_lite)

include(utility_assert_lite)

include(component_silicon_id_MIMX8MM6)

include(driver_common)

include(component_silicon_id_imx8)

include(driver_clock)

include(driver_enet)

include(driver_common)

include(driver_rdc)

include(device_CMSIS)

include(component_iuart_adapter)

include(component_lists)

include(driver_iuart)

include(device_startup)

include(driver_igpio)

include(driver_mdio-common)

include(CMSIS_Include_core_cm)

include(driver_phy-common)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_MIMX8MM6)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities)

include(driver_memory)

include(driver_phy-common_MIMX8MM6)

include(utilities_misc_utilities_MIMX8MM6)

include(device_system)

include(driver_gpt)

include(component_soem)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/soem_gpio_pulse_freertos.bin)

