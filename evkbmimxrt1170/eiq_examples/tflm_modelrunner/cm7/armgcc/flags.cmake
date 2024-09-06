IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE_UART")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_RAMFUNCTION \
    -DNDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_RAMFUNCTION \
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
    -DXIP_BOOT_HEADER_DCD_ENABLE=0 \
    -DXIP_BOOT_HEADER_XMCD_ENABLE=1 \
    -DUSE_SDRAM \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DNDEBUG \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCPU_MIMXRT1176DVMAA_cm7 \
    -DMODEL_SIZE=28*1024*1024 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DUSE_RTOS=1 \
    -DMODELRUNNER_HTTP=1 \
    -DFSL_RTOS_FREE_RTOS \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DSDK_OS_FREE_RTOS \
    -DFSL_RTOS_BM \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1 \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DCHECKSUM_GEN_UDP=1 \
    -DCHECKSUM_GEN_TCP=1 \
    -DCHECKSUM_GEN_ICMP=1 \
    -DCHECKSUM_GEN_ICMP6=1 \
    -DCHECKSUM_CHECK_UDP=1 \
    -DCHECKSUM_CHECK_TCP=1 \
    -DCHECKSUM_CHECK_ICMP=1 \
    -DCHECKSUM_CHECK_ICMP6=1 \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -O3 \
    -Wall \
    -Wno-maybe-uninitialized \
    -Wno-strict-aliasing \
    -mcpu=cortex-m7 \
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
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=0 \
    -DXIP_BOOT_HEADER_XMCD_ENABLE=1 \
    -DUSE_SDRAM \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DDEBUG \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCPU_MIMXRT1176DVMAA_cm7 \
    -DMODEL_SIZE=28*1024*1024 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DUSE_RTOS=1 \
    -DMODELRUNNER_HTTP=1 \
    -DFSL_RTOS_FREE_RTOS \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DSDK_OS_FREE_RTOS \
    -DFSL_RTOS_BM \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -DFSL_SDK_DRIVER_QUICK_ACCESS_ENABLE=1 \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DCHECKSUM_GEN_UDP=1 \
    -DCHECKSUM_GEN_TCP=1 \
    -DCHECKSUM_GEN_ICMP=1 \
    -DCHECKSUM_GEN_ICMP6=1 \
    -DCHECKSUM_CHECK_UDP=1 \
    -DCHECKSUM_CHECK_TCP=1 \
    -DCHECKSUM_CHECK_ICMP=1 \
    -DCHECKSUM_CHECK_ICMP6=1 \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -g \
    -O0 \
    -O3 \
    -Wall \
    -Wno-maybe-uninitialized \
    -Wno-strict-aliasing \
    -mcpu=cortex-m7 \
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
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -DNDEBUG \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCPU_MIMXRT1176DVMAA_cm7 \
    -DMODEL_SIZE=28*1024*1024 \
    -DMODELRUNNER_HTTP=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DUSE_RTOS=1 \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
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
    -mcpu=cortex-m7 \
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
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -DDEBUG \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCPU_MIMXRT1176DVMAA_cm7 \
    -DMODEL_SIZE=28*1024*1024 \
    -DMODELRUNNER_HTTP=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DUSE_RTOS=1 \
    -DARM_MATH_CM7 \
    -D__FPU_PRESENT=1 \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DTF_LITE_STATIC_MEMORY \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -O3 \
    -Wall \
    -fno-rtti \
    -fno-exceptions \
    -Wno-maybe-uninitialized \
    -Wno-sign-compare \
    -Wno-strict-aliasing \
    -Wno-deprecated-declarations \
    -mcpu=cortex-m7 \
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
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -mcpu=cortex-m7 \
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
    --defsym=__use_flash64MB__=1 \
    -Xlinker \
    --defsym=__heap_size__=0x2000000 \
    -Xlinker \
    --defsym=__stack_size__=0x17000 \
    -Xlinker \
    --defsym=__stack_size__=0x2000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1176xxxxx_cm7_flexspi_nor_sdram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -g \
    -mcpu=cortex-m7 \
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
    --defsym=__use_flash64MB__=1 \
    -Xlinker \
    --defsym=__heap_size__=0x2000000 \
    -Xlinker \
    --defsym=__stack_size__=0x17000 \
    -Xlinker \
    --defsym=__stack_size__=0x2000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1176xxxxx_cm7_flexspi_nor_sdram.ld\" -static \
")
