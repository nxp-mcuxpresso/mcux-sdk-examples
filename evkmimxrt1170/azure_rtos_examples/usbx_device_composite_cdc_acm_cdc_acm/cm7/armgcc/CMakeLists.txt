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


project(usbx_device_composite_cdc_acm_cdc_acm)

set(MCUX_BUILD_TYPES flexspi_nor_release debug release flexspi_nor_debug)

set(MCUX_SDK_PROJECT_NAME usbx_device_composite_cdc_acm_cdc_acm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

SET(UX_USER_FILE ${ProjDirPath}/../ux_user.h)

SET(NX_USER_FILE ${ProjDirPath}/../nx_user.h)

SET(FX_USER_FILE ${ProjDirPath}/../fx_user.h)

SET(TX_USER_FILE ${ProjDirPath}/../tx_user.h)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci.h"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_callback.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_control_callback.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_endpoint_create.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_endpoint_destroy.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_endpoint_reset.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_endpoint_stall.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_endpoint_status.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_frame_number_get.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_function.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_initialize.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_initialize_complete.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_transfer_callback.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_transfer_request.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_nxp_dci_uninitialize.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_setup.c"
"${ProjDirPath}/../board_setup.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../usbx_device_composite_cdc_acm_cdc_acm.c"
"${ProjDirPath}/../ux_device_descriptor.c"
"${ProjDirPath}/../ux_device_descriptor.h"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../fx_user.h"
"${ProjDirPath}/../nx_user.h"
"${ProjDirPath}/../ux_user.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/../usbx_device_controllers
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/rtos/azure-rtos
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/middleware/usb
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm7
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_azure_rtos_ux)

include(component_osa_thread)

include(middleware_usb_device_controller_driver_MIMXRT1176_cm7)

include(component_osa)

include(middleware_usb_device_ehci_MIMXRT1176_cm7)

include(middleware_usb_phy)

include(component_lists)

include(component_lpuart_adapter)

include(component_serial_manager)

include(driver_common)

include(component_serial_manager_uart_MIMXRT1176_cm7)

include(driver_lpuart)

include(device_CMSIS)

include(device_startup_MIMXRT1176_cm7)

include(driver_cache_armv7_m7)

include(driver_clock)

include(driver_dcdc_soc)

include(driver_igpio)

include(driver_iomuxc)

include(driver_lpi2c)

include(driver_pmu_1)

include(driver_xip_board_evkmimxrt1170)

include(driver_xip_device)

include(utility_assert)

include(utility_debug_console)

include(middleware_azure_rtos_ux_template_MIMXRT1176_cm7)

include(middleware_azure_rtos_nxd)

include(middleware_azure_rtos_nxd_template_MIMXRT1176_cm7)

include(middleware_azure_rtos_fx)

include(middleware_azure_rtos_fx_template_MIMXRT1176_cm7)

include(middleware_azure_rtos_tx)

include(middleware_azure_rtos_tx_template_MIMXRT1176_cm7)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(driver_memory)

include(middleware_usb_device_common_header)

include(utilities_misc_utilities)

include(device_system_MIMXRT1176_cm7)


add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/filex ./CMakeFiles/libfilex)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/netxduo ./CMakeFiles/libnetxduo)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/usbx ./CMakeFiles/libusbx)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::usbx")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::netxduo")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::filex")
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


