IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -DNDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -DDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DSKIP_SYSCLK_INIT \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DNDEBUG \
    -DCPU_MIMXRT1042XJM5B \
    -DFSL_RTOS_FREE_RTOS \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DFSL_RTOS_BM \
    -D_POSIX_C_SOURCE=200809L \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DSDK_OS_BAREMETAL \
    -DCPU_MIMXRT1042DVL6A \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
    -DUSE_RTOS=1 \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCMSIS_NN \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DTF_LITE_STATIC_MEMORY \
    -Os \
    -Wno-strict-aliasing \
    -fno-strict-aliasing \
    -Wno-unused-function \
    -mcpu=cortex-m7 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    -std=gnu99 \
    -Wall \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DSKIP_SYSCLK_INIT \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DDEBUG \
    -DCPU_MIMXRT1042XJM5B \
    -DFSL_RTOS_FREE_RTOS \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DFSL_RTOS_BM \
    -D_POSIX_C_SOURCE=200809L \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DSDK_OS_BAREMETAL \
    -DCPU_MIMXRT1042DVL6A \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_FREE_RTOS \
    -DUSE_RTOS=1 \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCMSIS_NN \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DTF_LITE_STATIC_MEMORY \
    -g \
    -O0 \
    -Wno-strict-aliasing \
    -Os \
    -fno-strict-aliasing \
    -Wno-unused-function \
    -mcpu=cortex-m7 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    -std=gnu99 \
    -Wall \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMXRT1042XJM5B \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DFSL_RTOS_BM \
    -DSKIP_SYSCLK_INIT \
    -DSDK_OS_BAREMETAL \
    -DCPU_MIMXRT1042DVL6A \
    -DMCUXPRESSO_SDK \
    -DCMSIS_NN \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DTF_LITE_STATIC_MEMORY \
    -O3 \
    -fno-rtti \
    -fno-exceptions \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -mcpu=cortex-m7 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    -Wall \
    -Wno-deprecated-declarations \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -DDEBUG \
    -DCPU_MIMXRT1042XJM5B \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DFSL_RTOS_BM \
    -DSKIP_SYSCLK_INIT \
    -DSDK_OS_BAREMETAL \
    -DCPU_MIMXRT1042DVL6A \
    -DMCUXPRESSO_SDK \
    -DCMSIS_NN \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DTF_LITE_STATIC_MEMORY \
    -O3 \
    -g \
    -O0 \
    -fno-rtti \
    -fno-exceptions \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -mcpu=cortex-m7 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -mapcs \
    -Wall \
    -Wno-deprecated-declarations \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -Xlinker \
    --no-wchar-size-warning \
    -mcpu=cortex-m7 \
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
    -print-memory-usage \
    -Xlinker \
    --defsym=__heap_size__=0x200000 \
    -Xlinker \
    --defsym=__stack_size__=0x10000 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMXRT1042xxxxx_flexspi_nor_sdram.ld -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -g \
    -Xlinker \
    --no-wchar-size-warning \
    -mcpu=cortex-m7 \
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
    -print-memory-usage \
    -Xlinker \
    --defsym=__heap_size__=0x200000 \
    -Xlinker \
    --defsym=__stack_size__=0x10000 \
    ${FPU} \
    ${SPECS} \
    -T${ProjDirPath}/MIMXRT1042xxxxx_flexspi_nor_sdram.ld -static \
")
