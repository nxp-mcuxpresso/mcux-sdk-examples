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


project(dsp_xaf_playback_cm33)

set(MCUX_BUILD_TYPES flash_debug flash_release)

set(MCUX_SDK_PROJECT_NAME dsp_xaf_playback_cm33.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../main_cm33.c"
"${ProjDirPath}/../main_cm33.h"
"${ProjDirPath}/../cmd.c"
"${ProjDirPath}/../cmd.h"
"${ProjDirPath}/../dsp_ipc.c"
"${ProjDirPath}/../dsp_ipc.h"
"${ProjDirPath}/../ffconf.h"
"${ProjDirPath}/../srtm_config.h"
"${ProjDirPath}/../srtm_config_audio.h"
"${ProjDirPath}/../srtm_config_nn.h"
"${ProjDirPath}/../rpmsg_config.h"
"${ProjDirPath}/../hihat_dec_out.pcm.h"
"${ProjDirPath}/../hihat.mp3.h"
"${ProjDirPath}/../hihat_48k.mp3.h"
"${ProjDirPath}/../hihat.ogg.h"
"${ProjDirPath}/../hihat_trim.pcm.h"
"${ProjDirPath}/../hihat_trim.sbc.h"
"${ProjDirPath}/../hihat_vorbis_dec_out_trim.pcm.h"
"${ProjDirPath}/../hihat_vor.h"
"${ProjDirPath}/../hihat_pcm.h"
"${ProjDirPath}/../sbc_test_02_trim.sbc.h"
"${ProjDirPath}/../sbc_test_02_trim.out.h"
"${ProjDirPath}/../testvector04-48000-2ch_trim.out.h"
"${ProjDirPath}/../testvector04.bit.h"
"${ProjDirPath}/../testvector11-16000-1ch_trim.out.h"
"${ProjDirPath}/../testvector11-16kHz-1ch-20kbps_trim.bit.h"
"${ProjDirPath}/../aac.adts.h"
"${ProjDirPath}/../aac.ref.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../incbin.S"
"${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary/dsp_data_debug.bin"
"${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary/dsp_data_release.bin"
"${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary/dsp_reset_debug.bin"
"${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary/dsp_reset_release.bin"
"${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary/dsp_text_debug.bin"
"${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary/dsp_text_release.bin"
"${ProjDirPath}/../dsp_support.c"
"${ProjDirPath}/../dsp_support.h"
"${ProjDirPath}/../FreeRTOSConfig.h"
"${ProjDirPath}/../pmic_support.c"
"${ProjDirPath}/../pmic_support.h"
"${ProjDirPath}/../sdmmc_config.c"
"${ProjDirPath}/../sdmmc_config.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt595/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${SdkRootDirPath}/examples/evkmimxrt595/dsp_examples/xaf_playback/dsp/binary
    ${ProjDirPath}/..
    ${SdkRootDirPath}/examples/evkmimxrt595
    ${SdkRootDirPath}/core/boards/evkmimxrt595
)

set_source_files_properties("${ProjDirPath}/../ffconf.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_fatfs_template_sd")
set_source_files_properties("${ProjDirPath}/../FreeRTOSConfig.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_freertos-kernel_template")
set_source_files_properties("${ProjDirPath}/../sdmmc_config.c" PROPERTIES COMPONENT_CONFIG_FILE "middleware_sdmmc_usdhc_template")
set_source_files_properties("${ProjDirPath}/../sdmmc_config.h" PROPERTIES COMPONENT_CONFIG_FILE "middleware_sdmmc_usdhc_template")

include(${SdkRootDirPath}/core/devices/MIMXRT595S/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/sdk20-app.bin")

