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


project(filex_sdcard)

set(MCUX_BUILD_TYPES flexspi_nor_release debug release flexspi_nor_debug)

set(MCUX_SDK_PROJECT_NAME filex_sdcard.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

SET(FX_USER_FILE ${ProjDirPath}/../fx_user.h)

SET(TX_USER_FILE ${ProjDirPath}/../tx_user.h)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_setup.c"
"${ProjDirPath}/../board_setup.h"
"${ProjDirPath}/../filex_sdcard.c"
"${ProjDirPath}/../fx_sdcard_driver.c"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../fx_user.h"
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
    ${SdkRootDirPath}/core/devices/MIMXRT1166/drivers/cm7
    ${SdkRootDirPath}/core/drivers/usdhc
    ${SdkRootDirPath}/middleware/sdmmc
    ${SdkRootDirPath}/core/components/gpio
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1166
    ${SdkRootDirPath}/core/devices/MIMXRT1166/drivers
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/boards/evkmimxrt1160/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1166/xip
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_azure_rtos_fx)

include(driver_cache_armv7_m7)

include(driver_usdhc)

include(middleware_sdmmc_sd)

include(middleware_sdmmc_common)

include(middleware_sdmmc_host_usdhc)

include(middleware_sdmmc_host_usdhc_azurertos)

include(component_igpio_adapter)

include(component_lists)

include(component_lpuart_adapter)

include(component_serial_manager)

include(driver_common)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1166_cm7)

include(driver_clock)

include(driver_dcdc_soc)

include(driver_igpio)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_pmu_1)

include(driver_xip_board_evkmimxrt1160)

include(driver_xip_device)

include(utility_assert)

include(utility_debug_console)

include(middleware_azure_rtos_fx_template_MIMXRT1166_cm7)

include(middleware_azure_rtos_tx)

include(middleware_azure_rtos_tx_template_MIMXRT1166_cm7)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(driver_memory)

include(middleware_sdmmc_osa_azurertos)

include(component_osa_thread)

include(utilities_misc_utilities_MIMXRT1166_cm7)

include(device_system_MIMXRT1166_cm7)


add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/filex ./CMakeFiles/libfilex)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::filex")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


