IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -DDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -DNDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DSKIP_SYSCLK_INIT \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DDEBUG \
    -D_POSIX_SOURCE \
    -DCPU_MIMXRT1062DVL6B \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DMBEDTLS_SSL_CIPHERSUITES=MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DUSE_RTOS=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DHTTPSRV_CFG_WEBSOCKET_ENABLED=1 \
    -DHTTPSRV_CFG_MBEDTLS_ENABLE=1 \
    -DCPU_MIMXRT1062DVL6B_cm7 \
    -DMCUXPRESSO_SDK \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DCHECKSUM_GEN_ICMP6=1 \
    -DCHECKSUM_CHECK_ICMP6=1 \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DMBEDTLS_THREADING_C \
    -DMBEDTLS_THREADING_ALT \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DSDK_OS_FREE_RTOS \
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
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
    -fomit-frame-pointer \
    -Wno-unused-function \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DXIP_BOOT_HEADER_DCD_ENABLE=1 \
    -DSKIP_SYSCLK_INIT \
    -DDATA_SECTION_IS_CACHEABLE=1 \
    -DNDEBUG \
    -D_POSIX_SOURCE \
    -DCPU_MIMXRT1062DVL6B \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DMBEDTLS_SSL_CIPHERSUITES=MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DUSE_RTOS=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DHTTPSRV_CFG_WEBSOCKET_ENABLED=1 \
    -DHTTPSRV_CFG_MBEDTLS_ENABLE=1 \
    -DCPU_MIMXRT1062DVL6B_cm7 \
    -DMCUXPRESSO_SDK \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DCHECKSUM_GEN_ICMP6=1 \
    -DCHECKSUM_CHECK_ICMP6=1 \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DMBEDTLS_THREADING_C \
    -DMBEDTLS_THREADING_ALT \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DSDK_OS_FREE_RTOS \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
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
    -fomit-frame-pointer \
    -Wno-unused-function \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -DDEBUG \
    -DCPU_MIMXRT1062DVL6B \
    -DMCUXPRESSO_SDK \
    -DSERIAL_PORT_TYPE_UART=1 \
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
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -DNDEBUG \
    -DCPU_MIMXRT1062DVL6B \
    -DMCUXPRESSO_SDK \
    -DSERIAL_PORT_TYPE_UART=1 \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
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
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_DEBUG} \
    -g \
    -mcpu=cortex-m7 \
    -Wall \
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
    --defsym=__stack_size__=1000 \
    -Xlinker \
    --defsym=__heap_size__=100000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1062xxxxx_flexspi_nor_sdram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_SDRAM_RELEASE} \
    -mcpu=cortex-m7 \
    -Wall \
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
    --defsym=__stack_size__=1000 \
    -Xlinker \
    --defsym=__heap_size__=100000 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMXRT1062xxxxx_flexspi_nor_sdram.ld\" -static \
")
