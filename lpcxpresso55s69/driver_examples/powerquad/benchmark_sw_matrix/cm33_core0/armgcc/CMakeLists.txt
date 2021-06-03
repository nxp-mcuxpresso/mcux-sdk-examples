# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.1.1)

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


project(powerquad_benchmark_sw_matrix)

set(MCUX_SDK_PROJECT_NAME powerquad_benchmark_sw_matrix.elf)

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_bitreversal2.c"
"${ProjDirPath}/../powerquad_benchmark_sw_matrix.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/BasicMathFunctions/arm_mult_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/BasicMathFunctions/arm_shift_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mult_cmplx_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_init_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix4_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix4_init_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/BasicMathFunctions/arm_mult_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/BasicMathFunctions/arm_shift_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mult_cmplx_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix2_init_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix4_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_radix4_init_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_add_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_add_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_add_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_sub_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_sub_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_sub_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_mult_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_mult_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_mult_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_inverse_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_trans_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_trans_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_trans_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_scale_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_scale_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_scale_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_init_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_init_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/MatrixFunctions/arm_mat_init_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_sqrt_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_sqrt_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_sin_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_sin_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_sin_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_cos_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_cos_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FastMathFunctions/arm_cos_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_fir_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_fir_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_fir_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_fir_init_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_conv_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_conv_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_conv_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_correlate_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_correlate_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_correlate_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_biquad_cascade_df2T_init_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/FilteringFunctions/arm_biquad_cascade_df2T_f32.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_rfft_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_rfft_init_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_rfft_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_rfft_init_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_cfft_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_dct4_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_dct4_init_q15.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_dct4_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/TransformFunctions/arm_dct4_init_q31.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/CommonTables/arm_common_tables.c"
"${ProjDirPath}/../../../../../../../core/CMSIS/DSP/Source/CommonTables/arm_const_structs.c"
"${ProjDirPath}/../../../../../../../core/boards/lpcxpresso55s69/board.c"
"${ProjDirPath}/../../../../../../../core/boards/lpcxpresso55s69/board.h"
"${ProjDirPath}/../../../../../../../core/boards/lpcxpresso55s69/clock_config.c"
"${ProjDirPath}/../../../../../../../core/boards/lpcxpresso55s69/clock_config.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../../../../../../core/boards/lpcxpresso55s69
    ${ProjDirPath}/../../../../../../../examples/lpcxpresso55s69/driver_examples/powerquad/benchmark_sw_matrix
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../../../core
)

#Include Entry cmake component
include(all_devices)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_common)

include(driver_power)

include(driver_lpc_iocon)

include(driver_lpc_gpio)

include(driver_inputmux)

include(driver_clock)

include(device_CMSIS)

include(device_startup_LPC55S69_cm33_core0)

include(driver_flexcomm_usart)

include(driver_flexcomm)

include(component_usart_adapter)

include(component_lists)

include(CMSIS_Include_core_cm33)

include(CMSIS_Include_common)

include(CMSIS_Include_dsp)

include(driver_reset)

include(driver_inputmux_connections)

include(utilities_misc_utilities)

include(device_system_LPC55S69_cm33_core0)


TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

if(CMAKE_BUILD_TYPE STREQUAL debug)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../../core/devices/LPC55S69/gcc/libpower_hardabi.a)
endif()

if(CMAKE_BUILD_TYPE STREQUAL release)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${ProjDirPath}/../../../../../../../core/devices/LPC55S69/gcc/libpower_hardabi.a)
endif()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


