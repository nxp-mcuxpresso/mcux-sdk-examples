IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv4-sp-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -DNO_CRP \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MCIMX7U5DVP07 \
    -DCPU_MCIMX7U5DVP07_cm4 \
    -DENABLE_RAM_VECTOR_TABLE \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8960_ENABLE \
    -DSDK_OS_FREE_RTOS \
    -Os \
    -mcpu=cortex-m4 \
    -Wall \
    -Wno-address-of-packed-member \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_RELEASE " \
    ${CMAKE_CXX_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MCIMX7U5DVP07 \
    -DCPU_MCIMX7U5DVP07_cm4 \
    -DMCUXPRESSO_SDK \
    -DSERIAL_PORT_TYPE_UART=1 \
    -Os \
    -mcpu=cortex-m4 \
    -Wall \
    -Wno-address-of-packed-member \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -mcpu=cortex-m4 \
    -Wall \
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MCIMX7U5xxxxx_cm4_ram.ld -static \
")
