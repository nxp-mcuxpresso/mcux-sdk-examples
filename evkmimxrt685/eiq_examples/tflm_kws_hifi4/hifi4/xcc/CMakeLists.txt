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


project(tflm_kws_hifi4)

SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DXOS_CLOCK_FREQ=594000000")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDISABLEFLOAT16")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DXTENSA")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DHIFI4")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DNNLIB_V2")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DMODEL_INT16")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Dhifi4")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mno-mul16")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mno-mul32")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mno-div32")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-exceptions")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mlongcalls")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -INLINE:requested")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mtext-section-literals")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-unsafe-math-optimizations")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ffunction-sections")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fdata-sections")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI2021_8_newlib")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu99")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DXOS_CLOCK_FREQ=594000000")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DDISABLEFLOAT16")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DXTENSA")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DHIFI4")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNNLIB_V2")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMODEL_INT16")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Dhifi4")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mno-mul16")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mno-mul32")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mno-div32")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-exceptions")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mlongcalls")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -INLINE:requested")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mtext-section-literals")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fno-unsafe-math-optimizations")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -ffunction-sections")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fdata-sections")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI2021_8_newlib")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=gnu99")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DXTENSA")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DHIFI4")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DNNLIB_V2")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMODEL_INT16")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Dhifi4")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --stdlib=libc++")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mno-mul16")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mno-mul32")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mno-div32")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-rtti")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-exceptions")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-unwind-tables")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mlongcalls")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -INLINE:requested")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mtext-section-literals")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-unsafe-math-optimizations")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-variable")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ffunction-sections")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fdata-sections")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI2021_8_newlib")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DXTENSA")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DHIFI4")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNNLIB_V2")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMODEL_INT16")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Dhifi4")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --stdlib=libc++")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mno-mul16")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mno-mul32")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mno-div32")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-unwind-tables")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mlongcalls")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -INLINE:requested")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtext-section-literals")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-unsafe-math-optimizations")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-variable")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fdata-sections")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI2021_8_newlib")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -mlsp=${ProjDirPath}/../gdbio")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--gc-sections")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI2021_8_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -mlsp=${ProjDirPath}/../min-rt")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI2021_8_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/common/audio)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/common/model)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/common)

include_directories(${ProjDirPath}/../../../../../../core/components/uart)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart)

include_directories(${ProjDirPath}/../../../../../../core/utilities/str)

include_directories(${ProjDirPath}/../../../../../../core/utilities/debug_console_lite)

include_directories(${ProjDirPath}/../../../../../../core/drivers/dmic)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s)

include_directories(${ProjDirPath}/../../../../../../core/drivers/inputmux)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_dma)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_gpio)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/common/audio/rtxxx)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/fft2d)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/DSP/PrivateInclude)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/DistanceFunctions)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite)

include_directories(${ProjDirPath}/../../../../../../core/drivers/common)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/Core/Include)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/ruy)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib)

include_directories(${ProjDirPath}/..)

add_executable(tflm_kws_hifi4.elf 
"${ProjDirPath}/../../../../../../middleware/eiq/common/demo_info.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/get_top_n.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/get_top_n.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/model.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/model/output_postproc.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/audio/audio.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/kws_mfcc.hpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/audio/mfcc.hpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/model.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/output_postproc.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/audio_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/demo_config.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/common/demo_info.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/labels.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/main.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/model_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/examples/kws/model_ds_cnn_ops_micro.cpp"
"${ProjDirPath}/../core_dsp.h"
"${ProjDirPath}/../gdbio/ldscripts/elf32xtensa.x"
"${ProjDirPath}/../gdbio/specs"
"${ProjDirPath}/../min-rt/ldscripts/elf32xtensa.x"
"${ProjDirPath}/../min-rt/specs"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_uart.h"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_usart.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.h"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.c"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.h"
"${ProjDirPath}/../../../../../../core/utilities/assert/fsl_assert.c"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic.c"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic.h"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/dmic/fsl_dmic_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/i2s/fsl_i2s_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/inputmux/fsl_inputmux.h"
"${ProjDirPath}/../../../../../../core/drivers/inputmux/fsl_inputmux.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_inputmux_connections.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_dma/fsl_dma.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_dma/fsl_dma.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl/fsl_iopctl.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/audio/rtxxx/dsp/audio_stream.c"
"${ProjDirPath}/../../../../../../middleware/eiq/common/audio/rtxxx/audio_capture.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/common/audio/rtxxx/audio_stream.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/fft2d/fftsg.c"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/fft2d/fft.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/fft2d/fft2d.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_common_tables.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_const_structs.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_math.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_common_tables_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_const_structs_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_helium_utils.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_math_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_math_memory.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_math_types.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_math_types_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_mve_tables.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_mve_tables_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_vec_math.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/arm_vec_math_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/basic_math_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/basic_math_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/bayes_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/bayes_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/complex_math_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/complex_math_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/controller_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/controller_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/distance_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/distance_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/fast_math_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/fast_math_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/filtering_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/filtering_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/interpolation_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/interpolation_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/matrix_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/matrix_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/none.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/quaternion_math_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/statistics_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/statistics_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/support_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/support_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/svm_defines.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/svm_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/svm_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/transform_functions.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/transform_functions_f16.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Include/dsp/utils.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/PrivateInclude/arm_sorting.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/PrivateInclude/arm_vec_fft.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/PrivateInclude/arm_vec_filtering.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/BayesFunctions/BayesFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/BayesFunctions/BayesFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/CommonTables/CommonTables.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/CommonTables/CommonTablesF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/ControllerFunctions/ControllerFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/DistanceFunctions/DistanceFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/DistanceFunctions/arm_boolean_distance_template.h"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/DistanceFunctions/DistanceFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/FastMathFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/FastMathFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/FilteringFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/FilteringFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/InterpolationFunctions/InterpolationFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/InterpolationFunctions/InterpolationFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/MatrixFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/MatrixFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/QuaternionMathFunctions/QuaternionMathFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/SVMFunctions/SVMFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/SVMFunctions/SVMFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/SupportFunctions/SupportFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/SupportFunctions/SupportFunctionsF16.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/TransformFunctions/TransformFunctions.c"
"${ProjDirPath}/../../../../../../core/CMSIS/DSP/Source/TransformFunctions/TransformFunctionsF16.c"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/context_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/portable_type_to_tflitetype.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/c/builtin_op_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/c/c_api_types.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/c/common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/error_reporter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/flatbuffer_conversions.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/op_resolver.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/error_reporter.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/flatbuffer_conversions.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/op_resolver.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/tensor_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/builtin_op_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/c_api_types.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/kernel_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/kernel_util.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/op_macros.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/padding.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/compatibility.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/cppmath.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/max.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/min.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/portable_tensor.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/portable_tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/portable_tensor_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/quantization_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/quantization_util.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/runtime_shape.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/strided_slice_logic.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/tensor_ctypes.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/tensor_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/types.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/optimized/neon_check.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/add_n.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/arg_min_max.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/batch_to_space_nd.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/binary_function.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/broadcast_args.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/broadcast_to.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/ceil.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/comparisons.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/concatenation.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/cumsum.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/depth_to_space.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/depthwiseconv_float.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/depthwiseconv_uint8.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/dequantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/div.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/elu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/exp.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/fill.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/floor.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/floor_div.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/floor_mod.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/fully_connected.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/hard_swish.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/l2normalization.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/leaky_relu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/log_softmax.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/logistic.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/maximum_minimum.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/mul.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/neg.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/pad.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/pooling.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/portable_tensor_utils_impl.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/prelu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/process_broadcast_shapes.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/quantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/reduce.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/requantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/resize_bilinear.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/resize_nearest_neighbor.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/round.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/select.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/slice.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/softmax.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/space_to_batch_nd.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/space_to_depth.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/strided_slice.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/sub.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/tanh.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/transpose.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/transpose_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/depthwise_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/fully_connected.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/l2normalization.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/logistic.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/mean.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/mul.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/pooling.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/tanh.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/integer_ops/transpose_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/all_ops_resolver.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/all_ops_resolver.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/compatibility.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/debug_log.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/debug_log.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/fake_micro_context.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/fake_micro_context.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/flatbuffer_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/flatbuffer_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_helpers.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_helpers.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_allocation_info.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_allocation_info.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_allocator.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_arena_constants.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_context.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_context.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_graph.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_graph.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_interpreter.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_interpreter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_log.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_log.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_mutable_op_resolver.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_op_resolver.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_op_resolver.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_profiler.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_profiler.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_profiler_interface.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_resource_variable.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_resource_variable.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_string.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_string.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_time.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_time.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/recording_micro_allocator.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/recording_micro_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/activation_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/activations.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/circular_buffer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/depthwise_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/dequantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/ethosu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/fully_connected.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/hard_swish.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/kernel_runner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/kernel_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/leaky_relu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logical.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logistic.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/lstm_eval.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/lstm_shared.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/micro_ops.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/micro_tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/mul.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/pooling.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/prelu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/quantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/reduce.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/softmax.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/sub.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/svdf.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/activations_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/add_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/add_n.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/arg_min_max.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/assign_variable.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/batch_to_space_nd.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/broadcast_args.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/broadcast_to.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/call_once.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/cast.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/ceil.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/circular_buffer.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/circular_buffer_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/comparisons.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/concatenation.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/conv_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/cumsum.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/depth_to_space.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/depthwise_conv_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/dequantize.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/dequantize_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/detection_postprocess.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/div.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/elementwise.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/elu.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/exp.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/expand_dims.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/fill.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/floor_div.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/floor_mod.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/fully_connected_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/gather.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/gather_nd.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/hard_swish.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/hard_swish_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/if.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/kernel_util.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/l2_pool_2d.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/l2norm.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/leaky_relu_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/log_softmax.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logical.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logical_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logistic_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/lstm_eval_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/maximum_minimum.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/micro_tensor_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/mirror_pad.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/mul_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/neg.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/pack.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/pooling_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/prelu.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/prelu_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/quantize_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/read_variable.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/reduce_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/resize_bilinear.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/round.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/select.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/shape.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/slice.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/softmax_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/space_to_batch_nd.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/space_to_depth.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/split.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/split_v.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/squared_difference.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/squeeze.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/sub_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/svdf_common.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/tanh.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/transpose.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/unpack.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/var_handle.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/while.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/zeros_like.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/ibuffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/greedy_memory_planner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/linear_memory_planner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/memory_plan_struct.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/micro_memory_planner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/linear_memory_planner.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/schema/schema_utils.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/schema/schema_generated.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/schema/schema_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/add.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/add_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/conv.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/conv_hifi.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/conv_int16_reference.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/conv_int8_reference.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/conv_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/depthwise_conv.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/depthwise_conv_hifi.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/depthwise_conv_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/fully_connected.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/fully_connected_common_xtensa.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/fully_connected_int8.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/fully_connected_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/leaky_relu.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/logistic.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/lstm_eval.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/lstm_eval_hifi.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/pad.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/pad_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/pooling.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/pooling_int8.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/pooling_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/quantize.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/reduce.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/reduce_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/reshape.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/reshape_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/softmax.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/softmax_int8_int16.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/softmax_vision.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/strided_slice.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/sub.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/svdf.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/transpose_conv.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/unidirectional_sequence_lstm.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/lstm_eval.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/lstm_shared.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_depthwise_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_fully_connected.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_pad.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_pooling.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_reduce.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_reshape.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_softmax.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/xtensa/xtensa_svdf.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/activations.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/ethosu.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/floor.cpp"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/mul.cpp"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.h"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.c"
"${ProjDirPath}/../../../../../../core/drivers/common/fsl_common_dsp.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_flexcomm.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/fsl_flexcomm.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_reset.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_reset.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_clock.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers/fsl_clock.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/MIMXRT685S_dsp.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/MIMXRT685S_dsp_features.h"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/system_MIMXRT685S_dsp.c"
"${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/system_MIMXRT685S_dsp.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/array.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/base.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/buffer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/buffer_ref.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/default_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/detached_buffer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/flatbuffer_builder.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/flatbuffers.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/flexbuffers.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/stl_emulation.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/strings.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/struct.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/table.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/vector.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/vector_downward.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include/flatbuffers/verifier.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp/fixedpoint/fixedpoint.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp/internal/detect_platform.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/ruy/ruy/profiler/instrumentation.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/xa_type_def.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib/xa_nnlib_api.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib/xa_nnlib_cnn_api.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib/xa_nnlib_gru_api.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib/xa_nnlib_kernels_api.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib/xa_nnlib_lstm_api.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/include/nnlib/xa_nnlib_standards.h"
"${ProjDirPath}/../board_hifi4.c"
"${ProjDirPath}/../board_hifi4.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/timer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/common/timer_xtensa.c"
)


TARGET_LINK_LIBRARIES(tflm_kws_hifi4.elf -Wl,--start-group)
target_link_libraries(tflm_kws_hifi4.elf debug xos)

target_link_libraries(tflm_kws_hifi4.elf optimized xos)


target_link_libraries(tflm_kws_hifi4.elf debug ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/lib/mimxrt685s/libxa_nnlib_hifi4.a)


target_link_libraries(tflm_kws_hifi4.elf optimized ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/xa_nnlib_hifi4/lib/mimxrt685s/libxa_nnlib_hifi4.a)

TARGET_LINK_LIBRARIES(tflm_kws_hifi4.elf -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET tflm_kws_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/tflm_kws_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/eiq_examples/tflm_kws_hifi4/hifi4/binary/dsp_text_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI2021_8_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.ResetVector.text
--only-section=.WindowVectors.text
--only-section=.Level2InterruptVector.text
--only-section=.Level3InterruptVector.literal
--only-section=.Level3InterruptVector.text
--only-section=.DebugExceptionVector.literal
--only-section=.DebugExceptionVector.text
--only-section=.NMIExceptionVector.literal
--only-section=.NMIExceptionVector.text
--only-section=.KernelExceptionVector.text
--only-section=.UserExceptionVector.literal
--only-section=.UserExceptionVector.text
--only-section=.DoubleExceptionVector.text
)

ADD_CUSTOM_COMMAND(TARGET tflm_kws_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/tflm_kws_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/evkmimxrt685/eiq_examples/tflm_kws_hifi4/hifi4/binary/dsp_data_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI2021_8_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.text
--only-section=.rtos.rodata
--only-section=.rodata
--only-section=.clib.data
--only-section=.rtos.percpu.data
--only-section=.data
)

