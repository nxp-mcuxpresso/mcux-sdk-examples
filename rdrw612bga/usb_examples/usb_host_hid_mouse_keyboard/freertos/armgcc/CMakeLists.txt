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


project(host_hid_mouse_keyboard_freertos)

set(MCUX_BUILD_TYPES flash_debug flash_release debug release)

set(MCUX_SDK_PROJECT_NAME host_hid_mouse_keyboard_freertos.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../app.h"
"${ProjDirPath}/../app.c"
"${ProjDirPath}/../host_keyboard_mouse.h"
"${ProjDirPath}/../host_mouse.c"
"${ProjDirPath}/../host_mouse.h"
"${ProjDirPath}/../host_keyboard.c"
"${ProjDirPath}/../host_keyboard.h"
"${ProjDirPath}/../usb_host_config.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/rdrw612bga
    ${SdkRootDirPath}/core/boards/rdrw612bga
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(middleware_usb_host_ehci_RW612)

include(middleware_freertos-kernel_RW612)

include(middleware_freertos-kernel_heap_4)

include(middleware_usb_host_hid)

include(component_lists)

include(component_serial_manager)

include(driver_common)

include(component_serial_manager_uart)

include(component_usart_adapter)

include(driver_flexcomm_usart)

include(component_silicon_id_RW612)

include(component_silicon_id_rw610)

include(device_CMSIS)

include(device_startup)

include(driver_cache_cache64)

include(driver_clock)

include(driver_cns_io_mux)

include(driver_flash_config_rdrw612bga)

include(driver_flexcomm)

include(driver_flexspi)

include(driver_lpc_gpio)

include(driver_power)

include(driver_reset)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(middleware_usb_host_stack_RW612)

include(component_osa)

include(utility_assert)

include(utility_debug_console)

include(component_els_pkc_platform_rw61x)

include(driver_ocotp)

include(driver_memory)

include(CMSIS_Include_core_cm)

include(middleware_usb_host_common_header)

include(middleware_usb_common_header)

include(middleware_freertos-kernel_extension)

include(utilities_misc_utilities_RW612)

include(component_els_pkc)

include(component_els_pkc_els)

include(component_els_pkc_hash)

include(component_els_pkc_session)

include(component_els_pkc_flow_protection)

include(component_els_pkc_core)

include(component_els_pkc_secure_counter)

include(component_els_pkc_pre_processor)

include(component_els_pkc_memory)

include(component_els_pkc_param_integrity)

include(component_els_pkc_data_integrity)

include(component_els_pkc_toolchain)

include(component_els_pkc_random)

include(component_els_pkc_random_modes)

include(component_els_pkc_aes)

include(component_els_pkc_key)

include(component_els_pkc_ecc)

include(component_els_pkc_pkc)

include(component_els_pkc_math)

include(component_els_pkc_rsa)

include(component_els_pkc_trng)

include(component_els_pkc_prng)

include(component_els_pkc_hashmodes)

include(component_els_pkc_hmac)

include(component_els_pkc_mac_modes)

include(component_els_pkc_mac)

include(component_els_pkc_padding)

include(component_els_pkc_aead_modes)

include(component_els_pkc_aead)

include(component_els_pkc_cipher_modes)

include(component_els_pkc_cipher)

include(component_els_pkc_els_header_only)

include(component_els_pkc_els_common)

include(component_els_pkc_standalone_keyManagement)

include(component_els_pkc_standalone_gdet)

include(component_els_pkc_platform_rw61x_interface_files)

include(component_els_pkc_trng_type_rng4)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

