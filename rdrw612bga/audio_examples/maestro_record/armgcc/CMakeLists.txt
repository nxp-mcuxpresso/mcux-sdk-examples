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


project(maestro_record)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME maestro_record.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../streamer_pcm.c"
"${ProjDirPath}/../streamer_pcm.h"
"${ProjDirPath}/../app_streamer.c"
"${ProjDirPath}/../app_streamer.h"
"${ProjDirPath}/../cmd.c"
"${ProjDirPath}/../cmd.h"
"${ProjDirPath}/../main.c"
"${ProjDirPath}/../main.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../app_definitions.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/board.h"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.c"
"${SdkRootDirPath}/core/boards/rdrw612bga/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
    ${SdkRootDirPath}/core/boards/rdrw612bga
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_flexcomm_i2c)

include(component_codec_i2c_RW612)

include(driver_common)

include(component_flexcomm_i2c_adapter)

include(middleware_freertos-kernel_cm33_nonsecure_port)

include(driver_wm8904)

include(driver_codec)

include(component_wm8904_adapter)

include(driver_flexcomm_i2s_dma)

include(driver_dmic)

include(driver_dmic_dma)

include(driver_lpc_dma)

include(component_osa_free_rtos_RW612)

include(middleware_fatfs_RW612)

include(middleware_fatfs_ram)

include(middleware_vit_cm33-rw61x)

include(CMSIS_DSP_Source)

include(middleware_voice_seeker_cm33_nodsp)

include(middleware_freertos-kernel_heap_4)

include(middleware_maestro_framework)

include(utility_shell)

include(driver_clock)

include(device_CMSIS)

include(utility_debug_console)

include(utility_assert)

include(component_usart_adapter)

include(driver_power)

include(driver_flash_config_rdrw612bga)

include(driver_flexspi)

include(driver_cache_cache64)

include(component_serial_manager)

include(component_lists)

include(component_serial_manager_uart)

include(driver_flexcomm_usart)

include(device_startup)

include(driver_flexcomm)

include(driver_lpc_gpio)

include(driver_reset)

include(driver_cns_io_mux)

include(component_silicon_id_RW612)

include(component_silicon_id_rw610)

include(component_els_pkc_platform_rw61x_standalone_clib_gdet_sensor)

include(driver_ocotp)

include(CMSIS_Include_core_cm)

include(middleware_freertos-kernel_RW612)

include(middleware_freertos-kernel_extension)

include(driver_memory)

include(driver_flexcomm_i2s)

include(component_osa_interface)

include(middleware_maestro_framework_doc)

include(middleware_maestro_framework_streamer_RW612)

include(utility_str)

include(utilities_misc_utilities_RW612)

include(component_els_pkc_els_header_only)

include(component_els_pkc_els_common)

include(component_els_pkc_core)

include(component_els_pkc_standalone_keyManagement)

include(component_els_pkc_memory)

include(component_els_pkc_param_integrity)

include(component_els_pkc_flow_protection)

include(component_els_pkc_secure_counter)

include(component_els_pkc_pre_processor)

include(component_els_pkc_data_integrity)

include(component_els_pkc_toolchain)

include(component_els_pkc_standalone_gdet)

include(component_els_pkc_platform_rw61x_interface_files)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/voice_seeker/ARM_CortexM33_noDSP/lib/libvoiceseeker_no_aec.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libmp3.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libwav.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libaac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libflac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/voice_seeker/ARM_CortexM33_noDSP/lib/libvoiceseeker_no_aec.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libmp3.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libwav.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libaac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${SdkRootDirPath}/middleware/maestro/libs/cm33f/armgcc/release/libflac.a)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

