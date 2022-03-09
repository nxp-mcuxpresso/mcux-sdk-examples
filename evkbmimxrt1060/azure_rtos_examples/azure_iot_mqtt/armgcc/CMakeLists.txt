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

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(azure_iot_mqtt)

set(MCUX_SDK_PROJECT_NAME azure_iot_mqtt.elf)

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

SET(NX_USER_FILE ${ProjDirPath}/../nx_user.h)

SET(FX_USER_FILE ${ProjDirPath}/../fx_user.h)

SET(TX_USER_FILE ${ProjDirPath}/../tx_user.h)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../sample_azure_iot.c"
"${ProjDirPath}/../get_seed.c"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../nx_driver_imxrt_irq.c"
"${ProjDirPath}/../nx_driver_imxrt.c"
"${ProjDirPath}/../nx_driver_imxrt.h"
"${ProjDirPath}/../nx_user.h"
"${ProjDirPath}/../fx_user.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_azure_rtos_nxd)

include(driver_mdio-enet)

include(driver_phy-device-ksz8081)

include(driver_trng)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_lpuart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_lpuart)

include(device_startup)

include(driver_iomuxc)

include(utility_assert)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkbmimxrt1060)

include(middleware_azure_rtos_nxd_template_MIMXRT1062)

include(middleware_azure_rtos_fx)

include(middleware_azure_rtos_fx_template_MIMXRT1062)

include(middleware_azure_rtos_tx)

include(middleware_azure_rtos_tx_template_MIMXRT1062)

include(driver_mdio-common)

include(CMSIS_Include_core_cm)

include(driver_enet)

include(driver_phy-common)

include(utilities_misc_utilities)

include(device_system)


add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/netxduo ./CMakeFiles/libnetxduo)

add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/filex ./CMakeFiles/libfilex)

add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::netxduo")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::filex")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


