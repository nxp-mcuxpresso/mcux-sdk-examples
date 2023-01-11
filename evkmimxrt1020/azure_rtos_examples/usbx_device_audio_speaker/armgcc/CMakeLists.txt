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


project(usbx_device_audio_speaker)

set(MCUX_BUILD_TYPES sdram_debug sdram_release flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME usbx_device_audio_speaker.elf)

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
"${ProjDirPath}/../board_setup.c"
"${ProjDirPath}/../board_setup.h"
"${ProjDirPath}/../usb_device_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../usbx_device_audio_speaker.c"
"${ProjDirPath}/../ux_device_descriptor.c"
"${ProjDirPath}/../ux_device_descriptor.h"
"${ProjDirPath}/../audio_speaker.c"
"${ProjDirPath}/../audio_speaker.h"
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
    ${SdkRootDirPath}/core/drivers/cache/armv7-m7
    ${SdkRootDirPath}/core/components/codec/wm8960
    ${SdkRootDirPath}/core/components/codec
    ${SdkRootDirPath}/core/drivers/common
    ${SdkRootDirPath}/core/components/codec/
    ${SdkRootDirPath}/core/drivers/lpi2c
    ${SdkRootDirPath}/core/drivers/sai
    ${SdkRootDirPath}/core/drivers/dmamux
    ${SdkRootDirPath}/core/drivers/edma
    ${SdkRootDirPath}/core/components/codec/i2c
    ${SdkRootDirPath}/core/components/i2c
    ${SdkRootDirPath}/core/components/audio
    ${SdkRootDirPath}/core/components/osa
    ${SdkRootDirPath}/middleware/usb
    ${SdkRootDirPath}/core/devices/MIMXRT1021/drivers
    ${SdkRootDirPath}/core/devices/MIMXRT1021
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/serial_manager
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/drivers/lpuart
    ${SdkRootDirPath}/core/utilities/assert
    ${SdkRootDirPath}/core/drivers/igpio
    ${SdkRootDirPath}/core/devices/MIMXRT1021/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1020/xip
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/utilities/misc_utilities
)

# include modules
include(middleware_azure_rtos_ux)

include(driver_cache_armv7_m7)

include(driver_wm8960)

include(driver_codec)

include(driver_common)

include(component_wm8960_adapter)

include(driver_lpi2c)

include(driver_sai)

include(driver_sai_edma)

include(driver_dmamux)

include(driver_edma_MIMXRT1021)

include(component_codec_i2c_MIMXRT1021)

include(component_lpi2c_adapter)

include(component_audio_sai_edma_adapter)

include(component_osa_thread)

include(middleware_usb_device_controller_driver_MIMXRT1021)

include(component_osa)

include(middleware_usb_device_ehci_MIMXRT1021)

include(middleware_usb_phy)

include(driver_clock)

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

include(CMSIS_Include_core_cm)

include(middleware_usb_common_header)

include(middleware_usb_device_common_header)

include(utilities_misc_utilities_MIMXRT1021)

include(device_system)


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


