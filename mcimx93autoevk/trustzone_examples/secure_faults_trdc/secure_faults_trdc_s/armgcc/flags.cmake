IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-sp-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -DDEMO_CODE_START_NS=268369920 \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcmse \
    -mcpu=cortex-m33 \
    -Wall \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DDEMO_CODE_START_NS=268369920 \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcmse \
    -mcpu=cortex-m33 \
    -Wall \
    -mthumb \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DDEMO_CODE_START_NS=268369920 \
    -DDEBUG \
    -DCPU_MIMX9352DVVXM_cm33 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcmse \
    -mcpu=cortex-m33 \
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
    -DDEMO_CODE_START_NS=268369920 \
    -DNDEBUG \
    -DCPU_MIMX9352DVVXM_cm33 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcmse \
    -mcpu=cortex-m33 \
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
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcmse \
    -mcpu=cortex-m33 \
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
    -DMCUXPRESSO_SDK \
    -Os \
    -mcmse \
    -mcpu=cortex-m33 \
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
    -Wl,--cmse-implib \
    -Wl,--out-implib=./debug/secure_faults_trdc_s_CMSE_Lib.o \
    -g \
    -mcpu=cortex-m33 \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX9352_cm33_ram_s.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -Wl,--cmse-implib \
    -Wl,--out-implib=./release/secure_faults_trdc_s_CMSE_Lib.o \
    -mcpu=cortex-m33 \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX9352_cm33_ram_s.ld\" -static \
")
