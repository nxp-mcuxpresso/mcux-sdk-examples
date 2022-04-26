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


project(ethernet_over_usb)

set(MCUX_SDK_PROJECT_NAME ethernet_over_usb.elf)

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m33")

SET(THREADX_TOOLCHAIN "gnu")

SET(UX_USER_FILE ${ProjDirPath}/../ux_user.h)

SET(NX_USER_FILE ${ProjDirPath}/../nx_user.h)

SET(FX_USER_FILE ${ProjDirPath}/../fx_user.h)

SET(TX_USER_FILE ${ProjDirPath}/../tx_user.h)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_asynchronous_endpoint_create.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_asynchronous_endpoint_destroy.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_controller_disable.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_done_queue_process.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_endpoint_reset.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_entry.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_frame_number_get.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_frame_number_set.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516.h"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_initialize.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_interrupt_endpoint_create.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_interrupt_endpoint_destroy.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_interrupt_handler.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_isochronous_endpoint_create.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_isochronous_endpoint_destroy.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_least_traffic_list_get.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_port_disable.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_port_reset.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_port_resume.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_port_status_get.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_port_suspend.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_power_down_port.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_power_on_port.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_power_root_hubs.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_register_read.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_register_write.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_request_bulk_transfer.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_request_control_transfer.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_request_interrupt_transfer.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_request_isochronous_transfer.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_request_transfer.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_transfer_abort.c"
"${ProjDirPath}/../usbx_host_controllers_ip3516/ux_hcd_ip3516_transfer_request_process.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_setup.c"
"${ProjDirPath}/../board_setup.h"
"${ProjDirPath}/../ethernet_over_usb.c"
"${ProjDirPath}/../nx_iperf.c"
"${ProjDirPath}/../nx_iperf.h"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../fx_user.h"
"${ProjDirPath}/../nx_user.h"
"${ProjDirPath}/../ux_user.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../usbx_host_controllers_ip3516
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_phy)

include(middleware_azure_rtos_ux_sp)

include(driver_clock)

include(driver_power)

include(driver_common)

include(device_CMSIS)

include(utility_debug_console)

include(component_usart_adapter)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(device_startup_LPC55S69_cm33_core0)

include(driver_flexcomm)

include(driver_lpc_iocon)

include(driver_lpc_gpio)

include(utility_assert)

include(component_osa_bm)

include(middleware_usb_common_header)

include(component_osa)

include(driver_reset)

include(CMSIS_Include_core_cm)

include(middleware_azure_rtos_ux_template_LPC55S69_cm33_core0)

include(middleware_azure_rtos_nxd_sp)

include(middleware_azure_rtos_nxd_template_LPC55S69_cm33_core0)

include(middleware_azure_rtos_fx_sp)

include(middleware_azure_rtos_fx_template_LPC55S69_cm33_core0)

include(middleware_azure_rtos_tx_sp)

include(middleware_azure_rtos_tx_template_LPC55S69_cm33_core0)

include(utilities_misc_utilities)

include(device_system_LPC55S69_cm33_core0)


add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/filex ./CMakeFiles/libfilex)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/netxduo ./CMakeFiles/libnetxduo)

add_subdirectory(${SdkRootDirPath}/rtos/azure-rtos/usbx ./CMakeFiles/libusbx)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::usbx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::netxduo")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::filex")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


