IF(NOT DEFINED FPU)  
    SET(FPU "-mfloat-abi=hard -mfpu=fpv5-sp-d16")  
ENDIF()  

IF(NOT DEFINED SPECS)  
    SET(SPECS "--specs=nano.specs --specs=nosys.specs")  
ENDIF()  

IF(NOT DEFINED DEBUG_CONSOLE_CONFIG)  
    SET(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE_UART")  
ENDIF()  

SET(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -mcpu=cortex-m33 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -include ${ProjDirPath}/../app_preinclude.h \
    -DDEBUG \
    -DCPU_MCXW716CMFTA \
    -DSERIAL_MANAGER_NON_BLOCKING_MODE=1 \
    -DSERIAL_USE_CONFIGURE_STRUCTURE=1 \
    -DgButtonSupported_d=1 \
    -DOSA_USED \
    -DSDK_COMPONENT_INTEGRATION=1 \
    -DFSL_OSA_TASK_ENABLE=1 \
    -DCR_INTEGER_PRINTF \
    -DMCXW716C \
    -DCFG_BLE_PRJ=1 \
    -DENABLE_RAM_VECTOR_TABLE=1 \
    -DNXP_SSSAPI \
    -DNXP_ELE200 \
    -DHAL_RPMSG_SELECT_ROLE=0 \
    -DSERIAL_PORT_TYPE_RPMSG=1 \
    -DRPMSG_ADAPTER_NON_BLOCKING_MODE=1 \
    -DHAL_FLASH_ROMAPI_DRIVER=1 \
    -DgUseHciTransportDownward_d=1 \
    -DTM_ENABLE_TIME_STAMP=1 \
    -DBOARD_FRDMCXW71_SUPPORT=1 \
    -DMCUXPRESSO_SDK \
    -DTIMER_PORT_TYPE_LPTMR=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DSDK_OS_FREE_RTOS \
    -DMULTICORE_APP=1 \
    -DGENERIC_LIST_LIGHT=1 \
    -Og \
    -g \
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
    -include ${ProjDirPath}/../app_preinclude.h \
    -DNDEBUG \
    -DCPU_MCXW716CMFTA \
    -DSERIAL_MANAGER_NON_BLOCKING_MODE=1 \
    -DSERIAL_USE_CONFIGURE_STRUCTURE=1 \
    -DgButtonSupported_d=1 \
    -DOSA_USED \
    -DSDK_COMPONENT_INTEGRATION=1 \
    -DFSL_OSA_TASK_ENABLE=1 \
    -DCR_INTEGER_PRINTF \
    -DMCXW716C \
    -DCFG_BLE_PRJ=1 \
    -DENABLE_RAM_VECTOR_TABLE=1 \
    -DNXP_SSSAPI \
    -DNXP_ELE200 \
    -DHAL_RPMSG_SELECT_ROLE=0 \
    -DSERIAL_PORT_TYPE_RPMSG=1 \
    -DRPMSG_ADAPTER_NON_BLOCKING_MODE=1 \
    -DHAL_FLASH_ROMAPI_DRIVER=1 \
    -DgUseHciTransportDownward_d=1 \
    -DTM_ENABLE_TIME_STAMP=1 \
    -DBOARD_FRDMCXW71_SUPPORT=1 \
    -DMCUXPRESSO_SDK \
    -DTIMER_PORT_TYPE_LPTMR=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DSDK_OS_FREE_RTOS \
    -DMULTICORE_APP=1 \
    -DGENERIC_LIST_LIGHT=1 \
    -Os \
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
    -DTIMER_PORT_TYPE_LPTMR=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DSERIAL_PORT_TYPE_RPMSG=1 \
    -g \
    -O0 \
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
    -DTIMER_PORT_TYPE_LPTMR=1 \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DSERIAL_PORT_TYPE_RPMSG=1 \
    -Os \
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
    -g \
    -Xlinker \
    --sort-section=alignment \
    -Xlinker \
    --defsym=__ram_vector_table__=1 \
    -Xlinker \
    --defsym=gUseNVMLink_d=1 \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -print-memory-usage \
    -Xlinker \
    -Map="${BuildArtifactFileBaseName}.map" \
    -Xlinker \
    --defsym=gUseStackEnd_d=1 \
    -Xlinker \
    --defsym=m_lowpower_flag_start=0x489C007C \
    -Xlinker \
    --no-warn-rwx-segments \
    -mcpu=cortex-m33 \
    -Wall \
    -Wl,--print-memory-usage \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -Xlinker \
    --defsym=gUseInternalStorageLink_d=1 \
    -Xlinker \
    --defsym=__stack_size__=0x0A00 \
    -Xlinker \
    --defsym=__heap_size__=0x0A00 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/connectivity_ble.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -Xlinker \
    --sort-section=alignment \
    -Xlinker \
    --defsym=__ram_vector_table__=1 \
    -Xlinker \
    --defsym=gUseNVMLink_d=1 \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -print-memory-usage \
    -Xlinker \
    -Map="${BuildArtifactFileBaseName}.map" \
    -Xlinker \
    --defsym=gUseStackEnd_d=1 \
    -Xlinker \
    --defsym=m_lowpower_flag_start=0x489C007C \
    -Xlinker \
    --no-warn-rwx-segments \
    -mcpu=cortex-m33 \
    -Wall \
    -Wl,--print-memory-usage \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -fno-builtin \
    -mthumb \
    -mapcs \
    -Xlinker \
    -static \
    -Xlinker \
    -z \
    -Xlinker \
    muldefs \
    -Xlinker \
    -Map=output.map \
    -Xlinker \
    --defsym=gUseInternalStorageLink_d=1 \
    -Xlinker \
    --defsym=__stack_size__=0x0A00 \
    -Xlinker \
    --defsym=__heap_size__=0x0A00 \
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/connectivity_ble.ld\" -static \
")
