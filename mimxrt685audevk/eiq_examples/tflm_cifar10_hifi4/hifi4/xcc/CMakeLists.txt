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


project(tflm_cifar10_hifi4)

SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DXOS_CLOCK_FREQ=594000000")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

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

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=gnu99")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DXOS_CLOCK_FREQ=594000000")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

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

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=gnu99")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --stdlib=libc++")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mno-mul16")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mno-mul32")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mno-div32")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsigned-char")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-exceptions")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mlongcalls")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -INLINE:requested")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mtext-section-literals")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-unsafe-math-optimizations")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused-variable")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ffunction-sections")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fdata-sections")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-unused")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fmessage-length=0")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DSDK_DEBUGCONSOLE_UART")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DTF_LITE_STATIC_MEMORY")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DCPU_MIMXRT685SFVKB_dsp")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DMCUXPRESSO_SDK")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --stdlib=libc++")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mno-mul16")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mno-mul32")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mno-div32")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fsigned-char")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-exceptions")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mlongcalls")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -INLINE:requested")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtext-section-literals")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-unsafe-math-optimizations")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused-variable")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fdata-sections")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-unused")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-missing-braces")

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fmessage-length=0")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -mlsp=${ProjDirPath}/../gdbio")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--gc-sections")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--orphan-handling=place")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -mlsp=${ProjDirPath}/../min-rt")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-core=nxp_rt600_RI23_11_newlib")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} --xtensa-system=${XTENSA_SYSTEM}")

SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--orphan-handling=place")

include_directories(${ProjDirPath}/../source)

include_directories(${ProjDirPath}/../source/image)

include_directories(${ProjDirPath}/../source/model)

include_directories(${ProjDirPath}/../../../../../../core/components/uart)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_gpio)

include_directories(${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl)

include_directories(${ProjDirPath}/../../../../../../core/utilities/str)

include_directories(${ProjDirPath}/../../../../../../core/utilities/debug_console_lite)

include_directories(${ProjDirPath}/../../../../../../core/utilities/assert)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/flatbuffers/include)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp)

include_directories(${ProjDirPath}/../../../../../../core/drivers/common)

include_directories(${ProjDirPath}/../../../../../../core/drivers/flexcomm)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S/drivers)

include_directories(${ProjDirPath}/../../../../../../core/devices/MIMXRT685S)

include_directories(${ProjDirPath}/../../../../../../core/CMSIS/Core/Include)

include_directories(${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/ruy)

include_directories(${ProjDirPath}/..)

add_executable(tflm_cifar10_hifi4.elf 
"${ProjDirPath}/../source/demo_info.h"
"${ProjDirPath}/../source/image/image.h"
"${ProjDirPath}/../source/image/image_decode_raw.c"
"${ProjDirPath}/../source/image/image_utils.h"
"${ProjDirPath}/../source/model/get_top_n.cpp"
"${ProjDirPath}/../source/model/get_top_n.h"
"${ProjDirPath}/../source/model/model.h"
"${ProjDirPath}/../source/model/output_postproc.h"
"${ProjDirPath}/../source/model/output_postproc.cpp"
"${ProjDirPath}/../source/demo_config.h"
"${ProjDirPath}/../source/demo_info.cpp"
"${ProjDirPath}/../source/image/image_data.h"
"${ProjDirPath}/../source/labels.h"
"${ProjDirPath}/../source/model/model.cpp"
"${ProjDirPath}/../source/main.cpp"
"${ProjDirPath}/../source/model/model_cifarnet_ops_micro.cpp"
"${ProjDirPath}/../source/model/model_data.h"
"${ProjDirPath}/../core_dsp.h"
"${ProjDirPath}/../gdbio/ldscripts/elf32xtensa.x"
"${ProjDirPath}/../gdbio/specs"
"${ProjDirPath}/../min-rt/ldscripts/elf32xtensa.x"
"${ProjDirPath}/../min-rt/specs"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_uart.h"
"${ProjDirPath}/../../../../../../core/components/uart/fsl_adapter_usart.c"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart.h"
"${ProjDirPath}/../../../../../../core/drivers/flexcomm/usart/fsl_usart.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.h"
"${ProjDirPath}/../../../../../../core/drivers/lpc_gpio/fsl_gpio.c"
"${ProjDirPath}/../../../../../../core/drivers/lpc_iopctl/fsl_iopctl.h"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.c"
"${ProjDirPath}/../../../../../../core/utilities/str/fsl_str.h"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.c"
"${ProjDirPath}/../../../../../../core/utilities/debug_console_lite/fsl_debug_console.h"
"${ProjDirPath}/../../../../../../core/utilities/assert/fsl_assert.c"
"${ProjDirPath}/../../../../../../core/utilities/assert/fsl_assert.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/debug_log.cpp"
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
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/delay_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/energy_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/fft_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/filter_bank_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/filter_bank_log_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/filter_bank_spectral_subtraction_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/framer_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/irfft.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/overlap_add_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/pcan_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/rfft.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/stacker_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/micro/kernels/window_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/circular_buffer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/complex.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/energy.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/fft_auto_scale.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/filter_bank.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/filter_bank_log.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/filter_bank_spectral_subtraction.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/filter_bank_square_root.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/irfft.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/log.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/max_abs.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/msb.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/overlap_add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/pcan_argc_fixed.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/rfft.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/square_root.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/window.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/kiss_fft_wrappers/kiss_fft_common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/kiss_fft_wrappers/kiss_fft_float.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/kiss_fft_wrappers/kiss_fft_int16.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/signal/src/kiss_fft_wrappers/kiss_fft_int32.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/builtin_op_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/builtin_ops.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/context_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/portable_type_to_tflitetype.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/c/builtin_op_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/c/c_api_types.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/c/common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/macros.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/error_reporter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/flatbuffer_conversions.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/api/tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/builtin_op_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/c_api_types.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/core/c/common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/kernel_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/op_macros.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/padding.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/compatibility.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/cppmath.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/max.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/min.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/portable_tensor.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/portable_tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/quantization_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/runtime_shape.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/strided_slice_logic.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/tensor_ctypes.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/types.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/optimized/neon_check.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/add_n.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/arg_min_max.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/batch_matmul.h"
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
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/kernels/internal/reference/lstm_cell.h"
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
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/compatibility.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/debug_log.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/fake_micro_context.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/flatbuffer_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_helpers.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_allocation_info.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_arena_constants.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_common.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_context.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_graph.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_interpreter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_interpreter_context.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_interpreter_graph.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_log.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_mutable_op_resolver.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_op_resolver.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_profiler.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_profiler_interface.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_resource_variable.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_time.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/micro_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/mock_micro_graph.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/recording_micro_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/recording_micro_interpreter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/system_setup.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/test_helper_custom_ops.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/test_helpers.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/activation_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/activations.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/add.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/circular_buffer.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/circular_buffer_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/conv_test.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/depthwise_conv.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/dequantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/detection_postprocess_flexbuffers_generated_data.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/ethosu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/fully_connected.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/hard_swish.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/kernel_runner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/kernel_util.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/leaky_relu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logical.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/logistic.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/lstm_eval.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/lstm_eval_test.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/lstm_shared.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/micro_ops.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/micro_tensor_utils.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/mul.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/pad.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/pooling.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/prelu.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/quantize.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/reduce.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/reshape.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/softmax.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/strided_slice.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/sub.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/svdf.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/kernels/unidirectional_sequence_lstm.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/ibuffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/greedy_memory_planner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/linear_memory_planner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/memory_plan_struct.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/micro_memory_planner.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/schema/schema_generated.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/tensorflow/lite/schema/schema_utils.h"
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
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp/fixedpoint/fixedpoint_neon.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp/fixedpoint/fixedpoint_sse.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/gemmlowp/internal/detect_platform.h"
"${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/third_party/ruy/ruy/profiler/instrumentation.h"
"${ProjDirPath}/../source/image/image_load.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board_hifi4.c"
"${ProjDirPath}/../board_hifi4.h"
"${ProjDirPath}/../board_init.c"
"${ProjDirPath}/../board_init.h"
"${ProjDirPath}/../source/timer.h"
"${ProjDirPath}/../source/timer_xtensa.c"
)


TARGET_LINK_LIBRARIES(tflm_cifar10_hifi4.elf -Wl,--start-group)
target_link_libraries(tflm_cifar10_hifi4.elf debug xos)

target_link_libraries(tflm_cifar10_hifi4.elf optimized xos)


target_link_libraries(tflm_cifar10_hifi4.elf debug ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/lib/hifi4/xcc/libtflm.a)


target_link_libraries(tflm_cifar10_hifi4.elf optimized ${ProjDirPath}/../../../../../../middleware/eiq/tensorflow-lite/lib/hifi4/xcc/libtflm.a)

TARGET_LINK_LIBRARIES(tflm_cifar10_hifi4.elf -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET tflm_cifar10_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/tflm_cifar10_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/mimxrt685audevk/eiq_examples/tflm_cifar10_hifi4/hifi4/binary/dsp_literal_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI23_11_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.Level3InterruptVector.literal
--only-section=.DebugExceptionVector.literal
--only-section=.NMIExceptionVector.literal
--only-section=.UserExceptionVector.literal
)

ADD_CUSTOM_COMMAND(TARGET tflm_cifar10_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/tflm_cifar10_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/mimxrt685audevk/eiq_examples/tflm_cifar10_hifi4/hifi4/binary/dsp_text_${CMAKE_BUILD_TYPE}.bin
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

ADD_CUSTOM_COMMAND(TARGET tflm_cifar10_hifi4.elf POST_BUILD COMMAND ${CMAKE_OBJCOPY}
--xtensa-params= -Obinary ${EXECUTABLE_OUTPUT_PATH}/tflm_cifar10_hifi4.elf ${EXECUTABLE_OUTPUT_PATH}/../../../../../../../examples/mimxrt685audevk/eiq_examples/tflm_cifar10_hifi4/hifi4/binary/dsp_data_${CMAKE_BUILD_TYPE}.bin
--xtensa-core=nxp_rt600_RI23_11_newlib
--xtensa-system=${XTENSA_SYSTEM}
--only-section=.text
--only-section=.rtos.rodata
--only-section=.rodata
--only-section=.clib.data
--only-section=.rtos.percpu.data
--only-section=.data
)

