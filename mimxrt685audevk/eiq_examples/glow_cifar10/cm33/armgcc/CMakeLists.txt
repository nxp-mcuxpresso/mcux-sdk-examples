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


project(glow_cifar10_cm33)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME glow_cifar10_cm33.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${SdkRootDirPath}/examples/mimxrt685audevk/eiq_examples/glow_cifar10/dsp/binary/dsp_text_release.bin"
"${SdkRootDirPath}/examples/mimxrt685audevk/eiq_examples/glow_cifar10/dsp/binary/dsp_data_release.bin"
"${ProjDirPath}/../main_cm33.c"
"${SdkRootDirPath}/examples/mimxrt685audevk/eiq_examples/glow_cifar10/test/input.bin"
"${SdkRootDirPath}/examples/mimxrt685audevk/eiq_examples/glow_cifar10/test/output.bin"
"${ProjDirPath}/../counter.c"
"${ProjDirPath}/../counter.h"
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../srtm_config.h"
"${ProjDirPath}/../srtm_config_audio.h"
"${ProjDirPath}/../srtm_config_nn.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../incbin.S"
"${ProjDirPath}/../test_incbin.S"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../dsp_ipc.c"
"${ProjDirPath}/../dsp_ipc.h"
"${ProjDirPath}/../dsp_nn_utils.c"
"${ProjDirPath}/../dsp_nn_utils.h"
"${ProjDirPath}/../dsp_nn.c"
"${ProjDirPath}/../dsp_nn.h"
"${ProjDirPath}/../config_nn.h"
"${SdkRootDirPath}/middleware/eiq/glow/bundle_utils/glow_bundle_utils.h"
"${SdkRootDirPath}/middleware/eiq/glow/bundle_utils/glow_bundle_utils.c"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${ProjDirPath}/../glow_bundle/model.h"
"${ProjDirPath}/../glow_bundle/model.weights.bin"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/board.h"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.c"
"${SdkRootDirPath}/core/boards/mimxrt685audevk/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${SdkRootDirPath}/examples/mimxrt685audevk/eiq_examples/glow_cifar10/dsp/binary
    ${SdkRootDirPath}/examples/mimxrt685audevk/eiq_examples/glow_cifar10/test
    ${ProjDirPath}/..
    ${ProjDirPath}/../glow_bundle
    ${SdkRootDirPath}/examples/mimxrt685audevk/dsp_examples/nn_demo/cm33
    ${SdkRootDirPath}/middleware/eiq/glow/bundle_utils
    ${SdkRootDirPath}/core/boards/mimxrt685audevk
)

set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")

include(${SdkRootDirPath}/core/devices/MIMXRT685S/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

if(CMAKE_BUILD_TYPE STREQUAL flash_debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../glow_bundle/model.o)
endif(CMAKE_BUILD_TYPE STREQUAL flash_debug)

if(CMAKE_BUILD_TYPE STREQUAL flash_release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../glow_bundle/model.o)
endif(CMAKE_BUILD_TYPE STREQUAL flash_release)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin")

