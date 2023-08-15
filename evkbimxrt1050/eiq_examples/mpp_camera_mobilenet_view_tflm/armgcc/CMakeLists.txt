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
    SET(SdkRootDirPath "${ProjDirPath}/../../../../..")
endif()

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(mpp_camera_mobilenet_view_tflm)

set(MCUX_BUILD_TYPES flexspi_nor_sdram_debug flexspi_nor_sdram_release)

set(MCUX_SDK_PROJECT_NAME mpp_camera_mobilenet_view_tflm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_mobilenet_view/camera_mobilenet_view.c"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_mobilenet_view/mpp_config.h"
"${SdkRootDirPath}/middleware/eiq/mpp/boards/evkbimxrt1050/inc/board_config.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/get_top_n.cpp"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/mobilenet_v1_0.25_128_quant_int8_cm7/mobilenetv1_model_data_tflite.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/mobilenet_v1_0.25_128_quant_int8_cm7/mobilenetv1_ops_micro_tflite.cpp"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/mobilenet_v1_0.25_128_quant_int8_cm7/mobilenetv1_labels.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/mobilenet_v1_0.25_128_quant_int8_cm7/mobilenetv1_output_postproc.cpp"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/mobilenet_v1_0.25_128_quant_int8_cm7/mobilenetv1_output_postproc.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/get_top_n.h"
"${SdkRootDirPath}/middleware/eiq/mpp/examples/models/utils.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${SdkRootDirPath}/middleware/eiq/mpp/README.md"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../display_support.c"
"${ProjDirPath}/../display_support.h"
"${ProjDirPath}/../camera_support.c"
"${ProjDirPath}/../camera_support.h"
"${ProjDirPath}/../hal/hal_evkbimxrt1050.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/mpp/examples/camera_mobilenet_view
    ${SdkRootDirPath}/middleware/eiq/mpp/examples
    ${SdkRootDirPath}/middleware/eiq/mpp/boards/evkbimxrt1050/inc
)

set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")

include(${SdkRootDirPath}/core/devices/MIMXRT1052/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

