IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -DDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLEXSPI_NOR_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLEXSPI_NOR_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -DNDEBUG \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_DEBUG " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_DEBUG} \
    -include ${ProjDirPath}/../app_config.h \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DDEBUG \
    -DCPU_MIMXRT1062DVL6B \
    -DSDIO_ENABLED=1 \
    -DCPU_MIMXRT1062DVL6B_cm7 \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DAPPL_CONFIG_DISABLE \
    -DPXM \
    -DHTS \
    -DAPPL_MENU_OPS \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DSAI_XFER_QUEUE_SIZE=6 \
    -DSDK_COMPONENT_INTEGRATION=1 \
    -DIPSP_HAVE_6LO_NIFACE \
    -DLWIP_IPV6 \
    -DSUPPORT_HTTP_CLIENT \
    -DEDGEFAST_BT_LITTLEFS_MFLASH \
    -DFSL_FEATURE_FLASH_PAGE_SIZE_BYTES=4096 \
    -DFSL_OSA_MAIN_FUNC_ENABLE=0 \
    -DAPPL_GAP_CENTRAL \
    -DAPPL_GAP_PERIPHERAL \
    -DHAL_UART_ADAPTER_FIFO=1 \
    -DHAL_UART_ISR_PRIORITY=configLIBRARY_LOWEST_INTERRUPT_PRIORITY \
    -DSTORAGE_IDLE_TASK_SYNC_ENABLE=1 \
    -DNVRAM_WORKAROUND \
    -DBT_SNOOP_WRITE_TRUNCATE \
    -DOOB_WAKEUP \
    -DHAL_AUDIO_DMA_INIT_ENABLE=0 \
    -DHAL_UART_DMA_ENABLE=1 \
    -DLC3_DSP=1 \
    -DCOEX_APP_SUPPORT=1 \
    -DUSE_RTOS=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DSDK_OS_FREE_RTOS \
    -DFSL_OSA_TASK_ENABLE=1 \
    -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING \
    -DGATT_DB \
    -DGATT_CLIENT \
    -DGATT_SERVER \
    -DCONFIG_BT_SNOOP=1 \
    -DDEBUG_CONSOLE_SCANF_MAX_LOG_LEN=64 \
    -DCONFIG_BT_SETTINGS \
    -DAPPL_LIMIT_LOGS \
    -DNXP_CODE \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DMFLASH_FILE_BASEADDR=7340032 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8962_ENABLE \
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
    -DSO_REUSE=1 \
    -DOSA_USED \
    -DLWIP_DNS=1 \
    -DLWIP_NETIF_HOSTNAME=1 \
    -DLWIP_IGMP=1 \
    -D_XOPEN_SOURCE=500 \
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
    -fomit-frame-pointer \
    -Wno-unused-function \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_RELEASE " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_RELEASE} \
    -include ${ProjDirPath}/../app_config.h \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DNDEBUG \
    -DCPU_MIMXRT1062DVL6B \
    -DSDIO_ENABLED=1 \
    -DCPU_MIMXRT1062DVL6B_cm7 \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DAPPL_CONFIG_DISABLE \
    -DPXM \
    -DHTS \
    -DAPPL_MENU_OPS \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DSAI_XFER_QUEUE_SIZE=6 \
    -DSDK_COMPONENT_INTEGRATION=1 \
    -DIPSP_HAVE_6LO_NIFACE \
    -DLWIP_IPV6 \
    -DSUPPORT_HTTP_CLIENT \
    -DEDGEFAST_BT_LITTLEFS_MFLASH \
    -DFSL_FEATURE_FLASH_PAGE_SIZE_BYTES=4096 \
    -DFSL_OSA_MAIN_FUNC_ENABLE=0 \
    -DAPPL_GAP_CENTRAL \
    -DAPPL_GAP_PERIPHERAL \
    -DHAL_UART_ADAPTER_FIFO=1 \
    -DHAL_UART_ISR_PRIORITY=configLIBRARY_LOWEST_INTERRUPT_PRIORITY \
    -DSTORAGE_IDLE_TASK_SYNC_ENABLE=1 \
    -DNVRAM_WORKAROUND \
    -DBT_SNOOP_WRITE_TRUNCATE \
    -DOOB_WAKEUP \
    -DHAL_AUDIO_DMA_INIT_ENABLE=0 \
    -DHAL_UART_DMA_ENABLE=1 \
    -DLC3_DSP=1 \
    -DCOEX_APP_SUPPORT=1 \
    -DUSE_RTOS=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DSDK_OS_FREE_RTOS \
    -DFSL_OSA_TASK_ENABLE=1 \
    -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING \
    -DGATT_DB \
    -DGATT_CLIENT \
    -DGATT_SERVER \
    -DCONFIG_BT_SNOOP=1 \
    -DDEBUG_CONSOLE_SCANF_MAX_LOG_LEN=64 \
    -DCONFIG_BT_SETTINGS \
    -DAPPL_LIMIT_LOGS \
    -DNXP_CODE \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DMFLASH_FILE_BASEADDR=7340032 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8962_ENABLE \
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
    -DSO_REUSE=1 \
    -DOSA_USED \
    -DLWIP_DNS=1 \
    -DLWIP_NETIF_HOSTNAME=1 \
    -DLWIP_IGMP=1 \
    -D_XOPEN_SOURCE=500 \
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
    -fomit-frame-pointer \
    -Wno-unused-function \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_DEBUG} \
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
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_CXX_FLAGS_FLEXSPI_NOR_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLEXSPI_NOR_RELEASE} \
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
    -fno-builtin \
    -mapcs \
    -fno-rtti \
    -fno-exceptions \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_DEBUG} \
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
    --defsym=__use_flash16MB__=1 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/linker/MIMXRT1062_cm7_flexspi_nor.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLEXSPI_NOR_RELEASE} \
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
    --defsym=__use_flash16MB__=1 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/linker/MIMXRT1062_cm7_flexspi_nor.ld\" -static \
")
