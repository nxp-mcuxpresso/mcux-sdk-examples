IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-sp-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE_UART")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE} \
    -D__STARTUP_INITIALIZE_QADATA=1 \
    -D__STARTUP_INITIALIZE_RAMFUNCTION=1 \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA=1 \
    -mcpu=cortex-m33 \
    -Wall \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG} \
    -D__STARTUP_INITIALIZE_QADATA=1 \
    -D__STARTUP_INITIALIZE_RAMFUNCTION=1 \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS=1 \
    -D__STARTUP_INITIALIZE_NONCACHEDATA=1 \
    -g \
    -mcpu=cortex-m33 \
    -Wall \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=0 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=0 \
    -DUSE_HYPERRAM \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DNDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm33 \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -O3 \
    -Wall \
    -Wno-maybe-uninitialized \
    -Wno-strict-aliasing \
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
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=0 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=0 \
    -DUSE_HYPERRAM \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm33 \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -Wall \
    -Wno-maybe-uninitialized \
    -Wno-strict-aliasing \
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
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm33 \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -O3 \
    -Wall \
    -fno-rtti \
    -fno-exceptions \
    -Wno-maybe-uninitialized \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -Wno-deprecated-declarations \
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
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG} \
    -DDEBUG \
    -DCPU_MIMXRT1189CVM8B_cm33 \
    -DARM_MATH_CM33 \
    -D__FPU_PRESENT=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -Wall \
    -fno-rtti \
    -fno-exceptions \
    -Wno-maybe-uninitialized \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -Wno-deprecated-declarations \
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
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_HYPERRAM_RELEASE} \
    -mcpu=cortex-m33 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
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
    -T\"${ProjDirPath}/MIMXRT1189xxxxx_cm33_flexspi_nor_hyperram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_HYPERRAM_DEBUG} \
    -g \
    -mcpu=cortex-m33 \
    -Wall \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
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
    -T\"${ProjDirPath}/MIMXRT1189xxxxx_cm33_flexspi_nor_hyperram.ld\" -static \
")
