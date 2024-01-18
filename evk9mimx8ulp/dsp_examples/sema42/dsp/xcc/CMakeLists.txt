# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

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


project(dsp_sema42_fusionf1)

SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSDK_DEBUGCONSOLE=1")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCPU_MIMX8UD5DVK08_dsp0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-core=fusion_nxp02_dsp_prod")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu99")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DSDK_DEBUGCONSOLE=1")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DCPU_MIMX8UD5DVK08_dsp0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-core=fusion_nxp02_dsp_prod")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=gnu99")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -mlsp=${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/xtensa/gdbio")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-core=fusion_nxp02_dsp_prod")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--orphan-handling=place")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -mlsp=${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/xtensa/min-rt")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-core=fusion_nxp02_dsp_prod")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--orphan-handling=place")

include_directories(${ProjDirPath}/../../../../../../core/components/uart)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpuart)

include_directories(${ProjDirPath}/../../../../../../core/utilities/str)

include_directories(${ProjDirPath}/../../../../../../core/utilities/debug_console_lite)

include_directories(${ProjDirPath}/../../../../../../core/utilities/assert)

include_directories(${ProjDirPath}/../../../../../../core/drivers/mu1)

include_directories(${ProjDirPath}/../../../../../../core/drivers/rgpio)

include_directories(${ProjDirPath}/../../../../../../core/drivers/trdc)

include_directories(${ProjDirPath}/../../../../../../core/components/lists)

include_directories(${ProjDirPath}/../../../../../../core/drivers/sema42)

include_directories(${ProjDirPath}/../../../../../../core/drivers/common)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/drivers)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMX8UD5)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/Core/Include)

include_directories(${ProjDirPath}/..)

add_executable(dsp_sema42_fusionf1.elf 
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_uart.h"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_lpuart.c"
"${ProjDirPath}/../../../../../../core/drivers/lpuart/fsl_lpuart.h"
"${ProjDirPath}/../../../../../../core/drivers/lpuart/fsl_lpuart.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.h"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.c"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.h"
"${ProjDirPath}/../../../../../../core/utilities/assert/fsl_assert.c"
"${ProjDirPath}/../../../../../../core/utilities/assert/fsl_assert.h"
"${ProjDirPath}/../../../../../../core/drivers/mu1/fsl_mu.h"
"${ProjDirPath}/../../../../../../core/drivers/mu1/fsl_mu.c"
"${ProjDirPath}/../../../../../../core/drivers/rgpio/fsl_rgpio.h"
"${ProjDirPath}/../../../../../../core/drivers/rgpio/fsl_rgpio.c"
"${ProjDirPath}/../../../../../../core/drivers/trdc/fsl_trdc.h"
"${ProjDirPath}/../../../../../../core/drivers/trdc/fsl_trdc.c"
"${ProjDirPath}/../../../../../../core/components/lists/fsl_component_generic_list.h"
"${ProjDirPath}/../../../../../../core/components/lists/fsl_component_generic_list.c"
"${ProjDirPath}/../../../../../../core/drivers/sema42/fsl_sema42.h"
"${ProjDirPath}/../../../../../../core/drivers/sema42/fsl_sema42.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.h"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.h"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/drivers/fsl_clock.h"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/drivers/fsl_clock.c"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/drivers/fsl_reset.h"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/drivers/fsl_reset.c"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/MIMX8UD5_dsp0.h"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/MIMX8UD5_dsp0_features.h"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/system_MIMX8UD5_dsp0.c"
"${ProjDirPath}/../../../../../../core/devices/MIMX8UD5/system_MIMX8UD5_dsp0.h"
"${ProjDirPath}/../main_dsp.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_fusionf1.c"
"${ProjDirPath}/../board_fusionf1.h"
)



ADD_CUSTOM_COMMAND(TARGET dsp_sema42_fusionf1.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_sema42_fusionf1.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evk9mimx8ulp/dsp_examples/sema42/dsp/binary/dsp_reset_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=fusion_nxp02_dsp_prod
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.ResetVector.text
)

ADD_CUSTOM_COMMAND(TARGET dsp_sema42_fusionf1.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_sema42_fusionf1.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evk9mimx8ulp/dsp_examples/sema42/dsp/binary/dsp_text_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=fusion_nxp02_dsp_prod
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.WindowVectors.text
--only-section=.Level2InterruptVector.text
--only-section=.Level3InterruptVector.literal
--only-section=.Level3InterruptVector.text
--only-section=.DebugExceptionVector.literal
--only-section=.DebugExceptionVector.text
--only-section=.NMIExceptionVector.literal
--only-section=.NMIExceptionVector.text
--only-section=.KernelExceptionVector.text
--only-section=.UserExceptionVector.text
--only-section=.DoubleExceptionVector.text
--only-section=.text
)

ADD_CUSTOM_COMMAND(TARGET dsp_sema42_fusionf1.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_sema42_fusionf1.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evk9mimx8ulp/dsp_examples/sema42/dsp/binary/dsp_data_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=fusion_nxp02_dsp_prod
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.clib.rodata
--only-section=.rtos.rodata
--only-section=.rodata
--only-section=.clib.data
--only-section=.rtos.percpu.data
--only-section=.data
--only-section=.bss
)

