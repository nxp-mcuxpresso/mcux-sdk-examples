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

set(MCUX_BUILD_TYPES release debug)

set(MCUX_SDK_PROJECT_NAME mpp_camera_mobilenet_view_tflm.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../models/mobilenet_v1_0.25_128_quant_int8/mobilenetv1_model_data_tflite_npu16.h"
"${ProjDirPath}/../camera_mobilenet_view.c"
"${ProjDirPath}/../mpp_config.h"
"${ProjDirPath}/../models/get_top_n.cpp"
"${ProjDirPath}/../models/mobilenet_v1_0.25_128_quant_int8/mobilenetv1_ops_micro_tflite.cpp"
"${ProjDirPath}/../models/mobilenet_v1_0.25_128_quant_int8/mobilenetv1_labels.h"
"${ProjDirPath}/../models/mobilenet_v1_0.25_128_quant_int8/mobilenetv1_output_postproc.cpp"
"${ProjDirPath}/../models/mobilenet_v1_0.25_128_quant_int8/mobilenetv1_output_postproc.h"
"${ProjDirPath}/../models/get_top_n.h"
"${ProjDirPath}/../models/utils.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../hal/hal_frdmmcxn947.c"
"${ProjDirPath}/../utick_config.c"
"${ProjDirPath}/../utick_config.h"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../clock_config.c"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${SdkRootDirPath}/middleware/eiq/mpp/examples
    ${SdkRootDirPath}/middleware/eiq/mpp/boards/frdmmcxn947/examples/camera_mobilenet_view
    ${ProjDirPath}/../models/mobilenet_v1_0.25_128_quant_int8
)

set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")

include(${SdkRootDirPath}/core/devices/MCXN947/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map")

