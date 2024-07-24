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
    -DDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    -DNDEBUG \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_DDR_DEBUG " \
    ${CMAKE_ASM_FLAGS_DDR_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_DDR_RELEASE " \
    ${CMAKE_ASM_FLAGS_DDR_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLASH_DEBUG " \
    ${CMAKE_ASM_FLAGS_FLASH_DEBUG} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_ASM_FLAGS_FLASH_RELEASE " \
    ${CMAKE_ASM_FLAGS_FLASH_RELEASE} \
    -D__STARTUP_CLEAR_BSS \
    -D__STARTUP_INITIALIZE_NONCACHEDATA \
    -mcpu=cortex-m7 \
    -mthumb \
    ${FPU} \
")
SET(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    -DDEBUG \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DSDK_DELAY_USE_DWT \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8524_ENABLE \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DSDK_DELAY_USE_DWT \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8524_ENABLE \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_C_FLAGS_DDR_DEBUG " \
    ${CMAKE_C_FLAGS_DDR_DEBUG} \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DSDK_DELAY_USE_DWT \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8524_ENABLE \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_C_FLAGS_DDR_RELEASE " \
    ${CMAKE_C_FLAGS_DDR_RELEASE} \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DSDK_DELAY_USE_DWT \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8524_ENABLE \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_C_FLAGS_FLASH_DEBUG " \
    ${CMAKE_C_FLAGS_FLASH_DEBUG} \
    -DFLASH_TARGET \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DSDK_DELAY_USE_DWT \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8524_ENABLE \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_C_FLAGS_FLASH_RELEASE " \
    ${CMAKE_C_FLAGS_FLASH_RELEASE} \
    -DFLASH_TARGET \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DSDK_DELAY_USE_DWT \
    -DMCUXPRESSO_SDK \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DBOARD_USE_CODEC=1 \
    -DCODEC_WM8524_ENABLE \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_CXX_FLAGS_DDR_DEBUG " \
    ${CMAKE_CXX_FLAGS_DDR_DEBUG} \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_CXX_FLAGS_DDR_RELEASE " \
    ${CMAKE_CXX_FLAGS_DDR_RELEASE} \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_CXX_FLAGS_FLASH_DEBUG " \
    ${CMAKE_CXX_FLAGS_FLASH_DEBUG} \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
SET(CMAKE_CXX_FLAGS_FLASH_RELEASE " \
    ${CMAKE_CXX_FLAGS_FLASH_RELEASE} \
    -DCPU_MIMX8MN6DVTJZ \
    -DCPU_MIMX8MN6DVTJZ_cm7 \
    -DMCUXPRESSO_SDK \
    -Os \
    -mcpu=cortex-m7 \
    -Wall \
    -Wno-address-of-packed-member \
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
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX8MN6xxxxx_cm7_ram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    -mcpu=cortex-m7 \
    -Wall \
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX8MN6xxxxx_cm7_ram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DDR_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DDR_DEBUG} \
    -g \
    -mcpu=cortex-m7 \
    -Wall \
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX8MN6xxxxx_cm7_ddr_ram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_DDR_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_DDR_RELEASE} \
    -mcpu=cortex-m7 \
    -Wall \
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX8MN6xxxxx_cm7_ddr_ram.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLASH_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_FLASH_DEBUG} \
    -g \
    -mcpu=cortex-m7 \
    -Wall \
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX8MN6xxxxx_cm7_flash.ld\" -static \
")
SET(CMAKE_EXE_LINKER_FLAGS_FLASH_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_FLASH_RELEASE} \
    -mcpu=cortex-m7 \
    -Wall \
    -Wl,--print-memory-usage \
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
    ${FPU} \
    ${SPECS} \
    -T\"${ProjDirPath}/MIMX8MN6xxxxx_cm7_flash.ld\" -static \
")
