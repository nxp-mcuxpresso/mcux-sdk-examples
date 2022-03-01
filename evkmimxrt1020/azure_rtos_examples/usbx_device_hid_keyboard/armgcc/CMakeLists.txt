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


project(usbx_device_hid_keyboard)

set(MCUX_SDK_PROJECT_NAME usbx_device_hid_keyboard.elf)

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
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6.h"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_address_set.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_address_get.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_create.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_destroy.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_flush.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_reset.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_stall.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_stall_clear.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_endpoint_status.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_frame_number_get.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_function.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_initialize.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_initialize_complete.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_interrupt_handler.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_interrupt_thread.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_qtd_get.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_register_clear.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_register_read.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_register_set.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_register_write.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_state_change.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_status_phase_hook.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_transfer_abort.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_transfer_callback.c"
"${ProjDirPath}/../usbx_device_controllers/ux_dcd_mcimx6_transfer_request.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_setup.c"
"${ProjDirPath}/../board_setup.h"
"${ProjDirPath}/../../../../../middleware/usb/include/usb.h"
"${ProjDirPath}/../../../../../middleware/usb/include/usb_misc.h"
"${ProjDirPath}/../../../../../middleware/usb/include/usb_spec.h"
"${ProjDirPath}/../../../../../middleware/usb/phy/usb_phy.c"
"${ProjDirPath}/../../../../../middleware/usb/phy/usb_phy.h"
"${ProjDirPath}/../../../../../core/components/osa/fsl_os_abstraction.h"
"${ProjDirPath}/../../../../../core/components/osa/fsl_os_abstraction_bm.h"
"${ProjDirPath}/../../../../../core/components/osa/fsl_os_abstraction_config.h"
"${ProjDirPath}/../usbx_device_hid_keyboard.c"
"${ProjDirPath}/../tx_user.h"
"${ProjDirPath}/../fx_user.h"
"${ProjDirPath}/../nx_user.h"
"${ProjDirPath}/../ux_dcd_interrupt_handler.c"
"${ProjDirPath}/../ux_user.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../usbx_device_controllers
    ${ProjDirPath}/../../../../../middleware/usb/include
    ${ProjDirPath}/../../../../../middleware/usb/phy
    ${ProjDirPath}/../../../../../core/components/osa
    ${ProjDirPath}/..
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_azure_rtos_ux)

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

include(driver_xip_board_evkmimxrt1020)

include(middleware_azure_rtos_ux_template_MIMXRT1021)

include(middleware_azure_rtos_nxd)

include(middleware_azure_rtos_nxd_template_MIMXRT1021)

include(middleware_azure_rtos_fx)

include(middleware_azure_rtos_fx_template_MIMXRT1021)

include(middleware_azure_rtos_tx)

include(middleware_azure_rtos_tx_template_MIMXRT1021)

include(utilities_misc_utilities)

include(CMSIS_Include_core_cm)

include(device_system)


add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/threadx ./CMakeFiles/libthreadx)

add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/filex ./CMakeFiles/libfilex)

add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/netxduo ./CMakeFiles/libnetxduo)

add_subdirectory(${ProjDirPath}/../../../../../rtos/azure-rtos/usbx ./CMakeFiles/libusbx)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::usbx")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::netxduo")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::filex")

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE "azrtos::threadx")

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


