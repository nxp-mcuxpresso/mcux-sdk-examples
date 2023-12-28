# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION 2)
SET (MCUXPRESSO_CMAKE_FORMAT_MINOR_VERSION 0)

include(ide_overrides.cmake OPTIONAL)

if(CMAKE_SCRIPT_MODE_FILE)
  message("${MCUXPRESSO_CMAKE_FORMAT_MAJOR_VERSION}")
  return()
endif()


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


project(glow_multicore_secure_ml_cm4)

set(MCUX_BUILD_TYPES release debug)

set(MCUX_SDK_PROJECT_NAME glow_multicore_secure_ml_cm4.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../peripherals.h"
"${ProjDirPath}/../peripherals.c"
"${ProjDirPath}/../source/demo_info.h"
"${ProjDirPath}/../source/eiq_common.h"
"${ProjDirPath}/../source/eiq_iui.h"
"${ProjDirPath}/../source/eiq_iworker.h"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/m4/app.h"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/m4/hardware_init.c"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/m4/main_m4.c"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/source/erpc_config.h"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/m4/demo_config.h"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/m4/image_data.h"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/source/input_image.inc"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/source/FreeRTOSConfig.h"
"${ProjDirPath}/../source/image/image.h"
"${ProjDirPath}/../source/image/image_utils.h"
"${ProjDirPath}/../source/image/image_draw.c"
"${ProjDirPath}/../source/image/image_resize_nearest.c"
"${ProjDirPath}/../source/image/image_decode_raw.c"
"${ProjDirPath}/../source/image/image_conv.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM4F/portmacro.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/portable/MemMang/heap_2.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/croutine.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/event_groups.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/list.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/queue.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/stream_buffer.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/tasks.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/timers.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM4F/port.c"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/FreeRTOS.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/StackMacros.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/atomic.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/croutine.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/deprecated_definitions.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/event_groups.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/freertos_tasks_c_additions.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/list.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/message_buffer.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/mpu_prototypes.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/mpu_wrappers.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/portable.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/projdefs.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/queue.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/semphr.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/stack_macros.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/stream_buffer.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/task.h"
"${SdkRootDirPath}/rtos/freertos/freertos-kernel/include/timers.h"
"${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr.h"
"${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr_internal_core_api.h"
"${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr.c"
"${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr_mu_internal.c"
"${SdkRootDirPath}/middleware/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1170.c"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/erpc/secure_ml.h"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/erpc/secure_ml.erpc"
"${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/erpc/secure_ml_client.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_setup_mu.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_client_setup.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_setup_mbf_dynamic.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_mu_transport.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_port_freertos.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_threading_freertos.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_setup_extensions_freertos.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_basic_codec.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_client_manager.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_crc16.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_message_buffer.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_message_loggers.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_pre_post_action.cpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_client_manager.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_common.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_pre_post_action.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_version.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_arbitrated_client_manager.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_basic_codec.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_client_server_common.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_codec.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_crc16.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_framed_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_manually_constructed.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_message_buffer.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_message_loggers.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_server.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_simple_server.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_static_queue.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_transport_arbitrator.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra/erpc_utils.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_config_internal.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_endianness_agnostic_example.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_endianness_undefined.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_port.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_serial.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_setup_extensions.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_spidev.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_sysgpio.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port/erpc_threading.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/config/erpc_config.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_arbitrated_client_setup.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_client_setup.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_mbf_setup.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_server_setup.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup/erpc_transport_setup.h"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_dspi_master_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_dspi_slave_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_i2c_slave_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_inter_thread_buffer_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_lpi2c_slave_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_lpspi_slave_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_mu_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_rpmsg_linux_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_rpmsg_lite_base.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_rpmsg_lite_rtos_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_rpmsg_lite_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_rpmsg_tty_rtos_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_serial_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_spi_master_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_spi_slave_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_spidev_master_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_tcp_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_uart_cmsis_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_uart_zephyr_transport.hpp"
"${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports/erpc_usb_cdc_transport.hpp"
"${ProjDirPath}/../scripts/glow_process_image.py"
"${ProjDirPath}/../scripts/glow_process_sound.py"
"${ProjDirPath}/../scripts/glow_show_detections.py"
"${ProjDirPath}/../scripts/glow_show_segmentation.py"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/m4
    ${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/infra
    ${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/port
    ${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/config
    ${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/setup
    ${SdkRootDirPath}/middleware/multicore/erpc/erpc_c/transports
    ${SdkRootDirPath}/middleware/eiq/common/image
    ${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/erpc
    ${SdkRootDirPath}/middleware/eiq/glow/examples/multicore_secure_ml/source
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel/include
    ${SdkRootDirPath}/rtos/freertos/freertos-kernel/portable/GCC/ARM_CM4F
    ${SdkRootDirPath}/middleware/multicore/mcmgr/src
)


include(${SdkRootDirPath}/core/devices/MIMXRT1176/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/core1_image.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/core1_image.bin")

