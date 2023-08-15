IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv4-sp-d16")  
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
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_DDR_DEBUG " \
    ${CMAKE_ASM_FLAGS_DDR_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_DDR_RELEASE " \
    ${CMAKE_ASM_FLAGS_DDR_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLASH_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLASH_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLASH_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLASH_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m4 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DDEBUG \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
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
SET(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
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
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_DDR_DEBUG " \
    ${CMAKE_C_FLAGS_DDR_DEBUG} \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
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
SET(CMAKE_C_FLAGS_DDR_RELEASE " \
    ${CMAKE_C_FLAGS_DDR_RELEASE} \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
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
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_FLASH_DEBUG " \
    ${CMAKE_C_FLAGS_FLASH_DEBUG} \
    -DFLASH_TARGET \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
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
SET(CMAKE_C_FLAGS_FLASH_RELEASE " \
    ${CMAKE_C_FLAGS_FLASH_RELEASE} \
    -DFLASH_TARGET \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
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
    -std=gnu99 \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_DEBUG " \
    ${CMAKE_CXX_FLAGS_DEBUG} \
    -DDEBUG \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
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
SET(CMAKE_CXX_FLAGS_RELEASE " \
    ${CMAKE_CXX_FLAGS_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
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
SET(CMAKE_CXX_FLAGS_DDR_DEBUG " \
    ${CMAKE_CXX_FLAGS_DDR_DEBUG} \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
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
SET(CMAKE_CXX_FLAGS_DDR_RELEASE " \
    ${CMAKE_CXX_FLAGS_DDR_RELEASE} \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
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
SET(CMAKE_CXX_FLAGS_FLASH_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLASH_DEBUG} \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
    -DMCUXPRESSO_SDK \
    -DSERIAL_PORT_TYPE_UART=1 \
    -g \
    -O0 \
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
SET(CMAKE_CXX_FLAGS_FLASH_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLASH_RELEASE} \
    -DCPU_MIMX8MM6DVTLZ \
    -DCPU_MIMX8MM6DVTLZ_cm4 \
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
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
    -g \
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
    -Xlinker \
    --defsym=__stack_size__=0x400 \
    -Xlinker \
    --defsym=__heap_size__=0x400 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMX8MM6xxxxx_cm4_ram.ld -static \
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
    -Xlinker \
    --defsym=__stack_size__=0x400 \
    -Xlinker \
    --defsym=__heap_size__=0x400 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMX8MM6xxxxx_cm4_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DDR_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DDR_DEBUG} \
    -g \
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
    -Xlinker \
    --defsym=__stack_size__=0x400 \
    -Xlinker \
    --defsym=__heap_size__=0x400 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMX8MM6xxxxx_cm4_ddr_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DDR_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_DDR_RELEASE} \
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
    -Xlinker \
    --defsym=__stack_size__=0x400 \
    -Xlinker \
    --defsym=__heap_size__=0x400 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMX8MM6xxxxx_cm4_ddr_ram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLASH_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLASH_DEBUG} \
    -g \
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
    -Xlinker \
    --defsym=__stack_size__=0x400 \
    -Xlinker \
    --defsym=__heap_size__=0x400 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMX8MM6xxxxx_cm4_flash.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLASH_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLASH_RELEASE} \
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
    -Xlinker \
    --defsym=__stack_size__=0x400 \
    -Xlinker \
    --defsym=__heap_size__=0x400 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMX8MM6xxxxx_cm4_flash.ld -static \
")
