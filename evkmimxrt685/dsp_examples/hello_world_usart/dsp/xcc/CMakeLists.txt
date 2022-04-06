INCLUDE(CMakeForceCompiler)

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


SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI2020_5_newlib")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu99")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI2020_5_newlib")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=gnu99")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI2020_5_newlib")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI2020_5_newlib")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -mlsp=${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/xtensa/gdbio")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI2020_5_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -mlsp=${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/xtensa/min-rt")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI2020_5_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

include_directories(${ProjDirPath}/..)

include_directories(${ProjDirPath}/../../../../../../boards/evkmimxrt685/dsp_examples/hello_world_usart/dsp)

include_directories(${ProjDirPath}/../../../../../../core/components/uart)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm)

include_directories(${ProjDirPath}/../../../../../../core/utilities/debug_console_lite)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_gpio)

include_directories(${ProjDirPath}/../../../../../../core/drivers/common)

include_directories(${ProjDirPath}/../../../../../../core/components/lists)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/Core/Include)

add_executable(dsp_hello_world_usart_hifi4.elf 
"${ProjDirPath}/../main_dsp.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_hifi4.c"
"${ProjDirPath}/../board_hifi4.h"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_uart.h"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_usart.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_usart.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_usart.c"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.c"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.h"
"${ProjDirPath}/../../../../../../core/utilities/assert/fsl_assert.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.h"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.h"
"${ProjDirPath}/../../../../../../core/components/lists/fsl_component_generic_list.h"
"${ProjDirPath}/../../../../../../core/components/lists/fsl_component_generic_list.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl/fsl_iopctl.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_clock.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_clock.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_reset.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_reset.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/MIMXRT685S_dsp.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/MIMXRT685S_dsp_features.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/system_MIMXRT685S_dsp.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/system_MIMXRT685S_dsp.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_armcc.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_armclang.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_armclang_ltm.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_compiler.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_gcc.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_iccarm.h"
"${ProjDirPath}/../../../../../../core/CMSIS/Core/Include/cmsis_version.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_flexcomm.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_flexcomm.c"
)



ADD_CUSTOM_COMMAND(TARGET dsp_hello_world_usart_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_hello_world_usart_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/dsp_examples/hello_world_usart/dsp/binary/dsp_text_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI2020_5_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.dram0.rodata
--only-section=.Level2InterruptVector.literal
--only-section=.Level3InterruptVector.literal
--only-section=.DebugExceptionVector.literal
--only-section=.NMIExceptionVector.literal
--only-section=.KernelExceptionVector.literal
--only-section=.UserExceptionVector.literal
--only-section=.DoubleExceptionVector.literal
--only-section=.iram0.literal
--only-section=.ResetVector.literal
--only-section=.dram0.literal
--only-section=.dram0.data
--only-section=.dram0.bss
--only-section=.ResetVector.text
--only-section=.ResetHandler.literal
--only-section=.ResetHandler.text
--only-section=.WindowVectors.text
--only-section=.Level2InterruptVector.text
--only-section=.Level3InterruptVector.text
--only-section=.DebugExceptionVector.text
--only-section=.NMIExceptionVector.text
--only-section=.KernelExceptionVector.text
--only-section=.UserExceptionVector.text
--only-section=.DoubleExceptionVector.text
--only-section=.iram0.text
)

ADD_CUSTOM_COMMAND(TARGET dsp_hello_world_usart_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_hello_world_usart_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/dsp_examples/hello_world_usart/dsp/binary/dsp_data_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI2020_5_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.text
--only-section=.rodata
--only-section=.data
--only-section=.bss
--only-section=.literal
--only-section=.sram.text
--only-section=.sram.rodata
--only-section=.sram.data
--only-section=.sram.bss
--only-section=.sram.literal
--only-section=.clib.rodata
--only-section=.clib.text
--only-section=.clib.data
--only-section=.clib.percpu.data
--only-section=.clib.bss
--only-section=.clib.percpu.bss
--only-section=.clib.literal
--only-section=.rtos.rodata
--only-section=.rtos.text
--only-section=.rtos.data
--only-section=.rtos.percpu.data
--only-section=.rtos.bss
--only-section=.rtos.percpu.bss
--only-section=.rtos.literal
)

