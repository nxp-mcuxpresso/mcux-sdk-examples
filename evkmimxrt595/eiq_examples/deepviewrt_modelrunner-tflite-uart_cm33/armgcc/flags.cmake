IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-sp-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_FLASH_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLASH_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLASH_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLASH_DEBUG} \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_FLASH_RELEASE " \
    ${CMAKE_C_FLAGS_FLASH_RELEASE} \
    -DNDEBUG \
    -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1 \
    -DCPU_MIMXRT595SFFOC_cm33 \
    -DBOOT_HEADER_ENABLE=1 \
    -D_POSIX_C_SOURCE=200809L \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DCMSIS_NN \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -O3 \
    -Wno-strict-aliasing \
    -fno-strict-aliasing \
    -Wno-unused-function \
    -mcpu=cortex-m33 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_FLASH_DEBUG " \
    ${CMAKE_C_FLAGS_FLASH_DEBUG} \
    -DDEBUG \
    -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1 \
    -DCPU_MIMXRT595SFFOC_cm33 \
    -DBOOT_HEADER_ENABLE=1 \
    -D_POSIX_C_SOURCE=200809L \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DCMSIS_NN \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -Wno-strict-aliasing \
    -fno-strict-aliasing \
    -Wno-unused-function \
    -mcpu=cortex-m33 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLASH_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLASH_RELEASE} \
    -DNDEBUG \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DCPU_MIMXRT595SFFOC_cm33 \
    -DCMSIS_NN \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -O3 \
    -fno-rtti \
    -fno-exceptions \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -mcpu=cortex-m33 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLASH_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLASH_DEBUG} \
    -DDEBUG \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DCPU_MIMXRT595SFFOC_cm33 \
    -DCMSIS_NN \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -fno-rtti \
    -fno-exceptions \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -mcpu=cortex-m33 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLASH_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLASH_RELEASE} \
    -Xlinker \
    --no-wchar-size-warning \
    -mcpu=cortex-m33 \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -Wl,--print-memory-usage \
    -Xlinker \
    --defsym=__heap_size__=0x170000 \
    -Xlinker \
    --defsym=__stack_size__=0x10000 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMXRT595Sxxxx_cm33_flash.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLASH_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLASH_DEBUG} \
    -g \
    -Xlinker \
    --no-wchar-size-warning \
    -mcpu=cortex-m33 \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -Wl,--print-memory-usage \
    -Xlinker \
    --defsym=__heap_size__=0x170000 \
    -Xlinker \
    --defsym=__stack_size__=0x10000 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMXRT595Sxxxx_cm33_flash.ld -static \
")
