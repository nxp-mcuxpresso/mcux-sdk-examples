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


project(dsp_audio_demo_bm_hifi4)

SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSDK_DEBUGCONSOLE=1")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSERIAL_PORT_TYPE_UART=1")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_CONSOLE_RX_ENABLE=0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu99")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DSDK_DEBUGCONSOLE=1")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DSERIAL_PORT_TYPE_UART=1")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDEBUG_CONSOLE_RX_ENABLE=0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=gnu99")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSERIAL_PORT_TYPE_UART=1")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSERIAL_PORT_TYPE_UART=1")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -mlsp=${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/xtensa/gdbio")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--orphan-handling=place")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -mlsp=${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/xtensa/min-rt")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--orphan-handling=place")

include_directories(${ProjDirPath}/..)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2c)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s)

include_directories(${ProjDirPath}/../../../../../../core/drivers/inputmux)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_dma)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart)

include_directories(${ProjDirPath}/../../../../../../core/components/serial_manager)

include_directories(${ProjDirPath}/../../../../../../core/components/uart)

include_directories(${ProjDirPath}/../../../../../../core/drivers/pint)

include_directories(${ProjDirPath}/../../../../../../core/drivers/dmic)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_gpio)

include_directories(${ProjDirPath}/../../../../../../core/utilities/str)

include_directories(${ProjDirPath}/../../../../../../core/utilities/debug_console)

include_directories(${ProjDirPath}/../../../../../../core/components/lists)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl)

include_directories(${ProjDirPath}/../../../../../../core/utilities/shell)

include_directories(${ProjDirPath}/../../../../../../core/drivers/common)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/Core/Include)

add_executable(dsp_audio_demo_bm_hifi4.elf 
"${ProjDirPath}/../main_dsp.c"
"${ProjDirPath}/../dsp_config.h"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_hifi4.c"
"${ProjDirPath}/../board_hifi4.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2c/fsl_i2c.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2c/fsl_i2c.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2c/fsl_i2c_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2c/fsl_i2c_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/inputmux/fsl_inputmux.h"
"${ProjDirPath}/../../../../../../core/drivers/inputmux/fsl_inputmux.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_dma/fsl_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_dma/fsl_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart.c"
"${ProjDirPath}/../../../../../../core/components/serial_manager/fsl_component_serial_port_uart.h"
"${ProjDirPath}/../../../../../../core/components/serial_manager/fsl_component_serial_port_uart.c"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_uart.h"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_usart.c"
"${ProjDirPath}/../../../../../../core/drivers/pint/fsl_pint.h"
"${ProjDirPath}/../../../../../../core/drivers/pint/fsl_pint.c"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic.c"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic.h"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.c"
"${ProjDirPath}/../../../../../../core/components/serial_manager/fsl_component_serial_manager.h"
"${ProjDirPath}/../../../../../../core/components/serial_manager/fsl_component_serial_port_internal.h"
"${ProjDirPath}/../../../../../../core/components/serial_manager/fsl_component_serial_manager.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart_dma.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.h"
"${ProjDirPath}/../../../../../../core/utilities/debug_console/fsl_debug_console.c"
"${ProjDirPath}/../../../../../../core/utilities/debug_console/fsl_debug_console.h"
"${ProjDirPath}/../../../../../../core/utilities/debug_console/fsl_debug_console_conf.h"
"${ProjDirPath}/../../../../../../core/components/lists/fsl_component_generic_list.h"
"${ProjDirPath}/../../../../../../core/components/lists/fsl_component_generic_list.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl/fsl_iopctl.h"
"${ProjDirPath}/../../../../../../core/utilities/shell/fsl_shell.c"
"${ProjDirPath}/../../../../../../core/utilities/shell/fsl_shell.h"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.h"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_clock.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_clock.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_reset.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_reset.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_flexcomm.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_flexcomm.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/MIMXRT685S_dsp.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/MIMXRT685S_dsp_features.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/system_MIMXRT685S_dsp.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/system_MIMXRT685S_dsp.h"
)


TARGET_LINK_LIBRARIES(dsp_audio_demo_bm_hifi4.elf -Wl,--start-group)
target_link_libraries(dsp_audio_demo_bm_hifi4.elf debug xos)

target_link_libraries(dsp_audio_demo_bm_hifi4.elf optimized xos)

TARGET_LINK_LIBRARIES(dsp_audio_demo_bm_hifi4.elf -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET dsp_audio_demo_bm_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_audio_demo_bm_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/dsp_examples/audio_demo_bm/dsp/binary/dsp_literal_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI23_11_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.Level3InterruptVector.literal
--only-section=.DebugExceptionVector.literal
--only-section=.NMIExceptionVector.literal
--only-section=.UserExceptionVector.literal
)

ADD_CUSTOM_COMMAND(TARGET dsp_audio_demo_bm_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_audio_demo_bm_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/dsp_examples/audio_demo_bm/dsp/binary/dsp_text_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI23_11_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.ResetVector.text
--only-section=.WindowVectors.text
--only-section=.Level2InterruptVector.text
--only-section=.Level3InterruptVector.text
--only-section=.DebugExceptionVector.text
--only-section=.NMIExceptionVector.text
--only-section=.KernelExceptionVector.text
--only-section=.UserExceptionVector.text
--only-section=.DoubleExceptionVector.text
)

ADD_CUSTOM_COMMAND(TARGET dsp_audio_demo_bm_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_audio_demo_bm_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/dsp_examples/audio_demo_bm/dsp/binary/dsp_data_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI23_11_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.clib.rodata
--only-section=.rtos.rodata
--only-section=.rodata
--only-section=.text
--only-section=.clib.data
--only-section=.rtos.percpu.data
--only-section=.data
--only-section=.bss
)

ADD_CUSTOM_COMMAND(TARGET dsp_audio_demo_bm_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/dsp_audio_demo_bm_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/dsp_examples/audio_demo_bm/dsp/binary/dsp_ncache_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI23_11_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=NonCacheable
)

