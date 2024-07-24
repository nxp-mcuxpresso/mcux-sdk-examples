IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA=1 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA=1 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_HYPERRAM_DEBUG " \
    ${CMAKE_ASM_FLAGS_HYPERRAM_DEBUG} \
    -D__STARTUP_INITIALIZE_QADATA=1 \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA=1 \
    -g \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_HYPERRAM_RELEASE " \
    ${CMAKE_ASM_FLAGS_HYPERRAM_RELEASE} \
    -D__STARTUP_INITIALIZE_QADATA=1 \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA=1 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DCACHE_MAINTAIN=1 \
    -DEXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK=kFLEXSPI_ReadSampleClkExternalInputFromDqsPad \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DCACHE_MAINTAIN=1 \
    -DEXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK=kFLEXSPI_ReadSampleClkExternalInputFromDqsPad \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_HYPERRAM_DEBUG " \
    ${CMAKE_C_FLAGS_HYPERRAM_DEBUG} \
    -DUSE_HYPERRAM \
    -DDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DCACHE_MAINTAIN=1 \
    -DEXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK=kFLEXSPI_ReadSampleClkExternalInputFromDqsPad \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_HYPERRAM_RELEASE " \
    ${CMAKE_C_FLAGS_HYPERRAM_RELEASE} \
    -DUSE_HYPERRAM \
    -DNDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DCACHE_MAINTAIN=1 \
    -DEXAMPLE_FLEXSPI_RX_SAMPLE_CLOCK=kFLEXSPI_ReadSampleClkExternalInputFromDqsPad \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_DEBUG " \
    ${CMAKE_CXX_FLAGS_DEBUG} \
    -DDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_RELEASE " \
    ${CMAKE_CXX_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_HYPERRAM_DEBUG " \
    ${CMAKE_CXX_FLAGS_HYPERRAM_DEBUG} \
    -DDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_HYPERRAM_RELEASE " \
    ${CMAKE_CXX_FLAGS_HYPERRAM_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm7 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
    -g \
    -mcpu=cortex-m7 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
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
    --defsym=__stack_size__=0x2000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1189xxxxx_cm7_ram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -mcpu=cortex-m7 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
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
    --defsym=__stack_size__=0x2000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1189xxxxx_cm7_ram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_HYPERRAM_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_HYPERRAM_DEBUG} \
    -g \
    -mcpu=cortex-m7 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
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
    --defsym=__stack_size__=0x2000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1189xxxxx_cm7_hyperram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_HYPERRAM_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_HYPERRAM_RELEASE} \
    -mcpu=cortex-m7 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
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
    --defsym=__stack_size__=0x2000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1189xxxxx_cm7_hyperram.ld\" -static \
")
