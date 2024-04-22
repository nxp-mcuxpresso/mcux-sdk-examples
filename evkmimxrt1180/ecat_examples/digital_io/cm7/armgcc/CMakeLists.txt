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


project(ecat_examples_digital_io_cm7)

set(MCUX_BUILD_TYPES debug release flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME ecat_examples_digital_io_cm7.elf)

if (NOT DEFINED SdkRootDirPath)
    SET(SdkRootDirPath ${SdkRootDirPath})
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../ecat_hw.c"
"${ProjDirPath}/../ecat_hw.h"
"${ProjDirPath}/../digital_io.c"
"${ProjDirPath}/../SSC/digital_io.xlsx"
"${ProjDirPath}/../SSC/digital_io.xml"
"${ProjDirPath}/../SSC/Src/applInterface.h"
"${ProjDirPath}/../SSC/Src/coeappl.c"
"${ProjDirPath}/../SSC/Src/coeappl.h"
"${ProjDirPath}/../SSC/Src/digital_io.h"
"${ProjDirPath}/../SSC/Src/digital_ioObjects.h"
"${ProjDirPath}/../SSC/Src/ecatappl.c"
"${ProjDirPath}/../SSC/Src/ecatappl.h"
"${ProjDirPath}/../SSC/Src/ecatcoe.c"
"${ProjDirPath}/../SSC/Src/ecatcoe.h"
"${ProjDirPath}/../SSC/Src/ecat_def.h"
"${ProjDirPath}/../SSC/Src/ecatslv.c"
"${ProjDirPath}/../SSC/Src/ecatslv.h"
"${ProjDirPath}/../SSC/Src/esc.h"
"${ProjDirPath}/../SSC/Src/mailbox.c"
"${ProjDirPath}/../SSC/Src/mailbox.h"
"${ProjDirPath}/../SSC/Src/objdef.c"
"${ProjDirPath}/../SSC/Src/objdef.h"
"${ProjDirPath}/../SSC/Src/sdoserv.c"
"${ProjDirPath}/../SSC/Src/sdoserv.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1180/board.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1180/board.h"
"${SdkRootDirPath}/core/boards/evkmimxrt1180/clock_config.c"
"${SdkRootDirPath}/core/boards/evkmimxrt1180/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../SSC/Src
    ${SdkRootDirPath}/core/boards/evkmimxrt1180
)


include(${SdkRootDirPath}/core/devices/MIMXRT1189/all_lib_device.cmake)

IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)  
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")  
ENDIF()  

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin)

set_target_properties(${MCUX_SDK_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES "output.map;${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.bin")

